/**
 * main_xml.c
 *
 * Exemple d'utilisation du parser XML pour construire une interface GTK.
 * Compare avec l'ancien main.c : toute l'UI est dans ui_exemple.xml,
 * ce fichier ne contient plus que l'application + les callbacks.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../widgets/headers/xml_parser.h"

#ifndef UI_EXEMPLE_PATH
#define UI_EXEMPLE_PATH "ui_exemple.txt"
#endif

/* ================================================================
 *  CALLBACKS
 * ================================================================ */

static void on_connexion(GtkWidget *w, gpointer data)
{
    printf("[ACTION] Bouton connexion clique\n");
    (void)w;
    (void)data;
}

static void on_menu_click(const char *id, gpointer data)
{
    printf("[MENU] Item clique : %s\n", id);
    (void)data;
}

/* ================================================================
 *  ACTIVATE
 * ================================================================ */
static void activate(GtkApplication *app, gpointer user_data)
{
    char *ui_path;
    GtkWidget *window;

    (void)user_data;

    ui_path = malloc(strlen(UI_EXEMPLE_PATH) + 1);
    strcpy(ui_path, UI_EXEMPLE_PATH);

    window = xml_load_file(ui_path, app);

    if (!window) {
        fprintf(stderr, "Erreur : impossible de charger %s\n", ui_path);
        free(ui_path);
        return;
    }

    gtk_window_present(GTK_WINDOW(window));
    free(ui_path);
}

/* ================================================================
 *  MAIN
 * ================================================================ */
int main(int argc, char **argv)
{
    char *app_id;
    GtkApplication *app;
    int status;

    app_id = malloc(strlen("org.zcode.xml_demo") + 1);
    strcpy(app_id, "org.zcode.xml_demo");

    app = gtk_application_new(app_id, G_APPLICATION_DEFAULT_FLAGS);
    free(app_id);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
