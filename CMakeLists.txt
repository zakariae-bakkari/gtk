cmake_minimum_required(VERSION 3.26)
project(gtk C)

set(CMAKE_C_STANDARD 17)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_executable(gtk
        widgets/sources/bouton.c
        widgets/sources/fenetre.c
        widgets/sources/bouton_radio.c
        widgets/sources/bouton_checklist.c
        src/main.c
)

target_link_libraries(gtk PRIVATE PkgConfig::GTK4)
target_include_directories(gtk PRIVATE widgets/headers)
