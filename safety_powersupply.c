/*******************************************************************************
 * Copyright (c) 2013-2021 TWK-ELEKTRONIK GmbH. All rights reserved.
 ******************************************************************************/

// Gemeinsame Headerdateien einbinden --------------------------------------
#include "config/version.h"

#include "ADC/ADC_Driver.h"

#include "eventdef.h"
#include "error_def.h"
#include "safety_startup.h"
#include "DataProcess_Averaging/averaging.h"
#include "DataProcess_Averaging/averaging_f32.h"
#include "EventSystem/Event.h"
#include "Parameter_Table/parameter_tab.h"
#include "Devices_ADC_MAX116XX/MAX116XX_Task.h"
#include "Devices_ADC_MAX116XX/MAX116XX.h"

#if FEAT_MSG_INTERPRETER
#include "Factory_Device_Communication/msg_interpreter.h"
#endif

#include "safety_powersupply.h"


#ifdef TMP144_UART_CHANNEL
#include "Devices_Temperature_TMP144/TMP144.h"
#endif

#include "ErrorHandler/default_events.h"

// Spezielle Headerdateien einbinden ---------------------------------------

// Compiler Direktiven -----------------------------------------------------

// Makros ------------------------------------------------------------------

#if ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))
// Definition of the following macros should happen in the platine file.

// -------------------------------- FIRST VOLTAGE EXTERNAL ADC -----------------------------------------------------------

// Macro to be set by user. Defines if the first external adc voltage to check if coming from a 12 channel external adc. If it is false, it is assumed that it comes from a 4 channel external adc.
#ifndef EXT_ADC_CHANNEL1_TO_CHECK_IS_12CHANNEL
#pragma message "EXT_ADC_CHANNEL1_TO_CHECK_IS_12CHANNEL is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL1_TO_CHECK_IS_12CHANNEL                  (false)
#endif

// Macro to be set by user. Defines from which channel of the external adc the voltage to check is coming. It can range from 0-11 for the 12-channel and from 0-3 for the 4-channel external adc.
#ifndef EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL
#pragma message "EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL                       (0)
#endif

#ifndef EXT_ADC_CHANNEL1_TO_CHECK_R1
#pragma message "R1 of the voltage divider associated with CHANNEL1 to check is not defined! Fallback to default values."
#define EXT_ADC_CHANNEL1_TO_CHECK_R1                            (0.0f)
#endif
#ifndef EXT_ADC_CHANNEL1_TO_CHECK_R2
#pragma message "R2 of the voltage divider associated with CHANNEL1 to check is not defined! Fallback to default values."
#define EXT_ADC_CHANNEL1_TO_CHECK_R2                            (1.0f)
#endif

// The external adc is mostly used with a voltage divider. The values of the used resistors shall be defined. R2 represents the resistors on which the voltage is measured.
#define EXT_ADC_CHANNEL1_TO_CHECK_V_DIVIDER_MULTIPLIKATOR       (1.0f /(EXT_ADC_CHANNEL1_TO_CHECK_R2 / (EXT_ADC_CHANNEL1_TO_CHECK_R1 + EXT_ADC_CHANNEL1_TO_CHECK_R2)))

// Macro to be set by user. It defines the lower boundary of allowed voltages against which the associated voltage is checked.
#ifndef EXT_ADC_CHANNEL1_TO_CHECK_LIMIT_MIN_VOLT
#pragma message "EXT_ADC_CHANNEL1_TO_CHECK_LIMIT_MIN_VOLT is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL1_TO_CHECK_LIMIT_MIN_VOLT                (12.0f * 0.9f)
#endif

// Macro to be set by user. It defines the upper boundary of allowed voltages against which the associated voltage is checked.
#ifndef EXT_ADC_CHANNEL1_TO_CHECK_LIMIT_MAX_VOLT
#pragma message "EXT_ADC_CHANNEL1_TO_CHECK_LIMIT_MAX_VOLT is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL1_TO_CHECK_LIMIT_MAX_VOLT                (12.0f * 1.1f)
#endif

//------------------------------------------------------------------------------------------------------------------------

// -------------------------------- SECOND VOLTAGE EXTERNAL ADC ----------------------------------------------------------

#ifndef EXT_ADC_CHANNEL2_TO_CHECK_IS_12CHANNEL
#pragma message "EXT_ADC_CHANNEL2_TO_CHECK_IS_12CHANNEL is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL2_TO_CHECK_IS_12CHANNEL                  (false)
#endif

// Macro to be set by user. Defines from which channel of the external adc the voltage to check is coming. It can range from 0-11 for the 12-channel and from 0-3 for the 4-channel external adc.
#ifndef EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL
#pragma message "EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL                       (0)
#endif

#ifndef EXT_ADC_CHANNEL2_TO_CHECK_R1
#pragma message "R1 of the voltage divider associated with CHANNEL2 to check is not defined! Fallback to default values."
#define EXT_ADC_CHANNEL2_TO_CHECK_R1                            (0.0f)
#endif
#ifndef EXT_ADC_CHANNEL2_TO_CHECK_R2
#pragma message "R2 of the voltage divider associated with CHANNEL2 to check is not defined! Fallback to default values."
#define EXT_ADC_CHANNEL2_TO_CHECK_R2                            (1.0f)
#endif

// The external adc is mostly used with a voltage divider. The values of the used resistors shall be defined. R2 represents the resistors on which the voltage is measured.
#define EXT_ADC_CHANNEL2_TO_CHECK_V_DIVIDER_MULTIPLIKATOR       (1.0f /(EXT_ADC_CHANNEL2_TO_CHECK_R2 / (EXT_ADC_CHANNEL2_TO_CHECK_R1 + EXT_ADC_CHANNEL2_TO_CHECK_R2)))

// Macro to be set by user. It defines the lower boundary of allowed voltages against which the associated voltage is checked.
#ifndef EXT_ADC_CHANNEL2_TO_CHECK_LIMIT_MIN_VOLT
#pragma message "EXT_ADC_CHANNEL2_TO_CHECK_LIMIT_MIN_VOLT is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL2_TO_CHECK_LIMIT_MIN_VOLT                (12.0f * 0.9f)
#endif

// Macro to be set by user. It defines the upper boundary of allowed voltages against which the associated voltage is checked.
#ifndef EXT_ADC_CHANNEL2_TO_CHECK_LIMIT_MAX_VOLT
#pragma message "EXT_ADC_CHANNEL2_TO_CHECK_LIMIT_MAX_VOLT is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL2_TO_CHECK_LIMIT_MAX_VOLT                (12.0f * 1.1f)
#endif

//------------------------------------------------------------------------------------------------------------------------

// -------------------------------- THIRD VOLTAGE EXTERNAL ADC -----------------------------------------------------------

#ifndef EXT_ADC_CHANNEL3_TO_CHECK_IS_12CHANNEL
#pragma message "EXT_ADC_CHANNEL3_TO_CHECK_IS_12CHANNEL is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL3_TO_CHECK_IS_12CHANNEL                  (false)
#endif

// Macro to be set by user. Defines from which channel of the external adc the voltage to check is coming. It can range from 0-11 for the 12-channel and from 0-3 for the 4-channel external adc.
#ifndef EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL
#pragma message "EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL                       (0)
#endif

#ifndef EXT_ADC_CHANNEL3_TO_CHECK_R1
#pragma message "R1 of the voltage divider associated with CHANNEL3 to check is not defined! Fallback to default values."
#define EXT_ADC_CHANNEL3_TO_CHECK_R1                            (0.0f)
#endif
#ifndef EXT_ADC_CHANNEL3_TO_CHECK_R2
#pragma message "R2 of the voltage divider associated with CHANNEL3 to check is not defined! Fallback to default values."
#define EXT_ADC_CHANNEL3_TO_CHECK_R2                            (1.0f)
#endif

// The external adc is mostly used with a voltage divider. The values of the used resistors shall be defined. R2 represents the resistors on which the voltage is measured.
#define EXT_ADC_CHANNEL3_TO_CHECK_V_DIVIDER_MULTIPLIKATOR       (1.0f /(EXT_ADC_CHANNEL3_TO_CHECK_R2 / (EXT_ADC_CHANNEL3_TO_CHECK_R1 + EXT_ADC_CHANNEL3_TO_CHECK_R2)))

// Macro to be set by user. It defines the lower boundary of allowed voltages against which the associated voltage is checked.
#ifndef EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MIN_VOLT
#pragma message "EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MIN_VOLT is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MIN_VOLT                (12.0f * 0.9f)
#endif

// Macro to be set by user. It defines the upper boundary of allowed voltages against which the associated voltage is checked.
#ifndef EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MAX_VOLT
#pragma message "EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MAX_VOLT is not defined. Falling back to the default setting."
#define EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MAX_VOLT                (12.0f * 1.1f)
#endif

//------------------------------------------------------------------------------------------------------------------------
#endif // ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))


#ifndef SYSPWR_NUM_VALUES
    #define SYSPWR_NUM_VALUES   (8)
#endif

#ifndef SYSPWR_VCC_LOW_TIMEOUT
/// Zeit in Ticks bis eine Unterspannung als Fehler gemeldet wird.
/// Die Zeitkonstante, die sich durch die Filterung der Eingangwerte ergibt,
/// muss noch für die Gesamtzeit hinzugerechnet werden.
    #define SYSPWR_VCC_LOW_TIMEOUT (42)
#endif

