require "debug"
-- в составе бинарника есть 2 библиотеки для сериализации / десериализации
require "cjson" -- https://kyne.com.au/~mark/software/lua-cjson-manual.html
require "cmsgpack" -- https://github.com/antirez/lua-cmsgpack

function serialization_example()
        local lua_table_example = {val=1,val2=2}
        local cmsgpack_msg = cmsgpack.pack(lua_table_example)
        print(msgpack_msg) -- двоичные данные, на терминале будет мусор...
        local json_msg=cjson.encode(ec)
        print(json_msg)
end

function make_error(ci)
--         local ec = {code=ci.ResultCode, message=ci.ResultCodeDescription}
        local ec = '{"code":' .. ci.ResultCode .. ' , "message": "'..ci.ResultCodeDescription..'"}'
        return ec
end

function check_result (ci, fn)
    local ret = fn(ci)
    if ret ~= 0 then
        print(debug.traceback())
        error(make_error(ci))
    end
end

local ci = classic_fr_drv_ng.classic_interface("im_from_lua") -- конструируем classic
ci.ConnectionURI = "tcp://192.168.137.111:7778?timeout=30000&plain_transfer=auto" -- свойства назначаются и получаются не через сеттеры/геттеры, а через установку атрибутов объекта через оператор .
-- строки - это обычные utf8 lua строки, все целые - обычные lua целые, в том числе Date и Time
ci.Password = 30

local vec = classic_fr_drv_ng.VectorOfBytes() -- специальная обертка для вектора
vec:push_back(1)
vec:push_back(2)
vec:push_back(3)

local ret = ci:Connect() -- в lua методы вызываются через :, а свойства через .
if ret ~= 0 and not ci.Connected then error(make_error(ci)) end
-- для Connect требуется особая обработка т.к. может вернуть ошибку получение служебной информации но соединение будет установлено. Поэтому стоит проверять свойство Connected
-- далее можно работать как обычно
check_result(ci,ci.Beep) -- если произойдет ошибка - скрипт будет завершен
-- -- если хотим иметь возможность обработать

local status, ec = pcall(check_result, ci, ci.Beep)
if not status then
    print("случилась ошибка: ".. ec)
    error(ec)
end

ci.StringForPrinting = "Просто какая-то строка"
ci.WrapStrings = true
check_result(ci, ci.PrintString)




