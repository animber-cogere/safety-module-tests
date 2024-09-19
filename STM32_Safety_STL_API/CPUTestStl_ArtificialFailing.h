/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
 * \defgroup CPUTestStl_ArtificialFailing CPU-Test Fehlereinbau
 *
 * Fehlereinbau-Makros zum Testen desr CPU-Tests. Es werden gezielt Fehlschläge der
 * Self-Test-Library-Funktionen eingebaut.
 *
 * Es handelt sich hierbei nicht um Unittests. Ungültige Funktionsargumente werden nicht getestet.
 *
 * Über die Makros kann eingestellt werden, welche Funktionen der Tests fehlschlagen sollen.
 *
 * (#) Makros für kompletten CPU-Test (Startuptest)
 *  |Testmakros                          | Funktion         | Hard-Error              |
 *  |-----------------------------------:|:----------------:|:------------------------|
 *  | CPUTEST_ARTI_FAILING_RUN_ALL       | Ausführung       | HARD_ERR_CPU            |
 *
 * (#) Makros für zyklischen CPU-Test (Laufzeittest)
 *  |Testmakros                          | Funktion         | Hard-Error              |
 *  |-----------------------------------:|:----------------:|:------------------------|
 *  | CPUTEST_ARTI_FAILING_RUN_CYCLIC    | Ausführung       | HARD_ERR_CPU_CYCLIC     |
 *
 *(#) Makros für Single-CPU-Test
 *  |Testmakros                          | Funktion         | Hard-Error              |
 *  |-----------------------------------:|:----------------:|:------------------------|
 *  | CPUTEST_ARTI_FAILING_RUN_SINGLE    | Ausführung       | Bisher nicht verwendet  |
 *
 * @{
 */
#ifndef STM32_SAFETY_STL_CPUTEST_CPU_TEST_STL_ARTIFICIAL_FAILING_H
#define STM32_SAFETY_STL_CPUTEST_CPU_TEST_STL_ARTIFICIAL_FAILING_H

// Headerdateien einbinden -----------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Allgemeine Definitionen -----------------------------------------------------

// Makros ----------------------------------------------------------------------

#if FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL

/* Einstellung, welche Testfunktion fehlschlagen soll */
// Nur eine auf einmal aktivieren
// Fehlschlag bei der Testausführung
#define CPUTEST_ARTI_FAILING_RUN_ALL           (0)
#define CPUTEST_ARTI_FAILING_RUN_CYCLIC        (0)
#define CPUTEST_ARTI_FAILING_RUN_SINGLE        (0)

// Testmodul-Statuswerte, die durch das Artificial-Failing erzwungen werden. CPU-Test-Stati werden auf @ref STL_FAILED gesetzt.
STL_ArtifFailingConfig_t cpuFail =
                {
                    {STL_NOT_TESTED, STL_FAILED, STL_FAILED, STL_FAILED, STL_FAILED, STL_FAILED,
                     STL_FAILED, STL_FAILED, STL_FAILED, STL_FAILED, STL_FAILED, STL_FAILED},
                    STL_NOT_TESTED,
                    STL_NOT_TESTED
                };

// Funktions-Makros zum Starten Beenden des Artificial Failings
#define CPUTEST_ARTI_FAILING_START                                    \
                if(STL_SCH_StartArtifFailing(&cpuFail) != STL_OK)     \
                            {                                         \
                            __asm("BKPT #0\n");                       \
                            }

#define CPUTEST_ARTI_FAILING_STOP                                     \
                if(STL_SCH_StopArtifFailing() != STL_OK)              \
                            {                                         \
                             __asm("BKPT #0\n");                      \
                            }

#endif /* FEATURE_SAFETYCHECK_ARTIFICIAL_TEST_CPU_STL */

// Typdefinitionen--------------------------------------------------------------

// externe Variablen -----------------------------------------------------------

// Prototypen ------------------------------------------------------------------


#ifdef __cplusplus
}
#endif
#endif /* STM32_SAFETY_STL_CPUTEST_CPU_TEST_STL_ARTIFICIAL_FAILING_H */
/**
* @}
*/
