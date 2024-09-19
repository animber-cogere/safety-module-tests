/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/


// Headerdateien einbinden -----------------------------------------------------
#include "config/version.h"

#include "SafetyStl.h"

#ifndef FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL
#define FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL    (0)
#endif

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL
#include "CPUTestStl_ArtificialFailing.h"
#endif

#include "CPUTestStl.h"
// Allgemeine Definitionen -----------------------------------------------------

typedef STL_Status_t (*func_CpuTestStl)(STL_TmStatus_t * const pSingleTmStatus);

/// Statuswerte des zyklischen CPU-Tests
/// Zeigt an, ob der zyklische Test konfiguriert ist.
typedef enum
{
    CPU_CYCLIC_IDLE,        ///< CPU_CYCLIC_IDLE Ausgangszustand
    CPU_CYCLIC_CONFIGURED,  ///< CPU_CYCLIC_CONFIGURED konfiguriert, bereit zum Starten
} CPU_CYCLIC_STATE;

/// Struktur für einen CPU-Test
typedef struct
{
    STL_TmStatus_t tmStatus;         ///< Testmodul-Status von STL
    STL_TmEnable_t tmEnable;         ///< Aktivieren
    func_CpuTestStl cpuTestFunction; ///< Funktion zum Aufruf des Tests
} CPU_TEST;

CPU_TEST cpuTest[STL_CPU_TM_MAX] = { { STL_ERROR, STL_TEST_DISABLE, STL_SCH_RunCpuTM1 },   //0  TM1
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM1L },   //1  TM1L
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM2 },    //2  TM2
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM3 },    //3  TM3
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM4 },    //4  TM4
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM5 },    //5  TM5
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM6 },    //6  TM6
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM7 },    //7  TM7
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM8 },    //8  TM8
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM9 },    //9  TM9
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM10 },   //10 TM10
                                     { STL_ERROR, STL_TEST_ENABLE, STL_SCH_RunCpuTM11 } }; //11 TM11

/// Struktur für den zyklischen CPU-Test, enthält den gerade ausgeführten Test
typedef struct
{
    CPU_TEST * currentTest;         ///< Zeiger auf aktuellen CPU-Test
    STL_CpuTmxIndex_t testIndex;    ///< Aktueller Testindex
    CPU_CYCLIC_STATE cyclicState;   ///< Status des zyklischen CPU-Tests.
} CPU_TEST_CYCLIC;

/// Maximum tick value to regard tick overflow
#define TICKS_MAX_VALUE                  (0xFFFFFFFF)

CPU_TEST_CYCLIC cpuTestCyclic = {NULL, CPU_CYCLIC_IDLE, STL_CPU_TM1_IDX};

/// Last time in ticks at which the cyclic test passed
static U32 lastTestpassTicks = 0;
/// Process safety time in ticks set by the user
static U32 processSafetyTimeTicksInt = 0;

//------------------------------------------------------------------------------
// Prototypen interne Funktionen -----------------------------------------------
//------------------------------------------------------------------------------
/// Behandelt die Ausführung des übergebenen CPU-Tests.
/// Aktivierte Tests werden ausgeführt, deaktivierte Tests werden übersprungen
/// und bekommen den Status auf @ref STL_NOT_TESTED gesetzt.
/// \param cpuTestHandle Zeiger auf den CPU-Test, der ausgeführt werden soll.
/// \return true, wenn die Testausführung erfolgreich abgearbeitet wurde.
/// false, wenn bei der Ausführung der Testfunktion ein Fehler auftritt.
static bool CPUTestStl_HandleExecution(CPU_TEST * const cpuTestHandle);

/// Überprüft die Testergebnisse im CPU-Teststatus (@ref STL_TmStatus_t).
/// Aktivierte Tests müssen den Status @ref STL_PASSED haben, deaktivierte Tests den
/// Status @ref STL_NOT_TESTED. Andere Zustände sind nicht erlaubt.
/// \param cpuTestHandle Handle zum CPU-Test.
/// \return true bei Erfolg, sonst false.
static bool CPUTestStl_CheckStatusResult(CPU_TEST const * const cpuTestHandle);

