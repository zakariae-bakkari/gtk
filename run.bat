@echo off
setlocal enabledelayedexpansion

:: Add MSYS2 MinGW to PATH so gcc and pkg-config can be found automatically
set "MSYS_PATH=C:\msys64\mingw64\bin"
if exist "%MSYS_PATH%" (
    set "PATH=%MSYS_PATH%;%PATH%"
)

:: Validate input
if "%~1"=="" (
    echo ================================================================
    echo                   GTK4 EXAM WORKSPACE RUNNER
    echo ================================================================
    echo [ERROR] No C file or exam name specified.
    echo.
    echo Usage:
    echo   run.bat [path_to_c_file_or_exam_name]
    echo.
    echo Examples:
    echo   run.bat src/tests/exams/exame2
    echo   run.bat exame2                 ^(Resolves to src/tests/exams/exame2.c^)
    echo   run.bat src/tests/test_menu.c
    echo ================================================================
    exit /b 1
)

set "TARGET=%~1"

:: If target does not contain "/" or "\", assume it is in src/tests/exams/
echo %TARGET% | findstr /R /C:"[/\\]" >nul
if errorlevel 1 (
    set "TARGET=src/tests/exams/%TARGET%"
)

:: Append .c extension if not present
if /i not "%TARGET:~-2%"==".c" (
    set "TARGET=%TARGET%.c"
)

:: Replace forward slashes with backslashes for Windows file system operations
set "TARGET_WIN=!TARGET:/=\!"

:: Extract directory and filename
for %%F in ("!TARGET_WIN!") do (
    set "TARGET_DIR=%%~dpF"
    set "TARGET_NAME=%%~nF"
)

:: Count how many directory parts exist to dynamically set REL_PATH
set "COUNT=0"
set "TEMP_TARGET=!TARGET!"
set "TEMP_TARGET=!TEMP_TARGET:/= !"
set "TEMP_TARGET=!TEMP_TARGET:\= !"
for %%x in (!TEMP_TARGET!) do (
    set /a COUNT+=1
)
set /a COUNT-=1

set "REL_PATH="
if %COUNT% gtr 0 (
    for /L %%i in (1,1,%COUNT%) do (
        set "REL_PATH=!REL_PATH!../"
    )
)
if "!REL_PATH!"=="" set "REL_PATH=./"

:: If file does not exist, create it with your custom widgets GTK4 boilerplate
if not exist "!TARGET_WIN!" (
    echo [INFO] File '!TARGET!' does not exist.
    echo [INFO] Creating directory '!TARGET_DIR!'...
    if not exist "!TARGET_DIR!" mkdir "!TARGET_DIR!"
    
    echo [INFO] Creating file '!TARGET!' with default Minimal GTK4 Fenetre template...
    (
        echo #include ^<gtk/gtk.h^>
        echo #include ^<stdio.h^>
        echo #include ^<stdlib.h^>
        echo #include ^<string.h^>
        echo.
        echo #include "!REL_PATH!widgets/headers/fenetre.h"
        echo.
        echo static void on_activate^(GtkApplication *app, gpointer user_data^) {
        echo     ^(void^)user_data;
        echo.
        echo     // 1. Initialize and Create the Custom Fenetre with title as file name
        echo     Fenetre fenetre;
        echo     fenetre_initialiser^(^&fenetre^);
        echo     g_free^(fenetre.title^);
        echo     fenetre.title = malloc^(strlen^("!TARGET_NAME!"^) + 1^);
        echo     strcpy^(fenetre.title, "!TARGET_NAME!"^);
        echo     fenetre.taille.width = 800;
        echo     fenetre.taille.height = 600;
        echo.
        echo     GtkWidget *window = fenetre_creer^(^&fenetre, app^);
        echo.
        echo     // 2. Present the window
        echo     gtk_window_present^(GTK_WINDOW^(window^)^);
        echo }
        echo.
        echo int main^(int argc, char *argv[]^) {
        echo     GtkApplication *app = gtk_application_new^("fr.exam.!TARGET_NAME!", G_APPLICATION_DEFAULT_FLAGS^);
        echo     g_signal_connect^(app, "activate", G_CALLBACK^(on_activate^), NULL^);
        echo     int status = g_application_run^(G_APPLICATION^(app^), argc, argv^);
        echo     g_object_unref^(app^);
        echo     return status;
        echo }
    ) > "!TARGET_WIN!"
    echo [OK] Boilerplate created successfully at '!TARGET!'.
)

:: Terminate any previously running instances of this executable to release the file lock
taskkill /f /im "!TARGET_NAME!.exe" >nul 2>&1

:: Compile the C file
echo [INFO] Compiling '!TARGET!'...

:: Fetch compiler flags and libs using pkg-config
set "CFLAGS="
set "LIBS="
for /f "delims=" %%i in ('pkg-config --cflags gtk4 2^>nul') do set "CFLAGS=%%i"
for /f "delims=" %%i in ('pkg-config --libs gtk4 2^>nul') do set "LIBS=%%i"

:: If pkg-config is not found or failed, notify user
if "!CFLAGS!"=="" (
    echo [WARNING] pkg-config failed or was not found in PATH.
    echo [WARNING] Compiling using default guess paths...
)

set "EXE_NAME=!TARGET_DIR!!TARGET_NAME!.exe"

:: Compile with GCC, combining target C file and custom widgets sources
echo [COMMAND] gcc -o "!EXE_NAME!" "!TARGET_WIN!" widgets/sources/*.c -Isrc -Iwidgets/headers !CFLAGS! !LIBS! -lwinmm -lm
gcc -o "!EXE_NAME!" "!TARGET_WIN!" widgets/sources/*.c -Isrc -Iwidgets/headers !CFLAGS! !LIBS! -lwinmm -lm

if %ERRORLEVEL% equ 0 (
    echo [OK] Compilation successful! Output binary: '!EXE_NAME!'
    echo [INFO] Running application...
    echo.
    "!EXE_NAME!"
    echo.
    echo [INFO] Execution completed.
) else (
    echo [ERROR] Compilation failed. Please check compiler errors above.
    exit /b %ERRORLEVEL%
)
