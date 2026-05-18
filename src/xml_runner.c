/**
 * xml_runner.c
 *
 * Un exécuteur générique de fichiers XML/TXT d'interface GTK4.
 * Ce fichier charge dynamiquement un fichier d'interface (par défaut "interface.xml" ou "interface.txt")
 * et l'affiche à l'aide de notre parser XML de widgets.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../widgets/headers/xml_parser.h"

// Variable globale pour stocker le chemin du fichier XML à charger
static char *g_xml_path = NULL;

/* ================================================================
 *  CALLBACKS ET ACTIONS DE L'APPLICATION
 * ================================================================ */

// Callback par défaut pour les clics de bouton (si définis dans l'XML)
static void on_bouton_action(GtkWidget *w, gpointer data)
{
    const char *id = gtk_widget_get_name(w);
    printf("[XML-RUNNER] Bouton cliqué : ID='%s'\n", id ? id : "inconnu");
    (void)data;
}

// Callback par défaut pour les clics de menu
static void on_menu_click(const char *id, gpointer data)
{
    printf("[XML-RUNNER] Item de menu cliqué : '%s'\n", id);
    (void)data;
}

/* ================================================================
 *  ACTIVATE
 * ================================================================ */
static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    printf("[XML-RUNNER] Chargement du fichier d'interface : '%s'...\n", g_xml_path);

    // Charger et construire l'UI à partir du fichier XML
    GtkWidget *window = xml_load_file(g_xml_path, app);

    if (!window) {
        fprintf(stderr, "[ERREUR] Impossible de charger ou parser le fichier : '%s'\n", g_xml_path);
        return;
    }

    // Connecter les actions de base si nécessaire ou présenter la fenêtre
    printf("[XML-RUNNER] Interface chargée avec succès ! Présentation de la fenêtre...\n");
    gtk_window_present(GTK_WINDOW(window));
}

/* ================================================================
 *  MAIN
 * ================================================================ */
int main(int argc, char **argv)
{
    // Déterminer quel fichier charger
    // Priorité 1 : Argument en ligne de commande
    // Priorité 2 : "interface.xml" s'il existe
    // Priorité 3 : "interface.txt" s'il existe
    // Priorité 4 : "ui_exemple.txt" par défaut
    if (argc > 1) {
        g_xml_path = argv[1];
    } else {
        FILE *f = fopen("interface.xml", "r");
        if (f) {
            g_xml_path = "interface.xml";
            fclose(f);
        } else {
            f = fopen("interface.txt", "r");
            if (f) {
                g_xml_path = "interface.txt";
                fclose(f);
            } else {
                g_xml_path = "ui_exemple.txt";
            }
        }
    }

    // Initialiser GtkApplication
    GtkApplication *app = gtk_application_new("org.zcode.xml_runner", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Lancer la boucle principale (on ne passe pas les arguments pour éviter que Gtk les intercepte)
    int dummy_argc = 1;
    char *dummy_argv[] = { argv[0], NULL };
    int status = g_application_run(G_APPLICATION(app), dummy_argc, dummy_argv);

    g_object_unref(app);
    return status;
}
