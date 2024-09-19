/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/


// Headerdateien einbinden -----------------------------------------------------
#include "config/version.h"
#include "stm32g4xx_hal.h"

#include "SafetyStl.h"

/// Baut Artificial-Failing Funktionalitäten zum Testen des Moduls ein
#ifndef FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
#define FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL              (0)
#endif

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
#include "ROMTestStl_ArtificialFailing.h"
#endif

#include "ROMTestStl.h"
// Allgemeine Definitionen -----------------------------------------------------

/// Safety-relevant ROM-testregion defined by user in linker file (SOFTQM-413)
/// must be a contiguous region in the FLASH
extern U32 __FLASH_TESTREGION_START;
extern U32 __FLASH_TESTREGION_SIZE;
extern U32 __FLASH_TEST_STL_CRC_START;

/// User configuration number of tested ROM sections
/// per test execution for cyclic runtime ROM test (SOFTQM-1076)
/**
 * Minimum value: @ref ROMTEST_NUM_SECTIONS_ATOMIC_MIN
 * Maximum value for 1-shot-execution: @ref ROMTEST_NUM_SECTIONS_ATOMIC_MAX
 *
 * Any value larger than the number of sections in the whole testregion
 * automatically leads to a 1-shot execution.
 * 1-shot: One test execution checks the whole test region
 */
#ifndef ROMTEST_CYCLIC_NUM_SECTIONS
#pragma message "Please define the number of ROM sections to be tested per test execution in the cyclic ROM test"
#define ROMTEST_CYCLIC_NUM_SECTIONS      (ROMTEST_NUM_SECTIONS_ATOMIC_MIN)
#endif

/// Blockgröße 4 Bytes. Die getesteten Speicherbereiche müssen auf 4-Byte-Adressen enden.
#define STL_FLASH_BLOCK_SIZE      (sizeof(U32))

/// Maximum tick value to regard tick overflow
#define TICKS_MAX_VALUE                  (RTOS_MAX_TIMEOUT)

/// Enumeration für den Zustand des ROM-Tests
/// Zeigt an, ob der Test konfiguriert ist.
typedef enum
{
    ROM_IDLE = 0,         ///< ROM_IDLE Ausgangszustand
    ROM_CONFIGURED = 1,   ///< ROM_CONFIGURED konfiguriert, bereit zum Starten
} ROM_TEST_STATE;

/// Struktur für den Teststatus zur Laufzeit
typedef struct
{
    STL_TmStatus_t tmStatus;          ///< Testmodul-Status von STL
    ROM_TEST_STATE romTestState;      ///< Status des ROM-Tests
    STL_MemConfig_t memoryConfig;     ///< Eingestellte Testkonfiguration
#if FEAT_DEBUG
    U32 testRoundCounter;             ///< Counts number of test executions
#endif
} ROM_TEST;

/// Laufzeitwerte für kompletten ROM-Testdurclauf
static ROM_TEST romTestAll =
                {
                    STL_ERROR,
                    ROM_IDLE,
                    {NULL},
                #if FEAT_DEBUG
                    0,
                #endif
                };

/// Laufzeitwerte für zyklischen ROM-Testdurchlauf
static ROM_TEST romTestCyclic =
                {
                    STL_ERROR,
                    ROM_IDLE,
                    {NULL},
                #if FEAT_DEBUG
                    0,
                #endif
                };

/// Definition der Flash-Bereiche, die beim ROM-Test getestet werden
static EN61508_MEM_REGION flashRegions[] =
                {
                    {(U8 *) &__FLASH_TESTREGION_START, (U32) &__FLASH_TESTREGION_SIZE }
                };

/// Anzahl der Flashbereiche, die getestet werden
#define NUM_FLASH_REGIONS (sizeof(flashRegions)/sizeof(EN61508_MEM_REGION))

/// Subset-Einstellung für STL
static STL_MemSubset_t flashSubsets[NUM_FLASH_REGIONS];

/// Last time in ticks at which the cyclic test passed
static U32 lastTestpassTicks = 0;
/// Process safety time in ticks set by the user
static U32 processSafetyTimeTicksInt = 0;

