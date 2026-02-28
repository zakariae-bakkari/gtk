#include <gtk/gtk.h>

/* Callback quand le texte change */
static void on_changed(GtkEditable *editable, gpointer user_data)
{
    const char *texte = gtk_editable_get_text(editable);
    g_print("Texte actuel : %s\n", texte);
}

/* Fonction appelée au lancement */
static void activate(GtkApplication *app, gpointer user_data)
{
    // Créer la fenêtre
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Exemple GtkEntry");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

    // Créer un conteneur (box verticale)
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_window_set_child(GTK_WINDOW(window), box);

    // Créer le champ texte
    GtkWidget *entry = gtk_entry_new();

    // Ajouter un placeholder
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Entrez votre nom");

    // Connecter le signal "changed"
    g_signal_connect(entry, "changed", G_CALLBACK(on_changed), NULL);

    // Ajouter le champ dans la box
    gtk_box_append(GTK_BOX(box), entry);

    // Afficher la fenêtre
    gtk_widget_show(window);
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.entry", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    return status;
}