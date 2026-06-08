#include <gtk/gtk.h>

#include "pages/screen_bassin.h"
#include "pages/screen_home.h"
#include "fenetre.h"

Fenetre g_app_window;

static void show_screen(GtkWidget *child)
{
    if (!g_app_window.wind)
    {
        return;
    }

    fenetre_ajouter(&g_app_window, child);
}

void nav_to_accueil(void)
{
    show_screen(screen_accueil_create());
}

void nav_to_bassin(void)
{
    show_screen(screen_bassin_create());
}

void nav_to_createur(void)
{
    show_screen(screen_createur_create());
}

void nav_to_predateur(void)
{
    show_screen(screen_predateur_create());
}

void nav_to_survie(void)
{
    show_screen(screen_survie_create());
}

static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    fenetre_initialiser(&g_app_window);

    if (g_app_window.title)
        free(g_app_window.title);
    g_app_window.title = g_strdup("DEEP SHARK ATTACK");

    g_app_window.demarrer_maximisee = TRUE;

    g_app_window.icon_path = "resources/icons/app.png";
    g_app_window.ico_path = "resources/icons/app.ico";

    fenetre_creer(&g_app_window, app);
#ifdef _WIN32
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, &g_app_window);
#endif
    nav_to_accueil();

    gtk_window_present(GTK_WINDOW(g_app_window.wind));
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("fr.banc_poisson.app",
                                              G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}