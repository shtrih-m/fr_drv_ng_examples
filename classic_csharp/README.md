### C# wrapper example

1. генерируем обертку через swig:
```bash
swig -c++ -csharp -namespace ru.shtrih_m.fr_drv_ng.classic_interface -dllimport classic_fr_drv_ng_csharp -o  csharp/classic_interface_wrap.cxx classic_interface.i
```
2. собираем библиотеку
```cmake
cmake_minimum_required(VERSION 3.1)
project(classic_fr_drv_ng_csharp_example)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

add_library(classic_fr_drv_ng_csharp SHARED
		csharp/classic_interface_wrap.cxx
)
set_target_properties(classic_fr_drv_ng_csharp PROPERTIES CXX_VISIBILITY_PRESET default)
target_link_libraries(classic_fr_drv_ng_csharp classic_fr_drv_ng)
target_link_directories(classic_fr_drv_ng_csharp PRIVATE ${CMAKE_SOURCE_DIR})

```
3. запуск
```bash
mcs -out:Program.exe *.cs
mono Program.exe
```
