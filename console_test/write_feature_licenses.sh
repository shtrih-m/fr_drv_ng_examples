#!/bin/bash
usage="$(basename "$0") файл_лицензий"
if [ "$#" -ne 1 ]; then
    echo "Необходимо передать в качестве аргумента путь до файла с лицензиями"
    echo ${usage}
    exit 1
fi

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

export FR_DRV_DEBUG_CONSOLE=1
export FR_DRV_NG_CT_PASSWORD=${FR_DRV_NG_CT_PASSWORD:='30'}
# URI транспорта ФР
export FR_DRV_NG_CT_URL=${FR_DRV_NG_CT_URL:='serial://ttyACM1?baudrate=115200&timeout=10000'}
STATUS=$(${EXE} status)
if [ $? -eq 0 ]
then
  echo "Получен статус ККТ!"
else
  BAD_STATE=$?
  echo "Невозможно получить статус ККТ"
  exit ${BAD_STATE}
fi
SERIAL_NUMBER=$(${EXE} read 18.1.1)
echo "Серийный номер: "${SERIAL_NUMBER}
while IFS='' read -r line || [[ -n "$line" ]]; do
    SERIAL_FROM_FILE=$(echo "${line}"| cut -f1 )
    if [ "${SERIAL_NUMBER}" -eq  "${SERIAL_FROM_FILE}" ]
    then
      echo "Найдена функциональные лицензии"
      LICENSE_FROM_FILE=$(echo "${line}"| cut -f2 )
      SIGNATURE_FROM_FILE=$(echo "${line}"| cut -f3 )
      ${EXE} write-feature-licenses "${LICENSE_FROM_FILE}" "${SIGNATURE_FROM_FILE}"
      WRITE_STATE=$?
      if [ ${WRITE_STATE} -eq 0 ]
        then
          echo "Лицензии успешно установлены"
          exit 0
        else
          echo "Ошибка при записи лицензий"
      fi
      exit ${WRITE_STATE}
    fi
done < "$1"
