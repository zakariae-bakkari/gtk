#include <gtk/gtk.h>
#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/bouton.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/bouton_checklist.h"
#include "../widgets/headers/bouton_radio.h"
#include "../widgets/headers/champ_texte.h"
#include "../widgets/headers/champ_motdepasse.h"
#include "../widgets/headers/champ_nombre.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/champ_zone_texte.h"
#include <stdio.h>

/* --- Callbacks pour les evenements --- */

static void on_button_clicked(GtkWidget *widget, gpointer data)
{
    printf("[INFO] Button clicked!\n");
}

static void on_checkbox_toggled(GtkCheckButton *widget, gpointer data)
{
    const char *label = (const char *)data;
    gboolean active = gtk_check_button_get_active(widget);
    printf("[INFO] Checkbox '%s' : %s\n", label, active ? "CHECKED" : "UNCHECKED");
}

static void on_radio_toggled(GtkCheckButton *widget, gpointer data)
{
    const char *label = (const char *)data;
    gboolean active = gtk_check_button_get_active(widget);
    if (active)
    {
        printf("[INFO] Radio selected : '%s'\n", label);
    }
}

static void on_submit_clicked(GtkWidget *widget, gpointer data)
{
    printf("\n=== FORM SUBMITTED ===\n");
    printf("User clicked Submit button!\n");
    printf("================================\n\n");
}

// Inputs callbacks
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
static void on_input_invalid(GtkWidget *widget, const char *message, gpointer data)
{
    const char *id = (const char *)data;
    printf("[INPUT][INVALID] %s: %s\n", id, message);
}
static void on_select_changed(GtkDropDown *dd, gpointer data)
{
    (void)dd;
    const char *id = (const char *)data;
    printf("[SELECT] changed: %s\n", id);
}
static void on_number_changed(GtkSpinButton *spin, gpointer data)
{
    const char *id = (const char *)data;
    printf("[NUMBER] %s -> %.2f\n", id, gtk_spin_button_get_value(spin));
}

/* --- Fonction principale de creation de l'interface --- */

