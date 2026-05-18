#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../widgets/headers/fenetre.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    // 1. Initialize and Create the Custom Fenetre with title as file name
    Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = malloc(strlen("runbat") + 1);
    strcpy(fenetre.title, "runbat");
    fenetre.taille.width = 800;
    fenetre.taille.height = 600;

    GtkWidget *window = fenetre_creer(&fenetre, app);

    // 2. Present the window
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("fr.exam.runbat", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
