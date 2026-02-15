#include <gtk/gtk.h>
#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/conteneur.h"

static void activate(GtkApplication *app, gpointer user_data) {
    // =========================================================================
    // 1. LA FENÊTRE (Le cadre)
    // =========================================================================
    Fenetre ma_fenetre;
    fenetre_initialiser(&ma_fenetre);

    ma_fenetre.title = "Démonstration Complète";
    ma_fenetre.taille.width = 800;
    ma_fenetre.taille.height = 600;
    ma_fenetre.color_bg = "#ecf0f1"; // Fond gris très clair (Clouds)
    ma_fenetre.demarrer_maximisee = false;

    // Boutons de la fenêtre
    ma_fenetre.bouton_fermer = true;
    ma_fenetre.bouton_agrandir = true;
    ma_fenetre.bouton_reduire = true;

    GtkWidget *le_widget_fenetre = fenetre_creer(&ma_fenetre);

    // =========================================================================
    // 2. CONTENEUR PRINCIPAL (Layout vertical global)
    // =========================================================================
    Conteneur main_layout;
    conteneur_initialiser(&main_layout);

    main_layout.orientation = CONTENEUR_VERTICAL;
    main_layout.espacement = 30; // Espace entre les cartes
    main_layout.align_x = ALIGNEMENT_REMPLIR;
    main_layout.align_y = ALIGNEMENT_CENTRE; // On centre tout verticalement

    // Marges globales pour ne pas coller aux bords de la fenêtre
    main_layout.marges.haut = 20;
    main_layout.marges.bas = 20;
    main_layout.marges.gauche = 50;
    main_layout.marges.droite = 50;

    GtkWidget *widget_principal = conteneur_creer(&main_layout);

    // =========================================================================
    // 3. CARTE 1 : Une boite de style "Login" (Centrée, Bordure bleue)
    // =========================================================================
    Conteneur carte_login;
    conteneur_initialiser(&carte_login);

    carte_login.orientation = CONTENEUR_VERTICAL;
    carte_login.espacement = 15;

    // Dimensions Fixes
    carte_login.taille.largeur = 300;
    carte_login.taille.hauteur = -1; // Hauteur auto selon contenu

    // Alignement : Centré horizontalement dans le conteneur principal
    carte_login.align_x = ALIGNEMENT_CENTRE;

    // Style visuel
    carte_login.couleur_fond = "white";
    carte_login.bordure_largeur = 2;
    carte_login.bordure_couleur = "#3498db"; // Bleu
    carte_login.bordure_rayon = 10;          // Coins arrondis

    // Padding interne (pour que le texte ne touche pas la bordure)
    carte_login.padding.haut = 20;
    carte_login.padding.bas = 20;
    carte_login.padding.gauche = 20;
    carte_login.padding.droite = 20;

    GtkWidget *widget_carte1 = conteneur_creer(&carte_login);

    // Ajout de contenu factice dans la carte
    GtkWidget *lbl_titre = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(lbl_titre), "<span size='x-large' weight='bold' color='#2c3e50'>Connexion</span>");
    conteneur_ajouter(&carte_login, lbl_titre);

    GtkWidget *btn_login = gtk_button_new_with_label("Se Connecter");
    conteneur_ajouter(&carte_login, btn_login);


    // =========================================================================
    // 4. CARTE 2 : Une boite d'information (Large, Bordure Rouge)
    // =========================================================================
    Conteneur carte_info;
    conteneur_initialiser(&carte_info);

    carte_info.orientation = CONTENEUR_HORIZONTAL; // Texte à gauche, bouton à droite
    carte_info.espacement = 20;

    // Dimensions : Largeur max, hauteur fixe
    carte_info.taille.largeur = 500;
    carte_info.taille.hauteur = 80;

    carte_info.align_x = ALIGNEMENT_CENTRE;

    // Style
    carte_info.couleur_fond = "#fab1a0"; // Saumon clair
    carte_info.bordure_largeur = 1;
    carte_info.bordure_couleur = "#e17055"; // Rouge foncé
    carte_info.bordure_rayon = 5;

    // Padding
    carte_info.padding.haut = 10;
    carte_info.padding.bas = 10;
    carte_info.padding.gauche = 20;
    carte_info.padding.droite = 20;

    GtkWidget *widget_carte2 = conteneur_creer(&carte_info);

    // Contenu
    GtkWidget *lbl_info = gtk_label_new("Message d'alerte système !");
    // On force l'alignement du label à gauche
    gtk_widget_set_hexpand(lbl_info, TRUE);
    gtk_widget_set_halign(lbl_info, GTK_ALIGN_START);

    GtkWidget *btn_ok = gtk_button_new_with_label("OK");

    conteneur_ajouter(&carte_info, lbl_info);
    conteneur_ajouter(&carte_info, btn_ok);


    // =========================================================================
    // 5. ASSEMBLAGE FINAL
    // =========================================================================

    // Ajouter les cartes au conteneur principal
    conteneur_ajouter(&main_layout, widget_carte1);
    conteneur_ajouter(&main_layout, widget_carte2);

    // Ajouter le conteneur principal à la fenêtre
    gtk_window_set_child(GTK_WINDOW(le_widget_fenetre), widget_principal);

    // Afficher
    gtk_window_set_application(GTK_WINDOW(le_widget_fenetre), app);
    gtk_window_present(GTK_WINDOW(le_widget_fenetre));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.ilisi.demo.detaillee", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