/// Messmodus: Spannung UB wird \b vor dem Shunt gemessen.
#define SYSPWR_VCC_MEASURE_MODE_VERROR  (1)
/// Messmodus: Spannung UB wird \b nach dem Shunt gemessen.
#define SYSPWR_VCC_MEASURE_MODE_IERROR  (2)
#ifndef SYSPWR_VCC_MEASURE_MODE
    #define SYSPWR_VCC_MEASURE_MODE     SYSPWR_VCC_MEASURE_MODE_IERROR
#endif

#ifndef SYSPWR_STARTUP_DELAY
/// Wartezeit bevor die Prüfungen starten.
    #define SYSPWR_STARTUP_DELAY   (10)
#endif

/// Spannungsfaktor für Versorgungsspannung
#if !defined(VOLTAGE_VCC_FACTOR) & defined(fpADCIN_VCC)
#pragma message "VOLTAGE_VCC_FACTOR muss in der Platinendatei definiert werden."
#define VOLTAGE_VCC_FACTOR  (16.902f)           // 1566 (Spannungsteiler 47K/3K)
#endif

/// Stromfaktor
#if !defined(CURRENT_FACTOR) & defined(fpADCIN_ICC)
#pragma message "CURRENT_FACTOR muss in der Platinendatei definiert werden."
#define CURRENT_FACTOR      (0.17f)
#endif

/// Spannungsfaktoren für interne Spannungen
#ifndef VOLTAGE_1_FACTOR
    #define VOLTAGE_1_FACTOR    (1.000f)
#endif
#ifndef VOLTAGE_2_FACTOR
    #define VOLTAGE_2_FACTOR    (1.000f)
#endif
#ifndef VOLTAGE_3_FACTOR
    #define VOLTAGE_3_FACTOR    (1.000f)
#endif
#ifndef VOLTAGE_4_FACTOR
    #define VOLTAGE_4_FACTOR    (1.000f)
#endif
#ifndef VOLTAGE_5_FACTOR
    #define VOLTAGE_5_FACTOR    (1.000f)
#endif

/// Fehlergrenzwert-Einstellung für interne Spannung
#ifndef VOLTAGE_1_VALUE_VOLT
#define VOLTAGE_1_VALUE_VOLT          (1.000f)            ///< Sollwert interne Spannung VCC1
#endif
#ifndef VOLTAGE_1_DIFF_VOLT
#define VOLTAGE_1_DIFF_VOLT           (0.1f)              ///< Erlaubte Abweichung 0.1V
#endif
#define VOLTAGE_1_LIMIT_MIN_MILLIVOLT ((VOLTAGE_1_VALUE_VOLT - VOLTAGE_1_DIFF_VOLT) * DECIMAL_FIXPOINT)
#define VOLTAGE_1_LIMIT_MAX_MILLIVOLT ((VOLTAGE_1_VALUE_VOLT + VOLTAGE_1_DIFF_VOLT) * DECIMAL_FIXPOINT)

#ifndef VOLTAGE_2_VALUE_VOLT
#define VOLTAGE_2_VALUE_VOLT          (1.000f)            ///< Sollwert interne Spannung VCC2
#endif
#ifndef VOLTAGE_2_DIFF_VOLT
#define VOLTAGE_2_DIFF_VOLT           (0.1f)             ///< Erlaubte Abweichung 0.1V
#endif
#define VOLTAGE_2_LIMIT_MIN_MILLIVOLT ((VOLTAGE_2_VALUE_VOLT - VOLTAGE_2_DIFF_VOLT) * DECIMAL_FIXPOINT)
#define VOLTAGE_2_LIMIT_MAX_MILLIVOLT ((VOLTAGE_2_VALUE_VOLT + VOLTAGE_2_DIFF_VOLT) * DECIMAL_FIXPOINT)

#ifndef VOLTAGE_3_VALUE_VOLT
#define VOLTAGE_3_VALUE_VOLT          (1.000f)            ///< Sollwert interne Spannung VCC3
#endif
#ifndef VOLTAGE_3_DIFF_VOLT
#define VOLTAGE_3_DIFF_VOLT           (0.1f)              ///< Erlaubte Abweichung 0.1V
#endif
#define VOLTAGE_3_LIMIT_MIN_MILLIVOLT ((VOLTAGE_3_VALUE_VOLT - VOLTAGE_3_DIFF_VOLT) * DECIMAL_FIXPOINT)
#define VOLTAGE_3_LIMIT_MAX_MILLIVOLT ((VOLTAGE_3_VALUE_VOLT + VOLTAGE_3_DIFF_VOLT) * DECIMAL_FIXPOINT)

#ifndef VOLTAGE_4_VALUE_VOLT
#define VOLTAGE_4_VALUE_VOLT          (1.000f)            ///< Sollwert interne Spannung VCC4
#endif
#ifndef VOLTAGE_4_DIFF_VOLT
#define VOLTAGE_4_DIFF_VOLT           (0.1f)              ///< Erlaubte Abweichung 0.1V
#endif
#define VOLTAGE_4_LIMIT_MIN_MILLIVOLT ((VOLTAGE_4_VALUE_VOLT - VOLTAGE_4_DIFF_VOLT) * DECIMAL_FIXPOINT)
#define VOLTAGE_4_LIMIT_MAX_MILLIVOLT ((VOLTAGE_4_VALUE_VOLT + VOLTAGE_4_DIFF_VOLT) * DECIMAL_FIXPOINT)

#ifndef VOLTAGE_5_VALUE_VOLT
#define VOLTAGE_5_VALUE_VOLT          (1.000f)            ///< Sollwert interne Spannung VCC5
#endif
#ifndef VOLTAGE_5_DIFF_VOLT
#define VOLTAGE_5_DIFF_VOLT           (0.1f)              ///< Erlaubte Abweichung 0.1V
#endif
#define VOLTAGE_5_LIMIT_MIN_MILLIVOLT ((VOLTAGE_5_VALUE_VOLT - VOLTAGE_5_DIFF_VOLT) * DECIMAL_FIXPOINT)
#define VOLTAGE_5_LIMIT_MAX_MILLIVOLT ((VOLTAGE_5_VALUE_VOLT + VOLTAGE_5_DIFF_VOLT) * DECIMAL_FIXPOINT)

// Maximum power levels
#if defined(fpADCIN_ICC) && defined(fpADCIN_VCC)

#ifndef POWER_LIMIT_MAX_WATT
/// If the measured power exceeds this value, enter the hard error state.
#define POWER_LIMIT_MAX_WATT        (3.5f)
#error "Please define a value for POWER_LIMIT_MAX_WATT in your Project."
#endif
#define POWER_LIMIT_MAX_MILLIWATT   (POWER_LIMIT_MAX_WATT * DECIMAL_FIXPOINT)

#ifndef POWER_LIMIT_WARNING_MAX_WATT
/// If the measured power exceeds this value, raise a warning.
#define POWER_LIMIT_WARNING_MAX_WATT     (3.4f)
#error "Please define a value for POWER_LIMIT_WARNING_MAX_WATT in your Project."
#endif

#endif // defined(fpADCIN_ICC) && defined(fpADCIN_VCC)

/// System power voltage status
typedef enum
{
    eSYSPWR_STAT_INIT_OK = 0x01,                //!< Initialization successful
    eSYSPWR_STAT_VCC_VALID = 0x02,              //!< Supply voltage measurement valid
    eSYSPWR_STAT_I_VALID = 0x04,                //!< Current measurement valid
    eSYSPWR_STAT_DETECT_VCC_LOW = 0x08,         //!< Detect supply voltage below error level
    eSYSPWR_STAT_ERROR_VCC_LOW = 0x10,          //!< Error state supply voltage too low
    eSYSPWR_STAT_ERROR_VCC_DROPOUT = 0x20,      //!< Error state supply voltage dropout
    eSYSPWR_STAT_ERROR_VCC_HIGH = 0x40,         //!< Error state supply voltage too high
    eSYSPWR_STAT_WARNING_VCC_HIGH = 0x80,       //!< Warning state supply voltage too high
    eSYSPWR_STAT_WARNING_VCC_LOW = 0x100,       //!< Warning state supply voltage too low
    eSYSPWR_STAT_ERROR_VCC1_LOW = 0x200,        //!< Error state internal voltage 1 too low
    eSYSPWR_STAT_ERROR_VCC1_HIGH = 0x400,       //!< Error state internal voltage 1 too high
    eSYSPWR_STAT_ERROR_VCC2_LOW = 0x800,        //!< Error state internal voltage 2 too low
    eSYSPWR_STAT_ERROR_VCC2_HIGH = 0x1000,      //!< Error state internal voltage 2 too high
    eSYSPWR_STAT_ERROR_VCC3_LOW = 0x2000,       //!< Error state internal voltage 3 too low
    eSYSPWR_STAT_ERROR_VCC3_HIGH = 0x4000,      //!< Error state internal voltage 3 too high
    eSYSPWR_STAT_ERROR_VCC4_LOW = 0x8000,       //!< Error state internal voltage 4 too low
    eSYSPWR_STAT_ERROR_VCC4_HIGH = 0x10000,     //!< Error state internal voltage 4 too high
    eSYSPWR_STAT_ERROR_VCC5_LOW = 0x20000,      //!< Error state internal voltage 5 too low
    eSYSPWR_STAT_ERROR_VCC5_HIGH = 0x40000,     //!< Error state internal voltage 5 too high
    eSYSPWR_STAT_ERROR_POWER_HIGH = 0x80000,    //!< Power consumption error, too high
    eSYSPWR_STAT_WARNING_POWER_HIGH = 0x100000, //!< Power consumption warning, too high
    eSYSPWR_STAT_ERROR_EXT_ADC_CH1_HIGH = 0x200000, //!< Error state external ADC voltage of first defined channel is to high
    eSYSPWR_START_ERROR_EXT_ADC_CH1_LOW = 0x400000, //!< Error state external ADC voltage of first defined channel is to low
    eSYSPWR_STAT_ERROR_EXT_ADC_CH2_HIGH = 0x800000, //!< Error state external ADC voltage of second defined channel is to high
    eSYSPWR_START_ERROR_EXT_ADC_CH2_LOW = 0x1000000, //!< Error state external ADC voltage of second defined channel is to low
    eSYSPWR_STAT_ERROR_EXT_ADC_CH3_HIGH = 0x2000000, //!< Error state external ADC voltage of third defined channel is to high
    eSYSPWR_START_ERROR_EXT_ADC_CH3_LOW = 0x4000000, //!< Error state external ADC voltage of third defined channel is to low
} SYSPWR_STAT;

