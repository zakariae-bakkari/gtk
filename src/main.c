#include <gtk/gtk.h>
#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/conteneur.h"

static void activate(GtkApplication *app, gpointer user_data) {
    // -----------------------------------------------------------
    // 1. CRÉATION DE LA FENÊTRE (Le cadre principal)
    // -----------------------------------------------------------
    Fenetre ma_fenetre;
    fenetre_initialiser(&ma_fenetre);

    ma_fenetre.title = "Démo : Fenêtre et Conteneurs Colorés";
    ma_fenetre.taille.width = 600;
    ma_fenetre.taille.height = 400;
    ma_fenetre.color_bg = "#2c3e50"; // Gris foncé (Midnight Blue)
    ma_fenetre.demarrer_maximisee = true;

    // Configuration des boutons
    ma_fenetre.bouton_fermer = true;
    ma_fenetre.bouton_agrandir = true;
    ma_fenetre.bouton_reduire = true;

    GtkWidget *le_widget_fenetre = fenetre_creer(&ma_fenetre);

    // -----------------------------------------------------------
    // 2. CONTENEUR PRINCIPAL (Sert de mise en page globale)
    // -----------------------------------------------------------
    // Il sera vertical pour empiler le HAUT et le BAS
    Conteneur box_principale;
    conteneur_initialiser(&box_principale);

    box_principale.orientation = CONTENEUR_VERTICAL;
    box_principale.espacement = 20; // 20px d'espace entre le haut et le bas

    // On ajoute des marges pour ne pas coller aux bords de la fenêtre
    box_principale.marge_haut = 0;
    box_principale.marge_bas = 0;
    box_principale.marge_gauche = 0;
    box_principale.marge_droite = 0;
    box_principale.couleur_fond= "black";

    // Pas de couleur pour celui-ci (transparent par défaut)

    GtkWidget *widget_principal = conteneur_creer(&box_principale);

    // -----------------------------------------------------------
    // 3. CONTENEUR "HAUT" (En-tête)
    // -----------------------------------------------------------
    Conteneur box_haut;
    conteneur_initialiser(&box_haut);

    box_haut.orientation = CONTENEUR_HORIZONTAL; // Alignement horizontal
    box_haut.couleur_fond = "#1abc9c"; // TURQUOISE
    box_haut.espacement = 10;

    // Marges internes pour que le texte ne colle pas aux bords colorés
    box_haut.marge_haut = 0;
    box_haut.marge_bas = 10;
    box_haut.marge_gauche = 0;
    box_haut.marge_droite = 0;
    box_haut.

    GtkWidget *widget_haut = conteneur_creer(&box_haut);

    // Ajout d'un texte pour donner du volume à la boite
    GtkWidget *label_titre = gtk_label_new("ZONE EN-TÊTE (Turquoise)");
    // On force le texte en blanc pour la lisibilité (via balises Pango)
    gtk_label_set_markup(GTK_LABEL(label_titre), "<span color='white' weight='bold' size='large'>ZONE EN-TÊTE</span>");
    conteneur_ajouter(&box_haut, label_titre);


    // -----------------------------------------------------------
    // 4. CONTENEUR "BAS" (Contenu)
    // -----------------------------------------------------------
    Conteneur box_bas;
    conteneur_initialiser(&box_bas);

    box_bas.orientation = CONTENEUR_VERTICAL;
    box_bas.couleur_fond = "#e67e22"; // ORANGE

    // Marges internes
    box_bas.marge_haut = 20;
    box_bas.marge_bas = 20;
    box_bas.marge_gauche = 20;
    box_bas.marge_droite = 20;

    GtkWidget *widget_bas = conteneur_creer(&box_bas);

    // Ajout d'un texte
    GtkWidget *label_contenu = gtk_label_new("ZONE CONTENU (Orange)");
    gtk_label_set_markup(GTK_LABEL(label_contenu), "<span color='white'>Ceci est le corps de l'application.</span>");
    conteneur_ajouter(&box_bas, label_contenu);

    // Astuce : Pour que la zone orange prenne tout l'espace restant
    gtk_widget_set_vexpand(widget_bas, TRUE);


    // -----------------------------------------------------------
    // 5. ASSEMBLAGE (Poupées Russes)
    // -----------------------------------------------------------

    // A. On met le HAUT et le BAS dans la boite PRINCIPALE
    conteneur_ajouter(&box_principale, widget_haut);
    conteneur_ajouter(&box_principale, widget_bas);

    // B. On met la boite PRINCIPALE dans la FENÊTRE
    // Rappel: Une GtkWindow ne peut avoir qu'un seul enfant direct
    gtk_window_set_child(GTK_WINDOW(le_widget_fenetre), widget_principal);

    // -----------------------------------------------------------
    // 6. AFFICHAGE
    // -----------------------------------------------------------
    gtk_window_set_application(GTK_WINDOW(le_widget_fenetre), app);
    gtk_window_present(GTK_WINDOW(le_widget_fenetre));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.ilisi.demo.couleurs", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
