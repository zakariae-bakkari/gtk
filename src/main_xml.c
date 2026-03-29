/**
 * main_xml.c
 *
 * Exemple d'utilisation du parser XML pour construire une interface GTK.
 * Compare avec l'ancien main.c : toute l'UI est dans ui_exemple.xml,
 * ce fichier ne contient plus que l'application + les callbacks.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include "../widgets/headers/xml_parser.h"

/* ================================================================
 *  CALLBACKS
 *  (à connecter dynamiquement si nécessaire, ou via signal_connect
 *   après avoir récupéré le widget par son id CSS)
 * ================================================================ */

static void on_connexion(GtkWidget *w, gpointer data)
{
    printf("[ACTION] Bouton connexion cliqué\n");
    (void)w; (void)data;
}

static void on_menu_click(const char *id, gpointer data)
{
    printf("[MENU] Item cliqué : %s\n", id);
    (void)data;
}

/* ================================================================
 *  ACTIVATE
 * ================================================================ */
static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    /* ── Chargement de l'interface depuis le fichier XML ── */
    GtkWidget *window = xml_load_file("ui_exemple.txt", app);

    if (!window) {
        fprintf(stderr, "Erreur : impossible de charger ui_exemple.xml\n");
        return;
    }

    /* ── Exemple d'utilisation avancée : parse + introspection ── */
    /*
    XmlNode *root = xml_parser_parse_file("ui_exemple.xml");
    if (root) {
        // Lire un attribut
        const char *titre = xml_attr_get(root, "title");
        printf("Titre fenêtre : %s\n", titre ? titre : "(aucun)");

        // Construire l'UI
        window = xml_build_ui(root, app);
        xml_node_free(root);   // libérer l'arbre après construction
    }
    */

    gtk_window_present(GTK_WINDOW(window));
}

/* ================================================================
 *  MAIN
 * ================================================================ */
int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("org.zcode.xml_demo",
                                               G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
