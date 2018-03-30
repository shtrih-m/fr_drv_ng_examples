#!/bin/bash
# Пример скрипта обновления используется консольный тест и unix утилиты
# используется утилита jq. взять можно https://stedolan.github.io/jq/download/
# пробуем несколько возможных путей до консольного теста
if [[ -x "console_test.sh" ]]
then
    EXE=./console_test.sh
elif [[ -x "console_test_fr_drv_ng_static" ]]
then
    EXE=./console_test_fr_drv_ng_static
elif [[ -x "console_test_fr_drv_ng_dynamic" ]]
then
    EXE=./console_test_fr_drv_ng_dynamic
elif [[ -x "console_test_fr_drv_ng" ]]
then
    EXE=./console_test_fr_drv_ng
elif [[ $(type -P "console_test_fr_drv_ng") ]]
then
    EXE=console_test_fr_drv_ng
else
    echo "исполняемый файл консольного теста не найден."
    exit 1
fi

# куда сохранять таблицы перед обновлением
SAVE_TABLES_PATH="tables_backup"
# Внимание данный синтаксис export означает: экспортировать переменную только если она не была экспортирована до этого.
# Соотв можно смело запускать скрипт с уже настроенным окружением
# URL запроса сервера обновления
export FR_UPDATE_SERVER_URL=${FR_UPDATE_SERVER_URL:='http://localhost:8888/check_firmware'}
# Пароль ФР
export FR_DRV_NG_CT_PASSWORD=${FR_DRV_NG_CT_PASSWORD:='30'}
# URI транспорта ФР
export FR_DRV_NG_CT_URL=${FR_DRV_NG_CT_URL:='serial://ttyACM0?baudrate=115200&timeout=10000'}
STATUS=$($EXE status)
if [ $? -eq 0 ]
then
  echo "Получен статус ККТ!"
else
  BAD_STATE=$?
  echo "Невозможно получить статус ККТ"
  exit $BAD_STATE
fi
SERIAL_NUMBER=$(grep  'Serial number' <<< $STATUS |cut -f2)
BUILD_DATE=$(grep  'Firmware date' <<< $STATUS |cut -f2)
echo "Заводской номер: $SERIAL_NUMBER, Дата ПО: $BUILD_DATE"
API_ANSWER=$(curl -s -H "Content-Type: application/json" -X POST -d "{\"build_date\":\"$BUILD_DATE\"}" $FR_UPDATE_SERVER_URL)
if [ $? -ne 0 ]
then
  BAD_STATE=$?
  echo "Нет связи с сервером обновлений"
  exit $BAD_STATE
fi
AVAILABLE=$(jq  -r '.update_available' <<< $API_ANSWER)
CRITICAL=$(jq  -r '.critical' <<< $API_ANSWER)
VERSION=$(jq  -r '.version' <<< $API_ANSWER)
if [ $AVAILABLE == "false" ]
then
    echo "Обновление не обнаружено."
    exit 0
fi

if [ $CRITICAL == "true" ]
then
echo "ВНИМАНИЕ: доступно критическое обновление"
fi
UIN=$($EXE read 23.11.1)

if [ $UIN == "---" ]
then
DOWNLOAD_URL=$(jq  -r '.url_old_frs' <<< $API_ANSWER)
else
echo "UIN: $UIN"
DOWNLOAD_URL=$(jq  -r '.url' <<< $API_ANSWER)
fi
mkdir -p $VERSION
FIRMWARE_FILENAME="$VERSION/upd_app_$SERIAL_NUMBER.bin"
curl -s "$DOWNLOAD_URL" -o $FIRMWARE_FILENAME
if [ $? -ne 0 ]
then
  BAD_STATE=$?
  echo "Ошибка при скачивании обновления"
  exit $BAD_STATE
fi
echo "Обновление скачано: $FIRMWARE_FILENAME"
mkdir -p $SAVE_TABLES_PATH
TABLES_FILENAME=$SAVE_TABLES_PATH/$SERIAL_NUMBER.tables
echo "Сохраняем таблицы $TABLES_FILENAME"
$EXE save-tables > $TABLES_FILENAME
if [ $? -ne 0 ]
then
  BAD_STATE=$?
  echo "Ошибка при сохранении таблиц"
  exit $BAD_STATE
fi
echo "Таблицы сохранены, перезагружаемся в режим dfu"
$EXE reboot-dfu
if [ $? -ne 0 ]
then
  BAD_STATE=$?
  echo "Ошибка при перезагрузке в режим dfu"
  exit $BAD_STATE
fi
sleep 3
dfu-util -D $FIRMWARE_FILENAME
sleep 5
DISCOVER=$($EXE discover)
if [[ $DISCOVER ]]; then
    KKT_URL=$(head -n1 <<< $DISCOVER)
else
    echo "Невозможно обнаружить устройство после обновления"
    exit 1
fi
export FR_DRV_NG_CT_URL=$KKT_URL
MODEL=$($EXE model)
if [ $? -eq 0 ]
then
  echo "Устройство обнаружено!"
else
  exit $?
fi
$EXE tech-reset
$EXE setcurrentdatetime
STATUS=$($EXE status)
if [ $? -eq 0 ]
then
  echo "Получен статус ККТ!"
else
  BAD_STATE=$?
  echo "Невозможно получить статус ККТ"
  exit $BAD_STATE
fi
SERIAL_NUMBER_DISCOVERED=$(grep  'Serial number' <<< $STATUS |cut -f2)
if [ $SERIAL_NUMBER == $SERIAL_NUMBER_DISCOVERED ]
then
  echo "Обнаружено устройство $SERIAL_NUMBER_DISCOVERED"
else
  echo "Заводской номер обрануженного устройства не совпадает с ожидаемым"
  exit 1
fi
echo "Восстанавливаем таблицы"
$EXE restore-tables <<< $TABLES_FILENAME
if [ $? -ne 0 ]
then
  BAD_STATE=$?
  echo "Невозможно восстановить таблицы"
  exit $BAD_STATE
fi
echo "Таблицы восстановлены, перезагрузка"
$EXE reboot
echo "Готово!"
exit 0