// Allgemeine Definitionen -------------------------------------------------

#if defined(fpADCIN_TEMPERATURE) && defined(TMP144_UART_CHANNEL)
#error "Die gleichzeitige Verwendung des analogen und digitalen Temperatursensors wird noch nicht unterstützt."
#endif

// externe Variablen -------------------------------------------------------

// 1. Überwachungskanal
// Versorgungsspannung
#ifdef fpADCIN_VCC
static TAVG_CALC tVCCAvg;
static S32 alVCCValues[SYSPWR_NUM_VALUES];
static U32 vccLowVoltageTimeout;
static U32 vccLimitMinMillivolt;
static U32 vccLimitMaxMillivolt;
#endif

// 2. Ueberwachungskanal
// Interne Spannung
#ifdef fpADCIN_VCC1
static TAVG_CALC tVCC1Avg;
static S32 alVCC1Values[SYSPWR_NUM_VALUES];
static S32 lPowerVoltage1;
#endif

// 3. Ueberwachungskanal
// Interne Spannung
#ifdef fpADCIN_VCC2
static TAVG_CALC tVCC2Avg;
static S32 alVCC2Values[SYSPWR_NUM_VALUES];
static S32 lPowerVoltage2;
#endif

// 4. Ueberwachungskanal
// Interne Spannung
#ifdef fpADCIN_VCC3
static TAVG_CALC tVCC3Avg;
static S32 alVCC3Values[SYSPWR_NUM_VALUES];
static S32 lPowerVoltage3;
#endif

// 5. Ueberwachungskanal
// Interne Spannung
#ifdef fpADCIN_VCC4
static TAVG_CALC tVCC4Avg;
static S32 alVCC4Values[SYSPWR_NUM_VALUES];
static S32 lPowerVoltage4;
#endif

// 6. Ueberwachungskanal
// Interne Spannung
#ifdef fpADCIN_VCC5
static TAVG_CALC tVCC5Avg;
static S32 alVCC5Values[SYSPWR_NUM_VALUES];
static S32 lPowerVoltage5;
#endif

#if ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))
static TAVG_CALC_F32 tVCCExternalAdcChannel1AvgInstance;
static F32 tVCCExternalAdcChannel1Values[SYSPWR_NUM_VALUES];
F32 tVCCExternalAdcChannel1Avg;

static TAVG_CALC_F32 tVCCExternalAdcChannel2AvgInstance;
static F32 tVCCExternalAdcChannel2Values[SYSPWR_NUM_VALUES];
F32 tVCCExternalAdcChannel2Avg;

static TAVG_CALC_F32 tVCCExternalAdcChannel3AvgInstance;
static F32 tVCCExternalAdcChannel3Values[SYSPWR_NUM_VALUES];
F32 tVCCExternalAdcChannel3Avg;
#endif // ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))

#ifdef fpADCIN_ICC
static TAVG_CALC tIAvg;
static S32 alIValues[SYSPWR_NUM_VALUES];
#endif

#ifdef TMP144_UART_CHANNEL
static float temperatureValue;

/// User configuration parameters (SOFTQM-431)
/// Min and max Temperature values
#ifndef TEMPERATURE_ERROR_MAX
/// If the measured temperature exceeds this value, enter the permanent hard error state.
#define TEMPERATURE_ERROR_MAX      (85.0f)
#error "Please define a value for TEMPERATURE_ERROR_MAX in your Project."
#endif
#ifndef TEMPERATURE_WARNING_MAX
/// If the measured temperature exceeds this value, a warning will be raised.
#define TEMPERATURE_WARNING_MAX    (80.0f)
#error "Please define a value for TEMPERATURE_WARNING_MAX in your Project."
#endif
#ifndef TEMPERATURE_WARNING_MIN
/// If the measured temperature drops below value, a warning will be raised.
#define TEMPERATURE_WARNING_MIN    (-35.0f)
#error "Please define a value for TEMPERATURE_WARNING_MIN in your Project."
#endif
#ifndef TEMPERATURE_ERROR_MIN
/// If the measured temperature drops below value, enter the permanent hard error state.
#define TEMPERATURE_ERROR_MIN      (-40.0f)
#error "Please define a value for TEMPERATURE_ERROR_MIN in your Project."
#endif

/// Number of ticks to wait for a valid temperature value provided by the temperature sensor.
#ifndef WAIT_TMP144_STARTUP_IN_SAFETYCYCLE_TICKS
/// Default value 150 ticks which corresponds to 150 ms if the temperature is checked every ms.
#define WAIT_TMP144_STARTUP_IN_SAFETYCYCLE_TICKS  (150)
#endif

/// Counter to wait for a valid temperature value from the temperature sensor
static U32 waitStartupTmpSensorTolerance;
#endif
#if (MAX116XX_FEAT_4CHANNEL_ADC || MAX116XX_FEAT_12CHANNEL_ADC)
#define WAIT_EXTERNAL_ADC_STARTUP_IN_SAFETYCYCLE_TICKS      (20)
static U32 waitStartupExternalAdcTolerance = WAIT_EXTERNAL_ADC_STARTUP_IN_SAFETYCYCLE_TICKS;
#endif

/// System power voltage status
static SYSPWR_STAT sysPowerStat;

static U32 startupDelayCount;
static SAFETY_POWERSUPPLY_CONFIG * powerSupplyUserConfig = NULL;

#ifdef fpADCIN_VCC
static S32 lPowerVoltage;
#endif
#ifdef fpADCIN_ICC
static S32 lPowerCurrent;
#endif
#if defined(fpADCIN_ICC) && defined(fpADCIN_VCC)
static U32 ulPower;
#endif
#if defined(fpADCIN_TEMPERATURE) || defined(TMP144_UART_CHANNEL)
static S32 systemTemperature;

/// System temperature status
static SYSTEM_TEMPERATURE_STATUS sysTemperatureStat;
#endif

#if FEAT_MSG_INTERPRETER
#ifdef fpADCIN_VCC
static T_RAM_VAR_ENTRY lPowerVoltageRamVar;
#endif
#ifdef fpADCIN_VCC1
static T_RAM_VAR_ENTRY lPowerVoltage1RamVar;
#endif
#ifdef fpADCIN_VCC2
static T_RAM_VAR_ENTRY lPowerVoltage2RamVar;
#endif
#ifdef fpADCIN_VCC3
static T_RAM_VAR_ENTRY lPowerVoltage3RamVar;
#endif
#ifdef fpADCIN_VCC4
static T_RAM_VAR_ENTRY lPowerVoltage4RamVar;
#endif
#ifdef fpADCIN_VCC5
static T_RAM_VAR_ENTRY lPowerVoltage5RamVar;
#endif
#ifdef fpADCIN_ICC
static T_RAM_VAR_ENTRY lPowerCurrentRamVar;
#endif
#if defined(fpADCIN_ICC) && defined(fpADCIN_VCC)
static T_RAM_VAR_ENTRY lPowerRamVar;
#endif
#if defined(fpADCIN_TEMPERATURE) || defined(TMP144_UART_CHANNEL)
static T_RAM_VAR_ENTRY systemTemperatureRamVar;
#endif
#endif


static ERROR_CODE_FORMAT errorCode;

// Funktionsbereich --------------------------------------------------------

