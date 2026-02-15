#include "../widgets/headers/fenetre.h" // On inclut l'interface de notre widget

static void activate(GtkApplication *app, gpointer user_data) {
    Fenetre ma_fenetre;
    fenetre_initialiser(&ma_fenetre);

    // Configuration avancée
    ma_fenetre.title = "Mon App";
    ma_fenetre.titre_align = TITRE_ALIGN_CENTRE; // Titre à gauche

    ma_fenetre.bouton_reduire = false;  // Pas de bouton réduire
    ma_fenetre.bouton_agrandir = false;
    ma_fenetre.bouton_fermer = true;

    ma_fenetre.demarrer_maximisee = false; // Prend tout l'écran au lancement

    GtkWidget *window = fenetre_creer(&ma_fenetre);
    // 5. Rattachement à l'application (Nécessaire pour que l'app gère la fermeture)
    gtk_window_set_application(GTK_WINDOW(window), app);

    // 6. Affichage de la fenêtre (En GTK4, on utilise present() au lieu de show_all())
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    // Création de l'application (identifiant unique obligatoire, ex: org.example.app)
    app = gtk_application_new("org.ilisi.projet", G_APPLICATION_DEFAULT_FLAGS);

    // Connexion du signal "activate" à notre fonction
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    // Lancement de la boucle d'événements
    status = g_application_run(G_APPLICATION(app), argc, argv);

    // Nettoyage
    g_object_unref(app);

    return status;
}
