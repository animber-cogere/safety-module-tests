/*******************************************************************************
 * Copyright (c) 2013-2022 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

/**
* \defgroup syspower_check Versorgungsspannungsueberwachung
* \ingroup safety_utils
* Die Versorgungsspannungsüberwachung kann bis zu 3 Spannungen überwachen. Dies
* sind die externe Versorung Ub, intern 1 Volt und intern 1,5 Volt. Zusätzlich
* kann die Stromaufnahme gemesen werden. Die Spannungen werden auf Über- und
* Unterspannung geprüft. Mit Hilfe der Stromaufnahme wird die max. Leistungsaufnahme
* überwacht.
*
* Bei der Spannung Ub werden bei einer Unterspannung zwei Fällen unterschieden.
* - Die Spannung fällt dauerhaft unter die Schwellspannung.
* - Die Spannung fällt nur kurzzeitig unter die Schwellspannung(Spannungseinbruch).
*
* Das Modul kann zusätzlich die Systemtemperatur messen. Es erfolgt aber keine
* weitere Prüfung der Temperatur.
* @{
*/
#ifndef GLOBAL_SAFETY_SAFETY_POWERSUPPLY_H_
#define GLOBAL_SAFETY_SAFETY_POWERSUPPLY_H_

// Gemeinsame Headerdateien einbinden ---------------------------------------

// Spezielle Headerdateien einbinden ----------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

// Compiler Direktiven ------------------------------------------------------
/// Configuration to activate and deactivate measurement channels (SOFTQM-681)
/// Only activated channels will be measured and monitored.
typedef struct
{
    bit supplyVoltageIsActive: 1;               ///< External supply voltage monitoring
    bit voltage1IsActive: 1;                    ///< Internal voltage 1 monitoring
    bit voltage2IsActive: 1;                    ///< Internal voltage 2 monitoring
    bit voltage3IsActive: 1;                    ///< Internal voltage 3 monitoring
    bit voltage4IsActive: 1;                    ///< Internal voltage 4 monitoring
    bit voltage5IsActive: 1;                    ///< Internal voltage 5 monitoring
    bit voltageExternalAdcChannel1IsActive: 1;  ///< Voltage monitoring with the external ADC. Channel 1 does not mean "hardware" channel 1 but the first channel to check. This has to be configured by the user by using the CHANNEL1_TO_CHECK_CHANNEL macro.
    bit voltageExternalAdcChannel2IsActive: 1;  ///< Voltage monitoring with the external ADC. Channel 2 does not mean "hardware" channel 2 but the second channel to check. This has to be configured by the user by using the CHANNEL2_TO_CHECK_CHANNEL macro.
    bit voltageExternalAdcChannel3IsActive: 1;  ///< Voltage monitoring with the external ADC. Channel 1 does not mean "hardware" channel 3 but the third channel to check. This has to be configured by the user by using the CHANNEL3_TO_CHECK_CHANNEL macro.
    bit currentIsActive: 1;                     ///< Current monitoring
    bit temperatureSensorIsActive: 1;           ///< Temperature monitoring using temperature sensor
    bit temperatureAdcIsActive: 1;              ///< Temperature monitoring using internal ADC
} SAFETY_POWERSUPPLY_CONFIG;


// Makros -------------------------------------------------------------------

/// Factor for turning float to integer in milli-units
#define DECIMAL_FIXPOINT                     (1000.0f)

/// Supply voltage error limits set by user
/// Minimum level in Volt
#ifndef VOLTAGE_SUPPLY_LIMIT_MIN_VOLT
#define VOLTAGE_SUPPLY_LIMIT_MIN_VOLT        (7.000f)
#endif

/// Maximum level in Volt
#ifndef VOLTAGE_SUPPLY_LIMIT_MAX_VOLT
#define VOLTAGE_SUPPLY_LIMIT_MAX_VOLT        (36.000f)
#endif

/// Minimum level in mV
#define VOLTAGE_SUPPLY_LIMIT_MIN_MILLIVOLT   (VOLTAGE_SUPPLY_LIMIT_MIN_VOLT * DECIMAL_FIXPOINT)

/// Maximum level in mV
#define VOLTAGE_SUPPLY_LIMIT_MAX_MILLIVOLT   (VOLTAGE_SUPPLY_LIMIT_MAX_VOLT * DECIMAL_FIXPOINT)

/// Supply voltage warning limits set by user
#if !defined(VOLTAGE_SUPPLY_WARNING_MIN_VOLT) & defined(fpADCIN_VCC)
/// Minimum warning limit in Volt
#define VOLTAGE_SUPPLY_WARNING_MIN_VOLT         (10.000f)
#endif
/// Maximum warning limit in Volt
#if !defined(VOLTAGE_SUPPLY_WARNING_MAX_VOLT) & defined(fpADCIN_VCC)
#define VOLTAGE_SUPPLY_WARNING_MAX_VOLT         (33.000f)
#endif

// externe Variablen --------------------------------------------------------

// Allgemeine Definitionen --------------------------------------------------
/// System temperature status
typedef enum
{
    eSYSTMP_STAT_TMP_NO_ERROR = 0x00,      //!< Temperature state without error
    eSYSTMP_STAT_TMP_STARTUP_VALID = 0x01, //!< Temperature measurement valid
    eSYSTMP_STAT_WARNING_TMP_HIGH = 0x02,  //!< Warning state temperature too high
    eSYSTMP_STAT_WARNING_TMP_LOW = 0x04,   //!< Warning state temperature too low
    eSYSTMP_STAT_ERROR_TMP_HIGH = 0x08,    //!< Error state temperature too high
    eSYSTMP_STAT_ERROR_TMP_LOW = 0x10,     //!< Error state temperature too low
} SYSTEM_TEMPERATURE_STATUS;



