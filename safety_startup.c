/*******************************************************************************
 * Copyright (c) 2021 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

// Gemeinsame Headerdateien einbinden --------------------------------------
#include "config/version.h"

#if FEATURE_SAFETYCHECK_STARTUP

#include "SYSTEM/System_Driver.h"

#include "error_def.h"
#include "EN61508_Program_Flow/EN61508.h"

#if FEATURE_ERROR_LOGGING
#include "ErrorLogging/error_logging.h"
#include "ErrorHandler/error.h"
#endif

#include "safety_runtime.h"

#if FEATURE_SAFETYCHECK_USE_STL
#include "STM32_Safety_STL_API/SafetyStl.h"
#include "STM32_Safety_STL_API/ROMTestStl.h"
#include "STM32_Safety_STL_API/CPUTestStl.h"
#include "STM32_Safety_STL_API/RAMTestStl.h"
#endif

#include "safety_startup.h"

#if !FEATURE_SAFETYCHECK_STARTUP_RAM
#pragma message "Startup RAM Test ist deaktiviert."
#endif

#if !FEATURE_SAFETYCHECK_STARTUP_ROM
#pragma message "Startup ROM Test ist deaktiviert."
#endif

// Compiler Direktiven -----------------------------------------------------

// Makros ------------------------------------------------------------------

#if FEATURE_ERROR_LOGGING
#define HARD_ERROR_BYTE ((U8) 0x08u) ///< First byte of error code for all hard errors (cf. SOFTQM-612).
#endif

// Allgemeine Definitionen -------------------------------------------------

// externe Variablen -------------------------------------------------------

enum
{
    RTC_REGNUM_ERROR,  ///< RTC Registernummer des letzten Fehlers.
};
//------------------------------------------------------------------------------
// Prototypen interne Funktionen -----------------------------------------------
//------------------------------------------------------------------------------

#if FEATURE_ERROR_LOGGING
/// Function to write a (permanent) hard error to the error log according to SOFTSIL3-612, SOFTSIL3-613, and SOFTSIL3-615.<br>
/// A permanent hard error is only written to the log if it differs from the log's most recent entry.
/// \param hardErrorCode The hard error code.
/// \param isPermanent \c true in case \c hardErrorCode denotes a permanent hard error, \c false otherwise.
static void Safety_AppendHardErrorCode(U8 const hardErrorCode, bool const isPermanent);

/// Function to check if a permanent hard error occurred before system reset according to SOFTQM-618 and SOFTQM-652.<br>
/// The check is done based on the content of the error log: If a permanent hard error was recorded in the error log,
/// the system is driven into an endless loop (via Safety_PermanentHardError()) in order to prevent further undefined behaviour.
static void Safety_CheckPermanentHardError(void);

#endif

/// Die Funktion prüft, ob der Watchdog den System-Reset ausgelöst hat.
/// Ursachen dafür können ein vorheriger Hard-Error-Zustand mit Endlosschleife
/// sein oder ein fehlerhafter Programmablauf.
/// Falls der Watchdog den Reset ausgelöst hat, wird das Backup-Register
/// aus dem RTC gelesen und der entsprechende Hard-Error-Zustand wird eingenommen.
static void Safety_CheckWatchdogReset(void);


//------------------------------------------------------------------------------
// Funktionsbereich externe Funktionen -----------------------------------------
//------------------------------------------------------------------------------

/// \note Default Implementierung per Weak Linkage.
/// \author m.neubauer \date 20.06.2018
__attribute__((weak)) void Safety_HardError_Custom_Action(U8 const hardErrorCode)
    {
    }
//------------------------------------------------------------------------------

void Safety_PermanentHardError(U8 const hardErrorCode)
    {
    // order of steps according to SOFTQM-587

    // write to error log (SOFTSQM-612)
#if FEATURE_ERROR_LOGGING
    Safety_AppendHardErrorCode(hardErrorCode, true);
#endif


#if FEATURE_RTOS_AL_MPU_ENABLE
    if(RTOS_IsRunning())
        {
        MPU_Safety_HardError(RTC_REGNUM_ERROR, hardErrorCode);
        }
    else
        {
        // disable all interrupts (SOFTQM-587)
        System_InterruptDisable();
    #if FEATURE_SAFETYCHECK_WATCHDOG
        // store hard error in backup register of internal RTC (SOFTQM-655)
        Safety_SetNonvolatileError(hardErrorCode);
    #endif
        //custom action, application-dependent (SOFTQM-616)
        Safety_HardError_Custom_Action(hardErrorCode);
        }
#else
    // disable all interrupts (SOFTQM-587)
    System_InterruptDisable();
#if FEATURE_SAFETYCHECK_WATCHDOG
    // store hard error in backup register of internal RTC (SOFTQM-655)
    Safety_SetNonvolatileError(hardErrorCode);
#endif
    //custom action, application-dependent (SOFTQM-616)
    Safety_HardError_Custom_Action(hardErrorCode);
#endif

#if FEAT_DEBUG
    __asm("BKPT #0\n");
    // Break into the debugger
#endif

    // final step, enter endless loop (SOFTQM-619)
    while(1)
        {
#ifdef fpERROR_LED
        ErrorLed_Set(hardErrorCode);
#endif
        }

    }

void Safety_HardError(U8 const hardErrorCode)
    {
    // order of steps according to SOFTQM-587

    // write to error log (SOFTSQM-612)
#if FEATURE_ERROR_LOGGING
    Safety_AppendHardErrorCode(hardErrorCode, false);
#endif

#if FEATURE_RTOS_AL_MPU_ENABLE
    if(RTOS_IsRunning())
        {
        MPU_Safety_HardError(RTC_REGNUM_ERROR, hardErrorCode);
        }
    else
        {
        // disable all interrupts (SOFTQM-587)
        System_InterruptDisable();
    #if FEATURE_SAFETYCHECK_WATCHDOG
        // store hard error in backup register of internal RTC (SOFTQM-655)
        Safety_SetNonvolatileError(hardErrorCode);
    #endif
        //custom action, application-dependent (SOFTQM-616)
        Safety_HardError_Custom_Action(hardErrorCode);
        }
#else
    // disable all interrupts (SOFTQM-587)
    System_InterruptDisable();
#if FEATURE_SAFETYCHECK_WATCHDOG
    // store hard error in backup register of internal RTC (SOFTQM-655)
    Safety_SetNonvolatileError(hardErrorCode);
#endif
    //custom action, application-dependent (SOFTQM-616)
    Safety_HardError_Custom_Action(hardErrorCode);
#endif

#if FEAT_DEBUG
    __asm("BKPT #0\n");
    // Break into the debugger
#endif

    // final step, enter endless loop (SOFTQM-619)
    while(1)
        {
#ifdef fpERROR_LED
        ErrorLed_Set(hardErrorCode);
#endif
        }

    }

//------------------------------------------------------------------------------

/// \author m.neubauer \date 25.05.2016
EN61508_TestResult Safety_Startup_PowerOnSelfTests(void)
    {
    EN61508_TestResult testResult = EN61508_TestPass;
#if FEATURE_SAFETYCHECK_USE_STL
    // STL Scheduler initialisieren
    if(!Stl_SchedulerInit())
        {
        Safety_HardError(HARD_ERR_SAFETY_INIT);
        }
#endif

    // Startup-RAM-Test ausführen
#if FEATURE_SAFETYCHECK_STARTUP_RAM
    // RAM Test Ergebnis prüfen
    testResult = EN61508_TestFail;
#if FEATURE_SAFETYCHECK_USE_STL
    testResult = RAMTestStl_RunAll();
#else
    testResult = EN61508_RAMTest_Result();
#endif
    if(testResult != EN61508_TestPass)
        {
        Safety_HardError(HARD_ERR_MEM_RAM);
        }
#endif

    // Startup-ROM-Test ausführen
#if FEATURE_SAFETYCHECK_STARTUP_ROM
    // ROM Test über Programmspeicher durchführen
    testResult = EN61508_TestFail;
#if FEATURE_SAFETYCHECK_USE_STL
    testResult = ROMTestStl_RunAll();
#else
    testResult = EN61508_ROMTest_CRC32();
#endif
    if(testResult != EN61508_TestPass)
        {
        Safety_HardError(HARD_ERR_MEM_ROM);
        }
#endif

#if FEATURE_SAFETYCHECK_STARTUP_CCMRAM
    // RAM Test (CCM Speicherbereich)
    testResult = EN61508_TestFail;
    testResult = EN61508_RAMTest((U32)&__CCM_RAM_START, (U32)&__CCM_RAM_SIZE);
    if(testResult != EN61508_TestPass)
        {
        Safety_HardError(HARD_ERR_MEM_RAM);
        }
#endif


#if FEATURE_SAFETYCHECK_USE_STL
    // CPU-Tests ausführen
#if FEATURE_SAFETYCHECK_STARTUP_CPU
    testResult = EN61508_TestFail;
    testResult = CPUTestStl_RunAll();
    if(testResult != EN61508_TestPass)
        {
        Safety_HardError(HARD_ERR_CPU);
        }
#endif
#endif

#if  FEATURE_ERROR_LOGGING
    // drive application to endless loop in case of permanent hard error
    // SOFTQM-652 and SOFTQM-618
    Safety_CheckPermanentHardError();
#endif

    // Prüfen, ob Watchdog einen System-Reset ausgelöst hat
    // und damit der Hard-Errorzustand ausgelöst werden muss
    Safety_CheckWatchdogReset();

    return testResult;
    }

//------------------------------------------------------------------------------
// Funktionsbereich interne Funktionen -----------------------------------------
//------------------------------------------------------------------------------

/// \author k.ehlen \date 06.12.2023
void Safety_CheckWatchdogReset(void)
    {
#if FEATURE_SAFETYCHECK_WATCHDOG
    U32 ulResetErrorCode;
    ESYSTEM_RESET_SOURCE resetSource;
#endif

#if FEATURE_SAFETYCHECK_WATCHDOG
    // RTC Device für Zugriff auf nichtflüchtigen Speicher zwischenspeichern
    // Zugriff auf RTC und Hibernation Modul aktivieren
    RTCDrv_Enable();

    // Resetquelle abfragen
    resetSource = System_GetResetSource();

    if(resetSource & eSystem_ResetSource_WDT)
        {  // Reset wurde durch Watchdog ausgelöst
           // gespeicherten Harderror Code lesen
        ulResetErrorCode = Safety_GetNonvolatileError();

        if(ulResetErrorCode == HARD_ERR_NO_ERROR)
            {
            Safety_HardError(HARD_ERR_INTERN_WDT);
            }

        Safety_HardError(ulResetErrorCode);
        }
    else
        {  // Gespeicherten Harderror Code loeschen
        Safety_SetNonvolatileError(HARD_ERR_NO_ERROR);
        }
#endif
    }
//------------------------------------------------------------------------------


#if FEATURE_SAFETYCHECK_WATCHDOG
/// \author m.neubauer \date 07.08.2013
void Safety_SetNonvolatileError(U32 ulErrorCode)
    {
    RTCDrv_SetNonVolatileMemory(RTC_REGNUM_ERROR, ulErrorCode);
    }
//------------------------------------------------------------------------------

/// \author m.neubauer \date 07.08.2013
U32 Safety_GetNonvolatileError(void)
    {
    U32 ulErrorCode;

    if(RTCDrv_GetNonVolatileMemory(RTC_REGNUM_ERROR, &ulErrorCode) != TRUE)
        {
        ulErrorCode = HARD_ERR_INTERN_RTC;
        }

    return ulErrorCode;
    }
#endif
//------------------------------------------------------------------------------

#if FEATURE_ERROR_LOGGING

static void Safety_AppendHardErrorCode(U8 const hardErrorCode, bool const isPermanent)
    {
    U32 errorLog[LOGDATA_NUM_ERRORS];

    ERROR_CODE_FORMAT errorCode;

    errorCode.value = (U32) ERROR_CODE_ZERO_STATE;
    errorCode.upperLevel = HARD_ERROR_BYTE;
    errorCode.intermediateLevel = hardErrorCode;

    errorLog[eErrorType] = INTERNAL_STATUS;
    errorLog[eErrorCode] = errorCode.value;

    if (ErrorLog_CountStoredErrors() == (U32) 0u)
        {
        // error log is empty; store error
        if (isPermanent)
            {
            ErrorLog_AppendHardError(errorLog, sizeof(errorLog));
            }
        else
            {
            ErrorLog_Append(errorLog, sizeof(errorLog));
            }
        }
    else
        {
        U32 lastError[LOGDATA_NUM_ERRORS];
        // log not empty; check the newest entry from log
        if (ErrorLog_Read(lastError, 1, sizeof(lastError)) == true)
            {
            if ((lastError[eErrorType] != errorLog[eErrorType])
                || (lastError[eErrorCode] != errorLog[eErrorCode]))
                {
                // store the error; it is different to newest one
                if (isPermanent)
                    {
                    ErrorLog_AppendHardError(errorLog, sizeof(errorLog));
                    }
                else
                    {
                    ErrorLog_Append(errorLog, sizeof(errorLog));
                    }
                }
            }
        }
    }

static void Safety_CheckPermanentHardError(void)
    {
    // check error log for permanent hard error (SOFTQM-618)
    U32 lastError[LOGDATA_NUM_ERRORS];
    HARD_ERROR_READ_STATUS readStatus = ErrorLog_ReadHardError(lastError, sizeof(lastError));

    if (readStatus == HARD_ERROR_READ_FOUND)
        {
        U32 const ERROR_TYPE = lastError[eErrorType];
        ERROR_CODE_FORMAT const * const  ERROR_CODE = (ERROR_CODE_FORMAT const *) &lastError[eErrorCode];
        if ((ERROR_CODE->upperLevel == HARD_ERROR_BYTE)
                && (ERROR_TYPE == INTERNAL_STATUS))
            {
            // permanent hard error found; prevent startup, drive to endless loop (SOFTQM-652)
            U8 const HARD_ERROR_CODE = (U8) ERROR_CODE->intermediateLevel;
            Safety_PermanentHardError(HARD_ERROR_CODE);
            }
        }
    }

#endif

#endif
