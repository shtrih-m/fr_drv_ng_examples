cmake_minimum_required(VERSION 3.1)
project(classic_interface_example)
set(CMAKE_CXX_STANDARD 14)
IF (MSVC)
    add_definitions(-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS)
ENDIF()
# QT
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
link_directories(.)

add_executable(classic_interface_example
  main.cpp
)
target_link_libraries(classic_interface_example classic_fr_drv_ng cppbase_fr_drv_ng)
