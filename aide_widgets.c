#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../widgets/headers/common.h"
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/texte.h"
#include "../../widgets/headers/bouton.h"
#include "../../widgets/headers/bouton_checklist.h"
#include "../../widgets/headers/bouton_radio.h"
#include "../../widgets/headers/champ_texte.h"
#include "../../widgets/headers/champ_motdepasse.h"
#include "../../widgets/headers/champ_nombre.h"
#include "../../widgets/headers/champ_select.h"
#include "../../widgets/headers/champ_zone_texte.h"
#include "../../widgets/headers/slider.h"
#include "../../widgets/headers/dialog.h"
#include "../../widgets/headers/image.h"
#include "../../widgets/headers/menu.h"

static void mon_bouton_action(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;
}

static void ma_checklist_action(GtkCheckButton *widget, gpointer data) {
    (void)widget;
    (void)data;
}

static void mon_radio_action(GtkCheckButton *widget, gpointer data) {
    (void)widget;
    (void)data;
}

static void mon_champ_change(GtkEditable *editable, gpointer user_data) {
    (void)editable;
    (void)user_data;
}

static void mon_champ_active(GtkEntry *entry, gpointer user_data) {
    (void)entry;
    (void)user_data;
}

static void mon_champ_invalide(GtkWidget *widget, const char *message, gpointer user_data) {
    (void)widget;
    (void)message;
    (void)user_data;
}

static void mon_select_change(GtkDropDown *dd, gpointer user_data) {
    (void)dd;
    (void)user_data;
}

static void ma_zone_texte_change(GtkTextBuffer *buffer, gpointer user_data) {
    (void)buffer;
    (void)user_data;
}

static void mon_slider_change(GtkRange *range, double valeur, gpointer user_data) {
    (void)range;
    (void)valeur;
    (void)user_data;
}

static void ma_dialog_reponse(int reponse_id, gpointer user_data) {
    (void)reponse_id;
    (void)user_data;
}