/// @author m.neubauer @date 07.08.2013
bool Safety_Powersuply_Init(SAFETY_POWERSUPPLY_CONFIG * const safetyPowerSupplyConfig)
    {
    bool result;
    bool initChannelResult;

    if(safetyPowerSupplyConfig == NULL)
        {
        return false;
        }

    powerSupplyUserConfig = safetyPowerSupplyConfig;

    result = true;

    if(safetyPowerSupplyConfig->supplyVoltageIsActive)
        {
        initChannelResult = false;

#ifdef fpADCIN_VCC
        if(safetyPowerSupplyConfig->supplyVoltageIsActive)
            {
            if(ADC_InitSingleChannel(fpADCIN_VCC) == eADC_TRUE)
                {
                initChannelResult = true;
                }

            if((initChannelResult) && (AVG_Init(&tVCCAvg, alVCCValues, sizeof(alVCCValues),
                                                 SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR))
                {
                initChannelResult = false;
                }

            vccLowVoltageTimeout = SYSPWR_VCC_LOW_TIMEOUT;

#ifdef PARNUM_VOLTAGE_VCC_VALUE_MIN
            if((initChannelResult) && (ParTab_GetValue(PARNUM_VOLTAGE_VCC_VALUE_MIN,
                                                       vccLimitMinMillivolt) != PARTAB_ERR_NONE))
                {
                initChannelResult = false;
                }
            if((initChannelResult) && (ParTab_GetValue(PARNUM_VOLTAGE_VCC_VALUE_MAX,
                                                       vccLimitMaxMillivolt) != PARTAB_ERR_NONE))
                {
                initChannelResult = false;
                }
#else
            vccLimitMinMillivolt = VOLTAGE_SUPPLY_LIMIT_MIN_MILLIVOLT;
            vccLimitMaxMillivolt = VOLTAGE_SUPPLY_LIMIT_MAX_MILLIVOLT;

#endif /* PARNUM_VOLTAGE_VCC_VALUE_MIN */

#if FEAT_MSG_INTERPRETER
            if((initChannelResult) && !MsgIntp_CreateRamVar(&lPowerVoltageRamVar, "Powersupply: Voltage In (mV)", 0,
                                                            sizeof(lPowerVoltage), &lPowerVoltage))
                {
                initChannelResult = false;
                }
#endif /* FEAT_MSG_INTERPRETER */
            }
#endif /* fpADCIN_VCC */

        if(initChannelResult == false)
            {
            result = false;
            }
        }

    if(safetyPowerSupplyConfig->currentIsActive)
        {
        initChannelResult = false;

#ifdef fpADCIN_ICC
        if(ADC_InitSingleChannel(fpADCIN_ICC) == eADC_TRUE)
            {
            initChannelResult = true;
            }

        if((initChannelResult) && (AVG_Init(&tIAvg, alIValues, sizeof(alIValues),
                                            SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR))
            {
            initChannelResult = false;
            }

#if FEAT_MSG_INTERPRETER
        if((initChannelResult) && !MsgIntp_CreateRamVar(&lPowerCurrentRamVar, "Powersupply: Current In (mA)", 0,
                                                        sizeof(lPowerCurrent), &lPowerCurrent))
            {
            initChannelResult = false;
            }
#endif /* FEAT_MSG_INTERPRETER */
#endif /* fpADCIN_ICC */

        if(initChannelResult == false)
            {
            result = false;
            }
        }

    if(safetyPowerSupplyConfig->voltage1IsActive)
        {
        initChannelResult = false;

#ifdef fpADCIN_VCC1
        if(ADC_InitSingleChannel(fpADCIN_VCC1) == eADC_TRUE)
            {
            initChannelResult = true;
            }

        if((initChannelResult) && (AVG_Init(&tVCC1Avg, alVCC1Values, sizeof(alVCC1Values),
                                            SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR))
            {
            initChannelResult = false;
            }

#if FEAT_MSG_INTERPRETER
        if((initChannelResult) && !MsgIntp_CreateRamVar(&lPowerVoltage1RamVar, "Powersupply: Voltage 1 In (mV)", 0,
                                                       sizeof(lPowerVoltage1), &lPowerVoltage1))
            {
            initChannelResult = false;
            }
#endif /* FEAT_MSG_INTERPRETER */
#endif /* fpADCIN_VCC1 */

        if(initChannelResult == false)
            {
            result = false;
            }
        }

    if(safetyPowerSupplyConfig->voltage2IsActive)
        {
        initChannelResult = false;
#ifdef fpADCIN_VCC2
        if(ADC_InitSingleChannel(fpADCIN_VCC2) == eADC_TRUE)
            {
            initChannelResult = true;
            }

        if((initChannelResult) && (AVG_Init(&tVCC2Avg, alVCC2Values, sizeof(alVCC2Values),
                                            SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR))
            {
            initChannelResult = false;
            }

#if FEAT_MSG_INTERPRETER
        if((initChannelResult) && !MsgIntp_CreateRamVar(&lPowerVoltage2RamVar, "Powersupply: Voltage 2 In (mV)", 0,
                                                        sizeof(lPowerVoltage2), &lPowerVoltage2))
            {
            initChannelResult = false;
            }
#endif /* FEAT_MSG_INTERPRETER */
#endif /* fpADCIN_VCC2 */

        if(initChannelResult == false)
            {
            result = false;
            }
        }

    if(safetyPowerSupplyConfig->voltage3IsActive)
        {
        initChannelResult = false;
#ifdef fpADCIN_VCC3
        if(ADC_InitSingleChannel(fpADCIN_VCC3) == eADC_TRUE)
            {
            initChannelResult = true;
            }

        if((initChannelResult) && (AVG_Init(&tVCC3Avg, alVCC3Values, sizeof(alVCC3Values),
                                            SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR))
            {
            initChannelResult = false;
            }

#if FEAT_MSG_INTERPRETER
        if((initChannelResult) && !MsgIntp_CreateRamVar(&lPowerVoltage3RamVar, "Powersupply: Voltage 3 In (mV)", 0,
                                                        sizeof(lPowerVoltage3), &lPowerVoltage3))
            {
            initChannelResult = false;
            }
#endif /* FEAT_MSG_INTERPRETER */
#endif /* fpADCIN_VCC3 */

        if(initChannelResult == false)
            {
            result = false;
            }
        }

    if(safetyPowerSupplyConfig->voltage4IsActive)
        {
        initChannelResult = false;

#ifdef fpADCIN_VCC4
        if(ADC_InitSingleChannel(fpADCIN_VCC4) == eADC_TRUE)
            {
            initChannelResult = true;
            }

        if((initChannelResult) && (AVG_Init(&tVCC4Avg, alVCC4Values, sizeof(alVCC4Values),
                                            SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR))
            {
            initChannelResult = false;
            }

#if FEAT_MSG_INTERPRETER
        if((initChannelResult) && !MsgIntp_CreateRamVar(&lPowerVoltage4RamVar, "Powersupply: Voltage 4 In (mV)", 0,
                                                        sizeof(lPowerVoltage4), &lPowerVoltage4))
            {
            initChannelResult = false;
            }
#endif /* FEAT_MSG_INTERPRETER */
#endif /* fpADCIN_VCC4 */

        if(initChannelResult == false)
            {
            result = false;
            }
        }

    if(safetyPowerSupplyConfig->voltage5IsActive)
        {
        initChannelResult = false;

#ifdef fpADCIN_VCC5
        if(ADC_InitSingleChannel(fpADCIN_VCC5) == eADC_TRUE)
            {
            initChannelResult = true;
            }

        if((initChannelResult) && (AVG_Init(&tVCC5Avg, alVCC5Values, sizeof(alVCC5Values),
                                            SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR))
            {
            initChannelResult = false;
            }

#if FEAT_MSG_INTERPRETER
        if((initChannelResult) && !MsgIntp_CreateRamVar(&lPowerVoltage5RamVar, "Powersupply: Voltage 5 In (mV)", 0,
                                                        sizeof(lPowerVoltage5), &lPowerVoltage5))
            {
            initChannelResult = false;
            }
#endif /* FEAT_MSG_INTERPRETER */
#endif /* fpADCIN_VCC5 */

        if(initChannelResult == false)
            {
            result = false;
            }
        }
//--------------------- Block external ADC -----------------------
#if ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))
    if(safetyPowerSupplyConfig->voltageExternalAdcChannel1IsActive)
        {
        initChannelResult = false;
        }

    if(AVG_InitF32(&tVCCExternalAdcChannel1AvgInstance, tVCCExternalAdcChannel1Values, sizeof(tVCCExternalAdcChannel1Values), SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR)
        {
        initChannelResult = false;
        }
    if(AVG_InitF32(&tVCCExternalAdcChannel2AvgInstance, tVCCExternalAdcChannel2Values, sizeof(tVCCExternalAdcChannel2Values), SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR)
        {
        initChannelResult = false;
        }
    if(AVG_InitF32(&tVCCExternalAdcChannel3AvgInstance, tVCCExternalAdcChannel3Values, sizeof(tVCCExternalAdcChannel3Values), SYSPWR_NUM_VALUES) != eAVERAGING_NO_ERROR)
        {
        initChannelResult = false;
        }
#endif // ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))
//----------------------------------------------------------------

    if(safetyPowerSupplyConfig->temperatureAdcIsActive)
        {
        initChannelResult = false;

#ifdef fpADCIN_TEMPERATURE
        if(ADC_InitSingleChannel(fpADCIN_TEMPERATURE) == eADC_TRUE)
            {
            initChannelResult = true;
            }
#endif
        if(initChannelResult == false)
            {
            result = false;
            }
        }

    if(safetyPowerSupplyConfig->temperatureSensorIsActive)
        {
        initChannelResult = false;

#ifdef TMP144_UART_CHANNEL
        waitStartupTmpSensorTolerance = WAIT_TMP144_STARTUP_IN_SAFETYCYCLE_TICKS;
        initChannelResult = true;
#endif

        if(initChannelResult == false)
            {
            result = false;
            }
        }

#if FEAT_MSG_INTERPRETER
#if defined(fpADCIN_ICC) && defined(fpADCIN_VCC)
    if(safetyPowerSupplyConfig->currentIsActive && safetyPowerSupplyConfig->supplyVoltageIsActive)
        {
        if((result) && !MsgIntp_CreateRamVar(&lPowerRamVar, "Powersupply: Power (mW)", 0,
                                             sizeof(ulPower), &ulPower))
            {
            result = false;
            }
        }
#endif /* fpADCIN_ICC, fpADCIN_VCC */

#if defined(fpADCIN_TEMPERATURE) || defined(TMP144_UART_CHANNEL)
    if(safetyPowerSupplyConfig->temperatureAdcIsActive || safetyPowerSupplyConfig->temperatureSensorIsActive)
        {
        if((result) && !MsgIntp_CreateRamVar(&systemTemperatureRamVar, "uC Chip Temperature", 0,
                                             sizeof(systemTemperature), &systemTemperature))
            {
            result = false;
            }
        }
#endif /* fpADCIN_TEMPERATURE, TMP144_UART_CHANNEL */

#endif /*FEAT_MSG_INTERPRETER*/

    startupDelayCount = SYSPWR_STARTUP_DELAY;
    sysPowerStat = eSYSPWR_STAT_INIT_OK;
    errorCode.value = 0;

    return result;
    }
//------------------------------------------------------------------------------

/// @author m.neubauer @date 07.08.2013
U8 Safety_Powersupply_Check(void)
    {
#ifdef fpADCIN_VCC
    U32 ulVoltage;
    float fVoltage;
#endif
#ifdef fpADCIN_ICC
    U32 ulCurrent;
    float fCurrent;
#endif
#ifdef fpADCIN_VCC1
    U32 ulVoltage1;
    float fVoltage1;
#endif
#ifdef fpADCIN_VCC2
    U32 ulVoltage2;
    float fVoltage2;
#endif
#ifdef fpADCIN_VCC3
    U32 ulVoltage3;
    float fVoltage3;
#endif
#ifdef fpADCIN_VCC4
    U32 ulVoltage4;
    float fVoltage4;
#endif
#ifdef fpADCIN_VCC5
    U32 ulVoltage5;
    float fVoltage5;
#endif
#ifdef fpADCIN_TEMPERATURE
    float temperatureVSense;
#endif


#if ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))
MAX116XX_ADC_VALUES externalAdcValuesList[MAX116XX_NUMBER_OF_ADCS];
#endif // ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))


    //Startverzögerung
    if(startupDelayCount > 0)
        {
        startupDelayCount--;
        return TRUE;
        }

    if(powerSupplyUserConfig == NULL)
        {
        return false;
        }

    // Auswertung Strom
#ifdef fpADCIN_ICC
    if(powerSupplyUserConfig->currentIsActive)
        {
        // Messwertaufnahme
        if(ADC_SampleSingleChannel(fpADCIN_ICC, &fCurrent) != eADC_TRUE)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        // Umwandlung in Integer
        ulCurrent = (U32)(fCurrent * DECIMAL_FIXPOINT * CURRENT_FACTOR);

        // Mittelwertberechnung
        if(AVG_Update(&tIAvg, ulCurrent) != eAVERAGING_NO_ERROR)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        if(AVG_GetIsValid(&tIAvg.data))
            {
            if(AVG_Get(&tIAvg, (S32*) &ulCurrent) != eAVERAGING_NO_ERROR)
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }

            // In RAM-Variable für Factory Device Communication eintragen
            lPowerCurrent = ulCurrent;
            sysPowerStat |= eSYSPWR_STAT_I_VALID;
            }
        }

#endif

    // Auswertung Vcc
#ifdef fpADCIN_VCC
    if(powerSupplyUserConfig->supplyVoltageIsActive)
        {
        // Messwertaufnahme
        if(ADC_SampleSingleChannel(fpADCIN_VCC, &fVoltage) != eADC_TRUE)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        ulVoltage = (U32)(fVoltage * DECIMAL_FIXPOINT * VOLTAGE_VCC_FACTOR);

#if defined(fpADCIN_ICC) && (SYSPWR_VCC_MEASURE_MODE == SYSPWR_VCC_MEASURE_MODE_VERROR)
            // UB wird vor dem Shunt gemessen. Spannungsabfall über dem Shunt
            // berücksichtigen.
            if(sysPowerStat & eSYSPWR_STAT_I_VALID)
                {
                ulVoltage -= ((U32)(INA168_SHUNT_RESISTOR * lPowerCurrent));

                // Mittelwertberechnung
                if(AVG_Update(&tVCCAvg, ulVoltage) != eAVERAGING_NO_ERROR)
                    {
                    // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                    Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                    }
                }
#else
            // Mittelwertberechnung
            if(AVG_Update(&tVCCAvg, ulVoltage) != eAVERAGING_NO_ERROR)
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }
#endif

            if(AVG_GetIsValid(&tVCCAvg.data))
                {
                if(AVG_Get(&tVCCAvg, (S32*) &ulVoltage) != eAVERAGING_NO_ERROR)
                    {
                    // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                    Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                    }

                // RAM-Variable für Factory Device Communication aktualisieren
                lPowerVoltage = ulVoltage;
                sysPowerStat |= eSYSPWR_STAT_VCC_VALID;

                // Check supply voltage error limits (SOFTQM-596)
                if(ulVoltage < vccLimitMinMillivolt)
                    {
                    // Supply voltage decreases below minimum error limit
                    sysPowerStat |= eSYSPWR_STAT_DETECT_VCC_LOW;

                    // Check if delay time for minimum voltage error is exceeded (SOFTQM-596)
                    if(vccLowVoltageTimeout > 0)
                        {
                        vccLowVoltageTimeout--;
                        }
                    else
                        {
                        if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC_LOW) == 0)
                            {
                            sysPowerStat |= eSYSPWR_STAT_ERROR_VCC_LOW;

                            // Send error event with supply voltage error if the voltage is below
                            // the minimum error level for the delay time (SOFTQM-596, SOFTQM-657)
                            SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_SUPPLY_VOLTAGE, eERROR_VOLTAGE_EXCEEDED_MIN,
                                              ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                            }
                        }
                    }
                else if(ulVoltage > vccLimitMaxMillivolt)
                    {
                    // Supply voltage is above maximum error limit
                    if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC_HIGH) == 0)
                        {
                        // Enter permanent hard error if voltage exceeds maximum error level (SOFTQM-636)
                        sysPowerStat |= eSYSPWR_STAT_ERROR_VCC_HIGH;
                        Safety_PermanentHardError(HARD_ERR_VOLTAGE_EXCEEDED);
                        }
                    }
                else
                    {
                    // Supply voltage within allowed voltage range, check if it was below
                    // minimum error limit previously (SOFTQM-596)

                    if((sysPowerStat & eSYSPWR_STAT_DETECT_VCC_LOW)
                            && !(sysPowerStat & eSYSPWR_STAT_ERROR_VCC_DROPOUT))
                        {
                        sysPowerStat |= eSYSPWR_STAT_ERROR_VCC_DROPOUT;

                        // Send error event with supply voltage dropout error was previously detected to
                        // be below the minimum error limit and the allowed voltage range is reentered (SOFTQM-657)
                        SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_SUPPLY_VOLTAGE, eERROR_VOLTAGE_VCC_DROPOUT,
                                          ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                        }
                    }

                // Check if supply voltage is below warning limit (SOFTQM-596)
                if((ulVoltage < (VOLTAGE_SUPPLY_WARNING_MIN_VOLT * DECIMAL_FIXPOINT)) &&
                   !(sysPowerStat & eSYSPWR_STAT_WARNING_VCC_LOW))
                    {
                    // Send warning (SOFTQM-638)
                    sysPowerStat |= eSYSPWR_STAT_WARNING_VCC_LOW;
                    SendErrorMsgEvent(eEVENT_VCC_CHECK_WARNING, eERROR_SUPPLY_VOLTAGE, eERROR_VOLTAGE_EXCEEDED_MIN,
                                      ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                    }

                // Check if supply voltage is above warning limit (SOFTQM-596)
                if((ulVoltage > (VOLTAGE_SUPPLY_WARNING_MAX_VOLT * DECIMAL_FIXPOINT)) &&
                   !(sysPowerStat & eSYSPWR_STAT_WARNING_VCC_HIGH))
                    {
                    // Send warning (SOFTQM-638)
                    sysPowerStat |= eSYSPWR_STAT_WARNING_VCC_HIGH;
                    SendErrorMsgEvent(eEVENT_VCC_CHECK_WARNING, eERROR_SUPPLY_VOLTAGE, eERROR_VOLTAGE_EXCEEDED_MAX,
                                      ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                    }
                }

        }

#endif

// Auswertung 2. Versorgung
#ifdef fpADCIN_VCC1
    if(powerSupplyUserConfig->voltage1IsActive)
        {
        if(ADC_SampleSingleChannel(fpADCIN_VCC1, &fVoltage1) != eADC_TRUE)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        // Umwandlung in Integer
        ulVoltage1 = (U32)(fVoltage1 * DECIMAL_FIXPOINT * VOLTAGE_1_FACTOR);

        // Mittelwertberechnung
        if(AVG_Update(&tVCC1Avg, ulVoltage1) != eAVERAGING_NO_ERROR)
             {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
             Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
             }

        if(AVG_GetIsValid(&tVCC1Avg.data))
            {
            if(AVG_Get(&tVCC1Avg, (S32*) &ulVoltage1) != eAVERAGING_NO_ERROR)
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }

            // Spannung in RAM-Variable für Factory Device Communication eintragen
            lPowerVoltage1 = ulVoltage1;

            // Check if voltage is below error level (SOFTQM-602)
            if((ulVoltage1 < VOLTAGE_1_LIMIT_MIN_MILLIVOLT))
                {
                // Only send error if supply voltage is not below error level (SOFTQM-602)
                if((sysPowerStat & eSYSPWR_STAT_DETECT_VCC_LOW) == 0)
                    {
                    if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC1_LOW) == 0)
                        {
                        sysPowerStat |= eSYSPWR_STAT_ERROR_VCC1_LOW;
                        // Send error event if the voltage is below the minimum error level (SOFTQM-657)
                        SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_INTERNAL_VOLTAGE_1, eERROR_VOLTAGE_EXCEEDED_MIN,
                                          ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                        }
                    }
                }

            // Check if voltage is above error level (SOFTQM-602)
            if((ulVoltage1 > VOLTAGE_1_LIMIT_MAX_MILLIVOLT))
                {
                if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC1_HIGH) == 0)
                    {
                    // Send a permanent hard error if voltage exceeds maximum error level (SOFTQM-636)
                    sysPowerStat |= eSYSPWR_STAT_ERROR_VCC1_HIGH;
                    Safety_PermanentHardError(HARD_ERR_VOLTAGE_EXCEEDED);
                    }
                }
            }
        }
#endif

// Auswertung 3. Versorgung
#ifdef fpADCIN_VCC2
    if(powerSupplyUserConfig->voltage2IsActive)
        {
        // Messwertaufnahme
        if(ADC_SampleSingleChannel(fpADCIN_VCC2, &fVoltage2) != eADC_TRUE)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        // Umwandlung in Integer
        ulVoltage2 = (U32)(fVoltage2 * DECIMAL_FIXPOINT * VOLTAGE_2_FACTOR);

        // Mittwelwertberechnung
        if(AVG_Update(&tVCC2Avg, ulVoltage2) != eAVERAGING_NO_ERROR)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        if(AVG_GetIsValid(&tVCC2Avg.data))
            {
            if(AVG_Get(&tVCC2Avg, (S32*) &ulVoltage2) != eAVERAGING_NO_ERROR)
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }

            // Spannung in RAM-Variable für Factory Device Communication eintragen
            lPowerVoltage2 = ulVoltage2;

            // Check if voltage is below error level (SOFTQM-602)
            if((ulVoltage2 < VOLTAGE_2_LIMIT_MIN_MILLIVOLT))
                {
                // Only send error if supply voltage is not below error level (SOFTQM-602)
                if((sysPowerStat & eSYSPWR_STAT_DETECT_VCC_LOW) == 0)
                    {
                    if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC2_LOW) == 0)
                        {
                        sysPowerStat |= eSYSPWR_STAT_ERROR_VCC2_LOW;
                        // Send error event if the voltage is below the minimum error level (SOFTQM-657)
                        SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_INTERNAL_VOLTAGE_2, eERROR_VOLTAGE_EXCEEDED_MIN,
                                          ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                        }
                    }
                }

            // Check if voltage is above error level (SOFTQM-602)
            if((ulVoltage2 > VOLTAGE_2_LIMIT_MAX_MILLIVOLT))
                {
                if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC2_HIGH) == 0)
                    {
                    // Send a permanent hard error if voltage exceeds maximum error level (SOFTQM-636)
                    sysPowerStat |= eSYSPWR_STAT_ERROR_VCC2_HIGH;
                    Safety_PermanentHardError(HARD_ERR_VOLTAGE_EXCEEDED);
                    }
                }
            }
        }
