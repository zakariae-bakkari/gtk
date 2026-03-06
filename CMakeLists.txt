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
        widgets/sources/conteneur.c
        widgets/sources/common.c
        widgets/sources/texte.c
        widgets/sources/image.c

        # Inputs
        widgets/sources/champ_texte.c
        widgets/sources/champ_motdepasse.c
        widgets/sources/champ_nombre.c
        widgets/sources/champ_select.c
        widgets/sources/champ_zone_texte.c
        widgets/sources/slider.c
        widgets/sources/menu.c
        widgets/sources/dialog.c
        # App
        src/main.c
        #        src/tests/test_champ_text.c
        #        src/tests/test_inputs.c
        #        src/tests/test_callbacks.c
        # src/tests/test_conteneur.c
        #        src/tests/test_buttons.c
        #        src/tests/test_texte.c
        #        src/tests/test_image.c
        #        src/tests/test_champ_nombre.c
        #        src/tests/test_slider.c
        #        src/tests/test_menu.c
        # src/tests/test_dialog.c
)

target_link_libraries(gtk PRIVATE PkgConfig::GTK4)
target_include_directories(gtk PRIVATE widgets/headers)
