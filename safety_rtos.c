/*******************************************************************************
 * Copyright (c) 2016-2021 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

// Gemeinsame Headerdateien einbinden --------------------------------------
#include "config/version.h"

/* RTOS Includes */
#include "RTOS_AL/RTOS_AL.h"

#include "error_def.h"
#include "EN61508_Program_Flow/EN61508.h"

#include "safety_startup.h"
#include "safety_runtime.h"

#include "safety_rtos.h"
#include "WATCHDOG/WATCHDOG_Driver.h"

#if FEAT_RTOS
// Allgemeine Definitionen -------------------------------------------------

#ifndef SAFETYTASK_STACKSIZE
#define SAFETYTASK_STACKSIZE  (RTOS_MINIMAL_STACKSIZE * 4)
#endif

// externe Variablen -------------------------------------------------------

static volatile U32 safetyTaskRestart;

// interne Variablen -------------------------------------------------------
RTOS_TASK_STRUCT(safetyTask, SAFETYTASK_STACKSIZE)

// Funktionsbereich --------------------------------------------------------
#if FEATURE_SAFETYCHECK_RUNTIME

/// @author m.neubauer @date 30.05.2016
void Safety_Task(TASK_PARA_STD * const tParam)
    {
    RTOS_TIME xLastWakeTime;

    safetyTaskRestart = TRUE;

#if FEATURE_SAFETYCHECK_WATCHDOG
    // Watchdog initialisieren
#ifdef WATCHDOG_WINDOW_PERCENT
    WATCHDOG_InitExtended(WDOG_TIMER_MS, WATCHDOG_WINDOW_PERCENT);
    // Mit der Initialisierung startet der Watchdog direkt, Zeitpunkt des Startens setzen
    Safety_InitWatchdogTime(RTOS_GetTime());

#else
    WATCHDOG_Init(WDOG_TIMER_MS);
#endif // WATCHDOG_WINDOW_PERCENT
#endif // FEATURE_SAFETYCHECK_WATCHDOG

    while(TRUE)
        {
        if(safetyTaskRestart)
            {
            xLastWakeTime = RTOS_GetTime();
            safetyTaskRestart = FALSE;
            }

        Safety_Runtime_Execute();

#if FEAT_DEBUG
#ifdef fpSafetyTask
        PORT_WRITE(fpSafetyTask, GPIO_LOW);
#endif
#endif
        // Ende der Sicherheitstaskueberwachung
        RTOS_DelayUntil(&xLastWakeTime, tParam->taskDelay);

#if FEAT_DEBUG
#ifdef fpSafetyTask
        PORT_WRITE(fpSafetyTask, GPIO_HIGH);
#endif
#endif

        }
    }
//------------------------------------------------------------------------------
#endif

/// @author m.neubauer @date 30.05.2016
U32 Safety_Task_Init(TASK_PARA_STD * const tParam)
    {
#if FEATURE_SAFETYCHECK_RUNTIME
    if(!Safety_Runtime_Startup(tParam))
        {
        return FALSE;
        }
#endif

    if(!RTOS_TaskCreate(&safetyTask, "Safety", (RTOS_TASK_FUNCTION)Safety_Task, tParam->ucPrioritaet, tParam))
        {
        return FALSE;
        }

    return TRUE;
    }
//------------------------------------------------------------------------------

/// @author m.neubauer @date 15.04.2015
void Safety_Restart(void)
    {
    safetyTaskRestart = TRUE;
    }
//------------------------------------------------------------------------------

#endif
