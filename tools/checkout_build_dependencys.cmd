@CHCP 1252
@echo off

cd ..

mkdir build

if exist build\Driver_Common rd /s /q build\Driver_Common
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/Driver_Common" build\Driver_Common

if exist build\googletest rd /s /q build\googletest
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Software/Extern/googletest" build\googletest

if exist build\EN61508_Program_Flow rd /s /q build\EN61508_Program_Flow
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibQM/EN61508_Program_Flow" build\EN61508_Program_Flow

if exist build\EventSystem rd /s /q build\EventSystem
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/EventSystem" build\EventSystem

if exist build\STM32_Safety_STL rd /s /q build\STM32_Safety_STL
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibQM/STM32_Safety_STL" build\STM32_Safety_STL

if exist build\RTOS_AL rd /s /q build\RTOS_AL
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/RTOS_AL" build\RTOS_AL

if exist build\Parameter_Table rd /s /q build\Parameter_Table
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibQM/Parameter_Table" build\Parameter_Table

if exist build\Error_Handler rd /s /q build\ErrorHandler
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/ErrorHandler" build\ErrorHandler

if exist build\ErrorLogging rd /s /q build\ErrorLogging
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/ErrorLogging" build\ErrorLogging

if exist build\embOS rd /s /q build\embOS
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/embOS" build\embOS

if exist build\DataProcess_Averaging rd /s /q build\DataProcess_Averaging
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/DataProcess_Averaging" build\DataProcess_Averaging

if exist build\Devices_Temperature_TMP144 rd /s /q build\Devices_Temperature_TMP144
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibQM/Devices_Temperature_TMP144" build\Devices_Temperature_TMP144

if exist build\Devices_RTC_M41T6X rd /s /q build\Devices_RTC_M41T6X
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/Devices_RTC_M41T6X" build\Devices_RTC_M41T6X

if exist build\LibQM\ST_Driver rd /s /q build\LibQM\ST_Driver
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibQM/ST_Driver" build\LibQM\ST_Driver

if exist build\LibCert\ST_Driver rd /s /q build\LibCert\ST_Driver
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/ST_Driver" build\LibCert\ST_Driver

if exist build\Devices_ADC_MAX116XX rd /s /q build\Devices_ADC_MAX116XX
git clone "ssh://%USERNAME%@twkw-build01.twk.local:29418/TB/Controller/LibCert/Devices_ADC_MAX116XX" build\Devices_ADC_MAX116XX

pause
