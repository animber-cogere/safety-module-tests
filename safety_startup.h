/*******************************************************************************
 * Copyright (c) 2021 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
 * \defgroup safety_startup Statische Tests
 * \ingroup safety_utils
 * @{
 */
#ifndef GLOBAL_SAFETY_SAFETY_STARTUP_H_
#define GLOBAL_SAFETY_SAFETY_STARTUP_H_

// Gemeinsame Headerdateien einbinden ---------------------------------------

// Spezielle Headerdateien einbinden ----------------------------------------
#include "EN61508_Program_Flow/EN61508.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Compiler Direktiven ------------------------------------------------------
#ifndef FEATURE_SAFETYCHECK_STARTUP
    /// \ingroup feature_flags
    /// Feature Flag zum aktivieren der Sicherheitsfunktionen im Startup,
    /// wie z.B. RAM und ROM Tests.
    /// Diese Tests sind per Default aktiv.
    #define FEATURE_SAFETYCHECK_STARTUP (1)
#pragma message "Konfiguration in version_def.h erforderlich."
#endif

#ifndef FEATURE_SAFETYCHECK_STARTUP_RAM
    /// \ingroup feature_flags
    /// Feature Flag zum aktivieren des RAM Tests im Startup.
    /// Dieser Test ist per Default aktiviert.
    #define FEATURE_SAFETYCHECK_STARTUP_RAM (1)
    #pragma message "Konfiguration in version_def.h erforderlich."
#endif

#ifndef FEATURE_SAFETYCHECK_STARTUP_ROM
    /// \ingroup feature_flags
    /// Feature Flag zum aktivieren des ROM Tests im Startup.
    /// Dieser Test ist per Default aktiviert.
    #define FEATURE_SAFETYCHECK_STARTUP_ROM (1)
    #pragma message "Konfiguration in version_def.h erforderlich."
#endif

#ifndef FEATURE_SAFETYCHECK_STARTUP_CCMRAM
/// \ingroup feature_flags
/// Feature Flag zum aktivieren des RAM Test für den CCM RAM Bereich.
/// Dieser Test ist per Default nicht aktiv.
#define FEATURE_SAFETYCHECK_STARTUP_CCMRAM (0)
#endif

#if FEATURE_SAFETYCHECK_USE_STL
#ifndef FEATURE_SAFETYCHECK_STARTUP_CPU
/// \ingroup feature_flags
/// Feature Flag zum Aktivieren der CPU-Tests.
/// Dieser Test ist per Default deaktiviert.
#define FEATURE_SAFETYCHECK_STARTUP_CPU  (0)
#pragma message "Konfiguration in version_def.h erforderlich."
#endif
#endif

#if FEAT_RTOS
#include "RTOS_AL/RTOS_AL.h"
#endif

#ifdef fpAlarm
    #pragma message "Veraltete Alarmausgangspinbezeichnung 'fpAlarm', neue Portpinbezeichnung 'fpAlarmOut'"
#endif

// Makros -------------------------------------------------------------------

// externe Variablen --------------------------------------------------------

// Allgemeine Definitionen --------------------------------------------------

// Prototypen ---------------------------------------------------------------
/// Initialisierung und Ausführung von Power-On-Self-Tests.
/// Hierzu gehören RAM-Test, ROM-Test und CPU-Test sowie die
/// Überprüfung eines Watchdog-Resets.
/// \return EN61508_TestPass, wenn der RAM/ROM Test erfolgreich war, sonst EN61508_TestFail.
extern EN61508_TestResult Safety_Startup_PowerOnSelfTests(void);

/// Hard error function to be called in case of severe (i.e., hard) <b>permanent</b> errors.<br>
/// During hard error state the operating system is not active;
/// instead the system is driven into an endless loop in order to prevent further undefined behaviour.
/// Each hard error that is triggered corresponds to a specific hard error code of 1 byte.
/// During hard error the following steps are executed to get a defined error behaviour:
/// - The hard error code of 1 byte is logged into the error log (cf. SOFTQM-613, SOFTQM-615).
/// - Interrupts are disabled, which stops the operating system (cf. SOFTQM-614).
/// - The hard error code is stored in the backup register of the internal Real Time Clock (RTC) (cf. SOFTQM-655).
/// - A custom hard error action is executed (cf. SOFTQM-616), which can be defined by overriding Safety_HardError_Custom_Action().
/// - An endless loop is entered (cf. SOFTQM-619).
/// \param hardErrorCode The hard error code.<br>
/// <b>NOTE:</b> This function does NOT return.
extern void Safety_PermanentHardError(U8 const hardErrorCode) __attribute__ ((noreturn));

/// Hard error function to be called in case of severe (i.e., hard) <b>NON-</b>permanent errors.<br>
/// During hard error state the operating system is not active;
/// instead the system is driven into an endless loop in order to prevent further undefined behaviour.
/// Each hard error that is triggered corresponds to a specific hard error code of 1 byte.
/// During hard error the following steps are executed to get a defined error behaviour:
/// - The hard error code of 1 byte is logged into the error log (cf. SOFTQM-613).
/// - Interrupts are disabled, which stops the operating system (cf. SOFTQM-614).
/// - The hard error code is stored in the backup register of the internal Real Time Clock (RTC) (cf. SOFTQM-655).
/// - A custom hard error action is executed (cf. SOFTQM-616), which can be defined by overriding Safety_HardError_Custom_Action().
/// - An endless loop is entered (cf. SOFTQM-619).
/// \param hardErrorCode The hard error code.
/// <b>NOTE:</b> This function does NOT return.
extern void Safety_HardError(U8 const hardErrorCode) __attribute__ ((noreturn));

/// Function to execute application/user specific actions in case of hard errors according to SOFTQM-616.
/// This functions serves as extension of the regular hard error functions Safety_HardError() and Safety_PermanentHardError(),
/// and is called from inside these functions.
/// \param hardErrorCode Hard-error-code to identify the occured error.
extern void Safety_HardError_Custom_Action(U8 const hardErrorCode);

/// Speichert den Harderror Code im Hibernationmodul des uC ab. Der Wert bleibt
/// bei einem normalen Systemreset erhalten. Erst durch trennen der Versorgungspannung
/// wird der Wert geloescht.
/// \param ulErrorCode Zu speichernder Harderror Code.
extern void Safety_SetNonvolatileError(U32 ulErrorCode);

/// Lesen des Harderror Code aus dem Hibernationmodul.
/// \return Gespeicherter Harderror Code.
extern U32 Safety_GetNonvolatileError(void);


#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_SAFETY_SAFETY_STARTUP_H_ */
/**
 * @}
 */
