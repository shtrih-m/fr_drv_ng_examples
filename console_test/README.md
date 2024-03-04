# Консольный тест

#### Позволяет выполнять различные команды на ККТ.
В том числе запуск json-rpc для классического интерфейса:
[json-rpc classic](https://github.com/shtrih-m/fr_drv_ng_examples/tree/master/classic_jsonrpc)
[lua-classic](https://github.com/shtrih-m/fr_drv_ng_examples/blob/master/console_test/lua)
#### Список команд
получить список команд можно запустив тест без аргументов:
```bash
$ console_test_fr_drv_ng
Usage: console_test_fr_drv_ng <command> [command args] [command params] [-a url] [-p  password]
       console_test_fr_drv_ng -v
        print versions and exit
command is one of:
    status - "long status"
    short-status - "short status"
    model - "device model info"
    font - "get font parameters"
        args: font_number
    print - "print from stdin"
        args: [font_number[.flags]]
    read - "read table"
        args: table[.row[.field]]
    write - "write table"
        args: table.row.field
        params: new value
    save-tables - "dump all tables to stdout"
        args: [table[.row[.field]]]
    restore-tables - "write all/some tables from stdin(look at format of save-tables)"
    fs-status - "Fiscal storage status"
    fs-version - "Fiscal storage version"
    fs-doc-cancel - "cancel current FS document"
    fs-archive - "dump fs archive in CRPT format to stdout"
        args: first_doc.last_doc[.tag_filter]
    fs-exchange-status - "OFD exchange status"
    fs-get-doc-classic - "read document from FS and print its contents in classic mode"
        args: doc_number
    fs-get-doc-json - "read document from FS and print its contents as JSON"
        args: doc_number
    fs-parse-doc-classic - "read document from FS and print its contents in classic mode"
        args: doc_number
    fs-parse-doc-json - "read document from FS and print its contents as JSON"
        args: doc_number
    fs-get-eol - "read FS End Of Life datetime"
    fs-find-doc - "find FS document by number"
        args: doc_number
    exchange-status - "alias for fs-exchange-status"
    fs-calculation-report - "print calculation report"
    z-report - "prints Z-report and closes day"
    put-tlv - "write arbitary tlv"
        args: tag
        params: value
    discover - "run device discovery and print connect URI if any found"
    beep - "make sound"
    dump - "dump data from device and write it to stdout NOTE: for most dump types you need CTO2 password"
        args: dump_type in hex, eq: 0A
    dump-abort - "abort dump output"
    hex-cmd - "send some data thru transport layer"
        args: hex encoded string: eq: "AB CD EF"
    reboot-dfu - "reboot for firmware update"
    ping - "ping host from device"
        args: host or ip
    reboot - "reboot device"
    tech-reset - "tech reset device"
    state-reset - "resets current KKT state if necessary"
    speedup-serial - "if using serial io - set speed to 115200"
    setcurrentdatetime - "set and confirm current system date time to device. must be called after tech-reset"
    put-font - "alias for cf-write"
        args: font file path
    put-sd-update - "write update file to sd card filename must be upd_app.bin or upd_ldr.bin"
        args: update file path
    bootloader-version - "read bootloader version from device"
    read-feature-licenses - "reading all feature licenses and prints them over \t (tab) symbol"
    print-feature-licenses - "print human readable feature licenses"
    write-feature-licenses - "write feature licenses "
        args: hex encoded license string: eq: "AB CD EF"
        params: hex encoded signature string: "AB CD EF"
    program-serial - "program-serial number"
        args: serial number in ASCII
    cf-write - "read spf font from file and put it in device"
        args: font file path
    cf-reset - "reset custom font in kkt, make it the same as 1st font"
    cf-sha256sum-kkt - "get sha256sum for custom font in device"
    cf-sha256sum-spf - "calculate sha256sum for spf font after sorting glyphs by code"
        args: font file path
    cf-sort-spf - "read spf font from file, sort glyphs by code and write sorted spf to stdout"
        args: font file path
    gen-mono-token - "generate and send new MONO token to remote server"
    kkt-rereg-report - "generate reregistration report"
        args: tax.workmode.reason
    kkt-reg-begin - "intialize KKT registration report"
        args: report_type
    kkt-reg-end - "form KKT registration report"
        args: inn.rnm.tax.workmode
    kkt-reg-result - "print KKT registration result"
        args: [registration number]
    download-firmware - "download firmware from SKOK"
        args: path to save, - stdout
        params: [firmware_version]
    remote-firmware-version - "get offered firmware version from SKOK"
    remote-firmware-date - "get offered firmware date from SKOK"
    mc-exchange-status - "Marking exchange status"
    mc-check-status - "Marking check status"
    run-http-api-server - "run http REST API server with console command"
        args: network interface address:port eq: 0.0.0.0:8080
    run-ws-json-rpc-server - "run websocket json-rpc server"
        args: network interface address:port eq: 0.0.0.0:8080
    run-lua-script - "run lua script with classic_interface + all default lua libs and external packages support"
        args: .lua script file path
    run-lua-script-safe - "run lua script with classic_interface + base, table, string, math, utf8 libs"
        args: .lua script file path
```
####  Рекомандованная последовательность работы
1. установить через переменные окружения URI подключения к ККТ:
```bash
export FR_DRV_DEBUG_CONSOLE=1 #печать отладки на терминал
export FR_DRV_NG_CT_URL='tcp://192.168.137.111:7778?timeout=30000&plain_transfer=auto'# айпи по умолчанию для подключения по usbnet
export FR_DRV_NG_CT_PASSWORD=30 #пароль администратора ККТ
```
2.  При выполнении команд, результат записывается на STDOUT, ошибки на STDERR. Код возврата если не 0 - ошибка.
```bash
console_test_fr_drv_ng read 1.2.3
console_test_fr_drv_ng write 1.2.3 "value to write"
console_test_fr_drv_ng write 1.2.4 36
```
