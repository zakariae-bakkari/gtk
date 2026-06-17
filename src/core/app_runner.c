#include "app_runner.h"

/**
 * gtk_app_run - Crée, connecte et lance une GtkApplication.
 *
 * Encapsule le pattern boilerplate répété dans chaque main GTK :
 *   gtk_application_new → g_signal_connect → g_application_run → g_object_unref
 *
 * Utilisable dans n'importe quel fichier main du projet en incluant app_runner.h.
 */
int runner(const char *app_id,
                GCallback   activate,
                gpointer    user_data,
                int         argc,
                char      **argv)
{
    GtkApplication *app = gtk_application_new(app_id, G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", activate, user_data);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
