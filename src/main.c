#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/texte.h"
#include "../widgets/headers/bouton.h"
#include "../widgets/headers/bouton_radio.h"
#include "../widgets/headers/bouton_checklist.h"
#include "../widgets/headers/champ_texte.h"
#include "../widgets/headers/champ_motdepasse.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/champ_zone_texte.h"
#include "../widgets/headers/slider.h"
#include "../widgets/headers/menu.h"
#include "../widgets/headers/dialog.h"

// =============================================================================
// HELPER — crée un widget Texte simple (label de champ ou section)
// =============================================================================

static GtkWidget *make_label(const char *contenu, TexteType type,
                             const char *couleur, gboolean gras)
{
    Texte *t = g_new0(Texte, 1);
    texte_initialiser(t);
    t->texte = malloc(strlen(contenu) + 1);
    strcpy(t->texte, contenu);
    t->type = type;
    if (couleur)
    {
        t->couleur_texte = malloc(strlen(couleur) + 1);
        strcpy(t->couleur_texte, couleur);
    }
    else
    {
        t->couleur_texte = NULL;
    }
    t->gras = gras;
    t->alignement = TEXTE_ALIGN_LEFT;
    GtkWidget *w = texte_creer(t);
    // libérer la structure Texte à la destruction du widget
    g_signal_connect_swapped(w, "destroy", G_CALLBACK(g_free), t->texte);
    g_signal_connect_swapped(w, "destroy", G_CALLBACK(g_free), t->couleur_texte);
    g_signal_connect_swapped(w, "destroy", G_CALLBACK(g_free), t);
    return w;
}

// =============================================================================
// CALLBACKS
// =============================================================================

/* --- Menu --- */
static void on_menu_click(const char *id, gpointer data)
{
    printf("[MENU]      item cliqué : %s\n", id);
}

/* --- Champ texte / mot de passe : GtkEditable "changed" --- */
static void on_texte_change(GtkEditable *editable, gpointer data)
{
    const char *id = (const char *)data;
    printf("[TEXTE]     %s = '%s'\n", id, gtk_editable_get_text(editable));
}

/* --- Champ zone texte : GtkTextBuffer "changed" --- */
static void on_zone_texte_change(GtkTextBuffer *buffer, gpointer data)
{
    const char *id = (const char *)data;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    char *texte = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    printf("[ZONE]      %s = '%s'\n", id, texte);
    g_free(texte);
}

/* --- Champ select : GtkDropDown "notify::selected" --- */
static void on_select_change(GtkDropDown *dd, gpointer data)
{
    const char *id = (const char *)data;
    GtkStringList *model = GTK_STRING_LIST(gtk_drop_down_get_model(dd));
    guint selected = gtk_drop_down_get_selected(dd);
    const char *valeur = gtk_string_list_get_string(model, selected);
    printf("[SELECT]    %s = '%s'\n", id, valeur);
}

/* --- Slider : GtkRange "value-changed" --- */
static void on_slider_change(GtkRange *range, double valeur, gpointer data)
{
    printf("[SLIDER]    %s = %.0f\n", (char *)data, valeur);
}

/* --- Radio --- */
static void on_radio(GtkCheckButton *w, gpointer data)
{
    if (gtk_check_button_get_active(w))
        printf("[RADIO]     sélectionné : %s\n", (char *)data);
}

/* --- Checklist --- */
static void on_check(GtkCheckButton *w, gpointer data)
{
    printf("[CHECK]     %s : %s\n", (char *)data,
           gtk_check_button_get_active(w) ? "ON" : "OFF");
}

/* --- Validation invalide --- */
static void on_invalid(GtkWidget *w, const char *msg, gpointer data)
{
    printf("[INVALIDE]  %s : %s\n", (char *)data, msg);
}

/* --- Boutons footer --- */
static void on_annuler(GtkWidget *w, gpointer data) { printf("[FORM]      Annuler\n"); }
static void on_submit(GtkWidget *w, gpointer data) { printf("[FORM]      Submit\n"); }

