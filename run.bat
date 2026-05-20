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
set "IS_XML=0"

:: Check if the input target ends in .xml or .txt (case-insensitive)
if /i "%TARGET:~-4%"==".xml" set "IS_XML=1"
if /i "%TARGET:~-4%"==".txt" set "IS_XML=1"

:: If target does not contain "/" or "\", and does not exist in root, assume it is in src/tests/exams/
if not exist "%TARGET%" (
    echo %TARGET% | findstr /R /C:"[/\\]" >nul
    if errorlevel 1 (
        if "!IS_XML!"=="1" (
            if exist "src/tests/exams/%TARGET%" (
                set "TARGET=src/tests/exams/%TARGET%"
            )
        ) else (
            set "TARGET=src/tests/exams/%TARGET%"
        )
    )
)

:: Append .c extension if not present and NOT an XML/TXT file
if "!IS_XML!"=="0" (
    if /i not "%TARGET:~-2%"==".c" (
        set "TARGET=%TARGET%.c"
    )
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

:: If file does not exist
if not exist "!TARGET_WIN!" (
    if "!IS_XML!"=="1" (
        echo [ERROR] XML/TXT interface file '!TARGET!' does not exist.
        exit /b 1
    )
    
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
        echo #ifdef _WIN32
        echo #include ^<windows.h^>
        echo #endif
        echo.
        echo #include "!REL_PATH!widgets/headers/fenetre.h"
        echo #include "!REL_PATH!widgets/headers/conteneur.h"
        echo #include "!REL_PATH!widgets/headers/export_xml.h"
        echo.
        echo static void on_activate^(GtkApplication *app, gpointer user_data^) {
            echo     ^(void^)user_data;
            echo.
            echo     // 1. Initialize and Create the Custom Fenetre with title as file name
            echo     static Fenetre fenetre;
            echo     fenetre_initialiser^(^&fenetre^);
            echo     g_free^(fenetre.title^);
            echo     fenetre.title = malloc^(strlen^("!TARGET_NAME!"^) + 1^);
            echo     strcpy^(fenetre.title, "!TARGET_NAME!"^);
            echo     fenetre.taille.width = 800;
            echo     fenetre.taille.height = 600;
            echo     fenetre.scroll_mode = SCROLL_VERTICAL;
            echo     fenetre.scroll_overlay = TRUE;
            echo.
            echo     fenetre.icon_path = "resources/icons/zcode.png";
            echo     fenetre.ico_path = "resources/icons/zcode.ico";
            echo.
            echo     GtkWidget *window = fenetre_creer^(^&fenetre, app^);
            echo.
            echo     #ifdef _WIN32
            echo     g_timeout_add^(100, ^(GSourceFunc^)fenetre_appliquer_icone_taskbar, ^&fenetre^);
            echo     #endif
            echo.
            echo     // 2. Initialize and Create the Custom Conteneur main_box by default
            echo     static Conteneur main_box;
            echo     conteneur_initialiser^(^&main_box^);
            echo     main_box.orientation = CONTENEUR_VERTICAL;
            echo     main_box.espacement = 15;
            echo     main_box.padding.haut = 15;
            echo     main_box.padding.bas = 15;
            echo     main_box.padding.gauche = 20;
            echo     main_box.padding.droite = 20;
            echo     main_box.enfants_hexpand = TRUE;
            echo     main_box.enfants_vexpand = TRUE;
            echo.
            echo     GtkWidget *p_main_box = conteneur_creer^(^&main_box^);
            echo     fenetre_ajouter^(^&fenetre, p_main_box^);
            echo.
            echo     // 3. XML export setup
            echo     ExportContext ctx;
            echo     export_context_init^(^&ctx^);
            echo     export_ajouter_fenetre^(^&ctx, ^&fenetre^);
            echo     export_ajouter_conteneur^(^&ctx, ^&main_box^);
            echo.
            echo     // --- Place your other widgets here and add them to the export context:
            echo     // export_ajouter_xxx^(^&ctx, ^&widget^);
            echo.
            echo     // 4. Generate XML interface file
            echo     generer_fichier_interface^(^&ctx, "interface.txt"^);
            echo.
            echo     // 5. Present the window
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

:: Determine compile and run configurations
if "!IS_XML!"=="1" (
    set "SRC_TO_COMPILE=src\xml_runner.c"
    set "EXE_NAME=src\xml_runner.exe"
    set "RUN_COMMAND="!EXE_NAME!" "!TARGET_WIN!""
    taskkill /f /im "xml_runner.exe" >nul 2>&1
) else (
    set "SRC_TO_COMPILE=!TARGET_WIN!"
    set "EXE_NAME=!TARGET_DIR!!TARGET_NAME!.exe"
    set "RUN_COMMAND="!EXE_NAME!""
    taskkill /f /im "!TARGET_NAME!.exe" >nul 2>&1
)

:: Compile the C file
echo [INFO] Compiling '!SRC_TO_COMPILE!'...

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

:: Compile with GCC, combining target C file and custom widgets sources
set "EXTRA_SRCS="
if /i "!TARGET_NAME!"=="game" (
    set "EXTRA_SRCS=src/bassin.c src/draw.c src/entities.c src/screen_accueil.c src/screen_createur.c src/screen_jeux.c src/assets.c src/sound.c"
)
:: Define ANSI Escape character for color and cursor manipulation
for /f %%A in ('"prompt $E & echo on & for %%B in (1) do rem"') do set "ESC=%%A"

echo !ESC![96m[INFO] Starting project compilation...!ESC![0m
del compile_done.tmp >nul 2>&1
del compile.log >nul 2>&1
del compile_bg.bat >nul 2>&1

:: Create a temporary helper batch file to handle background execution cleanly
(
    echo @echo off
    echo gcc -o "!EXE_NAME!" "!SRC_TO_COMPILE!" !EXTRA_SRCS! widgets/sources/*.c -Isrc -Iwidgets/headers !CFLAGS! !LIBS! -lwinmm -lm -Wno-deprecated-declarations ^> compile.log 2^>^&1
    echo echo %%ERRORLEVEL%% ^> compile_done.tmp
) > compile_bg.bat

:: Start the helper batch file in the background
start /B cmd /c compile_bg.bat

set "BAR_WIDTH=30"
set "PROGRESS=0"

:loop
if not exist compile_done.tmp (
    set /a "NUM_BARS=(PROGRESS * BAR_WIDTH) / 100"
    set /a "NUM_SPACES=BAR_WIDTH - NUM_BARS"
    
    set "BAR="
    if !NUM_BARS! gtr 0 (
        set /a "NUM_EQS=NUM_BARS - 1"
        if !NUM_EQS! gtr 0 (
            for /L %%i in (1,1,!NUM_EQS!) do set "BAR=!BAR!="
        )
        set "BAR=!BAR!>"
    )
    set "SPACES="
    if !NUM_SPACES! gtr 0 (
        for /L %%i in (1,1,!NUM_SPACES!) do set "SPACES=!SPACES! "
    )
    
    echo | set /p="!ESC![1G!ESC![2K!ESC![94mBuilding: [!BAR!!SPACES!] !PROGRESS!%%!ESC![0m"
    
    if !PROGRESS! LSS 95 (
        set /a PROGRESS+=5
    )
    
    :: Tiny delay (approx 80-100ms)
    for /L %%i in (1,1,5000) do rem
    goto loop
)

set /p STATUS=<compile_done.tmp
:: Trim any trailing spaces
for /f "tokens=1" %%A in ("!STATUS!") do set "STATUS=%%A"

if "!STATUS!"=="0" (
    set "BAR="
    for /L %%i in (1,1,%BAR_WIDTH%) do set "BAR=!BAR!="
    echo | set /p="!ESC![1G!ESC![2K!ESC![92mBuilding: [!BAR!] 100%% [OK]!ESC![0m"
    echo.
    echo !ESC![92m[OK] Compilation successful! Output binary: '!EXE_NAME!'!ESC![0m
    
    del compile_done.tmp >nul 2>&1
    del compile.log >nul 2>&1
    del compile_bg.bat >nul 2>&1
    
    echo [INFO] Running application...
    echo.
    !RUN_COMMAND!
    echo.
    echo [INFO] Execution completed.
) else (
    set "BAR="
    for /L %%i in (1,1,%BAR_WIDTH%) do set "BAR=!BAR!="
    echo | set /p="!ESC![1G!ESC![2K!ESC![91mBuilding: [!BAR!] Failed! [ERROR]!ESC![0m"
    echo.
    echo.
    echo !ESC![91m================================================================!ESC![0m
    echo !ESC![91m                      COMPILATION ERRORS                        !ESC![0m
    echo !ESC![91m================================================================!ESC![0m
    
    if exist compile.log (
        powershell -Command "Get-Content compile.log | ForEach-Object { Write-Host $_ -ForegroundColor Red }"
    )
    echo !ESC![91m================================================================!ESC![0m
    
    del compile_done.tmp >nul 2>&1
    del compile.log >nul 2>&1
    del compile_bg.bat >nul 2>&1
    exit /b !STATUS!
)
