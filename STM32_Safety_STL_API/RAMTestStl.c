/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/


// Headerdateien einbinden -----------------------------------------------------
#include "config/version.h"


#include "SafetyStl.h"

/// Baut Artificial-Failing Funktionalitäten zum Testen des Moduls ein
#ifndef FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
#define FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL              (0)
#endif

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
#include "RAMTestStl_ArtificialFailing.h"
#endif

#include "RAMTestStl.h"

// Allgemeine Definitionen -----------------------------------------------------

/// Safety-relevant RAM-testregions defined by user in linker file (SOFTQM-417)
/// Safety-relevant RAM-testregion 1, must be a contiguous area in RAM
extern U32 __RAM_TESTREGION1_START;  ///< Startaddress
extern U32 __RAM_TESTREGION1_SIZE;   ///< Size in bytes

/// Safety-relevant RAM-testregion 2, must be a contiguous area in RAM
#ifdef FEATURE_RAMTEST_INCLUDE_TESTREGION2
extern U32 __RAM_TESTREGION2_START;  ///< Startaddress
extern U32 __RAM_TESTREGION2_SIZE;   ///< Size in bytes
#endif

/// Safety-relevant RAM-testregion 3, must be a contiguous area in RAM
#ifdef FEATURE_RAMTEST_INCLUDE_TESTREGION3
extern U32 __RAM_TESTREGION3_START; ///< Startaddress
extern U32 __RAM_TESTREGION3_SIZE;  ///< Size in bytes
#endif

/// RAM-backup-buffer defined by user (SOFTQM-420), must be outside testregions
extern U32 __SRAM_RAMTEST_BACKUP_START; ///< Startadresse des RAM-Backup-Puffers, vom Linker bereitgestellt.
extern U32 __SRAM_RAMTEST_BACKUP_SIZE;  ///< Größe des RAM-Backup-Puffers, vom Linker bereitgestellt.

#if FEATURE_SAFETYCHECK_USE_STL
#if EN61508_RAMTEST_USE_TIMER
#error "RAM-Test mit Timer unter Verwendung der Safety-Library von ST nicht umgesetzt. Option EN61508_RAMTEST_USE_TIMER = 0 setzen."
#endif

#if !EN61508_RAMTEST_FROM_SAFETY_TASK
#error "RAM-Test der Safety-Library nur über Aufruf aus der Safety-Task möglich. Option EN61508_RAMTEST_FROM_SAFETY_TASK = 1 setzen."
#endif
#endif

/// User configuration number of tested RAM sections
/// per test execution for cyclic runtime RAM test (SOFTQM-1074)
/**
 * Minimum value: @ref RAMTEST_NUM_SECTIONS_ATOMIC_MIN
 * Maximum value for 1-shot-execution: @ref RAMTEST_NUM_SECTIONS_ATOMIC_MAX
 *
 * Any value larger than the number of sections in the whole testregion
 * automatically leads to a 1-shot execution.
 * 1-shot: One test execution checks the whole test region
 */
#ifndef RAMTEST_CYCLIC_NUM_SECTIONS
#pragma message "Please define the number of RAM sections to be tested per test execution in the cyclic RAM test"
#define RAMTEST_CYCLIC_NUM_SECTIONS      (RAMTEST_NUM_SECTIONS_ATOMIC_MIN)
#endif

/// Größendefinitionen RAM-Test, vorgegeben in STL (UM2590).
/// Section-Size in Bytes
#define STL_RAM_SECTION_SIZE 128U
/// Block-größe in Bytes
#define STL_RAM_BLOCK_SIZE (16)
/// Ausrichtung der RAM-Adressen für den RAM-Test in Bytes
#define STL_RAM_ADDRESS_ALIGNMENT (sizeof(U32))

/// Maximum tick value to regard tick overflow
#define TICKS_MAX_VALUE                  (RTOS_MAX_TIMEOUT)

/// Enumeration für den Zustand des RAM-Tests
/// Zeigt an, ob der Test konfiguriert ist.
typedef enum
{
    RAM_IDLE = 0,         ///< RAM_IDLE Ausgangszustand
    RAM_CONFIGURED = 1,   ///< RAM_CONFIGURED konfiguriert, bereit zum Starten
} RAM_TEST_STATE;

