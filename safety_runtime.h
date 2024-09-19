/*******************************************************************************
 * Copyright (c) 2021 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
 * \defgroup safety_runtime Laufzeittests
 * \ingroup safety_utils
 * @{
 */
#ifndef GLOBAL_SAFETY_SAFETY_RUNTIME_H_
#define GLOBAL_SAFETY_SAFETY_RUNTIME_H_

// Gemeinsame Headerdateien einbinden ---------------------------------------

// Spezielle Headerdateien einbinden ----------------------------------------
#include "EN61508_Program_Flow/EN61508.h"
#include "safety_powersupply.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Compiler Direktiven ------------------------------------------------------

// Test suite activation or deactivation by the user (SOFTQM-425)
// Only activated testsuites are executed (SOFTQM-873)

#ifndef FEATURE_SAFETYCHECK_RUNTIME
/// \ingroup feature_flags
/// Feature Flag zum aktivieren der Sicherheitsfunktionen zur Laufzeit,
/// wie z.B. RAM und ROM Tests.
/// Diese Tests sind per Default deaktiviert.
#define FEATURE_SAFETYCHECK_RUNTIME (0)
#pragma message "Konfiguration in version_def.h erforderlich."
#endif

#ifndef FEATURE_SAFETYCHECK_WATCHDOG
/// \ingroup feature_flags
/// Feature Flag zum aktivieren der RTRC/Watchdog Tests.
/// Diese Tests sind per Default deaktiviert.
#define FEATURE_SAFETYCHECK_WATCHDOG (0)
#pragma message "Konfiguration in version_def.h erforderlich."
#endif

#ifndef FEATURE_SAFETYCHECK_RUNTIME_RAM
/// \ingroup feature_flags
/// Feature Flag zum aktivieren des zyklischen RAM Tests.
/// Dieser Test ist per Default deaktiviert.
#define FEATURE_SAFETYCHECK_RUNTIME_RAM (0)
#pragma message "Konfiguration in version_def.h erforderlich."
#endif

#ifndef FEATURE_SAFETYCHECK_RUNTIME_RAM_STORE_STATE
#define FEATURE_SAFETYCHECK_RUNTIME_RAM_STORE_STATE (0) ///< Legt fest ob der Zustand des zyklischen RAM-Test im EEPROM gesichert wird.
#else
#ifndef TICKS_TO_SAFE_RAMTEST_STATE
/// Zeit in Systicks, nach welcher der zyklische RAM-Test seinen Zustand im
/// EEPROM sichert. Default: 30 Minuten.
#define TICKS_TO_SAFE_RAMTEST_STATE (configTICK_RATE_HZ_MS * 1000 * 60 * 30)
#endif
#endif // FEATURE_SAFETYCHECK_RUNTIME_RAM_STORE_STATE

#ifndef FEATURE_SAFETYCHECK_RUNTIME_ROM
/// \ingroup feature_flags
/// Feature Flag zum aktivieren des zyklischen ROM Tests.
/// Dieser Test ist per Default deaktiviert.
#define FEATURE_SAFETYCHECK_RUNTIME_ROM (0)
#pragma message "Konfiguration in version_def.h erforderlich."
#endif

#ifndef FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW
/// \ingroup feature_flags
/// Feature Flag zum aktivieren des Programmablaufkontrolle.
/// Dieser Test ist per Default deaktiviert.
#define FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW (0)
#pragma message "Konfiguration in version_def.h erforderlich."
#endif

// User configuration for programflow monitoring (SOFTQM-443)
// Check user configuration for programflow monitoring
#if FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW
// Currently only the usage of M41T6X is supported by the module (constraint SOFTQM-730)
#if !defined(FEATURE_SAFETY_RUNTIME_PROGFLOW_USE_RTC_M41T62)
#error "Please define RTC for programflow monitoring"
#endif
#endif /* FEATURE_SAFETYCHECK_RUNTIME_PROGFLOW */

#if FEATURE_SAFETYCHECK_USE_STL
#ifndef FEATURE_SAFETYCHECK_RUNTIME_CPU
/// \ingroup feature_flags
/// Feature Flag zum Aktivieren der zyklischen CPU-Tests.
/// Dieser Test ist per Default deaktiviert.
#define FEATURE_SAFETYCHECK_RUNTIME_CPU  (0)
#pragma message "Konfiguration in version_def.h erforderlich."
#endif
#endif

