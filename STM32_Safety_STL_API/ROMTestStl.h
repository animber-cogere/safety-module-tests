/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
 * \defgroup ROMTestStl ROM-Test Funktionen
 *
 * Funktionen zur Ausführung des ROM-Tests der Self-Test-Library von ST.
 * Zugehöriges Handbuch: UM2590
 *
 * Der Flashbereich wird in Sektoren von 1024 Bytes aufgeteilt. Für jeden Sektor gibt es
 * eine 4 Byte lange CRC im Speicher, die ausgelesen und geprüft wird.
 *
 * Der Test erfolgt sektorweise über definierte Speicherbereiche. Umgesetzt ist ein zyklischer ROM-Test
 * und ein kompletter ROM-Test:
 *
 * (#) Kompletter ROM-Test:
 *     Alle Sektoren in den Speicherbereichen werden mit einem Testaufruf geprüft.
 *
 *        (++) ROMTestStl_RunAll()
 *
 * (#) Zyklischer ROM-Test:
 *     Mit jedem Testaufruf wird die CRC von nur einem Sektor geprüft (1024 Bytes). Beim nächsten Aufruf
 *     wird der nächste Sektor geprüft bis alle definierten Speicherbereiche geprüft sind.
 *
 *       (++) Initialisierung: ROMTestStl_SetupTestCyclic()
 *       (++) Zyklischer Testaufruf: ROMTestStl_RunCyclic()
 *
 *
 * CRC-Bereich:
 *
 * (#) Enthält die CRCs aller Flash-Sektoren.
 * (#) Befindet sich am Ende des Flash-Memorys im Microcontroller (siehe Linker-Datei).
 * (#) Größe des CRC-Bereichs:
 *          __FLASH_TEST_STL_CRC_SIZE = (FLASH_MEMORY_SIZE /1024 Bytes) * 4 Bytes.
 * (#) Startadresse des CRC-Bereichs:
 *          __FLASH_TEST_STL_CRC_START = FLASH_BASE + FLASH_MEMORY_SIZE - __FLASH_TEST_STL_CRC_SIZE

 * (#) Die CRCs müssen als Post-Build-Kommando vom STM32CubeProgrammer eingefügt werden (Handbuch UM2237).
 *
 * @{
 */
#ifndef STM32_SAFETY_STL_ROMTEST_ROM_TEST_STL_H
#define STM32_SAFETY_STL_ROMTEST_ROM_TEST_STL_H

// Headerdateien einbinden -----------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Allgemeine Definitionen -----------------------------------------------------

// Makros ----------------------------------------------------------------------

/// Min, max values for user configuration to configure number of tested
/// ROM sections per test execution for cyclic runtime ROM test (SOFTQM-1076)

/// Maximum number of sections, which can be tested per
/// test execution. This maximum value sets the setting to
/// 1-shot, so that all sections are tested in one test execution
#define ROMTEST_NUM_SECTIONS_ATOMIC_MAX  (0xFFFFFFFF)

/// Minimum number of sections, which can be tested per
/// test execution.
#define ROMTEST_NUM_SECTIONS_ATOMIC_MIN  (0x1)

// Typdefinitionen--------------------------------------------------------------

// externe Variablen -----------------------------------------------------------

// Prototypen ------------------------------------------------------------------

/// Kompletter ROM-Test-Durchlauf über die in @c flashRegions angegebenen Bereiche.
/// Mit einem Aufruf wird alles getestet.
/// \return @c EN61508_TestPass bei Erfolg, sonst @c EN61508_TestFail.
extern EN61508_TestResult ROMTestStl_RunAll(void);

/// Initialisierung des zyklischen ROM-Tests, einmalig vor dem Test notwendig.
/// Es wird geprüft, ob der Test mindestens einmal innerhalb der Process Safety Time durchläuft.
/// \param processSafetyTimeTicks Process Safety Time (PST) in Ticks.
/// \return @c true bei Erfolg, sonst @c false.
extern bool ROMTestStl_SetupTestCyclic(U32 const processSafetyTimeTicks);

/// Zyklischer ROM-Test über die in @c flashRegions angegebenen Bereiche.
/// Mit einem Aufruf wird ein Sektor von 1024 Bytes getestet. Beim nächsten Aufruf
/// wird der nächste Sektor überprüft.
/// Der zyklische ROM-Test muss initialisiert werden.
/// \param currentTicks Aktuelle Zeit in Ticks.
/// \return @c EN61508_TestPass bei Erfolg, sonst @c EN61508_TestFail.
extern EN61508_TestResult ROMTestStl_RunCyclic(U32 const currentTicks);


#ifdef __cplusplus
}
#endif
#endif /* STM32_SAFETY_STL_ROMTEST_ROM_TEST_STL_H */
/**
* @}
*/
