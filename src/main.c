#include <gtk/gtk.h>
#include "screen_accueil.h"
#include "screen_createur.h"
#include "screen_jeux.h"

typedef struct {
    GtkWidget *stack;
    GtkWidget *screen_accueil;
    GtkWidget *screen_createur;
    GtkWidget *screen_predateur;
    GtkWidget *screen_survie;
} AppData;

static AppData app;

/* Fonctions de navigation — appelées par les écrans */
void nav_to_accueil    (void) { gtk_stack_set_visible_child_name(GTK_STACK(app.stack), "accueil"); }
void nav_to_createur   (void) { gtk_stack_set_visible_child_name(GTK_STACK(app.stack), "createur"); }
void nav_to_predateur  (void) { gtk_stack_set_visible_child_name(GTK_STACK(app.stack), "predateur"); }
void nav_to_survie     (void) { gtk_stack_set_visible_child_name(GTK_STACK(app.stack), "survie"); }

static void activate(GtkApplication *gapp, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(gapp);
    gtk_window_set_title(GTK_WINDOW(window), "Banc de Poisson");
    gtk_window_set_default_size(GTK_WINDOW(window), 1024, 700);

    app.stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app.stack),
                                  GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(app.stack), 400);

    app.screen_accueil   = screen_accueil_create();
    app.screen_createur  = screen_createur_create();
    app.screen_predateur = screen_predateur_create();
    app.screen_survie    = screen_survie_create();

    gtk_stack_add_named(GTK_STACK(app.stack), app.screen_accueil,   "accueil");
    gtk_stack_add_named(GTK_STACK(app.stack), app.screen_createur,  "createur");
    gtk_stack_add_named(GTK_STACK(app.stack), app.screen_predateur, "predateur");
    gtk_stack_add_named(GTK_STACK(app.stack), app.screen_survie,    "survie");

    gtk_window_set_child(GTK_WINDOW(window), app.stack);
    gtk_stack_set_visible_child_name(GTK_STACK(app.stack), "accueil");
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *gapp = gtk_application_new("fr.banc_poisson.app",
                                               G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(gapp, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(gapp), argc, argv);
    g_object_unref(gapp);
    return status;
}