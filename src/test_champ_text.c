#include "../widgets/headers/champ_texte.h"
#include "../widgets/headers/conteneur.h" // Inclure le conteneur
#include <gtk/gtk.h>
#include <stdio.h>

// Callback functions for testing
static void on_text_changed(GtkEditable *editable, gpointer data)
{
    const char *id = (const char *)data;
    printf("[INPUT] changed: %s -> '%s'\n", id, gtk_editable_get_text(editable));
}

void on_entry_activated(GtkEntry *entry, gpointer user_data) {
    printf("Entry activated: %s\n", gtk_editable_get_text(GTK_EDITABLE(entry)));
}

void on_validation_failed(GtkWidget *widget, const char *message, gpointer user_data) {
    printf("Validation failed: %s\n", message);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Test ChampTexte");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

    // --- Créer un conteneur principal ---
    Conteneur main_container;
    conteneur_initialiser(&main_container);
    main_container.orientation = CONTENEUR_VERTICAL;
    main_container.align_x = ALIGNEMENT_CENTRE; // Centrer horizontalement
    main_container.align_y = ALIGNEMENT_CENTRE; // Centrer verticalement
    main_container.espacement = 10;
    GtkWidget *main_box = conteneur_creer(&main_container);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // --- Configuration du ChampTexte ---
    ChampTexte cfg;
    champ_texte_initialiser(&cfg);

    cfg.id_css = "test-champ-texte";
    cfg.texte = "Texte initial";
    cfg.placeholder = "Entrez du texte";
    cfg.max_length = 20;
    cfg.required = true;
    cfg.regex_pattern = "^[a-zA-Z]+$";
    cfg.read_only = false;
    cfg.sensitive = true;// !!

    cfg.style.bg_normal = "red";
    cfg.style.bg_focus = "red";// !! doesn't work
    cfg.style.fg_normal = "black";
    cfg.style.placeholder_color = "green";//!! doesn't work -> and maybe we don't need it
    cfg.style.epaisseur_bordure = 4;
    cfg.style.couleur_bordure = "yellow";
    cfg.style.rayon_arrondi = 20;
    cfg.style.gras = true;//!! doesn't work i don't see any difference
    cfg.style.italique = false;// !! doesn't work i don't see any difference
    cfg.style.taille_texte_px = 456;// !! doesn't work i don't see any difference

    cfg.on_change = on_text_changed;// when we change the test it will print the new text in the console
    cfg.on_activate = on_entry_activated;// when we press enter it will print the text in the console (not implement for now)
    cfg.on_invalid = on_validation_failed;// when the validation fail it will print the message in the console (not implement for now)

    GtkWidget *entry = champ_texte_creer(&cfg);

    // --- Ajouter le champ de texte au conteneur ---
    conteneur_ajouter(&main_container, entry);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("org.gtk.test_champ_texte", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
