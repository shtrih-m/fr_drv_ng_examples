### C# wrapper example

1. генерируем обертку через swig:
```bash
swig -c++ -csharp -namespace ru.shtrih_m.fr_drv_ng.classic_interface -o  csharp/classic_interface_wrap.cxx classic_interface.i
```
2. собираем библиотеку
```cmake
add_library(classic_fr_drv_ng_csharp SHARED
		csharp/classic_interface_wrap.cxx
)
set_target_properties(classic_fr_drv_ng_csharp PROPERTIES CXX_VISIBILITY_PRESET default)
target_link_libraries(classic_fr_drv_ng_csharp classic_fr_drv_ng)
```
3. запуск
```bash

```