/* --- Dialog réponse --- */
static void on_dialog_reponse(int r, gpointer data)
{
    const char *s = "?";
    switch (r)
    {
    case DIALOG_REPONSE_OK:
        s = "OK";
        break;
    case DIALOG_REPONSE_ANNULER:
        s = "ANNULER";
        break;
    case DIALOG_REPONSE_OUI:
        s = "OUI";
        break;
    case DIALOG_REPONSE_NON:
        s = "NON";
        break;
    case DIALOG_REPONSE_FERMER:
        s = "FERMER";
        break;
    }
    printf("[DIALOG]    '%s' → %s\n", (char *)data, s);
}

/* --- Bouton "condition(dialog)" --- */
#if 0
static void on_conditions(GtkWidget *w, gpointer data)
{
    GtkWindow *parent = (GtkWindow *)data;
    Dialog *cfg = g_new0(Dialog, 1);
    dialog_initialiser(cfg);
    cfg->parent = parent;
    cfg->type = DIALOG_INFO;
    cfg->titre = malloc(strlen("Conditions d'utilisation") + 1);
    strcpy(cfg->titre, "Conditions d'utilisation");
    cfg->message = malloc(strlen(
        "En soumettant ce formulaire, vous acceptez nos conditions générales.\n\n"
        "• Vos données sont protégées et ne seront pas partagées.\n"
        "• Vous pouvez retirer votre consentement à tout moment.\n"
        "• Ce service est réservé aux personnes majeures.");
    cfg->boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
    cfg->on_reponse = on_dialog_reponse;
    cfg->user_data = "conditions";
    dialog_creer(cfg);
    dialog_afficher(cfg);
    g_signal_connect_swapped(cfg->window, "destroy", G_CALLBACK(dialog_free), cfg);
}
#endif

static void on_conditions(GtkWidget *w, gpointer data)
{
    const char *message =
        "En soumettant ce formulaire, vous acceptez nos conditions gÃ©nÃ©rales.\n\n"
        "â€¢ Vos donnÃ©es sont protÃ©gÃ©es et ne seront pas partagÃ©es.\n"
        "â€¢ Vous pouvez retirer votre consentement Ã  tout moment.\n"
        "â€¢ Ce service est rÃ©servÃ© aux personnes majeures.";
    GtkWindow *parent = (GtkWindow *)data;
    Dialog *cfg = g_new0(Dialog, 1);

    dialog_initialiser(cfg);
    cfg->parent = parent;
    cfg->type = DIALOG_INFO;
    cfg->titre = malloc(strlen("Conditions d'utilisation") + 1);
    strcpy(cfg->titre, "Conditions d'utilisation");
    cfg->message = malloc(strlen(message) + 1);
    strcpy(cfg->message, message);
    cfg->boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
    cfg->on_reponse = on_dialog_reponse;
    cfg->user_data = "conditions";
    dialog_creer(cfg);
    dialog_afficher(cfg);
    g_signal_connect_swapped(cfg->window, "destroy", G_CALLBACK(dialog_free), cfg);
    (void)w;
}

// =============================================================================
// ACTIVATE
// =============================================================================
static void activate(GtkApplication *app, gpointer user_data)
{
    // =========================================================================
    // FENETRE
    // =========================================================================
    Fenetre win;
    fenetre_initialiser(&win);
    g_free(win.title);
    win.title = malloc(strlen("Demo Widgets") + 1);
    strcpy(win.title, "Demo Widgets");
    win.taille.width = 720;
    win.taille.height = 540;
    win.resizable = TRUE;
    win.demarrer_maximisee = TRUE;

    GtkWidget *window = fenetre_creer(&win,app);

    // =========================================================================
    // CONTENEUR RACINE (vertical : menu + contenu)
    // =========================================================================
    Conteneur root_ct;
    conteneur_initialiser(&root_ct);
    root_ct.orientation = CONTENEUR_VERTICAL;
    root_ct.espacement = 0;
    GtkWidget *root_box = conteneur_creer(&root_ct);
    gtk_window_set_child(GTK_WINDOW(window), root_box);

    // =========================================================================
    // MENU HORIZONTAL
    // =========================================================================
    Menu *menu = g_new0(Menu, 1);
    menu_initialiser(menu);
    menu->orientation = MENU_HORIZONTAL; // barre principale horizontale
    menu->on_click = on_menu_click;
    menu->espacement = 4;
    g_free(menu->style.bg_barre);
    menu->style.bg_barre = malloc(strlen("#f0c040") + 1);
    strcpy(menu->style.bg_barre, "#f0c040");
    g_free(menu->style.fg_item);
    menu->style.fg_item = malloc(strlen("#2c2c2c") + 1);
    strcpy(menu->style.fg_item, "#2c2c2c");
    g_free(menu->style.bg_item);
    menu->style.bg_item = malloc(strlen("transparent") + 1);
    strcpy(menu->style.bg_item, "transparent");
    g_free(menu->style.bg_item_hover);
    menu->style.bg_item_hover = malloc(strlen("#e0b030") + 1);
    strcpy(menu->style.bg_item_hover, "#e0b030");
    g_free(menu->style.fg_item_hover);
    menu->style.fg_item_hover = malloc(strlen("#000000") + 1);
    strcpy(menu->style.fg_item_hover, "#000000");
    menu->style.rayon_item = 4;
    menu->style.gras = TRUE;

    // ── Item simple "menu" ─────────────────────────────────────────────────
    MenuItem *m_menu = menu_item_creer("menu", "menu", NULL, MENU_ITEM_NORMAL);
    m_menu->sous_menu_orientation = MENU_VERTICAL;

    // ── Item "sous menu >" — sous-menu HORIZONTAL ─────────────────────────
    MenuItem *m_sousmenu = menu_item_creer("sousmenu", "sous menu >", NULL, MENU_ITEM_NORMAL);
    m_sousmenu->sous_menu_orientation = MENU_HORIZONTAL; // ← horizontal

    MenuItem *sm1 = menu_item_creer("sm1", "sous item 1", NULL, MENU_ITEM_NORMAL);
    sm1->sous_menu_orientation = MENU_VERTICAL;

    MenuItem *sm2 = menu_item_creer("sm2", "sous item 2", NULL, MENU_ITEM_NORMAL);
    sm2->sous_menu_orientation = MENU_VERTICAL;

    MenuItem *sm3 = menu_item_creer("sm3", "sous item 3", NULL, MENU_ITEM_NORMAL);
    sm3->sous_menu_orientation = MENU_VERTICAL;

    menu_item_ajouter_sous_item(m_sousmenu, sm1);
    menu_item_ajouter_sous_item(m_sousmenu, sm2);
    menu_item_ajouter_sous_item(m_sousmenu, menu_item_separateur());
    menu_item_ajouter_sous_item(m_sousmenu, sm3);

    // ── Item "sous menu vertical" ──────────────────────────────────────────
    MenuItem *m_vertical = menu_item_creer("vertical", "sous menu vertical", NULL, MENU_ITEM_NORMAL);
    m_vertical->sous_menu_orientation = MENU_VERTICAL;

    MenuItem *sv1 = menu_item_creer("sv1", "option A", NULL, MENU_ITEM_NORMAL);
    sv1->sous_menu_orientation = MENU_VERTICAL;

    MenuItem *sv2 = menu_item_creer("sv2", "option B", NULL, MENU_ITEM_NORMAL);
    sv2->sous_menu_orientation = MENU_VERTICAL;

    MenuItem *sv3 = menu_item_creer("sv3", "option C", NULL, MENU_ITEM_NORMAL);
    sv3->sous_menu_orientation = MENU_VERTICAL;

    menu_item_ajouter_sous_item(m_vertical, sv1);
    menu_item_ajouter_sous_item(m_vertical, sv2);
    menu_item_ajouter_sous_item(m_vertical, sv3);

    // ── Item simple "item" ────────────────────────────────────────────────
    MenuItem *m_item = menu_item_creer("item", "item", NULL, MENU_ITEM_NORMAL);
    m_item->sous_menu_orientation = MENU_VERTICAL;

    menu_ajouter_item(menu, m_menu);
    menu_ajouter_item(menu, m_sousmenu);
    menu_ajouter_item(menu, m_vertical);
    menu_ajouter_item(menu, m_item);

    GtkWidget *menu_widget = menu_creer(menu);
    g_signal_connect_swapped(menu_widget, "destroy", G_CALLBACK(menu_free), menu);
    conteneur_ajouter(&root_ct, menu_widget);

    // =========================================================================
    // CONTAINER MAIN (horizontal : colonne gauche + colonne droite)
    // =========================================================================
    Conteneur main_ct;
    conteneur_initialiser(&main_ct);
    main_ct.orientation = CONTENEUR_HORIZONTAL;
    main_ct.espacement = 0;
    main_ct.enfants_hexpand = TRUE;
    main_ct.enfants_vexpand = TRUE;
    GtkWidget *main_box = conteneur_creer(&main_ct);
    gtk_widget_set_vexpand(main_box, TRUE);
    conteneur_ajouter(&root_ct, main_box);

    // =========================================================================
    // COLONNE GAUCHE — container 1 (formulaire)
    // =========================================================================
    Conteneur col1_ct;
    conteneur_initialiser(&col1_ct);
    col1_ct.orientation = CONTENEUR_VERTICAL;
    col1_ct.espacement = 10;
    col1_ct.padding.haut = 16;
    col1_ct.padding.bas = 16;
    col1_ct.padding.gauche = 20;
    col1_ct.padding.droite = 20;
    g_free(col1_ct.couleur_fond);
    col1_ct.couleur_fond = malloc(strlen("#ffffff") + 1);
    strcpy(col1_ct.couleur_fond, "#ffffff");
    col1_ct.bordure_largeur = 1;
    g_free(col1_ct.bordure_couleur);
    col1_ct.bordure_couleur = malloc(strlen("#cccccc") + 1);
    strcpy(col1_ct.bordure_couleur, "#cccccc");
    GtkWidget *col1_box = conteneur_creer(&col1_ct);
    gtk_widget_set_hexpand(col1_box, TRUE);
    gtk_widget_set_vexpand(col1_box, TRUE);
    conteneur_ajouter(&main_ct, col1_box);

    // Titre H1 "container 1 / text h1 : form1"
    Texte t1;
    texte_initialiser(&t1);
    t1.type = TEXTE_H3;
    t1.texte = malloc(strlen("container 1") + 1);
    strcpy(t1.texte, "container 1");
    conteneur_ajouter(&col1_ct, texte_creer(&t1));

    Texte t1b;
    texte_initialiser(&t1b);
    t1b.type = TEXTE_H1;
    t1b.texte = malloc(strlen("text h1 : form1") + 1);
    strcpy(t1b.texte, "text h1 : form1");
    t1b.couleur_texte = malloc(strlen("#555555") + 1);
    strcpy(t1b.couleur_texte, "#555555");
    conteneur_ajouter(&col1_ct, texte_creer(&t1b));

    // --- Grille de champs ---
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_widget_set_hexpand(grid, TRUE);
    conteneur_ajouter(&col1_ct, grid);

    int r = 0;

    // nom
    GtkWidget *lbl_nom = make_label("nom", TEXTE_NORMAL, "#2c3e50", FALSE);
    ChampTexte *ct_nom = g_new0(ChampTexte, 1);
    champ_texte_initialiser(ct_nom);
    g_free(ct_nom->id_css);
    ct_nom->id_css = malloc(strlen("inp_nom") + 1);
    strcpy(ct_nom->id_css, "inp_nom");
    g_free(ct_nom->placeholder);
    ct_nom->placeholder = malloc(strlen("") + 1);
    strcpy(ct_nom->placeholder, "");
    ct_nom->on_change = on_texte_change; // GtkEditable "changed"
    ct_nom->user_data = "nom";
    GtkWidget *w_nom = champ_texte_creer(ct_nom);
    g_signal_connect_swapped(w_nom, "destroy", G_CALLBACK(champ_texte_free), ct_nom);
    g_signal_connect(w_nom, "destroy", G_CALLBACK(g_free), ct_nom);
    gtk_widget_set_hexpand(w_nom, TRUE);
    gtk_grid_attach(GTK_GRID(grid), lbl_nom, 0, r, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w_nom, 1, r, 1, 1);
    r++;

    // mot_passe
    GtkWidget *lbl_mdp = make_label("mot_p", TEXTE_NORMAL, "#2c3e50", FALSE);
    ChampMotDePasse *ct_mdp = g_new0(ChampMotDePasse, 1);
    champ_motdepasse_initialiser(ct_mdp);
    g_free(ct_mdp->id_css);
    ct_mdp->id_css = malloc(strlen("inp_mdp") + 1);
    strcpy(ct_mdp->id_css, "inp_mdp");
    g_free(ct_mdp->placeholder);
    ct_mdp->placeholder = malloc(strlen("") + 1);
    strcpy(ct_mdp->placeholder, "");
    ct_mdp->on_change = on_texte_change; // GtkEditable "changed"
    ct_mdp->user_data = "mot_passe";
    GtkWidget *w_mdp = champ_motdepasse_creer(ct_mdp);
    g_signal_connect_swapped(w_mdp, "destroy", G_CALLBACK(champ_motdepasse_free), ct_mdp);
    g_signal_connect(w_mdp, "destroy", G_CALLBACK(g_free), ct_mdp);
    gtk_widget_set_hexpand(w_mdp, TRUE);
    gtk_grid_attach(GTK_GRID(grid), lbl_mdp, 0, r, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w_mdp, 1, r, 1, 1);
    r++;

    // select
    GtkWidget *lbl_sel = make_label("select", TEXTE_NORMAL, "#2c3e50", FALSE);
    ChampSelect *ct_sel = g_new0(ChampSelect, 1);
    champ_select_initialiser(ct_sel);
    g_free(ct_sel->id_css);
    ct_sel->id_css = malloc(strlen("inp_sel") + 1);
    strcpy(ct_sel->id_css, "inp_sel");
    champ_select_add_item(ct_sel, "Option 1");
    champ_select_add_item(ct_sel, "Option 2");
    champ_select_add_item(ct_sel, "Option 3");
    ct_sel->selected_index = 0;
    ct_sel->on_change = on_select_change; // GtkDropDown "notify::selected"
    ct_sel->user_data = "select";
    GtkWidget *w_sel = champ_select_creer(ct_sel);
    g_signal_connect_swapped(w_sel, "destroy", G_CALLBACK(champ_select_free), ct_sel);
    g_signal_connect(w_sel, "destroy", G_CALLBACK(g_free), ct_sel);
    gtk_widget_set_hexpand(w_sel, TRUE);
    gtk_grid_attach(GTK_GRID(grid), lbl_sel, 0, r, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w_sel, 1, r, 1, 1);
    r++;

    // zone texte "..."
    GtkWidget *lbl_zt = make_label("...", TEXTE_CAPTION, "#2c3e50", FALSE);
    ChampZoneTexte *ct_zt = g_new0(ChampZoneTexte, 1);
    champ_zone_texte_initialiser(ct_zt);
    g_free(ct_zt->id_css);
    ct_zt->id_css = malloc(strlen("inp_zt") + 1);
    strcpy(ct_zt->id_css, "inp_zt");
    ct_zt->wrap_word = TRUE;
    ct_zt->on_change = on_zone_texte_change; // GtkTextBuffer "changed"
    ct_zt->user_data = "zone_texte";
    GtkWidget *w_zt = champ_zone_texte_creer(ct_zt);
    g_signal_connect_swapped(w_zt, "destroy", G_CALLBACK(champ_zone_texte_free), ct_zt);
    g_signal_connect(w_zt, "destroy", G_CALLBACK(g_free), ct_zt);
    gtk_widget_set_hexpand(w_zt, TRUE);
    gtk_widget_set_size_request(w_zt, -1, 60);
    gtk_grid_attach(GTK_GRID(grid), lbl_zt, 0, r, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), w_zt, 1, r, 1, 1);

    // --- Ligne : checkbox "accepter les termes" + bouton dialog "condition(dialog)" ---
    GtkWidget *terms_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_hexpand(terms_row, TRUE);
    conteneur_ajouter(&col1_ct, terms_row);

    BoutonChecklist chk_terms;
    bouton_checklist_initialiser(&chk_terms);
    g_free(chk_terms.label);
    chk_terms.label = malloc(strlen("accepter les termes") + 1);
    strcpy(chk_terms.label, "accepter les termes");
    chk_terms.etat = CHECKLIST_UNCHECKED;
    chk_terms.on_toggled = on_check;
    chk_terms.user_data = "termes";
    GtkWidget *w_chk = bouton_checklist_creer(&chk_terms);
    gtk_widget_set_hexpand(w_chk, TRUE);
    gtk_box_append(GTK_BOX(terms_row), w_chk);

    // Bouton "condition(dialog)"
    Bouton b_cond;
    bouton_initialiser(&b_cond);
    g_free(b_cond.id_css);
    b_cond.id_css = malloc(strlen("btn_cond") + 1);
    strcpy(b_cond.id_css, "btn_cond");
    g_free(b_cond.texte);
    b_cond.texte = malloc(strlen("condition(dialog)") + 1);
    strcpy(b_cond.texte, "condition(dialog)");
    g_free(b_cond.style.bg_normal);
    b_cond.style.bg_normal = malloc(strlen("#3498db") + 1);
    strcpy(b_cond.style.bg_normal, "#3498db");
    g_free(b_cond.style.bg_hover);
    b_cond.style.bg_hover = malloc(strlen("#2980b9") + 1);
    strcpy(b_cond.style.bg_hover, "#2980b9");
    g_free(b_cond.style.fg_normal);
    b_cond.style.fg_normal = malloc(strlen("#ffffff") + 1);
    strcpy(b_cond.style.fg_normal, "#ffffff");
    b_cond.style.rayon_arrondi = 4;
    b_cond.on_clic = on_conditions;
    b_cond.user_data = GTK_WINDOW(window);
    gtk_box_append(GTK_BOX(terms_row), bouton_creer(&b_cond));

    // --- Ligne footer : Annuler + Submit ---
    GtkWidget *footer_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(footer_row, GTK_ALIGN_END);
    gtk_widget_set_hexpand(footer_row, TRUE);
    conteneur_ajouter(&col1_ct, footer_row);

    Bouton b_ann;
    bouton_initialiser(&b_ann);
    g_free(b_ann.id_css);
    b_ann.id_css = malloc(strlen("btn_ann") + 1);
    strcpy(b_ann.id_css, "btn_ann");
    g_free(b_ann.texte);
    b_ann.texte = malloc(strlen("annuler") + 1);
    strcpy(b_ann.texte, "annuler");
    g_free(b_ann.style.bg_normal);
    b_ann.style.bg_normal = malloc(strlen("#ecf0f1") + 1);
    strcpy(b_ann.style.bg_normal, "#ecf0f1");
    g_free(b_ann.style.bg_hover);
    b_ann.style.bg_hover = malloc(strlen("#d5dbdb") + 1);
    strcpy(b_ann.style.bg_hover, "#d5dbdb");
    g_free(b_ann.style.fg_normal);
    b_ann.style.fg_normal = malloc(strlen("#2c3e50") + 1);
    strcpy(b_ann.style.fg_normal, "#2c3e50");
    b_ann.style.rayon_arrondi = 4;
    b_ann.on_clic = on_annuler;
    gtk_box_append(GTK_BOX(footer_row), bouton_creer(&b_ann));

    Bouton b_sub;
    bouton_initialiser(&b_sub);
    g_free(b_sub.id_css);
    b_sub.id_css = malloc(strlen("btn_sub") + 1);
    strcpy(b_sub.id_css, "btn_sub");
    g_free(b_sub.texte);
    b_sub.texte = malloc(strlen("submit") + 1);
    strcpy(b_sub.texte, "submit");
    g_free(b_sub.style.bg_normal);
    b_sub.style.bg_normal = malloc(strlen("#3498db") + 1);
    strcpy(b_sub.style.bg_normal, "#3498db");
    g_free(b_sub.style.bg_hover);
    b_sub.style.bg_hover = malloc(strlen("#2980b9") + 1);
    strcpy(b_sub.style.bg_hover, "#2980b9");
    g_free(b_sub.style.fg_normal);
    b_sub.style.fg_normal = malloc(strlen("#ffffff") + 1);
    strcpy(b_sub.style.fg_normal, "#ffffff");
    b_sub.style.rayon_arrondi = 4;
    b_sub.on_clic = on_submit;
    gtk_box_append(GTK_BOX(footer_row), bouton_creer(&b_sub));

    // =========================================================================
    // COLONNE DROITE — container 2 (radio / check / range)
    // =========================================================================
    Conteneur col2_ct;
    conteneur_initialiser(&col2_ct);
    col2_ct.orientation = CONTENEUR_VERTICAL;
    col2_ct.espacement = 12;
    col2_ct.padding.haut = 16;
    col2_ct.padding.bas = 16;
    col2_ct.padding.gauche = 20;
    col2_ct.padding.droite = 20;
    g_free(col2_ct.couleur_fond);
    col2_ct.couleur_fond = malloc(strlen("#f8f9fa") + 1);
    strcpy(col2_ct.couleur_fond, "#f8f9fa");
    col2_ct.bordure_largeur = 1;
    g_free(col2_ct.bordure_couleur);
    col2_ct.bordure_couleur = malloc(strlen("#cccccc") + 1);
    strcpy(col2_ct.bordure_couleur, "#cccccc");
    GtkWidget *col2_box = conteneur_creer(&col2_ct);
    gtk_widget_set_hexpand(col2_box, TRUE);
    gtk_widget_set_vexpand(col2_box, TRUE);
    conteneur_ajouter(&main_ct, col2_box);

    // Titre "container 2 / text h1 : form2"
    Texte t2;
    texte_initialiser(&t2);
    t2.type = TEXTE_H3;
    t2.texte = malloc(strlen("container 2") + 1);
    strcpy(t2.texte, "container 2");
    conteneur_ajouter(&col2_ct, texte_creer(&t2));

    Texte t2b;
    texte_initialiser(&t2b);
    t2b.type = TEXTE_H1;
    t2b.texte = malloc(strlen("text h1 : form2") + 1);
    strcpy(t2b.texte, "text h1 : form2");
    t2b.couleur_texte = malloc(strlen("#555555") + 1);
    strcpy(t2b.couleur_texte, "#555555");
    conteneur_ajouter(&col2_ct, texte_creer(&t2b));

    // Séparateur
    conteneur_ajouter(&col2_ct, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // Label section
    GtkWidget *lbl_radio_section = make_label("ici va container bouton radio", TEXTE_CAPTION, "#7f8c8d", FALSE);
    conteneur_ajouter(&col2_ct, lbl_radio_section);

    // --- BOUTONS RADIO ---
    BoutonRadio rad1;
    bouton_radio_initialiser(&rad1);
    g_free(rad1.id_css);
    rad1.id_css = malloc(strlen("r1") + 1);
    strcpy(rad1.id_css, "r1");
    g_free(rad1.label);
    rad1.label = malloc(strlen("Option A") + 1);
    strcpy(rad1.label, "Option A");
    rad1.est_actif = TRUE;
    rad1.on_toggled = on_radio;
    rad1.user_data = "Option A";
    GtkWidget *w_r1 = bouton_radio_creer(&rad1);
    conteneur_ajouter(&col2_ct, w_r1);

    BoutonRadio rad2;
    bouton_radio_initialiser(&rad2);
    g_free(rad2.id_css);
    rad2.id_css = malloc(strlen("r2") + 1);
    strcpy(rad2.id_css, "r2");
    g_free(rad2.label);
    rad2.label = malloc(strlen("Option B") + 1);
    strcpy(rad2.label, "Option B");
    rad2.group_leader = GTK_CHECK_BUTTON(w_r1);
    rad2.on_toggled = on_radio;
    rad2.user_data = "Option B";
    conteneur_ajouter(&col2_ct, bouton_radio_creer(&rad2));

    BoutonRadio rad3;
    bouton_radio_initialiser(&rad3);
    g_free(rad3.id_css);
    rad3.id_css = malloc(strlen("r3") + 1);
    strcpy(rad3.id_css, "r3");
    g_free(rad3.label);
    rad3.label = malloc(strlen("Option C") + 1);
    strcpy(rad3.label, "Option C");
    rad3.group_leader = GTK_CHECK_BUTTON(w_r1);
    rad3.on_toggled = on_radio;
    rad3.user_data = "Option C";
    conteneur_ajouter(&col2_ct, bouton_radio_creer(&rad3));

    // Séparateur
    conteneur_ajouter(&col2_ct, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // Label section checklist
    GtkWidget *lbl_chk_section = make_label("check list", TEXTE_CAPTION, "#7f8c8d", FALSE);
    conteneur_ajouter(&col2_ct, lbl_chk_section);

    // --- CHECKLIST ---
    const char *chk_labels[] = {"Choix 1", "Choix 2", "Choix 3"};
    BoutonChecklistEtat chk_etats[] = {CHECKLIST_CHECKED, CHECKLIST_UNCHECKED, CHECKLIST_CHECKED};
    for (int i = 0; i < 3; i++)
    {
        BoutonChecklist chk;
        bouton_checklist_initialiser(&chk);
        g_free(chk.label);
        chk.label = malloc(strlen(chk_labels[i]) + 1);
        strcpy(chk.label, chk_labels[i]);
        chk.etat = chk_etats[i];
        chk.on_toggled = on_check;
        chk.user_data = (gpointer)chk_labels[i];
        conteneur_ajouter(&col2_ct, bouton_checklist_creer(&chk));
    }

    // Séparateur
    conteneur_ajouter(&col2_ct, gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // Label section range
    GtkWidget *lbl_range_section = make_label("range", TEXTE_CAPTION, "#7f8c8d", FALSE);
    conteneur_ajouter(&col2_ct, lbl_range_section);

    // --- SLIDER (range) ---
    Slider *sld = g_new0(Slider, 1);
    slider_initialiser(sld);
    g_free(sld->id_css);
    sld->id_css = malloc(strlen("sld_main") + 1);
    strcpy(sld->id_css, "sld_main");
    sld->min = 0;
    sld->max = 100;
    sld->valeur = 40;
    sld->step = 1;
    sld->digits = 0;
    sld->afficher_valeur = TRUE;
    sld->afficher_label = TRUE;
    sld->orientation = SLIDER_HORIZONTAL;
    sld->on_change = on_slider_change;
    sld->user_data = "range";
    sld->size.width = 220;
    GtkWidget *w_sld = slider_creer(sld);
    g_signal_connect_swapped(w_sld, "destroy", G_CALLBACK(slider_free), sld);
    g_signal_connect(w_sld, "destroy", G_CALLBACK(g_free), sld);
    conteneur_ajouter(&col2_ct, w_sld);

    // =========================================================================
    // AFFICHAGE
    // =========================================================================
    gtk_window_present(GTK_WINDOW(window));
}

// =============================================================================
// MAIN
// =============================================================================
int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("org.zcode.demo", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
