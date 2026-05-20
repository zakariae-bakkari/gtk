#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/b.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/export_xml.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    // 1. Initialize and Create the Custom Fenetre with title as file name
    static Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = malloc(strlen("meriem") + 1);
    strcpy(fenetre.title, "meriem");
    fenetre.taille.width = 800;
    fenetre.taille.height = 600;
    fenetre.scroll_mode = SCROLL_VERTICAL;
    fenetre.scroll_overlay = TRUE;
    fenetre.background_image = "resources/images/meriem.png";
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
    Button btn;

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
    GtkApplication *app = gtk_application_new("fr.exam.meriem", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