#endif

// Auswertung 3. Versorgung
#ifdef fpADCIN_VCC3
    if(powerSupplyUserConfig->voltage3IsActive)
        {
        // Messwertaufnahme
        if(ADC_SampleSingleChannel(fpADCIN_VCC3, &fVoltage3) != eADC_TRUE)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        // Umwandlung in Integer
        ulVoltage3 = (U32)(fVoltage3 * DECIMAL_FIXPOINT * VOLTAGE_3_FACTOR);

        // Mittwelwertberechnung
        if(AVG_Update(&tVCC3Avg, ulVoltage3) != eAVERAGING_NO_ERROR)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        if(AVG_GetIsValid(&tVCC3Avg.data))
            {
            if(AVG_Get(&tVCC3Avg, (S32*) &ulVoltage3) != eAVERAGING_NO_ERROR)
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }

            // Spannung in RAM-Variable für Factory Device Communication eintragen
            lPowerVoltage3 = ulVoltage3;

            // Check if voltage is below error level (SOFTQM-602)
            if((ulVoltage3 < VOLTAGE_3_LIMIT_MIN_MILLIVOLT))
                {
                // Only send error if supply voltage is not below error level (SOFTQM-602)
                if((sysPowerStat & eSYSPWR_STAT_DETECT_VCC_LOW) == 0)
                    {
                    if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC3_LOW) == 0)
                        {
                        sysPowerStat |= eSYSPWR_STAT_ERROR_VCC3_LOW;
                        // Send error event if the voltage is below the minimum error level (SOFTQM-657)
                        SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_INTERNAL_VOLTAGE_3, eERROR_VOLTAGE_EXCEEDED_MIN,
                                          ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                        }
                    }
                }

            // Check if voltage is above error level (SOFTQM-602)
            if((ulVoltage3 > VOLTAGE_3_LIMIT_MAX_MILLIVOLT))
                {
                if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC3_HIGH) == 0)
                    {
                    // Send a permanent hard error if voltage exceeds maximum error level (SOFTQM-636)
                    sysPowerStat |= eSYSPWR_STAT_ERROR_VCC3_HIGH;
                    Safety_PermanentHardError(HARD_ERR_VOLTAGE_EXCEEDED);
                    }
                }
            }
        }
