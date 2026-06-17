#ifndef APP_RUNNER_H
#define APP_RUNNER_H

#include <gtk/gtk.h>

/**
 * gtk_app_run - Lance une application GTK de manière standardisée.
 *
 * @param app_id   : Identifiant unique de l'application (ex: "fr.monapp.app")
 * @param activate : Callback appelé au signal "activate" de GtkApplication
 * @param user_data: Données passées au callback (peut être NULL)
 * @param argc     : Nombre d'arguments (depuis main)
 * @param argv     : Tableau d'arguments (depuis main)
 *
 * @return Code de sortie de l'application (0 = succès)
 *
 * Utilisation typique dans un main :
 *
 *   int main(int argc, char *argv[]) {
 *       return gtk_app_run("fr.monapp.app", on_activate, NULL, argc, argv);
 *   }
 */
int runner(const char *app_id,
                GCallback   activate,
                gpointer    user_data,
                int         argc,
                char      **argv);

#endif /* APP_RUNNER_H */
