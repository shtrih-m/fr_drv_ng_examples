@echo off
setlocal
rem Пример скрипта обновления ФР по DFU и записью функциональных лицензий
rem В пути к рабочей директории не должно быть пробелов и других спец символов.
rem Например, можно положить скрипт в C:\shtrih_dfu_update
rem Директория должна быть доступна на запись(будут сохраняться временные файлы)
rem рядом со скриптом необходимо положить dfu-util.exe и console_test_fr_drv_ng.exe, и файл с функциональными лицензиями под именем licenses.txt

echo Начинаем...
rem если раскомментировать следующую строку - отладка будет в консоли, иначе в файле fr_drv.log в рабочей директории
rem set /A FR_DRV_DEBUG_CONSOLE=1
rem Имя файла резервной копии таблиц
set SAVE_TABLES_PATH=tables_backup
cd /d %~dp0
echo %cd%>tmp
set /P PWD=<tmp
rem путь до исполняемого файла консольного теста, подразумевается что он находится в рабочей директории
set "FULL_EXE_PATH=%PWD%\console_test_fr_drv_ng.exe"
rem URI связи с ККТ, по умолчанию используется TCP транспорт, адрес 192.168.137.111, порт 7778, таймаут 10 секунда на команду, стандартный протокол 
rem если при старте скрипта в окружени нет FR_DRV_NG_CT_URL назначается стандартный, иначе из окружения
if "%FR_DRV_NG_CT_URL%" == "" set "FR_DRV_NG_CT_URL=tcp://192.168.137.111:7778?timeout=10000&protocol=v1&enq_mode=1"
rem пароль сист. администратора ККТ, по умолчанию 30
rem если при старте скрипта в окружени нет FR_DRV_NG_CT_PASSWORD используется стандартный, иначе из окружения
if "%FR_DRV_NG_CT_PASSWORD%" == "" set "FR_DRV_NG_CT_PASSWORD=30"
rem echo "%FR_DRV_NG_CT_URL%"
rem echo "%FR_DRV_NG_CT_PASSWORD%"
echo Таблицы будут сохранены по в файл %SAVE_TABLES_PATH%
rem echo "%FR_DRV_NG_CT_URL%"
echo Получаем статус ККТ...
console_test_fr_drv_ng status>NUL
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
echo Ок!
echo Получаем заводской номер...
console_test_fr_drv_ng read 18.1.1>tmp
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
set /P SERIAL=<tmp
echo Заводской номер: %SERIAL%
echo Получаем UIN...
console_test_fr_drv_ng read 23.1.11>tmp
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
set /P UIN=<tmp
echo UIN: %UIN%
set FIRMWARE_FILENAME=upd_app.bin
IF "%UIN%"=="---" (
   echo UIN отсутствует
   set FIRMWARE_FILENAME=upd_app_for_old_frs.bin
)

echo|set /p= Сохраняем таблицы...
console_test_fr_drv_ng save-tables > %SAVE_TABLES_PATH%
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
echo готово

echo Перезагружаемся в режим dfu...
console_test_fr_drv_ng reboot-dfu
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
rem перешли в режим DFU, ждём 3 секунды и запускаем dfu-util, она сама найдет устройство и установит прошивку, нужно только передать имя файла
timeout /t 3 /nobreak > NUL
echo|set /p= Запускаем обновление по DFU...
dfu-util -D %FIRMWARE_FILENAME%
IF %ERRORLEVEL% NEQ 0 EXIT /B %ERRORLEVEL%
echo готово

rem спим 5 секунд, ждём появления устройства в системе после обновления
timeout /t 5 /nobreak > NUL
rem удаляем наши правила файерволла, этот шаг нужен потому что при изменении пути к исполняемому файлу перестают работать и правила. Поэтому каждый раз удаляем старые правила и добавляем новые.
netsh advfirewall firewall del rule name="console_test_fr_drv_ng_allow"
echo Добавляем правила firewall
netsh advfirewall firewall add rule name="console_test_fr_drv_ng_allow" program="%FULL_EXE_PATH%" protocol=udp dir=in enable=yes action=allow profile=private,public
netsh advfirewall firewall add rule name="console_test_fr_drv_ng_allow" program="%FULL_EXE_PATH%" protocol=tcp dir=in enable=yes action=allow profile=private,public

echo|set /p= Ищем обновленный ККТ в сети и на COM портах...
console_test_fr_drv_ng discover > tmp
for /f %%i in ("tmp") do set TMP_SIZE=%%~zi
if %TMP_SIZE% EQU 0 "echo Не удалось обнаружить устройство после обновления" && EXIT /B 1

for /F "usebackq tokens=*" %%A in ("tmp") do (
set "FR_DRV_NG_CT_URL=%%A"
call :nextkkt
)
goto continue
:nextkkt
echo "Попытка соединения %FR_DRV_NG_CT_URL%"
console_test_fr_drv_ng model>NUL
IF %ERRORLEVEL% NEQ 0 GOTO:EOF
echo Устройство обнаружено!

echo|set /p= Выполняем техобнуление...
console_test_fr_drv_ng tech-reset
IF %ERRORLEVEL% NEQ 0 GOTO:EOF
echo готово

echo|set /p= Устанавливаем текущую дату-время...
console_test_fr_drv_ng setcurrentdatetime
IF %ERRORLEVEL% NEQ 0 GOTO:EOF
echo готово

echo Получаем заводской номер обнаруженного устройства...
console_test_fr_drv_ng read 18.1.1>tmp
IF %ERRORLEVEL% NEQ 0 GOTO:EOF
set /P NEW_SERIAL=<tmp
echo %SERIAL%
echo %NEW_SERIAL%
if not "%SERIAL%" == "%NEW_SERIAL%" "echo Заводской номер не совпадает с ожидаемым" && goto :EOF

findstr /B %SERIAL% licenses.txt > tmp
for /f %%i in ("tmp") do set TMP_SIZE=%%~zi
if %TMP_SIZE% EQU 0 "echo Лицензия для не обнаружена" && EXIT /B 1
set /P LICENSE_STRING=<tmp
set LICENSE=
set CRYPTO_SIGNATURE=
for /F "tokens=1,2,3" %%A in ("%LICENSE_STRING%") DO (
	echo %%B>tmp
	set /P LICENSE=<tmp
	echo %%C>tmp
	set /P CRYPTO_SIGNATURE=<tmp	
)
echo|set /p= Устанавливаем функциональные лицензии...
console_test_fr_drv_ng write-feature-licenses %LICENSE% %CRYPTO_SIGNATURE%
IF %ERRORLEVEL% NEQ 0 GOTO:EOF
echo готово

echo|set /p= Восстанавливаем таблицы...
console_test_fr_drv_ng restore-tables<%SAVE_TABLES_PATH%
IF %ERRORLEVEL% NEQ 0 GOTO:EOF
echo готово

echo|set /p= Перезагружаемся...
console_test_fr_drv_ng reboot
IF %ERRORLEVEL% NEQ 0 GOTO:EOF
echo готово
del /f tmp
exit /B 0
:continue
del /f tmp
exit /B 1
