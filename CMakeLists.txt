cmake_minimum_required(VERSION 4.0)
project(gtk C)

set(CMAKE_C_STANDARD 23)

add_executable(gtk src/main.c
        widgets/headers/button.h
        widgets/sources/button.h.cpp)