static void activate(GtkApplication *app, gpointer user_data)
{
    printf("GTK4 Dashboard - Widget Demo Application\n");

    /* ========== FENETRE PRINCIPALE ========== */
    Fenetre main_window;
    fenetre_initialiser(&main_window);
    main_window.title = "Dashboard - GTK4 Widgets";
    main_window.taille.width = 900;
    main_window.taille.height = 700;
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

    // Ensure content can expand; the scrolled window will manage overflow
    gtk_widget_set_hexpand(main_box, TRUE);
    gtk_widget_set_vexpand(main_box, TRUE);

    // NEW: Make the page scrollable by wrapping main_box in a GtkScrolledWindow
    GtkWidget *scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scroller, TRUE);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), main_box);

    // Set scroller as the window child instead of main_box directly
    gtk_window_set_child(GTK_WINDOW(window), scroller);

    /* ========== HEADER SECTION ========== */
    Conteneur header_container;
    conteneur_initialiser(&header_container);
    header_container.orientation = CONTENEUR_HORIZONTAL;
    header_container.espacement = 10;
    header_container.couleur_fond = "#2c3e50";
    header_container.padding.haut = 15;
    header_container.padding.bas = 15;
    header_container.padding.gauche = 15;
    header_container.padding.droite = 15;
    header_container.bordure_largeur = 2;
    header_container.bordure_couleur = "#1a252f";
    header_container.enfants_hexpand = false;

    GtkWidget *header_box = conteneur_creer(&header_container);

    GtkWidget *title_label = gtk_label_new("Configuration Panel");
    gtk_label_set_markup(
        GTK_LABEL(title_label), "<span font='16' color='white' weight='bold'>Configuration Panel</span>");
    conteneur_ajouter(&header_container, title_label);

    conteneur_ajouter(&main_container, header_box);

    /* ========== BUTTON SIZING SECTION ========== */
    GtkWidget *sizing_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(sizing_label), "<span font='14' weight='bold' color='#2c3e50'>Buttons</span>");
    gtk_label_set_xalign(GTK_LABEL(sizing_label), 0.0);
    conteneur_ajouter(&main_container, sizing_label);

    /* --- Sizing Container --- */
    Conteneur sizing_container;
    conteneur_initialiser(&sizing_container);
    sizing_container.orientation = CONTENEUR_VERTICAL;
    sizing_container.espacement = 12;
    sizing_container.couleur_fond = "grey";
    sizing_container.padding.haut = 15;
    sizing_container.padding.bas = 15;
    sizing_container.padding.gauche = 15;
    sizing_container.padding.droite = 15;
    sizing_container.bordure_largeur = 5;
    sizing_container.bordure_couleur = "black";
    sizing_container.bordure_rayon = 5;
    // Children manage their own expansion based on their TAILLE mode

    GtkWidget *sizing_box = conteneur_creer(&sizing_container);

    /* --- FIXED Mode Button (Full Width) --- */
    Bouton btn_full_width;
    bouton_initialiser(&btn_full_width);
    btn_full_width.id_css = "btn_full_width";
    btn_full_width.texte = "button 1";
    btn_full_width.taille.mode = TAILLE_FIXE;
    btn_full_width.taille.largeur = 500;
    btn_full_width.taille.hauteur = 50;
    btn_full_width.style.bg_normal = "#9b59b6";
    btn_full_width.style.bg_hover = "#8e44ad";
    btn_full_width.style.fg_normal = "white";
    btn_full_width.style.rayon_arrondi = 18;
    btn_full_width.nom_icone = "document-properties-symbolic";
    btn_full_width.on_clic = on_button_clicked;
    GtkWidget *btn_full_width_widget = bouton_creer(&btn_full_width);
    Conteneur button1_container;
    conteneur_initialiser(&button1_container);
    button1_container.orientation = CONTENEUR_HORIZONTAL;
    button1_container.align_x = ALIGNEMENT_FIN;
    button1_container.taille.largeur = 10;
    button1_container.marges.gauche = 100;
    button1_container.enfants_hexpand = false; // Ensure children do not expand
    GtkWidget *button1_box = conteneur_creer(&button1_container);
    conteneur_ajouter(&button1_container, btn_full_width_widget);
    conteneur_ajouter(&sizing_container, button1_box);

    /* --- FIXED Mode Button (Square) --- */
    Bouton btn_square;
    bouton_initialiser(&btn_square);
    btn_square.id_css = "btn_square";
    btn_square.texte = "SQ";
    btn_square.taille.mode = TAILLE_FIXE;
    btn_square.taille.largeur = 80;
    btn_square.taille.hauteur = 20;
    btn_square.style.bg_normal = "#EB4C4C";
    btn_square.style.bg_hover = "#FF7070";
    btn_square.style.fg_normal = "white";
    btn_square.style.rayon_arrondi = 60;
    btn_square.nom_icone = "edit-delete-symbolic";
    btn_square.pos_icone = ICONE_SEULE;
    btn_square.on_clic = on_button_clicked;
    btn_square.tooltip = "this is a square button";
    btn_square.style.couleur_bordure = "red";
    btn_square.style.epaisseur_bordure = 2;

    GtkWidget *btn_square_widget = bouton_creer(&btn_square);
    conteneur_ajouter(&sizing_container, btn_square_widget);
    /* --- FIXED Mode Button (Square) --- */
    Bouton btn2;
    bouton_initialiser(&btn2);
    btn2.id_css = "btn2";
    btn2.texte = "button 2";
    btn2.taille.mode = TAILLE_AUTO;
    btn2.style.bg_normal = "black";
    btn2.style.bg_hover = "#333333";
    btn2.style.fg_normal = "white";
    btn2.style.gras = true;
    btn2.style.rayon_arrondi = 60;
    btn2.nom_icone = "go-up-symbolic";
    btn2.pos_icone = ICONE_HAUT;
    btn2.on_clic = on_button_clicked;
    btn2.tooltip = "button";
    btn2.style.couleur_bordure = "white";
    btn2.style.epaisseur_bordure = 5;

    GtkWidget *btn2_widget = bouton_creer(&btn2);
    conteneur_ajouter(&sizing_container, btn2_widget);

    conteneur_ajouter(&main_container, sizing_box);

    /* ========== CHECKBOXES SECTION ========== */
    GtkWidget *checkbox_label = gtk_label_new(NULL);
    gtk_label_set_markup(
        GTK_LABEL(checkbox_label), "<span font='14' weight='bold' color='#2c3e50'>Preferences (Checkboxes)</span>");
    gtk_label_set_xalign(GTK_LABEL(checkbox_label), 0.0);
    conteneur_ajouter(&main_container, checkbox_label);

    /* --- Checkbox Container --- */
    Conteneur checkbox_container;
    conteneur_initialiser(&checkbox_container);
    checkbox_container.orientation = CONTENEUR_VERTICAL;
    checkbox_container.espacement = 10;
    checkbox_container.couleur_fond = "#fef5e7";
    checkbox_container.padding.haut = 12;
    checkbox_container.padding.bas = 12;
    checkbox_container.padding.gauche = 12;
    checkbox_container.padding.droite = 12;
    checkbox_container.bordure_largeur = 1;
    checkbox_container.bordure_couleur = "#f9e79f";
    checkbox_container.bordure_rayon = 5;
    checkbox_container.enfants_hexpand = false;

    GtkWidget *checkbox_box = conteneur_creer(&checkbox_container);

    /* --- Checkbox 1 --- */
    BoutonChecklist check1;
    bouton_checklist_initialiser(&check1);
    check1.id_css = "check_enable_notifications";
    check1.label = "Enable Notifications";
    check1.etat = CHECKLIST_CHECKED;
    check1.style.couleur_texte = "#2c3e50";
    check1.style.gras = false;
    check1.on_toggled = on_checkbox_toggled;
    check1.user_data = "Enable Notifications";

    GtkWidget *check1_widget = bouton_checklist_creer(&check1);
    conteneur_ajouter(&checkbox_container, check1_widget);

    /* --- Checkbox 2 --- */
    BoutonChecklist check2;
    bouton_checklist_initialiser(&check2);
    check2.id_css = "check_dark_mode";
    check2.label = "Dark Mode";
    check2.etat = CHECKLIST_UNCHECKED;
    check2.style.couleur_texte = "#2c3e50";
    check2.on_toggled = on_checkbox_toggled;
    check2.user_data = "Dark Mode";

    GtkWidget *check2_widget = bouton_checklist_creer(&check2);
    conteneur_ajouter(&checkbox_container, check2_widget);

    /* --- Checkbox 3 --- */
    BoutonChecklist check3;
    bouton_checklist_initialiser(&check3);
    check3.id_css = "check_save_password";
    check3.label = "Remember Password";
    check3.etat = CHECKLIST_UNCHECKED;
    check3.style.couleur_texte = "#2c3e50";
    check3.on_toggled = on_checkbox_toggled;
    check3.user_data = "Remember Password";

    GtkWidget *check3_widget = bouton_checklist_creer(&check3);
    conteneur_ajouter(&checkbox_container, check3_widget);

    conteneur_ajouter(&main_container, checkbox_box);

    /* ========== RADIO BUTTONS SECTION ========== */
    GtkWidget *radio_label = gtk_label_new(NULL);
    gtk_label_set_markup(
        GTK_LABEL(radio_label), "<span font='14' weight='bold' color='#2c3e50'>Theme Selection (Radio Buttons)</span>");
    gtk_label_set_xalign(GTK_LABEL(radio_label), 0.0);
    conteneur_ajouter(&main_container, radio_label);

    /* --- Radio Container --- */
    Conteneur radio_container;
    conteneur_initialiser(&radio_container);
    radio_container.orientation = CONTENEUR_VERTICAL;
    radio_container.espacement = 10;
    radio_container.couleur_fond = "#e8f8f5";
    radio_container.padding.haut = 12;
    radio_container.padding.bas = 12;
    radio_container.padding.gauche = 12;
    radio_container.padding.droite = 12;
    radio_container.bordure_largeur = 1;
    radio_container.bordure_couleur = "#a9dfbf";
    radio_container.bordure_rayon = 5;
    radio_container.enfants_hexpand = false;

    GtkWidget *radio_box = conteneur_creer(&radio_container);

    /* --- Radio 1 (Group Leader) --- */
    BoutonRadio radio1;
    bouton_radio_initialiser(&radio1);
    radio1.id_css = "radio_light_theme";
    radio1.label = "Light Theme";
    radio1.est_actif = true;
    radio1.style.couleur_texte = "#27ae60";
    radio1.on_toggled = on_radio_toggled;
    radio1.user_data = "Light Theme";
    radio1.style.couleur_point = "#27ae60";

    GtkWidget *radio1_widget = bouton_radio_creer(&radio1);
    conteneur_ajouter(&radio_container, radio1_widget);

    /* --- Radio 2 --- */
    BoutonRadio radio2;
    bouton_radio_initialiser(&radio2);
    radio2.id_css = "radio_dark_theme";
    radio2.label = "Dark Theme";
    radio2.est_actif = false;
    radio2.style.couleur_texte = "#27ae60";
    radio2.group_leader = GTK_CHECK_BUTTON(radio1_widget);
    radio2.on_toggled = on_radio_toggled;
    radio2.user_data = "Dark Theme";

    GtkWidget *radio2_widget = bouton_radio_creer(&radio2);
    conteneur_ajouter(&radio_container, radio2_widget);

    /* --- Radio 3 --- */
    BoutonRadio radio3;
    bouton_radio_initialiser(&radio3);
    radio3.id_css = "radio_auto_theme";
    radio3.label = "Auto (System)";
    radio3.est_actif = false;
    radio3.style.couleur_texte = "#27ae60";
    radio3.group_leader = GTK_CHECK_BUTTON(radio1_widget);
    radio3.on_toggled = on_radio_toggled;
    radio3.user_data = "Auto (System)";

    GtkWidget *radio3_widget = bouton_radio_creer(&radio3);
    conteneur_ajouter(&radio_container, radio3_widget);

    conteneur_ajouter(&main_container, radio_box);

    /* ========== FORM INPUTS SECTION ========== */
    GtkWidget *inputs_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(inputs_title), "<span font='14' weight='bold' color='#2c3e50'>Form Inputs</span>");
    gtk_label_set_xalign(GTK_LABEL(inputs_title), 0.0);
    conteneur_ajouter(&main_container, inputs_title);

    Conteneur inputs_container;
    conteneur_initialiser(&inputs_container);
    inputs_container.orientation = CONTENEUR_VERTICAL;
    inputs_container.espacement = 10;
    inputs_container.couleur_fond = "#f7f9fc";
    inputs_container.padding.haut = 12;
    inputs_container.padding.bas = 12;
    inputs_container.padding.gauche = 12;
    inputs_container.padding.droite = 12;
    inputs_container.bordure_largeur = 1;
    inputs_container.bordure_couleur = "#dfe6e9";
    inputs_container.bordure_rayon = 6;
    inputs_container.enfants_hexpand = false;
    GtkWidget *inputs_box = conteneur_creer(&inputs_container);

    // --- Name (ChampTexte) ---
    Conteneur row_name;
    conteneur_initialiser(&row_name);
    row_name.orientation = CONTENEUR_HORIZONTAL;
    row_name.espacement = 8;
    row_name.enfants_hexpand = false;
    GtkWidget *row_name_box = conteneur_creer(&row_name);

    GtkWidget *lbl_name = gtk_label_new_with_mnemonic("_Name:");
    ChampTexte *ct_name = g_new0(ChampTexte, 1);
    champtexte_initialiser(ct_name);
    ct_name->id_css = "input_name";
    ct_name->placeholder = "Enter your name";
    ct_name->required = true;
    ct_name->on_change = on_input_text_changed;
    ct_name->on_activate = on_input_text_activate;
    ct_name->on_invalid = on_input_invalid;
    ct_name->user_data = "name";
    GtkWidget *w_name = champ_texte_creer(ct_name);
    g_signal_connect(w_name, "destroy", G_CALLBACK(g_free), ct_name);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_name), w_name);
    conteneur_ajouter(&row_name, lbl_name);
    conteneur_ajouter(&row_name, w_name);
    conteneur_ajouter(&inputs_container, row_name_box);

    // --- Password (ChampMotDePasse) ---
    Conteneur row_pwd;
    conteneur_initialiser(&row_pwd);
    row_pwd.orientation = CONTENEUR_HORIZONTAL;
    row_pwd.espacement = 8;
    row_pwd.enfants_hexpand = false;
    GtkWidget *row_pwd_box = conteneur_creer(&row_pwd);

    GtkWidget *lbl_pwd = gtk_label_new_with_mnemonic("_Password:");
    ChampMotDePasse *cpw = g_new0(ChampMotDePasse, 1);
    champ_motdepasse_initialiser(cpw);
    cpw->id_css = "input_pwd";
    cpw->placeholder = "Enter a strong password";
    cpw->required = true;
    cpw->policy.min_len = 8;
    cpw->policy.require_digit = true;
    cpw->policy.require_upper = true;
    cpw->policy.require_symbol = true;
    cpw->on_change = on_input_text_changed;
    cpw->on_activate = on_input_text_activate;
    cpw->on_invalid = on_input_invalid;
    cpw->user_data = "password";
    GtkWidget *w_pwd = champ_motdepasse_creer(cpw);
    g_signal_connect(w_pwd, "destroy", G_CALLBACK(g_free), cpw);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_pwd), w_pwd);
    conteneur_ajouter(&row_pwd, lbl_pwd);
    conteneur_ajouter(&row_pwd, w_pwd);
    conteneur_ajouter(&inputs_container, row_pwd_box);

    // --- Age (ChampNombre) ---
    Conteneur row_age;
    conteneur_initialiser(&row_age);
    row_age.orientation = CONTENEUR_HORIZONTAL;
    row_age.espacement = 8;
    row_age.enfants_hexpand = false;
    GtkWidget *row_age_box = conteneur_creer(&row_age);

    GtkWidget *lbl_age = gtk_label_new_with_mnemonic("_Age:");
    ChampNombre *cnb = g_new0(ChampNombre, 1);
    champ_nombre_initialiser(cnb);
    cnb->id_css = "input_age";
    cnb->min = 0;
    cnb->max = 120;
    cnb->step = 1;
    cnb->digits = 0;
    cnb->valeur = 18;
    cnb->on_change = on_number_changed;
    cnb->user_data = "age";
    GtkWidget *w_age = champ_nombre_creer(cnb);
    g_signal_connect(w_age, "destroy", G_CALLBACK(g_free), cnb);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_age), w_age);
    conteneur_ajouter(&row_age, lbl_age);
    conteneur_ajouter(&row_age, w_age);
    conteneur_ajouter(&inputs_container, row_age_box);

    // --- Country (ChampSelect) ---
    Conteneur row_country;
    conteneur_initialiser(&row_country);
    row_country.orientation = CONTENEUR_HORIZONTAL;
    row_country.espacement = 8;
    row_country.enfants_hexpand = false;
    GtkWidget *row_country_box = conteneur_creer(&row_country);

    GtkWidget *lbl_country = gtk_label_new_with_mnemonic("_Country:");
    ChampSelect *csel = g_new0(ChampSelect, 1);
    champ_select_initialiser(csel);
    csel->id_css = "input_country";
    csel->required = true;
    csel->on_change = on_select_changed;
    csel->on_invalid = on_input_invalid;
    csel->user_data = "country";
    champ_select_add_item(csel, "Morocco");
    champ_select_add_item(csel, "France");
    champ_select_add_item(csel, "USA");
    champ_select_add_item(csel, "Japan");
    csel->selected_index = 0;
    GtkWidget *w_country = champ_select_creer(csel);
    g_signal_connect(w_country, "destroy", G_CALLBACK(g_free), csel);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_country), w_country);
    conteneur_ajouter(&row_country, lbl_country);
    conteneur_ajouter(&row_country, w_country);
    conteneur_ajouter(&inputs_container, row_country_box);

    // --- Bio (ChampZoneTexte) ---
    Conteneur row_bio;
    conteneur_initialiser(&row_bio);
    row_bio.orientation = CONTENEUR_HORIZONTAL;
    row_bio.espacement = 8;
    row_bio.enfants_hexpand = false;
    GtkWidget *row_bio_box = conteneur_creer(&row_bio);

    GtkWidget *lbl_bio = gtk_label_new_with_mnemonic("_Bio:");
    ChampZoneTexte *czt = g_new0(ChampZoneTexte, 1);
    champ_zone_texte_initialiser(czt);
    czt->id_css = "input_bio";
    czt->required = true;
    czt->max_length = 300;
    czt->wrap_word = true;
    czt->texte = "Short bio...";
    czt->on_change = NULL;
    czt->on_invalid = on_input_invalid;
    czt->user_data = "bio";
    GtkWidget *w_bio = champ_zone_texte_creer(czt);
    g_signal_connect(w_bio, "destroy", G_CALLBACK(g_free), czt);
    gtk_widget_set_size_request(w_bio, 300, 90);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_bio), w_bio);
    conteneur_ajouter(&row_bio, lbl_bio);
    conteneur_ajouter(&row_bio, w_bio);
    conteneur_ajouter(&inputs_container, row_bio_box);

    conteneur_ajouter(&main_container, inputs_box);

    /* ========== FOOTER SECTION ========== */
    Conteneur footer_container;
    conteneur_initialiser(&footer_container);
    footer_container.orientation = CONTENEUR_HORIZONTAL;
    footer_container.espacement = 10;
    footer_container.align_x = ALIGNEMENT_FIN;
    footer_container.marges.haut = 10;
    footer_container.marges.bas = 0;
    footer_container.marges.gauche = 0;
    footer_container.marges.droite = 0;
    footer_container.enfants_hexpand = false;

    GtkWidget *footer_box = conteneur_creer(&footer_container);

    /* --- Cancel Button --- */
    Bouton btn_cancel;
    bouton_initialiser(&btn_cancel);
    btn_cancel.id_css = "btn_cancel";
    btn_cancel.texte = "Cancel";
    btn_cancel.taille.mode = TAILLE_FIXE;
    btn_cancel.taille.largeur = 100;
    btn_cancel.style.bg_normal = "#95a5a6";
    btn_cancel.style.bg_hover = "#7f8c8d";
    btn_cancel.style.fg_normal = "white";
    btn_cancel.style.rayon_arrondi = 5;
    btn_cancel.tooltip = "cancel the form";
    btn_cancel.on_clic = on_button_clicked;
    btn_cancel.style.couleur_bordure = "red";
    btn_cancel.style.epaisseur_bordure = 4;

    GtkWidget *btn_cancel_widget = bouton_creer(&btn_cancel);
    conteneur_ajouter(&footer_container, btn_cancel_widget);

    /* --- Submit Button --- */
    Bouton btn_submit;
    bouton_initialiser(&btn_submit);
    btn_submit.id_css = "btn_submit";
    btn_submit.texte = "Submit";
    btn_submit.taille.mode = TAILLE_FIXE;
    btn_submit.taille.largeur = 100;
    btn_submit.style.bg_normal = "#16a085";
    btn_submit.style.bg_hover = "#138d75";
    btn_submit.style.fg_normal = "white";
    btn_submit.style.rayon_arrondi = 5;
    btn_submit.nom_icone = "document-send-symbolic";
    btn_submit.pos_icone = ICONE_DROITE;
    btn_submit.tooltip = "sumit the form";
    btn_submit.on_clic = on_submit_clicked;
    btn_submit.style.couleur_bordure = "blue";
    btn_submit.style.epaisseur_bordure = 4;
    btn_submit.curseur = CURSEUR_CROIX;
    GtkWidget *btn_submit_widget = bouton_creer(&btn_submit);
    conteneur_ajouter(&footer_container, btn_submit_widget);

    conteneur_ajouter(&main_container, footer_box);

    /* ========== AFFICHAGE DE LA FENETRE ========== */
    printf("[OK] All widgets created successfully!\n");
    printf("[OK] Window size: %dx%d pixels\n\n", main_window.taille.width, main_window.taille.height);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("org.zcode.dashboard", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
