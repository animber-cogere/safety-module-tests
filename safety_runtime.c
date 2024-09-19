/*******************************************************************************
 * Copyright (c) 2021 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

// Gemeinsame Headerdateien einbinden --------------------------------------
#include "config/version.h"

#include "SYSTEM/System_Driver.h"
#include "RTC/RTC_Driver.h"
#include "WATCHDOG/WATCHDOG_Driver.h"

#include "error_def.h"
#include "EN61508_Program_Flow/EN61508.h"

#include "safety_powersupply.h"
#include "safety_startup.h"

#ifdef DREHGEBER
#include "Abtastung/abtastung.h"
#endif

#include "EventSystem/Event.h"
#include "ErrorHandler/error.h"

#if FEATURE_SAFETYCHECK_USE_STL
#include "STM32_Safety_STL_API/SafetyStl.h"
#include "STM32_Safety_STL_API/ROMTestStl.h"
#include "STM32_Safety_STL_API/CPUTestStl.h"
#include "STM32_Safety_STL_API/RAMTestStl.h"
#endif

#if FEATURE_RTOS_AL_MPU_ENABLE
#include "RTOS_AL.h"
#endif

#include "safety_runtime.h"

#if FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW
#ifdef FEATURE_SAFETY_RUNTIME_PROGFLOW_USE_RTC_M41T62
#include "Devices_RTC_M41T6X/M41T62.h"
#endif
#endif

#if FEATURE_SAFETYCHECK_RUNTIME_RAM_STORE_STATE
#include "parameter_tab_00/parameter_tab.h"
#include "eeprom/eeprom.h"
#include "FreeRTOS_TWK.h"
#endif

#if FEATURE_SAFETYCHECK_RUNTIME
// Compiler Direktiven -----------------------------------------------------

// Makros ------------------------------------------------------------------

// Allgemeine Definitionen -------------------------------------------------

#ifdef WATCHDOG_WINDOW_PERCENT
/// Function to trigger window watchdog. Module does store last trigger of the watchdog by itself.
static void Trigger_Window_Watchdog(void);
#endif

#if FEATURE_SAFETYCHECK_WATCHDOG


/// Grenzwert für Window Watchdog.
/// Dieser wird nach (Timeout - Fenstergröße) SysTicks getriggert.
#ifdef WATCHDOG_WINDOW_PERCENT
#define HUNDRED_PERCENT                 (100u)
/// Time at which the watchdog is triggered after the watchdog is reloaded.
/// The trigger time is set in the center of the window
#define WATCHDOG_TRIGGER_TIME_TICKS     ((U32)(WDOG_TIMER_MS * configTICK_RATE_HZ_MS * (HUNDRED_PERCENT - (0.5f * WATCHDOG_WINDOW_PERCENT)) / HUNDRED_PERCENT))
/// time of last trigger in ticks
static U32 lastWdgTrigger = 0;
#endif

#endif

// externe Variablen -------------------------------------------------------
#if FEATURE_RTOS_AL_MPU_ENABLE
static bool safetyMPUFault = false;
#endif

#if FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW

#ifdef FEATURE_SAFETY_RUNTIME_PROGFLOW_USE_RTC_M41T62
static T_M41T62 m41t62Inst;
#endif

// Flag fuer die Kommunikation zwischen RTC-ISR und Sicherheitstask
// Wird fuer die Taskueberwachung benoetigt.
static volatile U32 gulRTCSekundeAbgelaufen;

static EN61508_PROGRAMMFLOW tgSafetyProgFlow;

// Mutex for programflow counter incrementation
static RTOS_MUTEX en61508SafetyTaskMutex;
#endif

// Funktionsbereich --------------------------------------------------------
#if FEATURE_SAFETYCHECK_RUNTIME_RAM_STORE_STATE
/// Sichert den Zustand des zyklischen RAM Tests in der Parameter Tabelle
/// und nach  Ticks im EEPROM.
/// \param currentTicks  Aktuelle Systemzeit.
/// \return  true, wenn die der Zustand erfolgreich in die Parameter Tabelle
///          gespeichert wurde, sonst false.
static bool Safety_Runtime_SafeRamTestState(U32 const currentTicks)
    {
    static U32 latestSave = 0;
    U8 * testCellPrimary;
    U8 * testCellSecundary;
    S32 ramIndex;
    ePARTAB_ERR parTabErr = PARTAB_ERR_NONE;

    // Speichere den Zustand des zyklischen RAM Tests in der Parameter Tabelle
    EN61508_RAMTest_GetCycleOffset(&testCellPrimary, &testCellSecundary, &ramIndex);
    parTabErr |= ParTab_SetValue(PARNUM_RAMTEST_DATA1, (UU32*) &testCellPrimary);
    parTabErr |= ParTab_SetValue(PARNUM_RAMTEST_DATA2, (UU32*) &testCellSecundary);
    parTabErr |= ParTab_SetValue(PARNUM_RAMTEST_INDEX, (UU32*) &ramIndex);

    if((PARTAB_ERR_NONE == parTabErr)
            && ((currentTicks - latestSave) > TICKS_TO_SAFE_RAMTEST_STATE))
        {
        // Sichere die Parameter Tabelle im EEPROM
        EEPROM_MESSAGE message = {EEPROM_COMMAND_PARTAB_SAVE, {PARTAB_GROUP_INTERN}};
        latestSave = currentTicks;
        return Eeprom_Send_Command(&message);
        }

    return false;
    }
#endif // FEATURE_SAFETYCHECK_RUNTIME_RAM_STORE_STATE

#ifdef WATCHDOG_WINDOW_PERCENT
static void Trigger_Window_Watchdog(void)
    {
    bool isTimeToTrigger = false;
    U32 const currentTicks = RTOS_GetTime();

    // Überprüfen, ob die Zeit bis zum nächsten Trigger abgelaufen ist
    if(currentTicks < lastWdgTrigger) // Überlauf bei currentTicks abfangen
        {
        isTimeToTrigger = (RTOS_MAX_TIMEOUT - lastWdgTrigger + currentTicks) > WATCHDOG_TRIGGER_TIME_TICKS;
        }
    else
        {
        isTimeToTrigger = (currentTicks - lastWdgTrigger) > WATCHDOG_TRIGGER_TIME_TICKS;
        }

    if(isTimeToTrigger)
        {
        lastWdgTrigger = currentTicks;
        WATCHDOG_Trigger();
        }
    }

void Safety_InitWatchdogTime(U32 const currentTicks)
    {
    lastWdgTrigger = currentTicks;
    }
#endif // WATCHDOG_WINDOW_PERCENT

/// \author m.neubauer \date 28.06.2016
/// \note Default Implementierung per Weak Linkage.
__attribute__((weak)) void Safety_Runtime_Custom_CyclicCheck(void)
    {
#ifdef DREHGEBER
    #if FEATURE_SAFETYCHECK_RUNTIME
        ERROR_CODE_FORMAT crcError;
        crcError.value = ERROR_CODE_ZERO_STATE;
        if(Korrektur_CRCIsValidCyclic() != EN61508_TestPass)
            {
            crcError.upperLevel = eFEHLER_KEIN_KANAL;
            crcError.intermediateLevel = eFEHLER_CALIBRATION;
            crcError.lowerLevel = eCALIBRATION_ERROR_CRC;
            SendMsgEvent(eEVENT_ERROR_SENSOR, crcError.value);
            }
    #endif
#endif
    }
//------------------------------------------------------------------------------

/// \author m.neubauer \date 26.05.2016
void Safety_Runtime_Init(SAFETY_POWERSUPPLY_CONFIG * const safetyPowerSupplyConfig)
    {
    bool initSucessful;

    initSucessful = true;


#if FEATURE_SAFETYCHECK_USE_STL
    if((initSucessful) && !Stl_SchedulerInit())
        {
        initSucessful = false;
        }
#endif

#if FEATURE_SAFETYCHECK_RUNTIME_ROM
#if FEATURE_SAFETYCHECK_USE_STL
    // Zyklischen ROM Test initialisieren
    if((initSucessful) && !ROMTestStl_SetupTestCyclic((U32) PROCESS_SAFETY_TIME_TICKS))
        {
        initSucessful = false;
        }
#endif
#endif

#if FEATURE_SAFETYCHECK_RUNTIME_RAM
    // Zyklischen RAM Test initialisieren
#if FEATURE_SAFETYCHECK_USE_STL
    // Zyklischen RAM Test initialisieren
    if((initSucessful) && !RAMTestStl_SetupTestCyclic((U32) PROCESS_SAFETY_TIME_TICKS))
        {
        initSucessful = false;
        }
#else

    if((initSucessful) && (EN61508_RAMTest_Cyclic_Init() != EN61508_True))
        {
        initSucessful = false;
        }
#if FEATURE_SAFETYCHECK_RUNTIME_RAM_STORE_STATE
    else
        {
        U8 * testCellPrimary;
        U8 * testCellSecundary;
        S32 ramIndex;
        ePARTAB_ERR parTabErr = PARTAB_ERR_NONE;

        parTabErr |= ParTab_GetValue(PARNUM_RAMTEST_DATA1, testCellPrimary);
        parTabErr |= ParTab_GetValue(PARNUM_RAMTEST_DATA2, testCellSecundary);
        parTabErr |= ParTab_GetValue(PARNUM_RAMTEST_INDEX, ramIndex);

        if(PARTAB_ERR_NONE == parTabErr)
            {
            if(EN61508_False == EN61508_RAMTest_SetCycleOffset(testCellPrimary, testCellSecundary, ramIndex))
                {
                initSucessful = false;
                }
            }
        else
            {
            initSucessful = false;
            }
        }
#endif // FEATURE_SAFETYCHECK_RUNTIME_RAM_STORE_STATE
#endif // FEATURE_SAFETYCHECK_USE_STL
#endif // FEATURE_SAFETYCHECK_RUNTIME_RAM

#if FEATURE_SAFETYCHECK_USE_STL
#if FEATURE_SAFETYCHECK_RUNTIME_CPU
    if((initSucessful) && !CPUTestStl_SetupTestCyclic((U32) PROCESS_SAFETY_TIME_TICKS))
        {
        initSucessful = false;
        }
#endif
#endif

#if FEAT_DEBUG
#ifdef fpSafetyTask
    if((initSucessful) && !GPIOInitPin(fpSafetyTask, eOut_PP)) // Lebenspin der Positionstask
        {
        initSucessful = false;
        }
#endif
#ifdef fpSafetyRTC
    if((initSucessful) && !GPIOInitPin(fpSafetyRTC, eOut_PP)) // Lebenspin der Positionstask
        {
        initSucessful = false;
        }
#endif
#endif

#if FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW

// Initialize selected RTC for programflow monitoring (SOFTQM-408)
#if defined(FEATURE_SAFETY_RUNTIME_PROGFLOW_USE_RTC_M41T62)
    if((initSucessful) && !M41T62_Init(&m41t62Inst, RTC_CHANNEL, RTC_I2C_ADDRESS, Safety_SecondCallback))
        {
        initSucessful = false;
        }
#else
    // No RTC selected
    initSucessful = false;
#endif

    gulRTCSekundeAbgelaufen = FALSE;
#endif /* FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW */

