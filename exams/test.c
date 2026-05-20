#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/export_xml.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    // 1. Initialize and Create the Custom Fenetre with title as file name
    static Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = malloc(strlen("test") + 1);
    strcpy(fenetre.title, "test");
    fenetre.taille.width = 800;
    fenetre.taille.height = 600;

    fenetre.icon_path = "resources/icons/zcode.png";
    fenetre.ico_path = "resources/icons/zcode.ico";

    GtkWidget *window = fenetre_creer(&fenetre, app);

    #ifdef _WIN32
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, &fenetre);
    #endif

    // 2. XML export setup
    ExportContext ctx;
    export_context_init(&ctx);
    export_ajouter_fenetre(&ctx, &fenetre);

    // --- Place your other widgets here and add them to the export context:
    // export_ajouter_xxx(&ctx, &widget);

    // 3. Generate XML interface file
    generer_fichier_interface(&ctx, "interface.txt");

    // 4. Present the window
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("fr.exam.test", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