//------------------------------------------------------------------------------
// Prototypen interne Funktionen -----------------------------------------------
//------------------------------------------------------------------------------

/// Initialisierung des kompletten ROM-Tests.
/// \return @c true bei Erfolg, sonst @c false.
static bool ROMTestStl_SetupTestAll(void);

/// Initialisiert und konfiguriert den ROM-Test anhand der übergebenen
/// Einstellungen.
/// \param romTest Handle des ROM-Tests mit den zu testenden Speicherbereichen.
/// \return @c true bei Erfolg, sonst @c false.
static bool ROMTestStl_SetupTest(ROM_TEST * const romTest);

/// Intialisiert den ROM-Test
/// \param romTest Handle des ROM-Tests.
/// \return @c true bei Erfolg, sonst @c false.
static bool ROMTestStl_Init(ROM_TEST * const romTest);

/// Setzt den ROM-Test zurück. Die Aktion ist erforderlich, wenn
/// der ROM-Test einmal durchgelaufen ist und neu gestartet werden soll.
/// \param romTest Handle des ROM-Tests.
/// \return @c true bei Erfolg, sonst @c false.
static bool ROMTestStl_Reset(ROM_TEST * const romTest);

/// Prüft die Speicherbereiche, die getestet werden sollen, auf korrekte Adressangaben.
/// (Handbuch UM2590)
/// \param config Konfiguration mit den definierten Speicherbereichen, die getestet werden sollen.
/// \return @c true bei Erfolg, sonst @c false.
static bool ROMTestStl_CheckMemConfig(STL_MemConfig_t const * const config);