#endif

// Auswertung 4. Versorgung
#ifdef fpADCIN_VCC4
    if(powerSupplyUserConfig->voltage4IsActive)
        {
        // Messwertaufnahme
        if(ADC_SampleSingleChannel(fpADCIN_VCC4, &fVoltage4) != eADC_TRUE)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        // Umwandlung in Integer
        ulVoltage4 = (U32)(fVoltage4 * DECIMAL_FIXPOINT * VOLTAGE_4_FACTOR);

        // Mittwelwertberechnung
        if(AVG_Update(&tVCC4Avg, ulVoltage4) != eAVERAGING_NO_ERROR)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        if(AVG_GetIsValid(&tVCC4Avg.data))
            {
            if(AVG_Get(&tVCC4Avg, (S32*) &ulVoltage4) != eAVERAGING_NO_ERROR)
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }

            // Spannung in RAM-Variable für Factory Device Communication eintragen
            lPowerVoltage4 = ulVoltage4;

            // Check if voltage is below error level (SOFTQM-602)
            if((ulVoltage4 < VOLTAGE_4_LIMIT_MIN_MILLIVOLT))
                {
                // Only send error if supply voltage is not below error level (SOFTQM-602)
                if((sysPowerStat & eSYSPWR_STAT_DETECT_VCC_LOW) == 0)
                    {
                    if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC4_LOW) == 0)
                        {
                        sysPowerStat |= eSYSPWR_STAT_ERROR_VCC4_LOW;
                        // Send error event if the voltage is below the minimum error level (SOFTQM-657)
                        SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_INTERNAL_VOLTAGE_4, eERROR_VOLTAGE_EXCEEDED_MIN,
                                          ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                        }
                    }
                }

            // Check if voltage is above error level (SOFTQM-602)
            if((ulVoltage4 > VOLTAGE_4_LIMIT_MAX_MILLIVOLT))
                {
                if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC4_HIGH) == 0)
                    {
                    // Send a permanent hard error if voltage exceeds maximum error level (SOFTQM-636)
                    sysPowerStat |= eSYSPWR_STAT_ERROR_VCC4_HIGH;
                    Safety_PermanentHardError(HARD_ERR_VOLTAGE_EXCEEDED);
                    }
                }
            }
        }
#endif

// Auswertung 5. Versorgung
#ifdef fpADCIN_VCC5
    if(powerSupplyUserConfig->voltage5IsActive)
        {
        // Messwertaufnahme
        if(ADC_SampleSingleChannel(fpADCIN_VCC5, &fVoltage5) != eADC_TRUE)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        // Umwandlung in Integer
        ulVoltage5 = (U32)(fVoltage5 * DECIMAL_FIXPOINT * VOLTAGE_5_FACTOR);

        // Mittwelwertberechnung
        if(AVG_Update(&tVCC5Avg, ulVoltage5) != eAVERAGING_NO_ERROR)
            {
            // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
            Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
            }

        if(AVG_GetIsValid(&tVCC5Avg.data))
            {
            if(AVG_Get(&tVCC5Avg, (S32*) &ulVoltage5) != eAVERAGING_NO_ERROR)
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }

            // Spannung in RAM-Variable für Factory Device Communication eintragen
            lPowerVoltage5 = ulVoltage5;

            // Check if voltage is below error level (SOFTQM-602)
            if((ulVoltage5 < VOLTAGE_5_LIMIT_MIN_MILLIVOLT))
                {
                // Only send error if supply voltage is not below error level (SOFTQM-602)
                if((sysPowerStat & eSYSPWR_STAT_DETECT_VCC_LOW) == 0)
                    {
                    if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC5_LOW) == 0)
                        {
                        sysPowerStat |= eSYSPWR_STAT_ERROR_VCC5_LOW;
                        // Send error event if the voltage is below the minimum error level (SOFTQM-657)
                        SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_INTERNAL_VOLTAGE_5, eERROR_VOLTAGE_EXCEEDED_MIN,
                                          ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                        }
                    }
                }

            // Check if voltage is above error level (SOFTQM-602)
            if((ulVoltage5 > VOLTAGE_5_LIMIT_MAX_MILLIVOLT))
                {
                if((sysPowerStat & eSYSPWR_STAT_ERROR_VCC5_HIGH) == 0)
                    {
                    // Send a permanent hard error if voltage exceeds maximum error level (SOFTQM-636)
                    sysPowerStat |= eSYSPWR_STAT_ERROR_VCC5_HIGH;
                    Safety_PermanentHardError(HARD_ERR_VOLTAGE_EXCEEDED);
                    }
                }
            }
        }
#endif

