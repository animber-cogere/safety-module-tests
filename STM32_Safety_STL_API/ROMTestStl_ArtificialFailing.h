/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
 * \defgroup ROMTestStl_ArtificialFailing ROM-Test Fehlerinbau
 *
 * Fehlereinbau-Makros zum Testen des ROM-Tests. Es werden gezielt Fehlschläge der
 * Self-Test-Library-Funktionen eingebaut.
 *
 * Es handelt sich hierbei nicht um Unittests. Ungültige Testkonfigurationen oder
 * ungültige Funktionsargumente werden nicht getestet.
 *
 * Über die Makros kann eingestellt werden, welche Funktionen der Tests fehlschlagen sollen.
 *
 * (#) Makros für kompletten ROM-Test (Startuptest)
 *  |Testmakros                          | Funktion         | Hard-Error                 |
 *  |-----------------------------------:|:----------------:|:---------------------------|
 *  | ROMTEST_ARTI_FAILING_INIT_ALL      | Initialisierung  | HARD_ERR_MEM_ROM           |
 *  | ROMTEST_ARTI_FAILING_CONFIGURE_ALL | Konfiguration    | HARD_ERR_MEM_ROM           |
 *  | ROMTEST_ARTI_FAILING_RUN_ALL       | Ausführung       | HARD_ERR_MEM_ROM           |
 *
 * (#) Makros für zyklischen ROM-Test (Laufzeittest)
 *  |Testmakros                             | Funktion         | Hard-Error              |
 *  |-----------------------------------   :|:----------------:|:------------------------|
 *  | ROMTEST_ARTI_FAILING_INIT_CYCLIC      | Initialisierung  | HARD_ERR_MEM_ROM_CYCLIC |
 *  | ROMTEST_ARTI_FAILING_CONFIGURE_CYCLIC | Konfiguration    | HARD_ERR_MEM_ROM_CYCLIC |
 *  | ROMTEST_ARTI_FAILING_RUN_CYCLIC       | Ausführung       | HARD_ERR_MEM_ROM_CYCLIC |
 *  | ROMTEST_ARTI_FAILING_RESET_CYCLIC     | Reset (Neustart) | HARD_ERR_MEM_ROM_CYCLIC |
 *
 * @{
 */
#ifndef STM32_SAFETY_STL_ROMTEST_ROM_TEST_STL_ARTIFICIAL_FAILING_H
#define STM32_SAFETY_STL_ROMTEST_ROM_TEST_STL_ARTIFICIAL_FAILING_H

// Headerdateien einbinden -----------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Allgemeine Definitionen -----------------------------------------------------

// Makros ----------------------------------------------------------------------

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL

/* Fehlereinbau-Testmakros */
// Nur ein Makro auf einmal aktivieren
// Fehlschlag der Initialisierungsfunktion von STL
#define ROMTEST_ARTI_FAILING_INIT_ALL          (0)
#define ROMTEST_ARTI_FAILING_INIT_CYCLIC       (0)

// Fehlschlag der Konfigurationsfunktion von STL
#define ROMTEST_ARTI_FAILING_CONFIGURE_ALL     (0)
#define ROMTEST_ARTI_FAILING_CONFIGURE_CYCLIC  (0)

// Fehlschlag bei der Testausführung
#define ROMTEST_ARTI_FAILING_RUN_ALL           (0)
#define ROMTEST_ARTI_FAILING_RUN_CYCLIC        (0)

// Fehlschlag beim Reset
#define ROMTEST_ARTI_FAILING_RESET_CYCLIC      (0)


// Testmodul-Statuswerte, die durch das Artificial-Failing erzwungen werden. ROM-Test-Status wird auf @ref STL_FAILED gesetzt.
STL_ArtifFailingConfig_t romFail =
                {
                    {STL_NOT_TESTED, STL_NOT_TESTED, STL_NOT_TESTED, STL_NOT_TESTED, STL_NOT_TESTED, STL_NOT_TESTED,
                     STL_NOT_TESTED, STL_NOT_TESTED, STL_NOT_TESTED, STL_NOT_TESTED, STL_NOT_TESTED, STL_NOT_TESTED},
                     STL_FAILED,
                     STL_NOT_TESTED
                };

// Funktions-Makros zum Starten Beenden des Artificial Failings
#define ROMTEST_ARTI_FAILING_START                                    \
                if(STL_SCH_StartArtifFailing(&romFail) != STL_OK)     \
                            {                                         \
                            __asm("BKPT #0\n");                       \
                            }

#define ROMTEST_ARTI_FAILING_STOP                                     \
                if(STL_SCH_StopArtifFailing() != STL_OK)              \
                            {                                         \
                             __asm("BKPT #0\n");                      \
                            }

#define ROMTEST_ARTI_FAILING_START_CYCLIC                             \
                if(romTest == &romTestCyclic)                         \
                    {                                                 \
                    ROMTEST_ARTI_FAILING_START                        \
                    }

#define ROMTEST_ARTI_FAILING_STOP_CYCLIC                              \
                if(romTest == &romTestCyclic)                         \
                    {                                                 \
                    ROMTEST_ARTI_FAILING_STOP                         \
                    }

#define ROMTEST_ARTI_FAILING_START_ALL                                \
                if(romTest == &romTestAll)                            \
                    {                                                 \
                    ROMTEST_ARTI_FAILING_START                        \
                    }

#define ROMTEST_ARTI_FAILING_STOP_ALL                                 \
                if(romTest == &romTestAll)                            \
                    {                                                 \
                    ROMTEST_ARTI_FAILING_STOP                         \
                    }

#endif /* FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_ROM_STL */

// Typdefinitionen--------------------------------------------------------------

// externe Variablen -----------------------------------------------------------

// Prototypen ------------------------------------------------------------------


#ifdef __cplusplus
}
#endif
#endif /* STM32_SAFETY_STL_ROMTEST_ROM_TEST_STL_ARTIFICIAL_FAILING_H */
/**
* @}
*/
