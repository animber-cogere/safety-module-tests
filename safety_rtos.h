/*******************************************************************************
 * Copyright (c) 2016-2021 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
* \defgroup safety_utils Sicherheitsfunktionen
* @{
*/
#ifndef GLOBAL_SAFETY_SAFETY_RTOS_H_
#define GLOBAL_SAFETY_SAFETY_RTOS_H_

// Gemeinsame Headerdateien einbinden ---------------------------------------

// Spezielle Headerdateien einbinden ----------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

// Compiler Direktiven ------------------------------------------------------
#if FEAT_RTOS
// Makros -------------------------------------------------------------------

// externe Variablen --------------------------------------------------------

// Allgemeine Definitionen --------------------------------------------------

// Prototypen ---------------------------------------------------------------

/// \ingroup tasklist
/// Task: Sicherheitsfunktionen, Ueberwachung der Controllerfunktionen.
/// In dieser Task wird der Taskaufrufzaehler, welcher zum Ueberwachen der
/// Sicherheitstask eingesetzt wird, geprueft. Die Sicherheitstask muss 500 Mal
/// pro Sekunde durchlaufen werden (Toleranz +- 3). Weicht der Wert 5 Mal in
/// Folge ab, so ist einer der beiden Resonatoren fehlerhaft oder es liegt ein
/// anderer Fehler vor, der den Scheduler des RTOS beeinflusst bzw.
/// behindert. Fuer diese Ueberwachung wird der RTC des Hibernate-Moduls und
/// ein externer 4,194304 MHz Resonator verwendet.\n
/// Zudem werden in der Sicherheitstask zyklische RAM- und ROM-Tests
/// ausgefuehrt und der Watchdog-Timer getriggert.\n
/// Die Positionstask wird ebenfalls von der Safety-Task ueberwacht. Hierzu
/// setzt die Positionstask, die im normalen Betrieb zyklisch jede Millisekunde
/// ausgefuehrt werden sollte, ein globales Flag. Die Sicherheitstask
/// ueberprueft dieses Flag. Sollte das Flag zweimal in Folge nicht gesetzt
/// sein, wird sofort der HardError ausgefuehrt.
/// Die Ueberwachung der Positionstask findet jedoch nur statt, wenn zwischen
/// zwei Durchlaeufen der Sicherheitstask mindestens 20 Ticks (entspricht 2ms)
/// liegen, damit die Positionstask ueberhaupt die Chance hat, mindestens
/// einmal ausgefuehrt zu werden. Eine Besonderheit stellt hierbei der
/// Ueberlauf des Tickzaehlers dar: In diesem Fall wird die
/// Positionstaskueberwachung gesondert einmalig ausgesetzt, da die zeitliche
/// Differenz nicht eindeutig bestimmt werden kann.
extern void Safety_Task(TASK_PARA_STD * const tParam);

/// Initialisierung der Sicherheitstask.
extern U32 Safety_Task_Init(TASK_PARA_STD * const tParam);

/// Neustart der Safetytask. Die Funktion setzt nach einer Unterbrechung der Task,
/// z.B. waehrend des Abgleichs, die Zykluszeit zurueck.
extern void Safety_Restart(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_SAFETY_SAFETY_RTOS_H_ */
/**
* @}
*/