/// Setzt die Zustände der CPU-Tests zurück auf "nicht-getestet" (@ref STL_NOT_TESTED).
static void CPUTestStl_ResetAll(void);
//------------------------------------------------------------------------------
// Funktionsbereich externe Funktionen -----------------------------------------
//------------------------------------------------------------------------------

/// @author k.ehlen @date 16.01.2023
EN61508_TestResult CPUTestStl_RunAll(void)
    {
    EN61508_TestResult testResult;
    U8 i;
    CPU_TEST * cpuTestHandle;

    if(!Stl_SchedulerIsStarted())
        {
        return EN61508_TestFail;
        }

    testResult = EN61508_TestPass;

    // Teststati zurücksetzen
    CPUTestStl_ResetAll();

    for (i = 0; i < STL_CPU_TM_MAX; i++)
        {
        cpuTestHandle = &cpuTest[i];

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL
#if CPUTEST_ARTI_FAILING_RUN_ALL
        CPUTEST_ARTI_FAILING_START
#endif
#endif

        // Testausführung
        if(!CPUTestStl_HandleExecution(cpuTestHandle))
            {
            testResult = EN61508_TestFail;
            }

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL
#if CPUTEST_ARTI_FAILING_RUN_ALL
        CPUTEST_ARTI_FAILING_STOP
#endif
#endif

        // Status prüfen
        if(!CPUTestStl_CheckStatusResult(cpuTestHandle))
            {
            testResult = EN61508_TestFail;
            }
        }

    return testResult;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 23.01.2023
EN61508_TestResult CPUTestStl_RunCyclic(U32 const currentTicks)
    {
    EN61508_TestResult testResult;
    bool processSafetyTimeFailure;
    static bool firstTestStart = true;

    if(cpuTestCyclic.cyclicState != CPU_CYCLIC_CONFIGURED)
        {
        return EN61508_TestFail;
        }

    testResult = EN61508_TestPass;
    processSafetyTimeFailure = false;

    // Set time reference after bootup
    if(firstTestStart)
        {
        firstTestStart = false;
        lastTestpassTicks = currentTicks;
        }

    // Teststati zurücksetzen, sobald von Vorne gestartet wird
    if(cpuTestCyclic.currentTest == &cpuTest[STL_CPU_TM1_IDX])
        {
        CPUTestStl_ResetAll();
        }
#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL
#if CPUTEST_ARTI_FAILING_RUN_CYCLIC
        CPUTEST_ARTI_FAILING_START
#endif
#endif

    // Testausführung
    if(!CPUTestStl_HandleExecution(cpuTestCyclic.currentTest))
        {
        testResult = EN61508_TestFail;
        cpuTestCyclic.cyclicState = CPU_CYCLIC_IDLE;
        }

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL
#if CPUTEST_ARTI_FAILING_RUN_CYCLIC
        CPUTEST_ARTI_FAILING_STOP
#endif
#endif

    // Prüfung Teststati
    if(!CPUTestStl_CheckStatusResult(cpuTestCyclic.currentTest))
        {
        testResult = EN61508_TestFail;
        cpuTestCyclic.cyclicState = CPU_CYCLIC_IDLE;
        }

    // Auswahl des nächsten CPU-Tests
    if(testResult == EN61508_TestPass)
        {
        cpuTestCyclic.testIndex++;

        if(cpuTestCyclic.testIndex == STL_CPU_TM_MAX)
            {
            lastTestpassTicks = currentTicks;
            cpuTestCyclic.testIndex = STL_CPU_TM1_IDX;
            }

        cpuTestCyclic.currentTest = &cpuTest[cpuTestCyclic.testIndex];
        }

    // Check process safety timeout
    // All CPU tests have to be executed at least once per Process Safety Time (SOFTQM-1040)
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

/// @author k.ehlen @date 23.01.2023
bool CPUTestStl_SetupTestCyclic(U32 const processSafetyTimeTicks)
    {
    bool result;

    if(Stl_SchedulerIsStarted())
        {
        cpuTestCyclic.testIndex = STL_CPU_TM1_IDX;
        cpuTestCyclic.currentTest = &cpuTest[cpuTestCyclic.testIndex];
        cpuTestCyclic.cyclicState = CPU_CYCLIC_CONFIGURED;
        result = true;
        processSafetyTimeTicksInt = processSafetyTimeTicks;
        }
    else
        {
        cpuTestCyclic.cyclicState = CPU_CYCLIC_IDLE;
        result = false;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 16.01.2023
EN61508_TestResult CPUTestStl_RunSingle(STL_CpuTmxIndex_t const cpuIndex)
    {
    EN61508_TestResult testResult;

    CPU_TEST * cpuTestHandle;

    if(cpuIndex >= STL_CPU_TM_MAX)
        {
        return EN61508_TestFail;
        }

    if(!Stl_SchedulerIsStarted())
        {
        return EN61508_TestFail;
        }

    cpuTestHandle = &cpuTest[cpuIndex];
    testResult = EN61508_TestPass;

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL
#if CPUTEST_ARTI_FAILING_RUN_SINGLE
        CPUTEST_ARTI_FAILING_START
#endif
#endif

    // Testausführung
    if(!CPUTestStl_HandleExecution(cpuTestHandle))
        {
        testResult = EN61508_TestFail;
        }

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL
#if CPUTEST_ARTI_FAILING_RUN_SINGLE
        CPUTEST_ARTI_FAILING_STOP
#endif
#endif

    // Prüfen, ob Test bestanden wurde
    if(cpuTestHandle->tmStatus != STL_PASSED)
        {
        testResult = EN61508_TestFail;
        }

    return testResult;
    }
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Funktionsbereich interne Funktionen -----------------------------------------

/// @author k.ehlen @date 18.01.2023
static bool CPUTestStl_HandleExecution(CPU_TEST * const cpuTestHandle)
    {
    bool result;
    STL_Status_t stlError;

    if(cpuTestHandle == NULL)
        {
        return false;
        }

    result = true;
    cpuTestHandle->tmStatus = STL_ERROR;

    // Testausführung, wenn Test aktiviert ist, sonst Statusänderung auf nicht-getestet
    if(cpuTestHandle->tmEnable == STL_TEST_ENABLE)
        {
        stlError = cpuTestHandle->cpuTestFunction(&cpuTestHandle->tmStatus);
        }
    else
        {
        cpuTestHandle->tmStatus = STL_NOT_TESTED;
        stlError = STL_OK;
        }

    if(stlError != STL_OK)
        {
        result = false;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 18.01.2023
static bool CPUTestStl_CheckStatusResult(CPU_TEST const * const cpuTestHandle)
    {
    bool result;

    if(cpuTestHandle == NULL)
        {
        return false;
        }

    result = false;

    // Aktivierte Tests, Prüfung auf bestanden
    if((cpuTestHandle->tmEnable == STL_TEST_ENABLE) && (cpuTestHandle->tmStatus == STL_PASSED))
        {
        result = true;
        }

    // Deaktivierte Tests, Prüfung auf nicht-getestet
    if((cpuTestHandle->tmEnable == STL_TEST_DISABLE) && (cpuTestHandle->tmStatus == STL_NOT_TESTED))
        {
        result = true;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 23.01.2023
static void CPUTestStl_ResetAll(void)
    {
    U8 i;
    for (i = 0; i < STL_CPU_TM_MAX; i++)
        {
        cpuTest[i].tmStatus = STL_NOT_TESTED;
        }
    }
//------------------------------------------------------------------------------