/// Setzt die Zustände der ROM-Tests auf den Ausgangszustand @ref ROM_IDLE zurück.
static void ROMTestStl_SetIdle(void);
//------------------------------------------------------------------------------
// Funktionsbereich externe Funktionen -----------------------------------------
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
EN61508_TestResult ROMTestStl_RunAll(void)
    {
    EN61508_TestResult testResult;
    STL_Status_t stlError;

    stlError = STL_KO;
    testResult = EN61508_TestFail;

    if(ROMTestStl_SetupTestAll())
        {

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
#if ROMTEST_ARTI_FAILING_RUN_ALL
        ROMTEST_ARTI_FAILING_START
#endif
#endif
        stlError = STL_SCH_RunFlashTM(&romTestAll.tmStatus);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
#if ROMTEST_ARTI_FAILING_RUN_ALL
        ROMTEST_ARTI_FAILING_STOP
#endif
#endif
#if FEAT_DEBUG
        // Increase test execution counter
        romTestAll.testRoundCounter++;
#endif
        }

    if((stlError == STL_OK) && (romTestAll.tmStatus == STL_PASSED))
        {
        testResult = EN61508_TestPass;
        }

    return testResult;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
EN61508_TestResult ROMTestStl_RunCyclic(U32 const currentTicks)
    {
    EN61508_TestResult testResult;
    STL_Status_t stlError;
    bool processSafetyTimeFailure;
    static bool firstTestStart = true;

    stlError = STL_KO;
    testResult = EN61508_TestFail;
    processSafetyTimeFailure = false;

    // Set time reference after bootup
    if(firstTestStart)
        {
        firstTestStart = false;
        lastTestpassTicks = currentTicks;
        }

    // Testausführung
    if(romTestCyclic.romTestState == ROM_CONFIGURED)
        {
#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
#if ROMTEST_ARTI_FAILING_RUN_CYCLIC
        ROMTEST_ARTI_FAILING_START
#endif
#endif
        stlError = STL_SCH_RunFlashTM(&romTestCyclic.tmStatus);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
#if ROMTEST_ARTI_FAILING_RUN_CYCLIC
        ROMTEST_ARTI_FAILING_STOP
#endif
#endif
#if FEAT_DEBUG
        // Increase test execution counter
        romTestCyclic.testRoundCounter++;
#endif
        }

    if(stlError == STL_OK)
        {
        // Statusüberprüfung
        switch(romTestCyclic.tmStatus)
            {
            case STL_PARTIAL_PASSED:
                // Teilstück bestanden
                testResult = EN61508_TestPass;
                break;
            case STL_PASSED:
                lastTestpassTicks = currentTicks;

                // Test completed successfully, reset test
                if(ROMTestStl_Reset(&romTestCyclic))
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
    // The Runtime ROM has to be completed at least once per Process Safety Time (SOFTQM-527)
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
bool ROMTestStl_SetupTestCyclic(U32 const processSafetyTimeTicks)
    {
    U8 i;

    processSafetyTimeTicksInt = processSafetyTimeTicks;

    for(i = 0; i < NUM_FLASH_REGIONS; i++)
        {
        flashSubsets[i].StartAddr = (U32) flashRegions[i].start;
        flashSubsets[i].EndAddr = (U32) flashRegions[i].start + flashRegions[i].length - 1;

        if(i == (NUM_FLASH_REGIONS -1))
            {
            flashSubsets[i].pNext = NULL;
            }
        else
            {
            flashSubsets[i].pNext = &flashSubsets[i + 1];
            }
        }

    romTestCyclic.memoryConfig.pSubset = flashSubsets;

    // Number of tested ROM sections per test execution (SOFTQM-526)
    romTestCyclic.memoryConfig.NumSectionsAtomic = ROMTEST_CYCLIC_NUM_SECTIONS;

#if FEAT_DEBUG
    // Set test round counter to zero
    romTestCyclic.testRoundCounter = 0;
#endif

    return ROMTestStl_SetupTest(&romTestCyclic);
    }
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Funktionsbereich interne Funktionen -----------------------------------------

/// @author k.ehlen @date 08.01.2023
static bool ROMTestStl_SetupTestAll(void)
    {
    U8 i;

    for(i = 0; i < NUM_FLASH_REGIONS; i++)
        {
        flashSubsets[i].StartAddr = (U32) flashRegions[i].start;
        flashSubsets[i].EndAddr = (U32) flashRegions[i].start + flashRegions[i].length - 1;

        if(i == (NUM_FLASH_REGIONS -1))
            {
            flashSubsets[i].pNext = NULL;
            }
        else
            {
            flashSubsets[i].pNext = &flashSubsets[i + 1];
            }
        }

    romTestAll.memoryConfig.pSubset = flashSubsets;

    // Number of tested ROM sections per test execution
    romTestAll.memoryConfig.NumSectionsAtomic = ROMTEST_NUM_SECTIONS_ATOMIC_MAX;

#if FEAT_DEBUG
    // Set test round counter to zero
    romTestAll.testRoundCounter = 0;
#endif

    return ROMTestStl_SetupTest(&romTestAll);
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
static bool ROMTestStl_SetupTest(ROM_TEST * const romTest)
    {
    bool result;
    bool isInitialized;
    STL_Status_t stlError;

    if(romTest == NULL)
        {
        return false;
        }

    if(!ROMTestStl_CheckMemConfig(&romTest->memoryConfig))
        {
        return false;
        }

    isInitialized = false;
    result = false;
    stlError = STL_KO;

    if(ROMTestStl_Init(romTest) == true)
        {
        isInitialized = true;
        }

    if(isInitialized)
        {
#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
    #if ROMTEST_ARTI_FAILING_CONFIGURE_ALL
            ROMTEST_ARTI_FAILING_START_ALL
    #elif ROMTEST_ARTI_FAILING_CONFIGURE_CYCLIC
            ROMTEST_ARTI_FAILING_START_CYCLIC
    #endif
#endif
        stlError = STL_SCH_ConfigureFlash(&romTest->tmStatus, &romTest->memoryConfig);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
    #if ROMTEST_ARTI_FAILING_CONFIGURE_ALL
            ROMTEST_ARTI_FAILING_STOP_ALL
    #elif ROMTEST_ARTI_FAILING_CONFIGURE_CYCLIC
            ROMTEST_ARTI_FAILING_STOP_CYCLIC
    #endif
#endif

        if((stlError == STL_OK) && (romTest->tmStatus == STL_NOT_TESTED))
            {
            romTest->romTestState = ROM_CONFIGURED;
            result = true;
            }
        }
    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
static bool ROMTestStl_Init(ROM_TEST * const romTest)
    {
    bool result;
    STL_Status_t stlError;

    stlError = STL_KO;
    result = false;

    if(romTest == NULL)
        {
        return false;
        }

    romTest->tmStatus = STL_ERROR;

    ROMTestStl_SetIdle();

    if(Stl_SchedulerIsStarted())
        {
#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
    #if ROMTEST_ARTI_FAILING_INIT_ALL
            ROMTEST_ARTI_FAILING_START_ALL
    #elif ROMTEST_ARTI_FAILING_INIT_CYCLIC
            ROMTEST_ARTI_FAILING_START_CYCLIC
    #endif
#endif
        stlError = STL_SCH_InitFlash(&romTest->tmStatus);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
    #if ROMTEST_ARTI_FAILING_INIT_ALL
            ROMTEST_ARTI_FAILING_STOP_ALL
    #elif ROMTEST_ARTI_FAILING_INIT_CYCLIC
            ROMTEST_ARTI_FAILING_STOP_CYCLIC
    #endif
#endif
        }

    if((stlError == STL_OK) && (romTest->tmStatus == STL_NOT_TESTED))
        {
        result = true;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
static bool ROMTestStl_Reset(ROM_TEST * const romTest)
    {
    bool result;
    STL_Status_t stlError;

    result = false;
    stlError = STL_KO;

    if(romTest == NULL)
        {
        return false;
        }

    // STL-Reset kann nur ausgeführt werden, wenn der ROM-Test konfiguriert wurde
    if(romTest->romTestState == ROM_CONFIGURED)
        {
#if FEAT_DEBUG
        // Reset test execution counter
        romTest->testRoundCounter = 0;
#endif

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
#if ROMTEST_ARTI_FAILING_RESET_CYCLIC
        ROMTEST_ARTI_FAILING_START
#endif
#endif
        stlError = STL_SCH_ResetFlash(&romTest->tmStatus);

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL
#if ROMTEST_ARTI_FAILING_RESET_CYCLIC
        ROMTEST_ARTI_FAILING_STOP
#endif
#endif
        }

    if((stlError == STL_OK) && (romTest->tmStatus == STL_NOT_TESTED))
        {
        result = true;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 18.01.2023
static bool ROMTestStl_CheckMemConfig(STL_MemConfig_t const * const config)
    {
    bool result;
    STL_MemSubset_t * subset;

    if(config == NULL)
        {
        return false;
        }

    result = true;
    subset = config->pSubset;

    while(subset != NULL)
        {
        // Testbereich Start- und Endadresse dürfen nicht im CRC-Bereich liegen
        if((subset->StartAddr < FLASH_BASE) || (subset->StartAddr >= (U32) &__FLASH_TEST_STL_CRC_START))
            {
            result = false;
            }

        if((subset->EndAddr < FLASH_BASE) || (subset->EndAddr >= (U32) &__FLASH_TEST_STL_CRC_START))
            {
            result = false;
            }

        // Startadresse muss kleiner sein als Endadresse
        if(subset->EndAddr <= subset->StartAddr)
            {
            result = false;
            }

        // Startadresse muss am Beginn eines Sektors (1024 Bytes) starten
        if((subset->StartAddr % STL_FLASH_SECTION_SIZE) != 0)
            {
            result = false;
            }

        // Endadresse muss 4-Byte-Ausrichtung haben
        if((subset->EndAddr + 1) % STL_FLASH_BLOCK_SIZE != 0)
            {
            result = false;
            }

        subset = subset->pNext;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 24.01.2023
static void ROMTestStl_SetIdle(void)
    {
    romTestAll.romTestState = ROM_IDLE;
    romTestCyclic.romTestState = ROM_IDLE;
    }
//------------------------------------------------------------------------------