#if FEATURE_SAFETYCHECK_RUNTIME_POWERSUPPLY
    if((initSucessful) && !Safety_Powersuply_Init(safetyPowerSupplyConfig))
        {
        initSucessful = false;
        }
#endif

    // Initialization failure leads to a non-permanent hard-error (SOFTQM-631)
    if(!initSucessful)
        {
        Safety_HardError(HARD_ERR_SAFETY_INIT);
        }
    }
//------------------------------------------------------------------------------

/// \author k.ehlen \date 24.05.2023
bool Safety_Runtime_Startup(TASK_PARA_STD * const param)
    {
    bool result;

#if FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW
    U32 referenceCount;
#endif

    result = true;

#if FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW
    referenceCount = 0;

    if(param->taskDelay > 0)
        {
        referenceCount = (RTOS_TICK_RATE + (param->taskDelay / 2)) / param->taskDelay;
        }
    else
        {
        result = false;
        }

    if((result) && !RTOS_MutexCreate(&en61508SafetyTaskMutex))
        {
        result = false;
        }

    if((result) && (!EN61508_ProgFlow_Init(&tgSafetyProgFlow, referenceCount, 3, &en61508SafetyTaskMutex)))
        {
        result = false;
        }

    if((result) && (!EN61508_ProgFlow_Add(&tgSafetyProgFlow)))
        {
        result = false;
        }
#endif

    return result;
    }