/// Struktur für den Teststatus zur Laufzeit
typedef struct
{
    STL_TmStatus_t tmStatus;          ///< Testmodul-Status von STL
    RAM_TEST_STATE ramTestState;      ///< Status des RAM-Tests
    STL_MemConfig_t memoryConfig;     ///< Eingestellte Testkonfiguration
#if FEAT_DEBUG
    U32 testRoundCounter;             ///< Counts number of test executions
#endif
} RAM_TEST;

/// Laufzeitwerte für kompletten RAM-Testdurclauf
static RAM_TEST ramTestAll =
                {
                    STL_ERROR,
                    RAM_IDLE,
                    {NULL},
                #if FEAT_DEBUG
                    0,
                #endif
                };

/// Laufzeitwerte für zyklischen RAM-Testdurchlauf
static RAM_TEST ramTestCyclic =
                {
                    STL_ERROR,
                    RAM_IDLE,
                    {NULL},
                #if FEAT_DEBUG
                    0,
                #endif
                };

/// Definition der Testbereiche im RAM
static EN61508_MEM_REGION const ramRegions[] =
    {
        { (U8*) &__RAM_TESTREGION1_START, (U32) &__RAM_TESTREGION1_SIZE },
#ifdef FEATURE_RAMTEST_INCLUDE_TESTREGION2
        { (U8*) &__RAM_TESTREGION2_START, (U32) &__RAM_TESTREGION2_SIZE },
#endif
#ifdef FEATURE_RAMTEST_INCLUDE_TESTREGION3
        { (U8*) &__RAM_TESTREGION3_START, (U32) &__RAM_TESTREGION3_SIZE },
#endif
    };

/// Anzahl der Testbereiche.
#define NUM_RAM_REGIONS (sizeof(ramRegions) / sizeof(EN61508_MEM_REGION))

static STL_MemSubset_t ramSubsets[NUM_RAM_REGIONS];

/// Last time in ticks at which the cyclic test passed
static U32 lastTestpassTicks = 0;
/// Process safety time in ticks set by the user
static U32 processSafetyTimeTicksInt = 0;

//------------------------------------------------------------------------------
// Prototypen interne Funktionen -----------------------------------------------
//------------------------------------------------------------------------------

/// Initialisierung des kompletten RAM-Tests.
/// \return @c true bei Erfolg, sonst @c false.
static bool RAMTestStl_SetupTestAll(void);

/// Initialisiert und konfiguriert den RAM-Test anhand der übergebenen
/// Einstellungen.
/// \param ramTest Handle des RAM-Tests mit den Konfigurationen für den Speicher.
/// \return @c true bei Erfolg, sonst @c false.
static bool RAMTestStl_SetupTest(RAM_TEST * const ramTest);

/// Intialisiert den RAM-Test.
/// \param ramTest Handle des zugehörigen RAM-Tests.
/// \return @c true bei Erfolg, sonst @c false.
static bool RAMTestStl_Init(RAM_TEST * const ramTest);

/// Setzt den RAM-Test zurück. Die Aktion ist erforderlich, wenn
/// der RAM-Test einmal durchgelaufen ist und neu gestartet werden soll.
/// \param ramTest Handle des zugehörigen RAM-Tests.
/// \return @c true bei Erfolg, sonst @c false.
static bool RAMTestStl_Reset(RAM_TEST * const ramTest);

/// Prüft die Speicherbereiche, die getestet werden sollen, auf korrekte Adressangaben.
/// \param config Konfiguration mit den definierten Speicherbereichen, die getestet werden sollen.
/// \return @c true bei Erfolg, sonst @c false.
static bool RAMTestStl_CheckConfig(STL_MemConfig_t const * const config);

/// Prüft, dass der RAM-Backup-Puffer nicht mit dem Testbereich überlappt.
/// Die beiden Bereiche dürfen nicht überlappen.
/// \param testStartAddress Startadresse des Testbereichs.
/// \param testEndAddress Endadresse des Testbereichs.
/// \return @c true, wenn der Backup-Puffer nicht im Testbereich liegt,
/// @c false, wenn Backup-Puffer und Testbreich sich überschneiden.
static bool RAMTestStl_CheckConfigRamBackup(U32 const testStartAddress, U32 const testEndAddress);

