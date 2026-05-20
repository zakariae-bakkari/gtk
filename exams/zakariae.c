#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/texte.h"
#include "../widgets/headers/bouton.h"
#include "../widgets/headers/bouton_radio.h"
#include "../widgets/headers/bouton_checklist.h"
#include "../widgets/headers/champ_texte.h"
#include "../widgets/headers/champ_motdepasse.h"
#include "../widgets/headers/champ_nombre.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/champ_zone_texte.h"
#include "../widgets/headers/image.h"
#include "../widgets/headers/video.h"
#include "../widgets/headers/slider.h"
#include "../widgets/headers/menu.h"
#include "../widgets/headers/dialog.h"
#include "../widgets/headers/export_xml.h"
#include "../widgets/headers/xml_parser.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    // 1. Initialize and Create the Custom Fenetre with title as file name
    static Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = malloc(strlen("zakariae") + 1);
    strcpy(fenetre.title, "zakariae");
    fenetre.taille.width = 800;
    fenetre.taille.height = 600;
    fenetre.scroll_mode = SCROLL_VERTICAL;
    fenetre.scroll_overlay = TRUE;

    fenetre.icon_path = "resources/icons/zcode.png";
    fenetre.ico_path = "resources/icons/zcode.ico";

    GtkWidget *window = fenetre_creer(&fenetre, app);

    #ifdef _WIN32
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, &fenetre);
    #endif

    // 2. Initialize and Create the Custom Conteneur main_box by default
    static Conteneur main_box;
    conteneur_initialiser(&main_box);
    main_box.orientation = CONTENEUR_VERTICAL;
    main_box.espacement = 15;
    main_box.padding.haut = 15;
    main_box.padding.bas = 15;
    main_box.padding.gauche = 20;
    main_box.padding.droite = 20;
    main_box.enfants_hexpand = TRUE;
    main_box.enfants_vexpand = TRUE;

    GtkWidget *p_main_box = conteneur_creer(&main_box);
    fenetre_ajouter(&fenetre, p_main_box);

    // 3. XML export setup
    ExportContext ctx;
    export_context_init(&ctx);
    export_ajouter_fenetre(&ctx, &fenetre);
    export_ajouter_conteneur(&ctx, &main_box);

    // --- Place your other widgets here and add them to the export context:
    // export_ajouter_xxx(&ctx, &widget);

    // 4. Generate XML interface file
    generer_fichier_interface(&ctx, "interface.txt");

    // 5. Present the window
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("fr.exam.zakariae", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
