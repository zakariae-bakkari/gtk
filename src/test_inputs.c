#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/champ_motdepasse.h"
#include "../widgets/headers/champ_nombre.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/champ_zone_texte.h"
#include "../widgets/headers/champ_texte.h"
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
    printf("GTK4 Input Components Test\n");

    /* ========== FENETRE PRINCIPALE ========== */
    Fenetre main_window;
    fenetre_initialiser(&main_window);
    main_window.title = "Input Components Size Test";
    main_window.taille.width = 500;
    main_window.taille.height = 600;
    main_window.color_bg = "#f5f5f5";
    main_window.icon_path = "";
    main_window.titre_align = TITRE_ALIGN_GAUCHE;
    main_window.bouton_agrandir = false;

    fenetre_set_scrollable(&main_window, SCROLL_VERTICAL);
    fenetre_set_scroll_content_size(&main_window, -1, 1100); // Augmenté pour le nouveau contenu
    fenetre_set_scroll_overlay(&main_window, true);

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

    if (main_window.scroll_mode != SCROLL_NONE && main_window.scroll_widget)
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(main_window.scroll_widget), main_box);
    else
        gtk_window_set_child(GTK_WINDOW(window), main_box);

    /* ========== CHAMP TEXTE - Texte libre, largeur fixe (250px) ========== */
    GtkWidget *lbl_nom = gtk_label_new_with_mnemonic("_Nom (250px width):");

    ChampTexte *ct_nom = g_new0(ChampTexte, 1);
    champ_texte_initialiser(ct_nom);
    ct_nom->id_css = "input_nom";
    ct_nom->placeholder = "Entrez votre nom...";
    ct_nom->size.width = 250;
    ct_nom->size.height = 0;
    ct_nom->required = true;
    ct_nom->max_length = 50;
    ct_nom->policy.min_len = 2;
    ct_nom->policy.no_digits = true;
    ct_nom->style.epaisseur_bordure = 2;
    ct_nom->style.couleur_bordure = g_strdup("#3498db");
    ct_nom->style.rayon_arrondi = 8;
    ct_nom->on_change = on_input_text_changed;
    ct_nom->on_activate = on_input_text_activate;
    ct_nom->on_invalid = on_input_invalid;
    ct_nom->user_data = "nom";

    GtkWidget *w_nom = champ_texte_creer(ct_nom);
    g_signal_connect_swapped(w_nom, "destroy", G_CALLBACK(champ_texte_free), ct_nom);
    g_signal_connect(w_nom, "destroy", G_CALLBACK(g_free), ct_nom);

    conteneur_ajouter(&main_container, lbl_nom);
    conteneur_ajouter(&main_container, w_nom);

    /* ========== CHAMP TEXTE - E-mail, largeur complète (100%) ========== */
    GtkWidget *lbl_email = gtk_label_new_with_mnemonic("_Email (100% width):");

    ChampTexte *ct_email = g_new0(ChampTexte, 1);
    champ_texte_initialiser(ct_email);
    ct_email->id_css = "input_email";
    ct_email->placeholder = "exemple@domaine.com";
    ct_email->size.width = 0;   // Largeur complète
    ct_email->size.height = 40;
    ct_email->required = true;
    ct_email->type = CHAMP_TEXTE_TYPE_EMAIL;
    ct_email->icon_primary = "mail-send-symbolic";
    ct_email->show_error_label = false;
    ct_email->style.epaisseur_bordure = 1;
    ct_email->style.couleur_bordure = g_strdup("#2ecc71");
    ct_email->style.rayon_arrondi = 5;
    ct_email->on_change = on_input_text_changed;
    ct_email->on_activate = on_input_text_activate;
    ct_email->on_invalid = on_input_invalid;
    ct_email->user_data = "email";

    GtkWidget *w_email = champ_texte_creer(ct_email);
    g_signal_connect_swapped(w_email, "destroy", G_CALLBACK(champ_texte_free), ct_email);
    g_signal_connect(w_email, "destroy", G_CALLBACK(g_free), ct_email);

    conteneur_ajouter(&main_container, lbl_email);
    conteneur_ajouter(&main_container, w_email);

    /* ========== CHAMP TEXTE - URL, largeur fixe (300px) ========== */
    GtkWidget *lbl_url = gtk_label_new_with_mnemonic("_Site web (300px width):");

    ChampTexte *ct_url = g_new0(ChampTexte, 1);
    champ_texte_initialiser(ct_url);
    ct_url->id_css = "input_url";
    ct_url->placeholder = "https://exemple.com";
    ct_url->size.width = 300;
    ct_url->size.height = 0;
    ct_url->required = false;
    ct_url->type = CHAMP_TEXTE_TYPE_URL;
    ct_url->icon_primary = "web-browser-symbolic";
    ct_url->style.epaisseur_bordure = 2;
    ct_url->style.couleur_bordure = g_strdup("#9b59b6");
    ct_url->style.rayon_arrondi = 6;
    ct_url->on_change = on_input_text_changed;
    ct_url->on_invalid = on_input_invalid;
    ct_url->user_data = "url";

    GtkWidget *w_url = champ_texte_creer(ct_url);
    g_signal_connect_swapped(w_url, "destroy", G_CALLBACK(champ_texte_free), ct_url);
    g_signal_connect(w_url, "destroy", G_CALLBACK(g_free), ct_url);

    conteneur_ajouter(&main_container, lbl_url);
    conteneur_ajouter(&main_container, w_url);

    /* ========== CHAMP TEXTE - Recherche, largeur complète ========== */
    GtkWidget *lbl_search = gtk_label_new_with_mnemonic("_Recherche (100% width):");

    ChampTexte *ct_search = g_new0(ChampTexte, 1);
    champ_texte_initialiser(ct_search);
    ct_search->id_css = "input_search";
    ct_search->placeholder = "Rechercher...";
    ct_search->size.width = 0;   // Largeur complète
    ct_search->size.height = 38;
    ct_search->required = false;
    ct_search->type = CHAMP_TEXTE_TYPE_SEARCH;
    ct_search->max_length = 100;
    ct_search->icon_primary = "edit-find-symbolic";
    ct_search->icon_secondary = "edit-clear-symbolic";
    ct_search->style.epaisseur_bordure = 1;
    ct_search->style.couleur_bordure = g_strdup("#bdc3c7");
    ct_search->style.rayon_arrondi = 20; // Arrondi prononcé style barre de recherche
    ct_search->on_change = on_input_text_changed;
    ct_search->on_invalid = on_input_invalid;
    ct_search->user_data = "search";

    GtkWidget *w_search = champ_texte_creer(ct_search);
    g_signal_connect_swapped(w_search, "destroy", G_CALLBACK(champ_texte_free), ct_search);
    g_signal_connect(w_search, "destroy", G_CALLBACK(g_free), ct_search);

    conteneur_ajouter(&main_container, lbl_search);
    conteneur_ajouter(&main_container, w_search);

    /* ========== PASSWORD INPUT - Fixed Width ========== */
    GtkWidget *lbl_pwd1 = gtk_label_new_with_mnemonic("_Password (250px width):");

    ChampMotDePasse *pwd1 = g_new0(ChampMotDePasse, 1);
    champ_motdepasse_initialiser(pwd1);
    pwd1->id_css = "input_password_fixed";
    pwd1->placeholder = "Enter password...";
    pwd1->size.width = 250;
    pwd1->size.height = 0;
    pwd1->required = true;
    pwd1->max_length = 5;
    pwd1->policy.min_len = 6;
    pwd1->policy.require_digit = true;
    pwd1->style.epaisseur_bordure = 2;
    pwd1->style.couleur_bordure = g_strdup("#e74c3c");
    pwd1->style.rayon_arrondi = 8;
    pwd1->on_change = on_input_text_changed;
    pwd1->on_activate = on_input_text_activate;
    pwd1->on_invalid = on_input_invalid;
    pwd1->user_data = "password_fixed";

    GtkWidget *w_pwd1 = champ_motdepasse_creer(pwd1);
    g_signal_connect_swapped(w_pwd1, "destroy", G_CALLBACK(champ_motdepasse_free), pwd1);
    g_signal_connect(w_pwd1, "destroy", G_CALLBACK(g_free), pwd1);

    conteneur_ajouter(&main_container, lbl_pwd1);
    conteneur_ajouter(&main_container, w_pwd1);

    /* ========== PASSWORD INPUT - Full Width ========== */
    GtkWidget *lbl_pwd2 = gtk_label_new_with_mnemonic("_Confirm Password (100% width):");

    ChampMotDePasse *pwd2 = g_new0(ChampMotDePasse, 1);
    champ_motdepasse_initialiser(pwd2);
    pwd2->id_css = "input_password_full";
    pwd2->placeholder = "Confirm your password...";
    pwd2->size.width = 0;
    pwd2->size.height = 40;
    pwd2->required = true;
    pwd2->style.epaisseur_bordure = 1;
    pwd2->style.couleur_bordure = g_strdup("#3498db");
    pwd2->style.rayon_arrondi = 5;
    pwd2->on_change = on_input_text_changed;
    pwd2->on_invalid = on_input_invalid;
    pwd2->user_data = "password_full";

    GtkWidget *w_pwd2 = champ_motdepasse_creer(pwd2);
    g_signal_connect_swapped(w_pwd2, "destroy", G_CALLBACK(champ_motdepasse_free), pwd2);
    g_signal_connect(w_pwd2, "destroy", G_CALLBACK(g_free), pwd2);

    conteneur_ajouter(&main_container, lbl_pwd2);
    conteneur_ajouter(&main_container, w_pwd2);

    /* ========== NUMBER INPUT - Small Fixed Size ========== */
    GtkWidget *lbl_age = gtk_label_new_with_mnemonic("_Age (120px width):");

    ChampNombre *age = g_new0(ChampNombre, 1);
    champ_nombre_initialiser(age);
    age->id_css = "input_age";
    age->min = 0;
    age->max = 120;
    age->step = 1;
    age->digits = 0;
    age->valeur = 25;
    age->size.width = 120;
    age->size.height = 35;
    age->required = true;
    age->style.epaisseur_bordure = 2;
    age->style.couleur_bordure = g_strdup("#2ecc71");
    age->style.rayon_arrondi = 6;
    age->on_change = on_input_number_changed;
    age->on_invalid = on_input_invalid;
    age->user_data = "age";

    GtkWidget *w_age = champ_nombre_creer(age);
    g_signal_connect_swapped(w_age, "destroy", G_CALLBACK(champ_nombre_free), age);
    g_signal_connect(w_age, "destroy", G_CALLBACK(g_free), age);

    conteneur_ajouter(&main_container, lbl_age);
    conteneur_ajouter(&main_container, w_age);

    /* ========== NUMBER INPUT - Medium Width ========== */
    GtkWidget *lbl_salary = gtk_label_new_with_mnemonic("_Salary (200px width):");

    ChampNombre *salary = g_new0(ChampNombre, 1);
    champ_nombre_initialiser(salary);
    salary->id_css = "input_salary";
    salary->min = 0;
    salary->max = 999999;
    salary->step = 100;
    salary->digits = 2;
    salary->valeur = 50000.00;
    salary->size.width = 200;
    salary->size.height = 0;
    salary->style.epaisseur_bordure = 1;
    salary->style.couleur_bordure = g_strdup("#f39c12");
    salary->style.rayon_arrondi = 4;
    salary->on_change = on_input_number_changed;
    salary->user_data = "salary";

    GtkWidget *w_salary = champ_nombre_creer(salary);
    g_signal_connect_swapped(w_salary, "destroy", G_CALLBACK(champ_nombre_free), salary);
    g_signal_connect(w_salary, "destroy", G_CALLBACK(g_free), salary);

    conteneur_ajouter(&main_container, lbl_salary);
    conteneur_ajouter(&main_container, w_salary);

    /* ========== SELECT INPUT - Fixed Width ========== */
    GtkWidget *lbl_country = gtk_label_new_with_mnemonic("_Country (180px width):");

    ChampSelect *country = g_new0(ChampSelect, 1);
    champ_select_initialiser(country);
    country->id_css = "input_country";
    country->size.width = 180;
    country->size.height = 0;
    country->required = true;
    country->style.epaisseur_bordure = 2;
    country->style.couleur_bordure = g_strdup("#9b59b6");
    country->style.rayon_arrondi = 7;
    country->on_change = on_select_changed;
    country->on_invalid = on_input_invalid;
    country->user_data = "country";

    champ_select_add_item(country, "Select a country...");
    champ_select_add_item(country, "France");
    champ_select_add_item(country, "Germany");
    champ_select_add_item(country, "Spain");
    champ_select_add_item(country, "Italy");
    champ_select_add_item(country, "United Kingdom");
    champ_select_set_index(country, 0);

    GtkWidget *w_country = champ_select_creer(country);
    g_signal_connect_swapped(w_country, "destroy", G_CALLBACK(champ_select_free), country);
    g_signal_connect(w_country, "destroy", G_CALLBACK(g_free), country);

    conteneur_ajouter(&main_container, lbl_country);
    conteneur_ajouter(&main_container, w_country);

    /* ========== SELECT INPUT - Full Width ========== */
    GtkWidget *lbl_language = gtk_label_new_with_mnemonic("_Programming Language (100% width):");

    ChampSelect *language = g_new0(ChampSelect, 1);
    champ_select_initialiser(language);
    language->id_css = "input_language";
    language->size.width = 0;
    language->size.height = 45;
    language->required = false;
    language->style.epaisseur_bordure = 1;
    language->style.couleur_bordure = g_strdup("#34495e");
    language->style.rayon_arrondi = 3;
    language->on_change = on_select_changed;
    language->user_data = "language";

    champ_select_add_item(language, "Choose your favorite...");
    champ_select_add_item(language, "C");
    champ_select_add_item(language, "C++");
    champ_select_add_item(language, "Python");
    champ_select_add_item(language, "JavaScript");
    champ_select_add_item(language, "Rust");
    champ_select_add_item(language, "Go");
    champ_select_set_index(language, 2);

    GtkWidget *w_language = champ_select_creer(language);
    g_signal_connect_swapped(w_language, "destroy", G_CALLBACK(champ_select_free), language);
    g_signal_connect(w_language, "destroy", G_CALLBACK(g_free), language);

    conteneur_ajouter(&main_container, lbl_language);
    conteneur_ajouter(&main_container, w_language);

    /* ========== ZONE TEXTE (ChampZoneTexte) ========== */
    GtkWidget *lbl_zone = gtk_label_new_with_mnemonic("_Comments:");

    ChampZoneTexte *czt = g_new0(ChampZoneTexte, 1);
    champ_zone_texte_initialiser(czt);
    czt->id_css = "input_zone_texte";
    czt->max_length = 200;
    czt->size.width = 0;
    czt->size.height = 90;
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
    czt2->size.width = 100;
    czt2->size.height = 90;
    czt2->wrap_word = true;
    czt2->on_change = NULL;
    czt2->on_invalid = on_input_invalid;
    czt2->user_data = "bio";
    czt2->texte = g_strdup("Short bio...");

    GtkWidget *w_bio = champ_zone_texte_creer(czt2);
    g_signal_connect_swapped(w_bio, "destroy", G_CALLBACK(champ_zone_texte_free), czt2);
    g_signal_connect(w_bio, "destroy", G_CALLBACK(g_free), czt2);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_bio), w_bio);

    conteneur_ajouter(&main_container, lbl_bio);
    conteneur_ajouter(&main_container, w_bio);

    /* ========== AFFICHAGE DE LA FENETRE ========== */
    printf("[OK] All input components with custom sizes created successfully!\n");
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