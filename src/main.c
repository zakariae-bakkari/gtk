/* ===========================================================================
 * main.c — Banc de Poisson
 * Point d'entrée de l'application GTK4
 *
 * Utilise les widgets de la bibliothèque :
 *   Fenetre  → fenêtre principale avec GtkStack
 *   Conteneur → layout général
 *
 * Compilation :
 *   make
 * ===========================================================================*/

#include <gtk/gtk.h>
#include "fenetre.h"
#include "conteneur.h"
#include "game.h"
#include "screens.h"

/* Données globales de l'application (une seule instance) */
static AppData g_app;

/* ---- Callback de fermeture propre ---- */
static void on_window_destroy(GtkWidget *w, gpointer data) {
    game_free((AppData *)data);
}

/* ---- Construction de l'UI principale ---- */
static void on_activate(GtkApplication *gapp, gpointer data) {
    AppData *app = (AppData *)data;
    app->app = gapp;

    /* -------------------------------------------------------
     * 1. FENETRE PRINCIPALE  (widget Fenetre de la biblio)
     * ------------------------------------------------------- */
    Fenetre fen;
    fenetre_initialiser(&fen);

    fen.title              = "🌊 Banc de Poisson";
    fen.taille.width       = 1100;
    fen.taille.height      = 760;
    fen.resizable          = TRUE;
    fen.color_bg           = "#0A1628";
    fen.position           = WIN_POS_CENTER;
    fen.bouton_fermer      = TRUE;
    fen.bouton_agrandir    = TRUE;
    fen.bouton_reduire     = TRUE;
    fen.titre_align        = TITRE_ALIGN_CENTRE;

    app->window = fenetre_creer(&fen, gapp);
    g_signal_connect(app->window, "destroy",
                     G_CALLBACK(on_window_destroy), app);

    /* -------------------------------------------------------
     * 2. STACK  — un enfant par écran
     * ------------------------------------------------------- */
    app->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(app->stack),
                                  GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(app->stack), 250);

    /* -------------------------------------------------------
     * 3. CONSTRUCTION DES ÉCRANS
     * ------------------------------------------------------- */
    app->screen_accueil   = screen_accueil_creer(app);
    app->screen_createur  = screen_createur_creer(app);
    app->screen_predateur = screen_predateur_creer(app);
    app->screen_survie    = screen_survie_creer(app);

    gtk_stack_add_named(GTK_STACK(app->stack),
                        app->screen_accueil,   "accueil");
    gtk_stack_add_named(GTK_STACK(app->stack),
                        app->screen_createur,  "createur");
    gtk_stack_add_named(GTK_STACK(app->stack),
                        app->screen_predateur, "predateur");
    gtk_stack_add_named(GTK_STACK(app->stack),
                        app->screen_survie,    "survie");

    /* -------------------------------------------------------
     * 4. ATTACH DU STACK À LA FENÊTRE
     * ------------------------------------------------------- */
    GtkWidget *content = fenetre_get_content_container(&fen);
    if (content) {
        gtk_box_append(GTK_BOX(content), app->stack);
    } else {
        gtk_window_set_child(GTK_WINDOW(app->window), app->stack);
    }

    /* Démarre sur l'écran d'accueil */
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "accueil");
    app->current_screen = SCREEN_ACCUEIL;

    /* -------------------------------------------------------
     * 5. AFFICHAGE
     * ------------------------------------------------------- */
    gtk_window_present(GTK_WINDOW(app->window));
}

/* ---- Entrée principale ---- */
int main(int argc, char *argv[]) {
    /* Initialise l'état du jeu */
    game_init(&g_app);

    /* Crée l'application GTK */
    GtkApplication *gapp = gtk_application_new(
        "com.bancdepoisson.app",
        G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(gapp, "activate",
                     G_CALLBACK(on_activate), &g_app);

    int status = g_application_run(G_APPLICATION(gapp), argc, argv);
    g_object_unref(gapp);
    return status;
}