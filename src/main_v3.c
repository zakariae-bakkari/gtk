#include <gtk/gtk.h>

#include "ui/screen_bassin.h"
#include "ui/screen_stubs.h"

GtkWidget *g_window = NULL;

static void show_screen(GtkWidget *child)
{
    if (!g_window)
    {
        return;
    }

    gtk_window_set_child(GTK_WINDOW(g_window), NULL);
    gtk_window_set_child(GTK_WINDOW(g_window), child);
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

    GtkWidget *window = gtk_application_window_new(app);
    g_window = window;

    gtk_window_set_title(GTK_WINDOW(window), "Banc de poisson");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 720);

    nav_to_bassin();

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("fr.banc_poisson.app_v3",
                                              G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}