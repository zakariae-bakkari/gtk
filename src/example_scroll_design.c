#include <gtk/gtk.h>
#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/bouton.h"

static void on_activate(GtkApplication *app, gpointer user_data)
{
    // 1. Configuration de la fenêtre avec défilement vertical
    Fenetre fenetre_config;
    fenetre_initialiser(&fenetre_config);
    fenetre_config.title = "Design avec Conteneurs Scrollables";
    fenetre_config.taille.width = 600;
    fenetre_config.taille.height = 400;

    // Configurer le défilement vertical pour la fenêtre
    fenetre_set_scrollable(&fenetre_config, SCROLL_VERTICAL);
    fenetre_set_scroll_content_size(&fenetre_config, 0, 800); // Hauteur minimale de 800px pour forcer le scroll

    GtkWidget *window = fenetre_creer(&fenetre_config);
    gtk_application_add_window(app, GTK_WINDOW(window));

    // Conteneur principal pour organiser les deux conteneurs
    Conteneur conteneur_principal;
    conteneur_initialiser(&conteneur_principal);
    conteneur_principal.orientation = CONTENEUR_VERTICAL;
    conteneur_principal.espacement = 20;
    conteneur_principal.padding.haut = 20;
    conteneur_principal.padding.bas = 20;
    conteneur_principal.padding.gauche = 20;
    conteneur_principal.padding.droite = 20;

    GtkWidget *main_container = conteneur_creer(&conteneur_principal);

    // 2. Conteneur 1 - Normal (pas de défilement)
    Conteneur conteneur1;
    conteneur_initialiser(&conteneur1);
    conteneur1.orientation = CONTENEUR_VERTICAL;
    conteneur1.espacement = 10;
    conteneur1.couleur_fond = "#E8F4FD"; // Bleu clair
    conteneur1.bordure_largeur = 2;
    conteneur1.bordure_couleur = "#2196F3";
    conteneur1.bordure_rayon = 10;
    conteneur1.padding.haut = 15;
    conteneur1.padding.bas = 15;
    conteneur1.padding.gauche = 15;
    conteneur1.padding.droite = 15;
    conteneur1.taille.hauteur = 150; // Hauteur fixe

    // Conteneur 1 reste SCROLL_NONE (normal)
    GtkWidget *container1 = conteneur_creer(&conteneur1);

    // Ajouter du contenu au conteneur 1
    GtkWidget *label1 = gtk_label_new("CONTENEUR 1 - Normal");
    gtk_label_set_markup(GTK_LABEL(label1), "<b><big>CONTENEUR 1 - Normal</big></b>");
    conteneur_ajouter(&conteneur1, label1);

    for (int i = 0; i < 3; i++) {
        char text[50];
        snprintf(text, sizeof(text), "Élément %d du conteneur 1", i + 1);
        GtkWidget *item = gtk_label_new(text);
        conteneur_ajouter(&conteneur1, item);
    }

    // 3. Conteneur 2 - Défilable dans les deux directions
    Conteneur conteneur2;
    conteneur_initialiser(&conteneur2);
    conteneur2.orientation = CONTENEUR_VERTICAL;
    conteneur2.espacement = 10;
    conteneur2.couleur_fond = "#FFF3E0"; // Orange clair
    conteneur2.bordure_largeur = 2;
    conteneur2.bordure_couleur = "#FF9800";
    conteneur2.bordure_rayon = 10;
    conteneur2.padding.haut = 15;
    conteneur2.padding.bas = 15;
    conteneur2.padding.gauche = 15;
    conteneur2.padding.droite = 15;
    conteneur2.taille.hauteur = 200; // Hauteur fixe

    // Configurer le défilement dans les deux directions
    conteneur_set_scrollable(&conteneur2, SCROLL_BOTH);
    conteneur_set_scroll_size(&conteneur2, 500, 300); // Taille minimale du contenu
    conteneur_set_scroll_overlay(&conteneur2, true);

    GtkWidget *container2 = conteneur_creer(&conteneur2);

    // Ajouter du contenu au conteneur 2 (plus de contenu pour déclencher le scroll)
    GtkWidget *label2 = gtk_label_new("CONTENEUR 2 - Scrollable (Both)");
    gtk_label_set_markup(GTK_LABEL(label2), "<b><big>CONTENEUR 2 - Scrollable (Both)</big></b>");
    conteneur_ajouter(&conteneur2, label2);

    // Créer un sous-conteneur horizontal pour forcer le défilement horizontal
    Conteneur sous_conteneur_h;
    conteneur_initialiser(&sous_conteneur_h);
    sous_conteneur_h.orientation = CONTENEUR_HORIZONTAL;
    sous_conteneur_h.espacement = 10;
    sous_conteneur_h.taille.largeur = 800; // Large pour forcer le scroll horizontal

    GtkWidget *sub_container_h = conteneur_creer(&sous_conteneur_h);

    for (int i = 0; i < 6; i++) {
        char text[100];
        snprintf(text, sizeof(text), "Élément horizontal %d (très long texte pour forcer le défilement)", i + 1);
        GtkWidget *wide_item = gtk_label_new(text);
        gtk_widget_set_size_request(wide_item, 150, -1);
        conteneur_ajouter(&sous_conteneur_h, wide_item);
    }

    conteneur_ajouter(&conteneur2, sub_container_h);

    // Ajouter plus d'éléments verticaux pour forcer le scroll vertical
    for (int i = 0; i < 10; i++) {
        char text[50];
        snprintf(text, sizeof(text), "Élément vertical %d du conteneur 2", i + 1);
        GtkWidget *item = gtk_label_new(text);
        conteneur_ajouter(&conteneur2, item);
    }

    // 4. Ajouter des conteneurs au conteneur principal
    conteneur_ajouter(&conteneur_principal, container1);
    conteneur_ajouter(&conteneur_principal, container2);

    // Ajouter du contenu supplémentaire pour forcer le défilement de la fenêtre
    GtkWidget *extra_label = gtk_label_new("Contenu supplémentaire pour tester le défilement de la fenêtre");
    gtk_label_set_markup(GTK_LABEL(extra_label), "<i>Contenu supplémentaire pour tester le défilement de la fenêtre</i>");
    conteneur_ajouter(&conteneur_principal, extra_label);

    for (int i = 0; i < 5; i++) {
        char text[50];
        snprintf(text, sizeof(text), "Élément supplémentaire %d", i + 1);
        GtkWidget *extra_item = gtk_label_new(text);
        conteneur_ajouter(&conteneur_principal, extra_item);
    }

    // 2. Conteneur 1 - Normal (pas de défilement)
    Conteneur conteneur3;
    conteneur_initialiser(&conteneur3);
    conteneur3.orientation = CONTENEUR_VERTICAL;
    conteneur3.espacement = 10;
    conteneur3.couleur_fond = "#E8F4FD"; // Bleu clair
    conteneur3.bordure_largeur = 2;
    conteneur3.bordure_couleur = "#2196F3";
    conteneur3.bordure_rayon = 10;
    conteneur3.padding.haut = 15;
    conteneur3.padding.bas = 15;
    conteneur3.padding.gauche = 15;
    conteneur3.padding.droite = 15;
    conteneur3.taille.hauteur = 150; // Hauteur fixe

    // Conteneur 1 reste SCROLL_NONE (normal)
    GtkWidget *container3 = conteneur_creer(&conteneur3);

    // Ajouter du contenu au conteneur 1
    GtkWidget *label3 = gtk_label_new("CONTENEUR 1 - Normal");
    gtk_label_set_markup(GTK_LABEL(label1), "<b><big>CONTENEUR 1 - Normal</big></b>");
    conteneur_ajouter(&conteneur1, label1);
    conteneur_ajouter(&conteneur_principal, container3);
    // 5. Attacher le conteneur principal à la fenêtre
    GtkWidget *content_container = fenetre_get_content_container(&fenetre_config);
    if (fenetre_config.scroll_mode != SCROLL_NONE && fenetre_config.scroll_widget) {
        // Si la fenêtre a le défilement activé, ajouter au scroll widget
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fenetre_config.scroll_widget), main_container);
    } else {
        // Sinon, ajouter directement à la fenêtre
        gtk_window_set_child(GTK_WINDOW(window), main_container);
    }

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("com.example.scroll.design", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
