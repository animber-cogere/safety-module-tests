/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/


// Headerdateien einbinden -----------------------------------------------------
#include "config/version.h"

#include "SafetyStl.h"
// Allgemeine Definitionen -----------------------------------------------------

/// Zustände des STL-Schedulers
typedef enum
{
    STL_SCHEDULER_IDLE,     ///< STL_SCHEDULER_IDLE, Ausgangszustand
    STL_SCHEDULER_STARTED,  ///< STL_SCHEDULER_STARTED, Gestartet, Testfunktionen können genutzt werden
} STL_SCHEDULER_STATE;

STL_SCHEDULER_STATE stlSchedulerState = STL_SCHEDULER_IDLE;

//------------------------------------------------------------------------------
// Prototypen interne Funktionen -----------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Funktionsbereich externe Funktionen -----------------------------------------
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
bool Stl_SchedulerInit(void)
    {
    bool result;
    result = false;

    stlSchedulerState = STL_SCHEDULER_IDLE;

    if(STL_SCH_Init() == STL_OK)
        {
        stlSchedulerState = STL_SCHEDULER_STARTED;
        result = true;
        }

    return result;
    }
//------------------------------------------------------------------------------

/// @author k.ehlen @date 08.01.2023
bool Stl_SchedulerIsStarted(void)
    {
    bool result;

    if(stlSchedulerState == STL_SCHEDULER_STARTED)
        {
        result = true;
        }
    else
        {
        result = false;
        }

    return result;
    }
//------------------------------------------------------------------------------