static void mon_menu_clic(const char *id, gpointer data) {
    (void)id;
    (void)data;
}

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    Fenetre f;
    fenetre_initialiser(&f);
    g_free(f.title);
    f.title = g_strdup("Mon Application GTK4");
    f.titre_align = TITRE_ALIGN_CENTRE;
    f.icon_path = NULL;
    f.ico_path = NULL;
    f.type = WIN_TYPE_TOPLEVEL;
    f.resizable = TRUE;
    f.demarrer_maximisee = FALSE;
    f.bouton_fermer = TRUE;
    f.bouton_agrandir = TRUE;
    f.bouton_reduire = TRUE;
    f.scroll_mode = SCROLL_BOTH;
    f.scroll_overlay = TRUE;
    f.content_min_width = 800;
    f.content_min_height = 600;
    f.taille.width = 800;
    f.taille.height = 600;
    f.color_bg = "#ffffff";
    f.background_image = NULL;
    f.position = WIN_POS_MOUSE;
    f.id = 1;

    GtkWidget *window = fenetre_creer(&f, app);

    Conteneur main_box;
    conteneur_initialiser(&main_box);
    main_box.orientation = CONTENEUR_VERTICAL;
    main_box.espacement = 10;
    main_box.homogene = FALSE;
    main_box.taille.largeur = -1;
    main_box.taille.hauteur = -1;
    main_box.align_x = ALIGNEMENT_REMPLIR;
    main_box.align_y = ALIGNEMENT_REMPLIR;
    main_box.enfants_hexpand = TRUE;
    main_box.enfants_vexpand = TRUE;
    main_box.marges.haut = 10;
    main_box.marges.bas = 10;
    main_box.marges.gauche = 10;
    main_box.marges.droite = 10;
    main_box.padding.haut = 10;
    main_box.padding.bas = 10;
    main_box.padding.gauche = 10;
    main_box.padding.droite = 10;
    main_box.id_css = "main_box_css";
    main_box.couleur_fond = "#f0f0f0";
    main_box.bordure_largeur = 1;
    main_box.bordure_couleur = "#cccccc";
    main_box.bordure_rayon = 5;
    main_box.scroll_mode = SCROLL_NONE;
    main_box.scroll_min_width = 0;
    main_box.scroll_min_height = 0;
    main_box.scroll_overlay = FALSE;

    GtkWidget *p_main_box = conteneur_creer(&main_box);
    fenetre_ajouter(&f, p_main_box);

    Texte t;
    texte_initialiser(&t);
    t.texte = g_strdup("Texte de demonstration");
    t.texte_markup = NULL;
    t.use_markup = FALSE;
    t.type = TEXTE_NORMAL;
    t.alignement = TEXTE_ALIGN_LEFT;
    t.decoration = TEXTE_DECORATION_NONE;
    t.famille_police = g_strdup("Arial");
    t.taille_police = 12;
    t.gras = TRUE;
    t.italique = FALSE;
    t.couleur_texte = "#333333";
    t.couleur_fond = "#ffffff";
    t.taille.largeur = -1;
    t.taille.hauteur = -1;
    t.marges.haut = 5;
    t.marges.bas = 5;
    t.marges.gauche = 5;
    t.marges.droite = 5;
    t.align_widget_x = WIDGET_ALIGN_START;
    t.align_widget_y = WIDGET_ALIGN_START;
    t.selectable = TRUE;
    t.wrap = TRUE;
    t.wrap_width = 300;
    t.ellipsize = FALSE;
    t.id_css = "text_css";
    t.bordure_largeur = 0;
    t.bordure_couleur = NULL;
    t.bordure_rayon = 0;

    GtkWidget *p_texte = texte_creer(&t);
    conteneur_ajouter(&main_box, p_texte);

    Bouton b;
    bouton_initialiser(&b);
    b.id_css = "btn_css";
    b.texte = g_strdup("Cliquez-moi");
    b.nom_icone = g_strdup("list-add-symbolic");
    b.pos_icone = ICONE_GAUCHE;
    b.espacement_icone = 5;
    b.taille.mode = TAILLE_AUTO;
    b.taille.largeur = -1;
    b.taille.hauteur = -1;
    b.taille.largeur_min = -1;
    b.taille.hauteur_min = -1;
    b.style.bg_normal = "#e0e0e0";
    b.style.fg_normal = "#000000";
    b.style.bg_hover = "#d0d0d0";
    b.style.fg_hover = "#111111";
    b.style.rayon_arrondi = 4;
    b.style.epaisseur_bordure = 1;
    b.style.couleur_bordure = "#bbbbbb";
    b.style.gras = TRUE;
    b.style.italique = FALSE;
    b.style.taille_texte_px = 14;
    b.curseur = CURSEUR_MAIN;
    b.est_actif = TRUE;
    b.tooltip = g_strdup("Bouton de test");
    b.on_clic = mon_bouton_action;
    b.user_data = NULL;

    GtkWidget *p_btn = bouton_creer(&b);
    conteneur_ajouter(&main_box, p_btn);

    BoutonChecklist chk;
    bouton_checklist_initialiser(&chk);
    chk.id_css = "checklist_css";
    chk.label = g_strdup("Case a cocher");
    chk.pos_label = CHECKLIST_LABEL_DROITE;
    chk.etat = CHECKLIST_UNCHECKED;
    chk.style.couleur_texte = "#000000";
    chk.style.couleur_texte_hover = "#333333";
    chk.style.taille_texte_px = 12;
    chk.style.gras = FALSE;
    chk.tooltip = g_strdup("Case a cocher");
    chk.est_actif = TRUE;
    chk.inconsistent = FALSE;
    chk.on_toggled = ma_checklist_action;
    chk.user_data = NULL;

    GtkWidget *p_chk = bouton_checklist_creer(&chk);
    conteneur_ajouter(&main_box, p_chk);

    BoutonRadio rad;
    bouton_radio_initialiser(&rad);
    rad.id_css = "radio_css";
    rad.label = g_strdup("Bouton radio");
    rad.pos_label = RADIO_LABEL_DROITE;
    rad.est_actif = TRUE;
    rad.sensible = TRUE;
    rad.style.couleur_texte = "#000000";
    rad.style.couleur_texte_hover = "#333333";
    rad.style.couleur_point = "#0000ff";
    rad.style.taille_texte_px = 12;
    rad.style.gras = FALSE;
    rad.tooltip = g_strdup("Bouton radio");
    rad.group_leader = NULL;
    rad.on_toggled = mon_radio_action;
    rad.user_data = NULL;

    GtkWidget *p_rad = bouton_radio_creer(&rad);
    conteneur_ajouter(&main_box, p_rad);

    ChampTexte ct;
    champ_texte_initialiser(&ct);
    ct.id_css = "input_text_css";
    ct.placeholder = g_strdup("Entrez du texte...");
    ct.max_length = 50;
    ct.required = TRUE;
    ct.type = CHAMP_TEXTE_TYPE_TEXT;
    ct.policy.min_len = 2;
    ct.policy.max_len = 50;
    ct.policy.pattern = NULL;
    ct.policy.no_whitespace = FALSE;
    ct.policy.no_digits = FALSE;
    ct.policy.only_digits = FALSE;
    ct.sensitive = TRUE;
    ct.editable = TRUE;
    ct.icon_primary = "edit-find-symbolic";
    ct.icon_secondary = "edit-clear-symbolic";
    ct.size.width = 200;
    ct.size.height = 35;
    ct.style.bg_normal = "#ffffff";
    ct.style.fg_normal = "#000000";
    ct.style.epaisseur_bordure = 1;
    ct.style.couleur_bordure = "#cccccc";
    ct.style.rayon_arrondi = 4;
    ct.style.gras = FALSE;
    ct.style.italique = FALSE;
    ct.style.taille_texte_px = 12;
    ct.style.couleur_bordure_error = "#ff0000";
    ct.style.bg_error = "#ffeae9";
    ct.show_error_label = TRUE;
    ct.erreur_couleur = "#e74c3c";
    ct.erreur_taille_px = 11;
    ct.on_change = mon_champ_change;
    ct.on_activate = mon_champ_active;
    ct.on_invalid = mon_champ_invalide;
    ct.user_data = NULL;

    GtkWidget *p_ct = champ_texte_creer(&ct);
    conteneur_ajouter(&main_box, p_ct);

    ChampMotDePasse cmdp;
    champ_motdepasse_initialiser(&cmdp);
    cmdp.id_css = "input_mdp_css";
    cmdp.placeholder = g_strdup("Mot de passe...");
    cmdp.max_length = 20;
    cmdp.required = TRUE;
    cmdp.policy.min_len = 6;
    cmdp.policy.require_digit = TRUE;
    cmdp.policy.require_upper = TRUE;
    cmdp.policy.require_symbol = FALSE;
    cmdp.reveal_toggle = TRUE;
    cmdp.sensitive = TRUE;
    cmdp.size.width = 200;
    cmdp.size.height = 35;
    cmdp.style.bg_normal = "#ffffff";
    cmdp.style.fg_normal = "#000000";
    cmdp.style.epaisseur_bordure = 1;
    cmdp.style.couleur_bordure = "#cccccc";
    cmdp.style.rayon_arrondi = 4;
    cmdp.style.gras = FALSE;
    cmdp.style.italique = FALSE;
    cmdp.style.taille_texte_px = 12;
    cmdp.show_error_label = TRUE;
    cmdp.erreur_couleur = "#e74c3c";
    cmdp.erreur_taille_px = 11;
    cmdp.on_change = mon_champ_change;
    cmdp.on_activate = mon_champ_active;
    cmdp.on_invalid = mon_champ_invalide;
    cmdp.user_data = NULL;

    GtkWidget *p_cmdp = champ_motdepasse_creer(&cmdp);
    conteneur_ajouter(&main_box, p_cmdp);

    ChampNombre cnum;
    champ_nombre_initialiser(&cnum);
    cnum.id_css = "input_nombre_css";
    cnum.min = 0.0;
    cnum.max = 100.0;
    cnum.step = 1.0;
    cnum.digits = 0;
    cnum.wrap = FALSE;
    cnum.valeur = 10.0;
    cnum.required = TRUE;
    cnum.size.width = 100;
    cnum.size.height = 35;
    cnum.style.bg_normal = "#ffffff";
    cnum.style.fg_normal = "#000000";
    cnum.style.epaisseur_bordure = 1;
    cnum.style.couleur_bordure = "#cccccc";
    cnum.style.rayon_arrondi = 4;
    cnum.style.gras = FALSE;
    cnum.style.italique = FALSE;
    cnum.style.taille_texte_px = 12;
    cnum.on_change = mon_champ_change;
    cnum.on_activate = mon_champ_active;
    cnum.on_invalid = mon_champ_invalide;
    cnum.user_data = NULL;

    GtkWidget *p_cnum = champ_nombre_creer(&cnum);
    conteneur_ajouter(&main_box, p_cnum);

    ChampSelect csel;
    champ_select_initialiser(&csel);
    csel.id_css = "select_css";
    csel.selected_index = -1;
    csel.required = TRUE;
    csel.enable_search = FALSE;
    csel.size.width = 200;
    csel.size.height = 35;
    csel.style.bg_normal = "#ffffff";
    csel.style.fg_normal = "#000000";
    csel.style.epaisseur_bordure = 1;
    csel.style.couleur_bordure = "#cccccc";
    csel.style.rayon_arrondi = 4;
    csel.style.gras = FALSE;
    csel.style.italique = FALSE;
    csel.style.taille_texte_px = 12;
    csel.on_change = mon_select_change;
    csel.on_invalid = mon_champ_invalide;
    csel.user_data = NULL;

    GtkWidget *p_csel = champ_select_creer(&csel);
    champ_select_add_item(&csel, "Item 1");
    champ_select_add_item(&csel, "Item 2");
    conteneur_ajouter(&main_box, p_csel);

    ChampZoneTexte czt;
    champ_zone_texte_initialiser(&czt);
    czt.id_css = "textarea_css";
    czt.texte = g_strdup("Zone de texte multi-lignes");
    czt.max_length = 200;
    czt.wrap_word = TRUE;
    czt.sensitive = TRUE;
    czt.required = FALSE;
    czt.size.width = 300;
    czt.size.height = 80;
    czt.style.bg_normal = "#ffffff";
    czt.style.fg_normal = "#000000";
    czt.style.epaisseur_bordure = 1;
    czt.style.couleur_bordure = "#cccccc";
    czt.style.rayon_arrondi = 4;
    czt.style.gras = FALSE;
    czt.style.italique = FALSE;
    czt.style.taille_texte_px = 12;
    czt.on_change = ma_zone_texte_change;
    czt.on_invalid = mon_champ_invalide;
    czt.user_data = NULL;

    GtkWidget *p_czt = champ_zone_texte_creer(&czt);
    conteneur_ajouter(&main_box, p_czt);

    Slider sld;
    slider_initialiser(&sld);
    sld.id_css = "slider_css";
    sld.min = 0.0;
    sld.max = 100.0;
    sld.step = 1.0;
    sld.valeur = 50.0;
    sld.digits = 0;
    sld.orientation = SLIDER_HORIZONTAL;
    sld.afficher_valeur = TRUE;
    sld.afficher_label = TRUE;
    sld.inverser = FALSE;
    sld.sensitive = TRUE;
    sld.marques_pos = SLIDER_MARQUES_AUCUNE;
    sld.marques_step = 0.0;
    sld.size.width = 250;
    sld.size.height = 40;
    sld.style.bg_normal = "#ffffff";
    sld.style.fg_normal = "#000000";
    sld.style.epaisseur_bordure = 1;
    sld.style.couleur_bordure = "#cccccc";
    sld.style.rayon_arrondi = 4;
    sld.style.gras = FALSE;
    sld.style.italique = FALSE;
    sld.style.taille_texte_px = 12;
    sld.on_change = mon_slider_change;
    sld.user_data = NULL;

    GtkWidget *p_sld = slider_creer(&sld);
    conteneur_ajouter(&main_box, p_sld);

    Image img;
    image_initialiser(&img);
    img.id_css = "image_css";
    img.source_type = IMAGE_SOURCE_ICON_NAME;
    img.file_path = NULL;
    img.icon_name = g_strdup("image-missing-symbolic");
    img.pixbuf = NULL;
    img.legende = g_strdup("Legende de l'image");
    img.width = 64;
    img.height = 64;
    img.fit_mode = IMAGE_FIT_CONTAIN;
    img.can_shrink = TRUE;
    img.sensitive = TRUE;
    img.halign = WIDGET_ALIGN_CENTER;
    img.style.bg_normal = "#ffffff";
    img.style.fg_normal = "#000000";
    img.style.epaisseur_bordure = 1;
    img.style.couleur_bordure = "#cccccc";
    img.style.rayon_arrondi = 4;
    img.style.gras = FALSE;
    img.style.italique = FALSE;
    img.style.taille_texte_px = 12;
    img.legende_couleur = "#7f8c8d";
    img.legende_taille_px = 11;
    img.rayon_arrondi = 4;

    GtkWidget *p_img = image_creer(&img);
    conteneur_ajouter(&main_box, p_img);

    Menu m;
    menu_initialiser(&m);
    m.id_css = "menu_css";
    m.orientation = MENU_HORIZONTAL;
    m.items = NULL;
    m.nb_items = 0;
    m.style.bg_barre = "#f0f0f0";
    m.style.couleur_bordure = "#cccccc";
    m.style.epaisseur_bordure = 1;
    m.style.rayon_arrondi = 0;
    m.style.bg_item = "#ffffff";
    m.style.fg_item = "#000000";
    m.style.bg_item_hover = "#e0e0e0";
    m.style.fg_item_hover = "#111111";
    m.style.bg_item_actif = "#d0d0d0";
    m.style.rayon_item = 4;
    m.style.taille_texte_px = 12;
    m.style.gras = FALSE;
    m.style.couleur_separateur = "#bbbbbb";
    m.style.bg_popover = "#ffffff";
    m.size.width = -1;
    m.size.height = 40;
    m.espacement = 5;
    m.on_click = mon_menu_clic;
    m.user_data = NULL;

    MenuItem *mi1 = menu_item_creer("m1", "Fichier", NULL, MENU_ITEM_NORMAL);
    menu_ajouter_item(&m, mi1);
    GtkWidget *p_menu = menu_creer(&m);
    gtk_box_prepend(GTK_BOX(p_main_box), p_menu);//! a changer

    Dialog dlg;
    dialog_initialiser(&dlg);
    dlg.window = NULL;
    dlg.box_header = NULL;
    dlg.box_corps = NULL;
    dlg.box_footer = NULL;
    dlg.parent = GTK_WINDOW(window);
    dlg.id_css = "dialog_css";
    dlg.type = DIALOG_INFO;
    dlg.titre = g_strdup("Dialogue");
    dlg.message = g_strdup("Message de dialogue");
    dlg.widget_contenu = NULL;
    dlg.boutons_preset = DIALOG_BOUTONS_OK;
    dlg.boutons = NULL;
    dlg.nb_boutons = 0;
    dlg.style.bg_header = "#3498db";
    dlg.style.fg_header = "#ffffff";
    dlg.style.taille_titre = 16;
    dlg.style.titre_gras = TRUE;
    dlg.style.bg_corps = "#ffffff";
    dlg.style.fg_corps = "#333333";
    dlg.style.bg_footer = "#f9f9f9";
    dlg.style.rayon_arrondi = 6;
    dlg.style.epaisseur_bordure = 1;
    dlg.style.couleur_bordure = "#cccccc";
    dlg.style.bg_bouton_principal = "#3498db";
    dlg.style.fg_bouton_principal = "#ffffff";
    dlg.style.bg_bouton_secondaire = "#e0e0e0";
    dlg.style.fg_bouton_secondaire = "#000000";
    dlg.taille.width = 400;
    dlg.taille.height = 250;
    dlg.modal = TRUE;
    dlg.fermeture_croix = TRUE;
    dlg.on_reponse = ma_dialog_reponse;
    dlg.user_data = NULL;

    GtkWidget *p_dlg = dialog_creer(&dlg);
    (void)p_dlg;

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("fr.aide.widgets", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