#ifndef FEATURE_SAFETYCHECK_RUNTIME_REGISTER
/// \ingroup feature_flags
/// Feature flag activating the cyclc testing of hardware configuration registers.
/// This test is deactivated by default.
#define FEATURE_SAFETYCHECK_RUNTIME_REGISTER    (0)
#pragma message "Konfiguration in version_def.h erforderlich."

#else

#ifndef HARD_ERR_INTERN_CHECK_REGISTER_CYCLIC
#define HARD_ERR_INTERN_CHECK_REGISTER_CYCLIC   0x52 // SOFTQM-648
#endif

#endif // FEATURE_SAFETYCHECK_RUNTIME_REGISTER

#ifndef FEATURE_SAFETYCHECK_RUNTIME_POWERSUPPLY
/// \ingroup feature_flags
/// Feature Flag for activating powersupply checks.
/// This test is deactivated by default.
#define FEATURE_SAFETYCHECK_RUNTIME_POWERSUPPLY (0)
#pragma message "Konfiguration in version_def.h erforderlich."
#endif

#if FEATURE_SAFETYCHECK_RUNTIME

#include "RTC/RTC_Driver.h"

// Makros -------------------------------------------------------------------

/// User configuration of process safety time in ms
#ifndef PROCESS_SAFETY_TIME_MS
#pragma message "Please define Process Safety Time (PST)."
#define PROCESS_SAFETY_TIME_MS  (4000u)
#endif

#if FEAT_RTOS
/// Process safety time in ticks
#define PROCESS_SAFETY_TIME_TICKS (PROCESS_SAFETY_TIME_MS * configTICK_RATE_HZ_MS)
#endif

// externe Variablen --------------------------------------------------------

// Allgemeine Definitionen --------------------------------------------------
/// Watchdogzeit in ms.
#ifndef WDOG_TIMER_MS
#define WDOG_TIMER_MS               (500)
#endif
// Prototypen ---------------------------------------------------------------
/// Initialisierung der Sicherheitsfunktionen, die zur Laufzeit ausgeführt werden.
/// Bei Verwendung eines Watchdogs, wird dieser in der Funktion direkt mit der Initialisierung gestartet.
/// \param safetyPowerSupplyConfig Aktivierung der Messkanäle. Die entsprechenden Pindefinitionen
/// müssen bei Aktivierung vorhanden sein.
extern void Safety_Runtime_Init(SAFETY_POWERSUPPLY_CONFIG * const safetyPowerSupplyConfig);

/// Konfiguration von Sicherheitsfunktionen, für die die Taskparameter notwendig sind,
/// zum Beispiel Programmlaufüberwachung.
/// \param param Taskparameter
/// \return true bei Erfolg, sonst false.
extern bool Safety_Runtime_Startup(TASK_PARA_STD * const param);

/// Interrupt Service Routine fuer den RTC-Interrupt
/// Hier wird das Sekundenflag fuer die Taskueberwachung gesetzt.
extern void Safety_SecondCallback (U32 count);

/// Interruptfunktion des Watchdog.
/// Wird aufgerufen, wenn die Watchdogzeit abgelaufen ist.
extern void Safety_WDTCallback(void);

/// Aktualisierungsfunktion fuer die zyklischen Laufzeittests.
/// Diese Funktion muss zyklisch aufgerufen werden.
extern void Safety_Runtime_Execute(void);

/// Benutzerspezifische Funktion zum durchführen von zyklischen Laufzeittests.
/// Die Funktion wird in der zyklischen Task aufgerufen. Es ist darauf zu achten,
/// das nur kurze Aktionen durchgeführt werden. Ansonsten kann es dazu kommen, das
/// die Programmablaufüberwachung einen Fehler meldet.
extern void Safety_Runtime_Custom_CyclicCheck(void);

#if FEATURE_SAFETYCHECK_RUNTIME_REGISTER
/// Test if configuration registers of hardware drivers are equal to their redundant twins.
/// \note This function shall be implemented by the user application,
///       to include the register tests of all hardware drivers used by that application (SOFTQM-750).
/// \return true on success, or false if any redundancy error is detected.
extern bool Safety_Runtime_RegisterTest(void);
#endif // FEATURE_SAFETYCHECK_RUNTIME_REGISTER

#ifdef WATCHDOG_WINDOW_PERCENT
/// Initialize the time tracing of the window watchdog.
/// \param currentTicks Current time in system ticks.
extern void Safety_InitWatchdogTime(U32 const currentTicks);
#endif

#if FEATURE_RTOS_AL_MPU_ENABLE
extern void Safety_Runtime_CheckMPUFault(void);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_SAFETY_SAFETY_RUNTIME_H_ */
/**
 * @}
 */