//------------------- BLOCK: Get values from queue -----------------------
#if ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))
    // Get value from queue of external adc task
    if(MAX116XX_AdcValuesPeek(externalAdcValuesList) != true)
        {
        // The external adc task needs some time to provide the first values. It does get a tolerance of WAIT_EXTERNAL_ADC_STARTUP_IN_SAFETYCYCLE_TICKS cycles of the safety task.
        if(waitStartupExternalAdcTolerance == 0)
            {
            Safety_PermanentHardError(HARD_ERR_SAFETY_MEASUREMENT);
            }
        else
            {
            waitStartupExternalAdcTolerance--;
            }
        }
    else
        {
        // If reading was successful once, the tolerance is set to 0.
        waitStartupExternalAdcTolerance = 0;

//------------------------------------------------------------------------

//------------------- BLOCK: first voltage of external ADC----------------
        // Checking of the voltages on the external ADC MAX166xx
        if(powerSupplyUserConfig->voltageExternalAdcChannel1IsActive)
            {
//------------------- BLOCK: Calculting Averages -------------------------
            // calculate the average
#if EXT_ADC_CHANNEL1_TO_CHECK_IS_12CHANNEL
            // Float values are normalized to the internal reference voltage of 2.048V. Calculate real voltage on the pin of the external adc.
            externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] * ((F32)MAX11611_REFERENCE_VOLTAGE / 1000.0f);;
            // The voltage measured is divided by a voltage divider. Recalculate the actual input voltage
            externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] * EXT_ADC_CHANNEL1_TO_CHECK_V_DIVIDER_MULTIPLIKATOR;
            if(AVG_UpdateF32(&tVCCExternalAdcChannel1AvgInstance, externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] ) != eAVERAGING_NO_ERROR)
#else
                // Float values are normalized to the internal reference voltage of 2.048V. Calculate real voltage on the pin of the external adc.
            externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] * ((F32)MAX11607_REFERENCE_VOLTAGE / 1000.0f);;
            // The voltage measured is divided by a voltage divider. Recalculate the actual input voltage
            externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] * EXT_ADC_CHANNEL1_TO_CHECK_V_DIVIDER_MULTIPLIKATOR;
            if(AVG_UpdateF32(&tVCCExternalAdcChannel1AvgInstance, externalAdcValuesList[MAX116XX_FEAT_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL1_TO_CHECK_CHANNEL] ) != eAVERAGING_NO_ERROR)
#endif // EXT_ADC_CHANNEL1_TO_CHECK_IS_12CHANNEL
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }

                if(AVG_IsValidF32(&tVCCExternalAdcChannel1AvgInstance))
                    {
                    if(AVG_GetF32(&tVCCExternalAdcChannel1AvgInstance, &tVCCExternalAdcChannel1Avg) != eAVERAGING_NO_ERROR)
                        {
                        // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                        Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                        }
//------------------------------------------------------------------------
//------------------- BLOCK: Comparing to thresholds----------------------
                        // Check if voltage is below error level (SOFTQM-602)
                    if((tVCCExternalAdcChannel1Avg < EXT_ADC_CHANNEL1_TO_CHECK_LIMIT_MIN_VOLT))
                        {
                        // Only send error if supply voltage is not below error level (SOFTQM-602)
                        if((sysPowerStat & eSYSPWR_STAT_DETECT_VCC_LOW) == 0)
                            {
                            if((sysPowerStat & eSYSPWR_START_ERROR_EXT_ADC_CH1_LOW) == 0)
                                {
                                sysPowerStat |= eSYSPWR_START_ERROR_EXT_ADC_CH1_LOW;
                                // Send error event if the voltage is below the minimum error level (SOFTQM-657)
                                SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_EXTERNAL_ADC_VOLTAGE_1, eERROR_VOLTAGE_EXCEEDED_MIN,
                                                ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                                }
                            }
                        }
                    if((tVCCExternalAdcChannel1Avg > EXT_ADC_CHANNEL1_TO_CHECK_LIMIT_MAX_VOLT))
                            {
                            if((sysPowerStat & eSYSPWR_STAT_ERROR_EXT_ADC_CH1_HIGH) == 0)
                                {
                                // Send a permanent hard error if voltage exceeds maximum error level (SOFTQM-636)
                                sysPowerStat |= eSYSPWR_STAT_ERROR_EXT_ADC_CH1_HIGH;
                                Safety_PermanentHardError(HARD_ERR_VOLTAGE_EXCEEDED);
                                }
                            }
                    } // if(AVG_IsValidF32(&tVCCExternalAdcChannel1AvgInstance))
            } // if(powerSupplyUserConfig->voltageExternalAdcChannel1IsActive)


//------------------- BLOCK: second voltage of external ADC---------------
        // Checking of the voltages on the external ADC MAX166xx
        if(powerSupplyUserConfig->voltageExternalAdcChannel2IsActive)
            {
//------------------- BLOCK: Calculting Averages -------------------------
            // calculate the average
#if EXT_ADC_CHANNEL2_TO_CHECK_IS_12CHANNEL
            // Float values are normalized to the internal reference voltage of 2.048V. Calculate real voltage on the pin of the external adc.
            externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] * ((F32)MAX11611_REFERENCE_VOLTAGE / 1000.0f);;
            // The voltage measured is divided by a voltage divider. Recalculate the actual input voltage
            externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] * EXT_ADC_CHANNEL2_TO_CHECK_V_DIVIDER_MULTIPLIKATOR;
            if(AVG_UpdateF32(&tVCCExternalAdcChannel2AvgInstance, externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] ) != eAVERAGING_NO_ERROR)
#else
            // Float values are normalized to the internal reference voltage of 2.048V. Calculate real voltage on the pin of the external adc.
            externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] * ((F32)MAX11607_REFERENCE_VOLTAGE / 1000.0f);;
            // The voltage measured is divided by a voltage divider. Recalculate the actual input voltage
            externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] * EXT_ADC_CHANNEL2_TO_CHECK_V_DIVIDER_MULTIPLIKATOR;
            if(AVG_UpdateF32(&tVCCExternalAdcChannel2AvgInstance, externalAdcValuesList[MAX116XX_FEAT_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL2_TO_CHECK_CHANNEL] ) != eAVERAGING_NO_ERROR)
#endif // EXT_ADC_CHANNEL2_TO_CHECK_IS_12CHANNEL
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }

                if(AVG_IsValidF32(&tVCCExternalAdcChannel2AvgInstance))
                    {
                    if(AVG_GetF32(&tVCCExternalAdcChannel2AvgInstance, &tVCCExternalAdcChannel2Avg) != eAVERAGING_NO_ERROR)
                        {
                        // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                        Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                        }
        //------------------------------------------------------------------------
        //------------------- BLOCK: Comparing to thresholds----------------------
                    // Check if voltage is below error level (SOFTQM-602)
                    if((tVCCExternalAdcChannel2Avg < EXT_ADC_CHANNEL2_TO_CHECK_LIMIT_MIN_VOLT))
                        {
                        // Only send error if supply voltage is not below error level (SOFTQM-602)
                        if((sysPowerStat & eSYSPWR_STAT_DETECT_VCC_LOW) == 0)
                            {
                            if((sysPowerStat & eSYSPWR_START_ERROR_EXT_ADC_CH2_LOW) == 0)
                                {
                                sysPowerStat |= eSYSPWR_START_ERROR_EXT_ADC_CH2_LOW;
                                // Send error event if the voltage is below the minimum error level (SOFTQM-657)
                                SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_EXTERNAL_ADC_VOLTAGE_2, eERROR_VOLTAGE_EXCEEDED_MIN,
                                                ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                                }
                            }
                        }
                    if((tVCCExternalAdcChannel2Avg > EXT_ADC_CHANNEL2_TO_CHECK_LIMIT_MAX_VOLT))
                            {
                            if((sysPowerStat & eSYSPWR_STAT_ERROR_EXT_ADC_CH2_HIGH) == 0)
                                {
                                // Send a permanent hard error if voltage exceeds maximum error level (SOFTQM-636)
                                sysPowerStat |= eSYSPWR_STAT_ERROR_EXT_ADC_CH2_HIGH;
                                Safety_PermanentHardError(HARD_ERR_VOLTAGE_EXCEEDED);
                                }
                            }
                    }
            } // if(powerSupplyUserConfig->voltageExternalAdcChannel2IsActive)

//------------------- BLOCK: third voltage of external ADC----------------
        // Checking of the voltages on the external ADC MAX166xx
        if(powerSupplyUserConfig->voltageExternalAdcChannel3IsActive)
            {
//------------------- BLOCK: Calculting Averages -------------------------
            // calculate the average
#if EXT_ADC_CHANNEL3_TO_CHECK_IS_12CHANNEL
            // Float values are normalized to the internal reference voltage of 2.048V. Calculate real voltage on the pin of the external adc.
            externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] * ((F32)MAX11611_REFERENCE_VOLTAGE / 1000.0f);;
            // The voltage measured is divided by a voltage divider. Recalculate the actual input voltage
            externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] * EXT_ADC_CHANNEL3_TO_CHECK_V_DIVIDER_MULTIPLIKATOR;
            if(AVG_UpdateF32(&tVCCExternalAdcChannel3AvgInstance, externalAdcValuesList[MAX116XX_POS_12CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] ) != eAVERAGING_NO_ERROR)
#else
            // Float values are normalized to the internal reference voltage of 2.048V. Calculate real voltage on the pin of the external adc.
            externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] * ((F32)MAX11607_REFERENCE_VOLTAGE / 1000.0f);;
            // The voltage measured is divided by a voltage divider. Recalculate the actual input voltage
            externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] = externalAdcValuesList[MAX116XX_POS_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] * EXT_ADC_CHANNEL3_TO_CHECK_V_DIVIDER_MULTIPLIKATOR;
            if(AVG_UpdateF32(&tVCCExternalAdcChannel3AvgInstance, externalAdcValuesList[MAX116XX_FEAT_4CHANNEL_ADC].f32Data[EXT_ADC_CHANNEL3_TO_CHECK_CHANNEL] ) != eAVERAGING_NO_ERROR)
