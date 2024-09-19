/*******************************************************************************
 * Copyright (c) 2023 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
 * \defgroup SafetyStl Allgemeine Self-Test Library-Funktionen
 *
 * Allgemeine Funktionen der Self-Test-Library, die sich nicht auf einen
 * bestimmten Test beziehen.
 *
 * @{
 */
#ifndef STM32_SAFETY_STL_SAFETYSTL_H
#define STM32_SAFETY_STL_SAFETYSTL_H

// Headerdateien einbinden -----------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#include "error_def.h"
#include "EN61508_Program_Flow/EN61508.h"
#include "STM32_Safety_STL/STM32G4_Safety_STL/Inc/stl_user_api.h"

// Allgemeine Definitionen -----------------------------------------------------

/// Struktur für die Definition zu prüfender Speicherbereiche
typedef struct
{
    U8 * start;  ///< Startadresse eines Speicherbereichs
    U32 length;  ///< Länge des Speicherbereichs in Bytes
} EN61508_MEM_REGION;

#ifndef FEATURE_SAFETYCHECK_USE_STL
/// \ingroup feature_flags
/// Feature Flag zum aktivieren der STL Tests.
/// Diese Tests sind per Default deaktiviert.
#define FEATURE_SAFETYCHECK_USE_STL (0)
#warning "Konfiguration in version_def.h erforderlich."
#endif
// Makros ----------------------------------------------------------------------

// Typdefinitionen--------------------------------------------------------------
/// Flash section size defined in UM2590 STM32 Safety STL user manual.
/// There will be one CRC per FLASH section.
#define STL_FLASH_SECTION_SIZE (0x400)
// externe Variablen -----------------------------------------------------------

// Prototypen ------------------------------------------------------------------

/// Initialisiert den Scheduler der Self-Test-Library
/// \return true bei Erfolg, sonst false.
extern bool Stl_SchedulerInit(void);

/// Prüft, ob der Scheduler gestartet ist. Erst dann können Tests ausgeführt werden.
/// \return true wenn der Scheduler initialisiert wurde, sonst false.
extern bool Stl_SchedulerIsStarted(void);

#ifdef __cplusplus
}
#endif
#endif /* STM32_SAFETY_STL_SAFETYSTL_H */
/**
* @}
*/