//------------------------------------------------------------------------------
void Safety_SecondCallback(U32 count)
    {
#if FEAT_DEBUG
#ifdef fpSafetyRTC
    PORT_WRITE(fpSafetyRTC, GPIO_HIGH);
#endif
#endif

#if FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW
    // Hier wird nur das Flag gesetzt, die Auswertung des
    // Sicherheitstaskaufrufzaehlers erfolgt direkt in der Sicherheitstask!
    gulRTCSekundeAbgelaufen = TRUE;
#endif

#if FEAT_DEBUG
#ifdef fpSafetyRTC
    PORT_WRITE(fpSafetyRTC, GPIO_LOW);
#endif
#endif
    }
//------------------------------------------------------------------------------

void Safety_WDTCallback(void)
    {
    Safety_HardError(HARD_ERR_INTERN_WDT);
    }
//------------------------------------------------------------------------------

/// \author m.neubauer \date 26.05.2016
void Safety_Runtime_Execute(void)
    {

#if FEATURE_SAFETYCHECK_RUNTIME_RAM
#if FEATURE_SAFETYCHECK_USE_STL
    EN61508_TestResult ramResultStl;
#endif
#endif

#if FEATURE_SAFETYCHECK_WATCHDOG
    // Watchdog Triggern
#ifdef WATCHDOG_WINDOW_PERCENT
    Trigger_Window_Watchdog();
#else
    WATCHDOG_Trigger();
#endif
#endif /* FEATURE_SAFETYCHECK_WATCHDOG */

#if FEATURE_RTOS_AL_MPU_ENABLE
    Safety_Runtime_CheckMPUFault();
#endif

#if FEATURE_SAFETYCHECK_RUNTIME_RAM
#if FEATURE_SAFETYCHECK_USE_STL
    ramResultStl = EN61508_TestFail;
#endif

#if !EN61508_RAMTEST_USE_TIMER && EN61508_RAMTEST_FROM_SAFETY_TASK
    // RAM Test wird nicht durch Timer durchgeführt, RAM Test Funktion
    // direkt aufrufen
#if FEATURE_SAFETYCHECK_USE_STL
    ramResultStl = RAMTestStl_RunCyclic(RTOS_GetTime());
#else
    EN61508_RAMTest_Cyclic();
#endif

#endif

// Zyklischen RAM Test auswerten
#if FEATURE_SAFETYCHECK_USE_STL
    if(ramResultStl != EN61508_TestPass)
        {
        Safety_HardError(HARD_ERR_MEM_RAM_CYCLIC);
        }
#else
    if(EN61508_RAMTest_Cyclic_Result() != EN61508_TestPass)
        {
        Safety_HardError(HARD_ERR_MEM_RAM_CYCLIC);
        }
#endif

#if FEATURE_SAFETYCHECK_RUNTIME_RAM_STORE_STATE
    Safety_Runtime_SafeRamTestState(currentTicks);
#endif

#endif // FEATURE_SAFETYCHECK_RUNTIME_RAM

#if FEATURE_SAFETYCHECK_RUNTIME_ROM

#if FEATURE_SAFETYCHECK_USE_STL
    if(ROMTestStl_RunCyclic(RTOS_GetTime()) != EN61508_TestPass)
        {
        Safety_HardError(HARD_ERR_MEM_ROM_CYCLIC);
        }
#else
    // Zyklischen ROM Test Programmspeicherbereich
    if(EN61508_ROMTest_CRC32_Cyclic() != EN61508_TestPass)
        {
        Safety_HardError(HARD_ERR_MEM_ROM_CYCLIC);
        }
#endif /* FEATURE_SAFETYCHECK_USE_STL */

#endif /* FEATURE_SAFETYCHECK_RUNTIME_ROM */

#if FEATURE_SAFETYCHECK_USE_STL
#if FEATURE_SAFETYCHECK_RUNTIME_CPU
    if(CPUTestStl_RunCyclic(RTOS_GetTime()) != EN61508_TestPass)
        {
        Safety_HardError(HARD_ERR_CPU_CYCLIC);
        }
#endif
#endif

    // Benutzerspezifische Laufzeitprüfungen durchführen
    Safety_Runtime_Custom_CyclicCheck();

#if FEATURE_SAFETYCHECK_RUNTIME_POWERSUPPLY
    // Versorgsspannung/Stromaufnahme pruefen
    Safety_Powersupply_Check();
#endif

#if FEATURE_SAFETYCHECK_RUNTIME_REGISTER
    if(!Safety_Runtime_RegisterTest()) // SOFTQM-609
        {
        Safety_HardError(HARD_ERR_INTERN_CHECK_REGISTER_CYCLIC); // SOFTQM-648
        }
#endif // FEATURE_SAFETYCHECK_RUNTIME_REGISTER

#if FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW
    // Sekunde abgelaufen. Pruefen, ob die Aufrufzaehler der zu pruefenden Tasks
    // den erwarteten Zaehlerstand haben.
    if(gulRTCSekundeAbgelaufen == TRUE)
        {
        // Flag des RTC wieder loeschen
        gulRTCSekundeAbgelaufen = FALSE;

        // Zykluszähler aller relevanten Tasks prüfen
        if(!EN61508_ProgFlow_CheckCycleCounterAll())
            {
            Safety_HardError(HARD_ERR_INTERN_SAFETY_CYCLIC);
            }
        }

    EN61508_ProgFlow_IncCycleCounter(&tgSafetyProgFlow);
#endif

    }
//------------------------------------------------------------------------------

#if FEATURE_RTOS_AL_MPU_ENABLE
/// @author M.Neubauer @date 22.02.2024
void RTOS_MPU_ErrorCallback(RTOS_TASK * const task, U32 const errorCode)
    {
    safetyMPUFault = true;
    }
//------------------------------------------------------------------------------

/// @author M.Neubauer @date 22.02.2024
void Safety_Runtime_CheckMPUFault(void)
    {
    if(safetyMPUFault != false)
        {
        Safety_HardError(HARD_ERR_INTERN_MPU_CYCLIC_TEST);
        }
    }
//------------------------------------------------------------------------------

#endif

#endif
