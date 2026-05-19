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
        widgets/sources/video.c

        # Inputs
        widgets/sources/champ_texte.c
        widgets/sources/champ_motdepasse.c
        widgets/sources/champ_nombre.c
        widgets/sources/champ_select.c
        widgets/sources/champ_zone_texte.c
        widgets/sources/slider.c
        widgets/sources/menu.c
        widgets/sources/dialog.c
        widgets/sources/xml_parser.c
        # App
        src/main.c
        src/bassin.c
        src/draw.c
        src/entities.c
        src/screen_accueil.c
        src/screen_createur.c
        src/screen_jeux.c
        src/assets.c
        src/sound.c
        widgets/sources/export_xml.c

)

target_link_libraries(gtk PRIVATE PkgConfig::GTK4 winmm)
target_include_directories(gtk PRIVATE widgets/headers)
target_compile_definitions(gtk PRIVATE
    UI_EXEMPLE_PATH="${CMAKE_SOURCE_DIR}/ui_exemple.txt"
    RESOURCES_PATH="${CMAKE_SOURCE_DIR}/resources"
)
