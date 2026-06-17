cmake_minimum_required(VERSION 3.26)
project(gtk C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

add_executable(gtk
    src/main.c
    src/modele/poisson.c
    src/pages/screen_bassin.c
    src/pages/screen_bassin_helpers.c
    src/pages/screen_home.c
    src/dialogs/bassin_dialogs.c
    src/actions/bassin_interactions.c
    src/actions/bassin_menu.c
    src/actions/bassin_sidebar.c
    src/simulation/bassin_simulation.c
    src/simulation/bassin_xml.c
    src/core/app_runner.c
    src/core/sound.c

    widgets/sources/bouton.c
    widgets/sources/fenetre.c
    widgets/sources/bouton_radio.c
    widgets/sources/bouton_checklist.c
    widgets/sources/conteneur.c
    widgets/sources/common.c
    widgets/sources/texte.c
    widgets/sources/image.c
    widgets/sources/video.c
    widgets/sources/barre_progression.c
    widgets/sources/separateur.c

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
    widgets/sources/export_xml.c

)

target_link_libraries(gtk PRIVATE PkgConfig::GTK4 winmm)
target_include_directories(gtk PRIVATE widgets/headers)
target_compile_definitions(gtk PRIVATE
    UI_EXEMPLE_PATH="ui_exemple.txt"
    RESOURCES_PATH="resources"
)

# Copy resources folder next to the built executable so relative paths work
add_custom_command(TARGET gtk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:gtk>/resources"
)
