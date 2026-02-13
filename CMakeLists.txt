cmake_minimum_required(VERSION 4.0)
project(gtk C)

set(CMAKE_C_STANDARD 23)

add_executable(gtk src/main.c
        widgets/headers/bouton.h
        widgets/sources/bouton.c
        widgets/headers/Fenetre.h
        widgets/sources/Fenetre.c
        widgets/headers/bouton_radio.h
        widgets/sources/bouton_radio.c
        widgets/sources/bouton_checklist.c
        widgets/headers/bouton_checklist.h)
