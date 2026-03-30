//
// Created by Noureddine on 3/3/2026.
//
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/conteneur.h"

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "l conteneur");
    gtk_window_set_default_size(GTK_WINDOW(window),500,500);

    Conteneur c;
    conteneur_initialiser(&c);


    c.orientation = CONTENEUR_VERTICAL;
    c.espacement = 10;
    c.homogene = FALSE;

    //STYLE
    c.couleur_fond = malloc(strlen("#DDEEFF") + 1);
    strcpy(c.couleur_fond, "#DDEEFF");
    c.bordure_largeur = 5;
    c.bordure_couleur = malloc(strlen("blue") + 1);
    strcpy(c.bordure_couleur, "blue");
    c.bordure_rayon = 20;

    //marges
    c.marges.haut = 20;
    c.marges.bas = 20;
    c.marges.gauche = 20;
    c.marges.droite = 20;

    //padding
    c.padding.haut = 15;
    c.padding.bas = 15;
    c.padding.gauche = 15;
    c.padding.droite = 15;

    c.taille.largeur = -1;
    c.taille.hauteur = -1;

    //alignement parent
    c.align_x = ALIGNEMENT_REMPLIR;

    //c.align_y = ALIGNEMENT_CENTRE;
    //scroll
    conteneur_set_scrollable(&c, SCROLL_VERTICAL);
    conteneur_set_scroll_size(&c, 300, 200);




    //creation du conteneur
    GtkWidget *conteneur_widget = conteneur_creer(&c);

    for (int i=1;i<=10;i++)
    {

        char texte[50];
        sprintf(texte, "Bouton %d", i);

        GtkWidget *btn = gtk_button_new_with_label(texte);
        conteneur_ajouter(&c, btn);
    }

    //ajout du conteneur dans fenetre
    gtk_window_set_child(GTK_WINDOW(window), conteneur_widget);
    gtk_window_present(GTK_WINDOW(window));


}


int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.test.conteneur", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);


    return status;
}



