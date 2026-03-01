#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/champ_motdepasse.h"
#include "../widgets/headers/champ_nombre.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/champ_zone_texte.h"
#include <stdio.h>

/* --- Callbacks pour les evenements --- */

static void on_input_text_changed(GtkEditable *editable, gpointer data)
{
    const char *id = (const char *)data;
    printf("[INPUT] changed: %s -> '%s'\n", id, gtk_editable_get_text(editable));
}

static void on_input_text_activate(GtkEntry *entry, gpointer data)
{
    const char *id = (const char *)data;
    printf("[INPUT] activate (submit): %s -> '%s'\n", id, gtk_editable_get_text(GTK_EDITABLE(entry)));
}

static void on_input_number_changed(GtkEditable *editable, gpointer data)
{
    const char *id = (const char *)data;
    // Cast to GtkSpinButton to get the numeric value
    GtkSpinButton *spin = GTK_SPIN_BUTTON(editable);
    double value = gtk_spin_button_get_value(spin);
    printf("[INPUT] number changed: %s -> %.2f\n", id, value);
}

static void on_zone_texte_changed(GtkTextBuffer *buffer, gpointer data)
{
    const char *id = (const char *)data;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    printf("[ZONE_TEXTE] changed: %s -> '%s'\n", id, texte);
    g_free(texte);
}

static void on_input_invalid(GtkWidget *widget, const char *message, gpointer data)
{
    const char *id = (const char *)data;
    printf("[INPUT][INVALID] %s: %s\n", id, message);
}

static void on_select_changed(GtkDropDown *dd, gpointer data)
{
    const char *id = (const char *)data;
    GtkStringList *model = GTK_STRING_LIST(gtk_drop_down_get_model(dd));
    guint selected = gtk_drop_down_get_selected(dd);
    const char *selected_item = gtk_string_list_get_string(model, selected);
    printf("[SELECT] changed: %s -> '%s'\n", id, selected_item);
}
/* --- Fonction principale de creation de l'interface --- */

static void activate(GtkApplication *app, gpointer user_data)
{
    printf("GTK4 Password Input Test\n");

    /* ========== FENETRE PRINCIPALE ========== */
    Fenetre main_window;
    fenetre_initialiser(&main_window);
    main_window.title = "Password Input Test";
    main_window.taille.width = 400;
    main_window.taille.height = 600;
    main_window.color_bg = "#f5f5f5";
    main_window.icon_path = "application-x-executable-symbolic";
    main_window.titre_align = TITRE_ALIGN_GAUCHE;
    main_window.bouton_agrandir = false;

    GtkWidget *window = fenetre_creer(&main_window);
    gtk_window_set_application(GTK_WINDOW(window), app);

    /* ========== CONTENEUR PRINCIPAL (Vertical) ========== */
    Conteneur main_container;
    conteneur_initialiser(&main_container);
    main_container.orientation = CONTENEUR_VERTICAL;
    main_container.espacement = 15;
    main_container.marges.haut = 20;
    main_container.marges.bas = 20;
    main_container.marges.gauche = 20;
    main_container.marges.droite = 20;
    main_container.couleur_fond = "#ffffff";
    main_container.enfants_hexpand = false;

    GtkWidget *main_box = conteneur_creer(&main_container);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    /* ========== ZONE TEXTE (ChampZoneTexte) ========== */
    GtkWidget *lbl_zone = gtk_label_new_with_mnemonic("_Comments:");

    ChampZoneTexte *czt = g_new0(ChampZoneTexte, 1);
    champ_zone_texte_initialiser(czt);
    czt->id_css = "input_zone_texte";
    czt->max_length = 200;
    // Option 1: Fixed width (300px exact)
    // czt->width = 300;
    // Option 2: Full width (100% of container)
    czt->width = 0; // 0 = 100% width
    czt->height = 90;
    czt->wrap_word = true;
    czt->sensitive = true;
    czt->required = false;
    czt->style.epaisseur_bordure = 2;
    czt->style.couleur_bordure = g_strdup("purple");
    czt->style.rayon_arrondi = 5;
    czt->style.gras = false;
    czt->style.italique = false;
    czt->style.taille_texte_px = 12;
    czt->on_change = on_zone_texte_changed;
    czt->on_invalid = on_input_invalid;
    czt->user_data = "zone_texte";

    // Set the initial text AFTER all other properties are configured
    czt->texte = g_strdup("this is a comment");

    GtkWidget *w_zone = champ_zone_texte_creer(czt);
    g_signal_connect_swapped(w_zone, "destroy", G_CALLBACK(champ_zone_texte_free), czt);
    g_signal_connect(w_zone, "destroy", G_CALLBACK(g_free), czt);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_zone), w_zone);

    conteneur_ajouter(&main_container, lbl_zone);
    conteneur_ajouter(&main_container, w_zone);

    /* ========== BIO ZONE TEXTE ========== */
    GtkWidget *lbl_bio = gtk_label_new_with_mnemonic("_Bio:");
    ChampZoneTexte *czt2 = g_new0(ChampZoneTexte, 1);
    champ_zone_texte_initialiser(czt2);
    czt2->id_css = "input_bio";
    czt2->required = true;
    czt2->max_length = 300;
    czt2->width = 100;
    czt2->height = 90; // Set height using the new attribute
    czt2->wrap_word = true;
    czt2->on_change = NULL;
    czt2->on_invalid = on_input_invalid;
    czt2->user_data = "bio";

    // Set the initial text consistently
    czt2->texte = g_strdup("Short bio...");

    GtkWidget *w_bio = champ_zone_texte_creer(czt2);
    g_signal_connect_swapped(w_bio, "destroy", G_CALLBACK(champ_zone_texte_free), czt2);
    g_signal_connect(w_bio, "destroy", G_CALLBACK(g_free), czt2);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_bio), w_bio);

    conteneur_ajouter(&main_container, lbl_bio);
    conteneur_ajouter(&main_container, w_bio);

    /* ========== AFFICHAGE DE LA FENETRE ========== */
    printf("[OK] Password and number inputs created successfully!\n");
    printf("[OK] Window size: %dx%d pixels\n\n", main_window.taille.width, main_window.taille.height);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("org.zcode.password.test", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
