/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
 * \defgroup RAMTestStl RAM-Test Funktionen
 *
 * Funktionen zur Ausführung des RAM-Tests der Self-Test-Library von ST.
 * Zugrhöriges Handbuch UM2590.
 *
 * Der RAM-Test funktioniert nach dem March-C-Algorithmus. Der Speicher
 * wird mit einem spezifischen Muster beschrieben und zurückgelesen.
 *
 * Der Test findet über definierte Testbereiche im RAM statt.
 * Umgesetzt ist ein zyklischer RAM-Test und ein kompletter RAM-Test.
 *
 * (#) Kompletter RAM-Test:
 *     Alle definierten Testbereiche werden mit einem Aufruf komplett
 *     getestet.
 *
 *        (++) RAMTestStl_RunAll()
 *
 * (#) Zyklischer RAM-Test:
 *     Die definierten Testbereiche werden stückweise getestet. Mit jedem Aufruf
 *     wird nur ein Sektor von 128 Bytes im Testbereich überprüft.
 *     Beim nächsten Aufruf wird der nächste Sektor im Testbereich überprüft usw.,
 *     bis alle Bereiche getestet sind.
 *
 *       (++) Initialisierung: RAMTestStl_SetupTestCyclic()
 *
 *       (++) Zyklischer Testaufruf:
 *              (+++) RAMTestStl_RunCyclic()
 *
 * @{
 */
#ifndef STM32_SAFETY_STL_RAMTEST_RAM_TEST_STL_H
#define STM32_SAFETY_STL_RAMTEST_RAM_TEST_STL_H

// Headerdateien einbinden -----------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Allgemeine Definitionen -----------------------------------------------------

// Makros ----------------------------------------------------------------------

/// Min, max values for user configuration to configure number of tested
/// RAM sections per test execution for cyclic runtime RAM test (SOFTQM-1074)

/// Maximum number of sections, which can be tested per
/// test execution. This maximum value sets the setting to
/// 1-shot, so that all sections are tested in one test execution
#define RAMTEST_NUM_SECTIONS_ATOMIC_MAX  (0xFFFFFFFF)

/// Minimum number of sections, which can be tested per
/// test execution.
#define RAMTEST_NUM_SECTIONS_ATOMIC_MIN  (0x1)

// Typdefinitionen--------------------------------------------------------------

// externe Variablen -----------------------------------------------------------

// Prototypen ------------------------------------------------------------------

/// Kompletter RAM-Test-Durchlauf über die in @c ramRegions angegebenen Bereiche.
/// Mit einem Aufruf wird alles getestet.
/// \return @c EN61508_TestPass bei Erfolg, sonst @c EN61508_TestFail.
extern EN61508_TestResult RAMTestStl_RunAll(void);

/// Initialisierung des zyklischen RAM-Tests, einmalig vor dem Test notwendig.
/// Es wird geprüft, ob der Test mindestens einmal innerhalb der Process Safety Time durchläuft.
/// \param processSafetyTimeTicks Process Safety Time (PST) in Ticks.
/// \return @c true bei Erfolg, sonst @c false.
extern bool RAMTestStl_SetupTestCyclic(U32 const processSafetyTimeTicks);

/// Zyklischer RAM-Test über die in @c ramRegions angegebenen Bereiche.
/// Mit einem Aufruf wird ein Sektor von 128 Bytes getestet. Beim nächsten Aufruf
/// wird der nächste Sektor überprüft.
/// Der zyklische RAM-Test muss initialisiert werden.
/// \param currentTicks Aktuelle Zeit in Ticks.
/// \return @c EN61508_TestPass bei Erfolg, sonst @c EN61508_TestFail.
extern EN61508_TestResult RAMTestStl_RunCyclic(U32 const currentTicks);

#ifdef __cplusplus
}
#endif
#endif /* STM32_SAFETY_STL_RAMTEST_RAM_TEST_STL_H */
/**
* @}
*/
