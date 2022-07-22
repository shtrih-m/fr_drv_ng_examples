# json-rpc интерфейс классического драйвера

#### С помощью консольного теста можно запустить http/ws API для вызова методов "классического" интерфейса через [json-rpc](https://www.jsonrpc.org/specification)

#### 1. запускаем сервер
```bash
JSONRPC_SERVER_LISTEN_HOST="0.0.0.0"
JSONRPC_SERVER_PORT="8080"
console_test_fr_drv_ng run-ws-json-rpc-server "$JSONRPC_SERVER_LISTEN_HOST:$JSONRPC_SERVER_PORT"
```
#### 2. для того чтобы создать экземпляр классического интерфейса, необходимо выполнить POST запрос по пути `/classic/{название экземпляра}`, должен вернуться код `201 Created` или `409 Conflict`, если экземпляр с таким именем уже есть
```bash
INSTANCE_NAME="my_classic_instance"
curl -vvv -X POST http://localhost:$JSONRPC_SERVER_PORT/classic/$INSTANCE_NAME
```
#### 3. теперь можно подключаться по протоколу `websocket` по тому же пути `/classic/{название экземпляра}` и вызывать методы экземпляра. Существует ограничение на одну websocket сессию к одному экземпляру.
```bash
websocat "ws://127.0.0.1:$JSONRPC_SERVER_PORT/classic/$INSTANCE_NAME"
```
#### 4. Можно воспользоваться любой json-rpc библиотекой для вашего окружения или формировать json самостоятельно. Сами вызовы можно осуществлять двумя способами:
4.1 "Обычный" json-rpc где все свойства выставляются через сеттеры и потом вызываются методы:
* отправим
```json
{"jsonrpc":"2.0","method":"Set_ConnectionURI","params":["tcp://192.168.137.111:7778?timeout=30000&plain_transfer=auto"],"id":"1"}
```
* получим
```json
{"jsonrpc":"2.0","result":null,"id":"1"}
```
* отправим
```json
{"jsonrpc":"2.0","method":"Connect","id":"2"}
```
* получим
```json
{"jsonrpc":"2.0","result":0,"id":"2"}
```
4.2 С именованными параметрами. Спецификация json-rpc позволяет передать в вызов словарь с именованными параметрами, для классического интерфейса мы адаптируем это так: все именованные параметры считаются свойствами, которые устанавливаются ПЕРЕД вызовом метода, в ответ пользователь получит словарь со всеми свойствами которые драйвер "потрогал" во время вызова метода
```json
{"jsonrpc":"2.0","method":"Connect","params":{"ConnectionURI":"tcp://192.168.137.111:7778?timeout=30000&plain_transfer=auto"},"id":"1"}
```
* получим
```json
{
  "jsonrpc": "2.0",
  "result": {
    "ResultCode": 0,
    "Connected": true,
    "UMajorProtocolVersion": 1,
    "UMinorProtocolVersion": 17,
    "UMajorType": 0,
    "UMinorType": 0,
    "UModel": 43,
    "UCodePage": 0,
    "UDescription": "ЭЛВЕС-ФР-Ф",
    "CapGetShortECRStatus": true,
    "OperatorNumber": 1,
    "ECRSoftVersion": "D.3",
    "ECRBuild": 2604,
    "ECRSoftDate": 1658228552,
    "LogicalNumber": 1,
    "OpenDocumentNumber": 8,
    "ECRFlags": 656,
    "JournalRibbonIsPresent": false,
    "ReceiptRibbonIsPresent": false,
    "SlipDocumentIsMoving": false,
    "SlipDocumentIsPresent": false,
    "PointPosition": true,
    "EKLZIsPresent": false,
    "JournalRibbonOpticalSensor": false,
    "ReceiptRibbonOpticalSensor": true,
    "JournalRibbonLever": false,
    "ReceiptRibbonLever": true,
    "LidPositionSensor": false,
    "IsDrawerOpen": false,
    "IsPrinterRightSensorFailure": false,
    "PresenterIn": false,
    "IsPrinterLeftSensorFailure": false,
    "PresenterOut": false,
    "IsEKLZOverflow": false,
    "QuantityPointPosition": 0,
    "ECRMode": 3,
    "ECRModeDescription": "Day opened, day exeed 24 hours",
    "ECRMode8Status": 3,
    "ECRModeStatus": 3,
    "ECRAdvancedMode": 0,
    "ECRAdvancedModeDescription": "Paper present",
    "PortNumber": 2,
    "FMSoftVersion": "N.A",
    "FMBuild": 0,
    "FMSoftDate": 1451646152,
    "Time": 1658487729,
    "Date": 1658487729,
    "TimeStr": "14:02:09",
    "FMFlags": 0,
    "FM1IsPresent": false,
    "FM2IsPresent": false,
    "LicenseIsPresent": "",
    "FMOverflow": false,
    "IsBatteryLow": false,
    "IsLastFMRecordCorrupted": false,
    "IsFMSessionOpen": false,
    "IsFM24HoursOver": false,
    "SerialNumber": "999997",
    "SessionNumber": 34,
    "FreeRecordInFM": 0,
    "RegistrationNumber": 0,
    "FreeRegistration": 0,
    "INN": "3664069397",
    "ResultCodeDescription": "No errors"
  },
  "id": "1"
}
```
Свойств в ответе так много потому что при `Connect` драйвер получает служебную информацию которую потом использует. Для большинства вызовов, ответ будет гораздо меньше и состоять из свойств которые задействованы только в этом методе. Код возврата можно получать по ключу: `ResultCode`, он, как и `ResultCodeDescription` будет в ответе всегда.
* отправим
```json
{"jsonrpc":"2.0","method":"PrintString","params":{"StringForPrinting":"hello"},"id":"3"}
```
* получим
```json
{"jsonrpc":"2.0","result":{"ResultCode":0,"OperatorNumber":1,"ResultCodeDescription":"No errors"},"id":"3"}
```

4.3 Перечисления в json-rpc можно передавать как целые числа или как соотв. строки:
```c++
    enum class TBarcodeAlignment {
        baCenter = 0, ///< по центру
        baLeft = 1, ///< влево
        baRight = 2, ///< вправо
    };
```
* может быть передано:
```json
{"jsonrpc":"2.0","method":"Set_BarcodeAlignment","params":["baLeft"],"id":"1"}
```
* или:
```json
{"jsonrpc":"2.0","method":"Set_BarcodeAlignment","params":[1],"id":"1"}
```
* при получении всегда будет строка, отправим:
```json
{"jsonrpc":"2.0","method":"Get_BarcodeAlignment","id":"1"}
```
* получим:
```json
{"jsonrpc":"2.0","result":"baLeft","id":"1"}
```
