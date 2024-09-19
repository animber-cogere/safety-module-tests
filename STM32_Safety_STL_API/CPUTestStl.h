/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
 * \defgroup CPUTestStl CPU-Test Funktionen
 *
 * Funktionen zur Ausführung der CPU-Tests der Self-Test-Library von ST.
 * Zugehöriges Handbuch: UM2590
 *
 * (#) Kompletter CPU-Test:
 *     Führt hintereinander alle CPU-Tests aus.
 *
 *        (++) CPUTestStl_RunAll()
 *
 * (#) Zyklischer CPU-Test:
 *     Mit jedem Testaufruf wird ein CPU-Test ausgeführt. Beim nächsten Aufruf
 *     wird der nächste CPU-Test ausgeführt usw.
 *
 *       (++) Initialisierung: CPUTestStl_SetupTestCyclic()
 *       (++) Zyklischer Testaufruf: CPUTestStl_RunCyclic()
 *
 *  (#) Single CPU-Test:
 *      Ein ausgewählter CPU-Test kann ausgeführt werden.
 *       (++) CPUTestStl_RunSingle()
 *
 * @{
 */
#ifndef STM32_SAFETY_STL_CPUTEST_CPU_TEST_STL_H
#define STM32_SAFETY_STL_CPUTEST_CPU_TEST_STL_H

// Headerdateien einbinden -----------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#include "SafetyStl.h"

// Allgemeine Definitionen -----------------------------------------------------

// Makros ----------------------------------------------------------------------

// Typdefinitionen--------------------------------------------------------------

// externe Variablen -----------------------------------------------------------

// Prototypen ------------------------------------------------------------------

/// Führt mit einem Aufruf alle aktivierten CPU-Tests aus. Deaktivierte Tests
/// werden übersprungen.
/// \return EN61508_TestPass, wenn die durchgeführten Tests bestanden sind,
/// sonst EN61508_TestFail.
extern EN61508_TestResult CPUTestStl_RunAll(void);

/// Führt stückweise alle aktivierten CPU-Tests aus. Pro Aufruf wird ein Test ausgeführt.
/// Deaktivierte Tests werden übersprungen.
/// Es wird geprüft, ob der Test mindestens einmal innerhalb der Process Safety Time durchläuft.
/// \param currentTicks Aktuelle Zeit in Ticks.
/// \return EN61508_TestPass, wenn die durchgeführten Tests bestanden sind,
/// sonst EN61508_TestFail.
extern EN61508_TestResult CPUTestStl_RunCyclic(U32 const currentTicks);

/// Initialisiert den zyklischen CPU-Test. Einmalig vor dem Test notwendig.
/// Es wird geprüft, ob der Test mindestens einmal innerhalb der Process Safety Time durchläuft.
/// \param processSafetyTimeTicks Process Safety Time (PST) in Ticks.
/// \return @c true bei Erfolg, sonst @c false.
extern bool CPUTestStl_SetupTestCyclic(U32 const processSafetyTimeTicks);

/// Führt einen einzelnen CPU-Test aus. Wenn der Test deaktiviert ist oder nicht bestanden ist,
/// wird ein Fehler zurückgegeben.
/// \param cpuIndex Index für die Auswahl des Tests.
/// \return EN61508_TestPass, wenn der Test erfolgreich durchgeführt wurde, sonst EN61508_TestFail.
extern EN61508_TestResult CPUTestStl_RunSingle(STL_CpuTmxIndex_t const cpuIndex);

#ifdef __cplusplus
}
#endif
#endif /* STM32_SAFETY_STL_CPUTEST_CPU_TEST_STL_H */
/**
* @}
*/