#endif // EXT_ADC_CHANNEL3_TO_CHECK_IS_12CHANNEL
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }

                if(AVG_IsValidF32(&tVCCExternalAdcChannel3AvgInstance))
                    {
                    if(AVG_GetF32(&tVCCExternalAdcChannel3AvgInstance, &tVCCExternalAdcChannel3Avg) != eAVERAGING_NO_ERROR)
                        {
                        // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                        Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                        }
//------------------------------------------------------------------------
//------------------- BLOCK: Comparing to thresholds----------------------
                    // Check if voltage is below error level (SOFTQM-602)
                    if((tVCCExternalAdcChannel3Avg < EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MIN_VOLT))
                        {
                        // Only send error if supply voltage is not below error level (SOFTQM-602)
                        if((sysPowerStat & eSYSPWR_STAT_DETECT_VCC_LOW) == 0)
                            {
                            if((sysPowerStat & eSYSPWR_START_ERROR_EXT_ADC_CH3_LOW) == 0)
                                {
                                sysPowerStat |= eSYSPWR_START_ERROR_EXT_ADC_CH3_LOW;
                                // Send error event if the voltage is below the minimum error level (SOFTQM-657)
                                SendErrorMsgEvent(eEVENT_VCC_CHECK_ERROR, eERROR_EXTERNAL_ADC_VOLTAGE_3, eERROR_VOLTAGE_EXCEEDED_MIN,
                                                ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                                }
                            }
                        } // if((tVCCExternalAdcChannel3Avg < EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MIN_VOLT))
                    if((tVCCExternalAdcChannel3Avg > EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MAX_VOLT))
                            {
                            if((sysPowerStat & eSYSPWR_STAT_ERROR_EXT_ADC_CH3_HIGH) == 0)
                                {
                                // Send a permanent hard error if voltage exceeds maximum error level (SOFTQM-636)
                                sysPowerStat |= eSYSPWR_STAT_ERROR_EXT_ADC_CH3_HIGH;
                                Safety_PermanentHardError(HARD_ERR_VOLTAGE_EXCEEDED);
                                }
                            } // if((tVCCExternalAdcChannel3Avg > EXT_ADC_CHANNEL3_TO_CHECK_LIMIT_MAX_VOLT))
                    } // if(AVG_IsValidF32(&tVCCExternalAdcChannel3AvgInstance))
            }
        } // if(MAX116XX_AdcValuesPeek(externalAdcValuesList) != true)
#endif // ((MAX116XX_FEAT_4CHANNEL_ADC) || (MAX116XX_FEAT_12CHANNEL_ADC))
//------------------------------------------------------------------------



// Leistungsaufnahme
#if defined(fpADCIN_ICC) && defined(fpADCIN_VCC)
    if(powerSupplyUserConfig->supplyVoltageIsActive && powerSupplyUserConfig->currentIsActive)
        {
        if((sysPowerStat & (eSYSPWR_STAT_VCC_VALID | eSYSPWR_STAT_I_VALID)) == (eSYSPWR_STAT_VCC_VALID | eSYSPWR_STAT_I_VALID))
            {
            ulPower = (U32)((lPowerVoltage * lPowerCurrent) / DECIMAL_FIXPOINT);

            if(ulPower > POWER_LIMIT_MAX_MILLIWATT)
                {
                if((sysPowerStat & eSYSPWR_STAT_ERROR_POWER_HIGH) == 0)
                    {
                    sysPowerStat |= eSYSPWR_STAT_ERROR_POWER_HIGH;
                    Safety_PermanentHardError(HARD_ERR_POWER_EXCEEDED);
                    }
                }

            if(ulPower > (U32)(POWER_LIMIT_WARNING_MAX_WATT * DECIMAL_FIXPOINT))
                {
                // Power too high
                if(!(sysPowerStat & eSYSPWR_STAT_WARNING_POWER_HIGH))
                    {
                    sysPowerStat |= eSYSPWR_STAT_WARNING_POWER_HIGH;
                    // Send warning if power exceeds maximum warning limit (SOFTQM-640)
                    SendErrorMsgEvent(eEVENT_POWER_CHECK, eERROR_SUPPLY_POWER, eERROR_POWER_EXCEEDED_MAX,
                                      ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                    }
                }
            }
        }

#endif

// Temperatur
#ifdef fpADCIN_TEMPERATURE
    if(powerSupplyUserConfig->temperatureAdcIsActive)
        {
        ADC_TemperatureSensorEnable();
        ADC_SampleSingleChannel(fpADCIN_TEMPERATURE, &temperatureVSense);
        ADC_TemperatureSensorDisable();
        systemTemperature = (S32)(ADC_ConvertTemperature(temperatureVSense) * DECIMAL_FIXPOINT);
        }
#endif

#ifdef TMP144_UART_CHANNEL
    if(powerSupplyUserConfig->temperatureSensorIsActive)
        {
        // Obtain new temperature value (SOFTQM-543)
        if(TMP144_TemperatureValuePeek(&temperatureValue))
            {
            S32 oldSystemTemperature;

            // Set measurement valid
            if(!(sysTemperatureStat & eSYSTMP_STAT_TMP_STARTUP_VALID))
                {
                sysTemperatureStat |= eSYSTMP_STAT_TMP_STARTUP_VALID;
                }

            // Check if temperature is below error level (SOFTQM-588)
            if(temperatureValue < TEMPERATURE_ERROR_MIN)
                {
                if(!(sysTemperatureStat & eSYSTMP_STAT_ERROR_TMP_LOW))
                    {
                    sysTemperatureStat |= eSYSTMP_STAT_ERROR_TMP_LOW;

                    // Enter permanent hard-error if temperature exceeds error limits (SOFTQM-643)
                    Safety_PermanentHardError(HARD_ERR_TEMPERATURE_EXCEEDED);
                    }
                }

            // Check if temperature is below warning level (SOFTQM-588)
            if(temperatureValue < TEMPERATURE_WARNING_MIN)
                {
                if(!(sysTemperatureStat & eSYSTMP_STAT_WARNING_TMP_LOW))
                    {
                    sysTemperatureStat |= eSYSTMP_STAT_WARNING_TMP_LOW;
                    // Send warning if temperature exceeds warning limits (SOFTQM-642)
                    SendErrorMsgEvent(eEVENT_TEMPERATURE, eERROR_SYSTEM_TEMPERATURE, eERROR_TEMPERATURE_EXCEEDED_MIN,
                                      ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                    }
                }

            // Check if temperature is above error level (SOFTQM-588)
            if(temperatureValue > TEMPERATURE_ERROR_MAX)
                {
                if(!(sysTemperatureStat & eSYSTMP_STAT_ERROR_TMP_HIGH))
                    {
                    sysTemperatureStat |= eSYSTMP_STAT_ERROR_TMP_HIGH;

                    // Enter permanent hard-error if temperature exceeds error limits (SOFTQM-643)
                    Safety_PermanentHardError(HARD_ERR_TEMPERATURE_EXCEEDED);
                    }
                }

            // Check if temperature is above warning level (SOFTQM-588)
            if(temperatureValue > TEMPERATURE_WARNING_MAX)
                {
                if(!(sysTemperatureStat & eSYSTMP_STAT_WARNING_TMP_HIGH))
                    {
                    sysTemperatureStat |= eSYSTMP_STAT_WARNING_TMP_HIGH;
                    // Send warning if temperature exceeds warning limits (SOFTQM-642)
                    SendErrorMsgEvent(eEVENT_TEMPERATURE, eERROR_SYSTEM_TEMPERATURE, eERROR_TEMPERATURE_EXCEEDED_MAX,
                                      ERROR_BYTE_LOWER_LEVEL_FILL_ZERO);
                    }
                }

            oldSystemTemperature = systemTemperature;
            temperatureValue = roundf(temperatureValue * DECIMAL_FIXPOINT);
            systemTemperature = (S32)temperatureValue;
            if(systemTemperature != oldSystemTemperature)
                {
                // Execute custom action if temperature value differs from previous measurement (SOFTQM-696)
                Safety_TemperatureChangedHook(temperatureValue, sysTemperatureStat);
                }
            }
        else
            {
            // Wait for startup of temperature sensor at startup
            if(!(sysTemperatureStat & eSYSTMP_STAT_TMP_STARTUP_VALID) && (waitStartupTmpSensorTolerance > 0))
                {
                waitStartupTmpSensorTolerance--;
                }
            else
                {
                // Enter non-permanent hard error state if measurement fails (SOFTQM-677)
                Safety_HardError(HARD_ERR_SAFETY_MEASUREMENT);
                }
            }
        }
#endif

    return TRUE;
    }
//------------------------------------------------------------------------------

/// @author M.Neubauer @date 04.08.2017
#if defined(fpADCIN_TEMPERATURE) || defined(TMP144_UART_CHANNEL)
S32 Safety_GetSystemTemperature(void)
    {
    return systemTemperature;
    }
//------------------------------------------------------------------------------

void TWK_WEAK Safety_TemperatureChangedHook(float temperature, SYSTEM_TEMPERATURE_STATUS const systemTemperatureState)
    {
    // Do nothing. This is intended to be overwritten by the application.
    }
#endif
//------------------------------------------------------------------------------

/// @author A.Fischer @date 20.12.2022
#ifdef fpADCIN_VCC
S32 Safety_GetPowerVoltage(void)
    {
    return lPowerVoltage;
    }
#endif
//------------------------------------------------------------------------------

