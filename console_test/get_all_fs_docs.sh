#!/bin/bash
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

STATUS=$($EXE fs-status)
if [ $? -eq 0 ]
then
  echo "Получен статус ФН!"
else
  BAD_STATE=$?
  echo "Невозможно получить статус ФН"
  exit $BAD_STATE
fi
NUM_DOCS=$(grep  'Last fiscal document number' <<< $STATUS |cut -f2)
DOC_INDEX=1
while [  $DOC_INDEX -lt $NUM_DOCS ]; do
    $EXE fs-get-doc-classic $DOC_INDEX

    if [ $? -ne 0 ]
    then
         exit 1
    fi
    let DOC_INDEX=DOC_INDEX+1
    echo
done
exit 0
