### go wrapper example

1. генерируем обертку через swig:
```bash
swig -c++ -go -cgo -use-shlib -intgosize 64  -o go/classic_interface_wrap.cxx classic_interface.i
```
2. собираем библиотеку
```cmake
add_library(classic_fr_drv_ng_csharp SHARED
		csharp/classic_interface_wrap.cxx # swig -c++ -csharp -namespace ru.shtrih_m.fr_drv_ng.classic_interface -o  csharp/classic_interface_wrap.cxx classic_interface.i
		)
target_link_libraries(classic_fr_drv_ng_csharp classic_fr_drv_ng)
set_target_properties(classic_fr_drv_ng_csharp PROPERTIES CXX_VISIBILITY_PRESET default)
```
3. правим в `.go` пути до библиотеки
```bash
go get classic_fr_drv_ng
go build runme.go
```