/** Error handler 3 byte error code for voltage monitoring (SOFTQM-638, SOFTQM-657) */

/// Upper level error byte for voltage monitoring error and warning events
/// Information about the voltage measurement channel to which the event refers.
typedef enum
{
    eERROR_INTERNAL_VOLTAGE_1 = 0x01,      ///< Internal voltage 1
    eERROR_INTERNAL_VOLTAGE_2 = 0x02,      ///< Internal voltage 2
    eERROR_INTERNAL_VOLTAGE_3 = 0x03,      ///< Internal voltage 3
    eERROR_INTERNAL_VOLTAGE_4 = 0x04,      ///< Internal voltage 4
    eERROR_INTERNAL_VOLTAGE_5 = 0x05,      ///< Internal voltage 5
    eERROR_EXTERNAL_ADC_VOLTAGE_1 = 0x06,  ///< External ADC voltage 1
    eERROR_EXTERNAL_ADC_VOLTAGE_2 = 0x07,  ///< External ADC voltage 2
    eERROR_EXTERNAL_ADC_VOLTAGE_3 = 0x08,  ///< External ADC voltage 3
    eERROR_SUPPLY_VOLTAGE = 0x10,          ///< External supply voltage
} eERROR_VOLTAGE_CHANNELS;

/// Intermediate level error byte for voltage monitoring error and warning events
/// Error definitions
typedef enum
{
    eERROR_VOLTAGE_OK = 0x00,            ///< Voltage ok
    eERROR_VOLTAGE_EXCEEDED_MAX = 0x01,  ///< Voltage exceeds maximum limit
    eERROR_VOLTAGE_EXCEEDED_MIN = 0x02,  ///< Voltage exceeds minimum limit
    eERROR_VOLTAGE_VCC_DROPOUT = 0x20,   ///< Voltage dropout detected
} eERROR_VOLTAGE;



/** Error handler 3 byte error codes for temperature monitoring (SOFTQM-642) */

/// Upper level error byte for temperature monitoring warning events
/// Information about the temperature measurement channel to which the event refers.
typedef enum
{
    eERROR_SYSTEM_TEMPERATURE = 0x01,  ///< System temperature
} eERROR_TEMPERATURE_CHANNELS;

/// Intermediate level error byte for temperature monitoring error and warning events
/// Error definitions
typedef enum
{
    eERROR_TEMPERATURE_OK = 0x00,            ///< Temperature ok
    eERROR_TEMPERATURE_EXCEEDED_MAX = 0x01,  ///< Temperature exceeds maximum limit
    eERROR_TEMPERATURE_EXCEEDED_MIN = 0x02,  ///< Temperature exceeds minimum limit
} eERROR_TEMPERATURE;



/** Error handler 3 byte error codes for power monitoring (SOFTQM-640) */

/// Upper level error byte for power monitoring warning event
/// Information about the power measurement channel to which the event refers.
typedef enum
{
    eERROR_SUPPLY_POWER = 0x01,        ///< Systemleistung fehlerhaft
} eERROR_POWER_CHANNELS;

/// Intermediate level error byte for power monitoring warning event
/// Error definitions
typedef enum
{
    eERROR_POWER_OK = 0x00,            ///< Power ok
    eERROR_POWER_EXCEEDED_MAX = 0x01,  ///< Power exceeds maximum limit
} eERROR_POWER;

/// Fill lower level error byte with 0
#define ERROR_BYTE_LOWER_LEVEL_FILL_ZERO   (0x00)

// Prototypen ---------------------------------------------------------------

/// Initialissierung der Spannungsüberwachung.
/// \param safetyPowerSupplyConfig Aktivierung der Messkanäle. Die entsprichenden Pindefinitionen
/// müssen bei Aktivierung vorhanden sein.
/// \return true bei Erfolg, sonst false.
extern bool Safety_Powersuply_Init(SAFETY_POWERSUPPLY_CONFIG * const safetyPowerSupplyConfig);

/// Die Zykluszeit mit der die Funktion aufgerufen wird, dient intern als
/// Zeitbasis für Timeouts, usw.
/// @return TRUE
extern U8 Safety_Powersupply_Check(void);

#if defined(fpADCIN_TEMPERATURE) || defined(TMP144_UART_CHANNEL)
/// Abfrage der internen Systemtemperatur.
/// Die Funktion ist nur aktiv, wenn der ADC Eingang, der genutzt werden soll,
/// als \e fpADCIN_TEMPERATURE definiert ist.
/// Die Temperatur wird mit \e DECIMAL_FIXPOINT skaliert.
/// \return Aktuelle Systemtemperatur.
extern S32 Safety_GetSystemTemperature(void);

/// This function is called whenever the measured temperature value changed.
/// The user may overwrite it with a custom definition. (SOFTQM-680)
/// @param temperature The currently measured device temperature.
/// @param temperatureState The state of the temperature giving information about warning and error states
extern void Safety_TemperatureChangedHook(float temperature, SYSTEM_TEMPERATURE_STATUS const temperatureState);
#endif

#ifdef fpADCIN_VCC
/// Abfrage der internen Versorgungsspannung.
/// Die Funktion ist nur aktiv, wenn der ADC Eingang, der genutzt werden soll,
/// als \e fpADCIN_VCC definiert ist.
/// Die Spannung wird mit \e DECIMAL_FIXPOINT skaliert.
/// \return Aktuelle Versorgungsspannung.
extern S32 Safety_GetPowerVoltage(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_SAFETY_SAFETY_POWERSUPPLY_H_ */
/**
* @}
*/