/// Setzt die Zustände der RAM-Tests auf den Ausgangszustand @ref RAM_IDLE zurück.
static void RAMTestStl_SetIdle(void);

//------------------------------------------------------------------------------
// Funktionsbereich externe Funktionen -----------------------------------------
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
EN61508_TestResult RAMTestStl_RunAll(void)
    {
    EN61508_TestResult testResult;
    STL_Status_t stlError;

    stlError = STL_KO;
    testResult = EN61508_TestFail;

    if(RAMTestStl_SetupTestAll())
        {
#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
#if RAMTEST_ARTI_FAILING_RUN_ALL
        RAMTEST_ARTI_FAILING_START
#endif
#endif

        stlError = STL_SCH_RunRamTM(&ramTestAll.tmStatus);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
#if RAMTEST_ARTI_FAILING_RUN_ALL
        RAMTEST_ARTI_FAILING_STOP
#endif
#endif

#if FEAT_DEBUG
        // Increase test execution counter
        ramTestAll.testRoundCounter++;
#endif
        }

    if((stlError == STL_OK) && (ramTestAll.tmStatus == STL_PASSED))
        {
        testResult = EN61508_TestPass;
        }

    return testResult;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
EN61508_TestResult RAMTestStl_RunCyclic(U32 const currentTicks)
    {
    EN61508_TestResult testResult;
    STL_Status_t stlError;
    bool runRam;
    bool processSafetyTimeFailure;
    static bool firstTestStart = true;

    runRam = true;
    stlError = STL_KO;
    testResult = EN61508_TestFail;
    processSafetyTimeFailure = false;

    // Set time reference after bootup
    if(firstTestStart)
        {
        firstTestStart = false;
        lastTestpassTicks = currentTicks;
        }

    if(ramTestCyclic.ramTestState != RAM_CONFIGURED)
        {
        runRam = false;
        }

    // Testausführung
    if(runRam)
        {
#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
#if RAMTEST_ARTI_FAILING_RUN_CYCLIC
        RAMTEST_ARTI_FAILING_START
#endif
#endif

        stlError = STL_SCH_RunRamTM(&ramTestCyclic.tmStatus);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
#if RAMTEST_ARTI_FAILING_RUN_CYCLIC
        RAMTEST_ARTI_FAILING_STOP
#endif
#endif

#if FEAT_DEBUG
        // Increase test execution counter
        ramTestCyclic.testRoundCounter++;
#endif
        }

    if(runRam && (stlError == STL_OK))
        {
        // Statusüberprüfung
        switch(ramTestCyclic.tmStatus)
            {
            case STL_PARTIAL_PASSED:
                // Teilstück bestanden
                testResult = EN61508_TestPass;
                break;
            case STL_PASSED:
                lastTestpassTicks = currentTicks;

                // Test completed successfully, reset test
                if(RAMTestStl_Reset(&ramTestCyclic))
                    {
                    testResult = EN61508_TestPass;
                    }
                break;
            default:
                testResult = EN61508_TestFail;
                break;
            }
        }

    // Check process safety timeout
    // The Runtime RAM has to be completed at least once per Process Safety Time (SOFTQM-462)
    if(currentTicks >= lastTestpassTicks)
        {
        processSafetyTimeFailure = ((currentTicks - lastTestpassTicks) > processSafetyTimeTicksInt);
        }
    else
        {
        processSafetyTimeFailure = ((TICKS_MAX_VALUE - lastTestpassTicks + currentTicks) > processSafetyTimeTicksInt);
        }

    if(processSafetyTimeFailure)
        {
        testResult = EN61508_TestFail;
        }

    return testResult;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
bool RAMTestStl_SetupTestCyclic(U32 const processSafetyTimeTicks)
    {
    U8 i;

    processSafetyTimeTicksInt = processSafetyTimeTicks;

    for(i = 0; i < NUM_RAM_REGIONS; i++)
        {
        ramSubsets[i].StartAddr = (U32) ramRegions[i].start;
        ramSubsets[i].EndAddr = (U32) ramRegions[i].start + ramRegions[i].length - 1;

        if(i == (NUM_RAM_REGIONS - 1))
            {
            ramSubsets[i].pNext = NULL;
            }
        else
            {
            ramSubsets[i].pNext = &ramSubsets[i + 1];
            }
        }

    ramTestCyclic.memoryConfig.pSubset = ramSubsets;

    // Number of tested RAM sections per test execution (SOFTQM-454)
    ramTestCyclic.memoryConfig.NumSectionsAtomic = RAMTEST_CYCLIC_NUM_SECTIONS;

#if FEAT_DEBUG
    // Set test round counter to zero
    ramTestCyclic.testRoundCounter = 0;
#endif

    return RAMTestStl_SetupTest(&ramTestCyclic);
    }
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Funktionsbereich interne Funktionen -----------------------------------------
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
static bool RAMTestStl_SetupTestAll(void)
    {
    U8 i;

    for(i = 0; i < NUM_RAM_REGIONS; i++)
        {
        ramSubsets[i].StartAddr = (U32) ramRegions[i].start;
        ramSubsets[i].EndAddr = (U32) ramRegions[i].start + ramRegions[i].length - 1;

        if(i == (NUM_RAM_REGIONS - 1))
            {
            ramSubsets[i].pNext = NULL;
            }
        else
            {
            ramSubsets[i].pNext = &ramSubsets[i + 1];
            }
        }

    ramTestAll.memoryConfig.pSubset = ramSubsets;

    // Number of tested RAM sections per test execution
    ramTestAll.memoryConfig.NumSectionsAtomic = RAMTEST_NUM_SECTIONS_ATOMIC_MAX;

#if FEAT_DEBUG
    // Set test round counter to zero
    ramTestAll.testRoundCounter = 0;
#endif

    return RAMTestStl_SetupTest(&ramTestAll);
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
static bool RAMTestStl_SetupTest(RAM_TEST * const ramTest)
    {
    bool result;
    bool isInitialized;
    STL_Status_t stlError;

    if(ramTest == NULL)
        {
        return false;
        }

    if(!RAMTestStl_CheckConfig(&ramTest->memoryConfig))
        {
        return false;
        }

    isInitialized = false;
    result = false;
    stlError = STL_KO;

    if(RAMTestStl_Init(ramTest))
        {
        isInitialized = true;
        }

    if(isInitialized)
        {
#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
    #if RAMTEST_ARTI_FAILING_CONFIGURE_ALL
            RAMTEST_ARTI_FAILING_START_ALL
    #elif RAMTEST_ARTI_FAILING_CONFIGURE_CYCLIC
            RAMTEST_ARTI_FAILING_START_CYCLIC
    #endif
#endif

        stlError = STL_SCH_ConfigureRam(&ramTest->tmStatus, &ramTest->memoryConfig);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
    #if RAMTEST_ARTI_FAILING_CONFIGURE_ALL
            RAMTEST_ARTI_FAILING_STOP_ALL
    #elif RAMTEST_ARTI_FAILING_CONFIGURE_CYCLIC
            RAMTEST_ARTI_FAILING_STOP_CYCLIC
    #endif
#endif

        if((stlError == STL_OK) && (ramTest->tmStatus == STL_NOT_TESTED))
            {
            // Setup erfolgreich
            ramTest->ramTestState = RAM_CONFIGURED;
            result = true;
            }
        }
    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
static bool RAMTestStl_Init(RAM_TEST * const ramTest)
    {
    bool result;
    STL_Status_t stlError;

    result = false;
    stlError = STL_KO;

    if(ramTest == NULL)
        {
        return false;
        }

    ramTest->tmStatus = STL_ERROR;

    // Alle Konfigurationen löschen
    RAMTestStl_SetIdle();

    if(Stl_SchedulerIsStarted())
        {
#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
    #if RAMTEST_ARTI_FAILING_INIT_ALL
            RAMTEST_ARTI_FAILING_START_ALL
    #elif RAMTEST_ARTI_FAILING_INIT_CYCLIC
            RAMTEST_ARTI_FAILING_START_CYCLIC
    #endif
#endif

        stlError = STL_SCH_InitRam(&ramTest->tmStatus);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
    #if RAMTEST_ARTI_FAILING_INIT_ALL
            RAMTEST_ARTI_FAILING_STOP_ALL
    #elif RAMTEST_ARTI_FAILING_INIT_CYCLIC
            RAMTEST_ARTI_FAILING_STOP_CYCLIC
    #endif
#endif
        }

    if((stlError == STL_OK) && (ramTest->tmStatus == STL_NOT_TESTED))
        {
        result = true;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
static bool RAMTestStl_Reset(RAM_TEST * const ramTest)
    {
    bool result;
    STL_Status_t stlError;

    result = false;
    stlError = STL_KO;

    if(ramTest == NULL)
        {
        return false;
        }

    // STL-Reset kann nur ausgeführt werden, wenn der RAM-Test konfiguriert wurde
    if(ramTest->ramTestState == RAM_CONFIGURED)
        {
#if FEAT_DEBUG
        // Reset test execution counter
        ramTest->testRoundCounter = 0;
#endif

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
#if RAMTEST_ARTI_FAILING_RESET_CYCLIC
        RAMTEST_ARTI_FAILING_START
#endif
#endif

        stlError = STL_SCH_ResetRam(&ramTest->tmStatus);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_RAM_STL
#if RAMTEST_ARTI_FAILING_RESET_CYCLIC
        RAMTEST_ARTI_FAILING_STOP
#endif
#endif
        }

    if((stlError == STL_OK) && (ramTest->tmStatus == STL_NOT_TESTED))
        {
        result = true;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 18.01.2023
static bool RAMTestStl_CheckConfig(STL_MemConfig_t const * const config)
    {
    bool result;
    STL_MemSubset_t * subset;

    result = true;

    if(config == NULL)
        {
        return false;
        }

    subset = config->pSubset;

    while(subset != NULL)
        {
        // Testbereich darf nicht mit dem RAM-Backup-Bereich überlappen
        if(!RAMTestStl_CheckConfigRamBackup(subset->StartAddr, subset->EndAddr))
            {
            result = false;
            }

        // Startadresse muss an 4 Byte ausgerichtet sein.
        if((subset->StartAddr % STL_RAM_ADDRESS_ALIGNMENT) != 0)
            {
            result = false;
            }

        // Endadresse muss an 4 Byte ausgerichtet sein.
        if((subset->EndAddr + 1) % STL_RAM_ADDRESS_ALIGNMENT != 0)
            {
            result = false;
            }

        // Subset-Größe muss ein Vielfaches von 2 * Ram-Blockgröße sein.
        if((subset->EndAddr + 1 - subset->StartAddr) % (2 * STL_RAM_BLOCK_SIZE))
            {
            result = false;
            }

        subset = subset->pNext;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 24.01.2023
static bool RAMTestStl_CheckConfigRamBackup(U32 const testStartAddress, U32 const testEndAddress)
    {
    bool result;
    U32 backupEnd;
    EN61508_MEM_REGION backupBuffer = {(U8*) &__SRAM_RAMTEST_BACKUP_START, (U32) &__SRAM_RAMTEST_BACKUP_SIZE};

    backupEnd = (U32) backupBuffer.start + backupBuffer.length - 1;

    result = false;

    // RAM-Backup-Puffer liegt vollständig vor dem Testbereich ist erlaubt
    if(((U32) backupBuffer.start < testStartAddress) && ((U32) backupEnd < testStartAddress))
        {
        result = true;
        }

    // RAM-Backup-Puffer liegt vollständig hinter dem Testbereich ist erlaubt
    if(((U32) backupBuffer.start > testEndAddress) && ((U32) backupEnd > testEndAddress))
        {
        result = true;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 24.01.2023
static void RAMTestStl_SetIdle(void)
    {
    ramTestAll.ramTestState = RAM_IDLE;
    ramTestCyclic.ramTestState = RAM_IDLE;
    }
//------------------------------------------------------------------------------


