#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/export_xml.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/bouton.h"
#include "../widgets/headers/bouton_checklist.h"
#include "../widgets/headers/bouton_radio.h"
#include "../widgets/headers/champ_nombre.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/menu.h"
#include "../widgets/headers/dialog.h"
#include "../widgets/headers/texte.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* ── Structure globale pour le contexte de l'application ───────────────── */
typedef struct
{
    GtkWidget *window;
    GtkWidget *main_box;
} AppContext;

static AppContext g_app_ctx;

static void on_menu_click(const char *id, gpointer data)
{
    (void)data;
    printf("[EXAM1] Item de menu cliqué : %s\n", id);
}

static void on_dialog_reponse(int reponse, gpointer data)
{
    AppContext *ctx = (AppContext *)data;
    if (!ctx || !ctx->window)
        return;

    if (reponse == DIALOG_REPONSE_OUI)
    {
        printf("[EXAM1] Réponse: Oui -> Affichage de l'image de fond.\n");
        fenetre_set_background_image(ctx->window, "resources/images/background.png");
    }
    else if (reponse == DIALOG_REPONSE_NON)
    {
        printf("[EXAM1] Réponse: Non -> Réinitialisation du fond d'écran.\n");
        fenetre_reset_background(ctx->window);
    }
    else
    {
        printf("[EXAM1] Réponse: Annuler ou Fermer\n");
    }
}

static void on_ok_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    AppContext *ctx = (AppContext *)data;
    if (!ctx || !ctx->window)
        return;

    Dialog *dlg = g_new0(Dialog, 1);
    dialog_initialiser(dlg);
    dlg->parent = GTK_WINDOW(ctx->window);
    dlg->type = DIALOG_AVERTISSEMENT;
    dlg->titre = g_strdup("Confirmation");
    dlg->message = g_strdup("Voulez vous afficher une image ?");

    /* Boutons dans l'ordre de l'image d'examen : Oui, Non, Annuler */
    dialog_ajouter_bouton(dlg, "Oui", NULL, DIALOG_REPONSE_OUI, TRUE);
    dialog_ajouter_bouton(dlg, "Non", NULL, DIALOG_REPONSE_NON, FALSE);
    dialog_ajouter_bouton(dlg, "Annuler", NULL, DIALOG_REPONSE_ANNULER, FALSE);

    dlg->on_reponse = on_dialog_reponse;
    dlg->user_data = ctx; // Passer le contexte pour changer le fond dans le callback

    dialog_creer(dlg);
    dialog_afficher(dlg);

    g_signal_connect_swapped(dlg->window, "destroy", G_CALLBACK(dialog_free), dlg);
}

static void on_texte_change(GtkEditable *editable, gpointer data)
{
    const char *id = (const char *)data;
    printf("[TEXTE]     %s = '%s'\n", id, gtk_editable_get_text(editable));
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    // 1. Initialize and Create the Custom Fenetre with title as file name
    static Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = malloc(strlen("une interface particuliere") + 1);
    strcpy(fenetre.title, "une interface particuliere");
    fenetre.taille.width = 600;
    fenetre.taille.height = 700;
    fenetre.icon_path = "resources/icons/zcode.png";
    fenetre.ico_path = "resources/icons/zcode.ico";
    // fenetre.background_image = "resources/images/image.jpg";
    fenetre.scroll_mode = SCROLL_BOTH;
    fenetre.scroll_overlay = TRUE;
    fenetre.bouton_agrandir = FALSE;
    fenetre.bouton_reduire = FALSE;

    GtkWidget *window = fenetre_creer(&fenetre, app);

#ifdef _WIN32
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, &fenetre);
#endif

    // 2. Initialize and Create the Custom Conteneur left_box by default
    static Conteneur main;
    conteneur_initialiser(&main);
    main.orientation = CONTENEUR_HORIZONTAL;
    main.espacement = 15;
    main.padding.haut = 15;
    main.padding.bas = 15;
    main.padding.gauche = 20;
    main.padding.droite = 20;
    main.enfants_hexpand = TRUE;
    main.enfants_vexpand = TRUE;

    GtkWidget *p_main = conteneur_creer(&main);
    fenetre_ajouter(&fenetre, p_main);

    static Conteneur left_box;
    conteneur_initialiser(&left_box);
    left_box.orientation = CONTENEUR_VERTICAL;
    left_box.espacement = 15;
    left_box.padding.haut = 15;
    left_box.padding.bas = 15;
    left_box.padding.gauche = 20;
    left_box.padding.droite = 20;
    left_box.enfants_hexpand = TRUE;
    left_box.enfants_vexpand = TRUE;

    GtkWidget *leftin_box = conteneur_creer(&left_box);
    conteneur_ajouter(&main, leftin_box);

    static Conteneur right;
    conteneur_initialiser(&right);
    right.orientation = CONTENEUR_VERTICAL;
    right.espacement = 15;
    right.padding.haut = 15;
    right.padding.bas = 15;
    right.padding.gauche = 20;
    right.padding.droite = 20;
    right.bordure_largeur = 2;
    right.bordure_couleur = "red";
    right.enfants_hexpand = TRUE;
    right.enfants_vexpand = TRUE;

    GtkWidget *p_right = conteneur_creer(&right);
    conteneur_ajouter(&main, p_right);

    /* 3. MENU */
    static Menu main_menu;
    menu_initialiser(&main_menu);
    main_menu.orientation = MENU_HORIZONTAL;
    main_menu.on_click = on_menu_click;

    MenuItem *mon_choix = menu_item_creer("ordinateur", "ordinateur", NULL, MENU_ITEM_NORMAL);
    MenuItem *votre_choix = menu_item_creer("portable", "portable", NULL, MENU_ITEM_NORMAL);
    MenuItem *leur_choix = menu_item_creer("ecouteur", "ecouteur", NULL, MENU_ITEM_NORMAL);

    votre_choix->sous_menu_orientation = MENU_VERTICAL;
    MenuItem *sous_choix1 = menu_item_creer("marque", "marque", NULL, MENU_ITEM_NORMAL);
    MenuItem *sous_choix2 = menu_item_creer("puissance", "puissance", NULL, MENU_ITEM_NORMAL);
    MenuItem *sous_choix3 = menu_item_creer("prix", "prix", NULL, MENU_ITEM_NORMAL);

    menu_item_ajouter_sous_item(votre_choix, sous_choix1);
    menu_item_ajouter_sous_item(votre_choix, sous_choix2);
    menu_item_ajouter_sous_item(votre_choix, sous_choix3);

    sous_choix2->sous_menu_orientation = MENU_HORIZONTAL;
    MenuItem *choix221 = menu_item_creer("processor", "processor", NULL, MENU_ITEM_NORMAL);
    MenuItem *choix222 = menu_item_creer("vitesse", "vitesse", NULL, MENU_ITEM_NORMAL);
    MenuItem *choix223 = menu_item_creer("disque_dur", "disque dur", NULL, MENU_ITEM_NORMAL);

    menu_item_ajouter_sous_item(sous_choix2, choix221);
    menu_item_ajouter_sous_item(sous_choix2, choix222);
    menu_item_ajouter_sous_item(sous_choix2, choix223);

    menu_ajouter_item(&main_menu, mon_choix);
    menu_ajouter_item(&main_menu, votre_choix);
    menu_ajouter_item(&main_menu, leur_choix);

    GtkWidget *p_menu = menu_creer(&main_menu);
    conteneur_ajouter(&left_box, p_menu);

    /* 4. Vos connaissances de GTK+ */
    static Texte txt_connaissances;
    texte_initialiser(&txt_connaissances);
    txt_connaissances.texte = "zone de saisie de texte";
    txt_connaissances.gras = TRUE;
    txt_connaissances.taille_police = 11;
    GtkWidget *w_txt_connaissances = texte_creer(&txt_connaissances);
    conteneur_ajouter(&left_box, w_txt_connaissances);

    ChampTexte ct;
    champ_texte_initialiser(&ct);
    ct.placeholder = g_strdup("Entrez du texte...");
    ct.max_length = 50;
    ct.type = CHAMP_TEXTE_TYPE_TEXT;
    ct.sensitive = TRUE;
    ct.editable = TRUE;
    ct.size.width = 200;
    ct.size.height = 35;
    ct.on_change = on_texte_change;
    ct.style.epaisseur_bordure = 1;
    ct.style.couleur_bordure = "#cccccc";
    ct.style.rayon_arrondi = 4;
    ct.style.italique = FALSE;
    ct.style.taille_texte_px = 12;
    ct.user_data = NULL;

    GtkWidget *p_ct = champ_texte_creer(&ct);
    conteneur_ajouter(&left_box, p_ct);

    static Texte txt_choix;
    texte_initialiser(&txt_choix);
    txt_choix.texte = "choix d'une ville";
    txt_choix.gras = TRUE;
    txt_choix.taille_police = 11;
    GtkWidget *w_txt_choix = texte_creer(&txt_choix);
    conteneur_ajouter(&left_box, w_txt_choix);

    static Conteneur box_choix;
    conteneur_initialiser(&box_choix);
    box_choix.orientation = CONTENEUR_HORIZONTAL;
    box_choix.espacement = 15;
    box_choix.bordure_largeur = 1;
    box_choix.bordure_couleur = "#7f8c8d";
    box_choix.bordure_rayon = 2;
    box_choix.padding.haut = 8;
    box_choix.padding.bas = 8;
    box_choix.padding.gauche = 12;
    box_choix.padding.droite = 12;
    box_choix.align_x = ALIGNEMENT_DEBUT;
    box_choix.align_y = ALIGNEMENT_DEBUT;
    GtkWidget *p_box_choix = conteneur_creer(&box_choix);

    static ChampSelect choix;
    champ_select_initialiser(&choix);
    choix.size.width = 200;
    GtkWidget *w_choix = champ_select_creer(&choix);
    const char *choice[] = {
        "ouezzane", "figuigue", "larache", "zhiligua", "bne mlal"};
    for (int i = 0; i < 5; ++i)
        champ_select_add_item(&choix, choice[i]);
    champ_select_set_index(&choix, 0);

    conteneur_ajouter(&box_choix, w_choix);
    conteneur_ajouter(&left_box, p_box_choix);
    // !radio
    static Texte txt_radio;
    texte_initialiser(&txt_radio);
    txt_radio.texte = "difficulte du test";
    txt_radio.gras = TRUE;
    txt_radio.taille_police = 11;
    GtkWidget *w_txt_radio = texte_creer(&txt_radio);
    conteneur_ajouter(&left_box, w_txt_radio);

    static Conteneur box_connaissances;
    conteneur_initialiser(&box_connaissances);
    box_connaissances.orientation = CONTENEUR_HORIZONTAL;
    box_connaissances.espacement = 20;
    box_connaissances.bordure_largeur = 3;
    box_connaissances.bordure_couleur = "#081516";
    box_connaissances.bordure_rayon = 2;
    box_connaissances.padding.haut = 6;
    box_connaissances.padding.bas = 6;
    box_connaissances.padding.gauche = 10;
    box_connaissances.padding.droite = 10;
    box_connaissances.couleur_fond = "#bbe3ed";
    box_connaissances.align_x = ALIGNEMENT_DEBUT;
    box_connaissances.align_y = ALIGNEMENT_DEBUT;
    GtkWidget *p_box_conn = conteneur_creer(&box_connaissances);

    static BoutonRadio rad_faible, rad_moyen, rad_bien, rad_tres_bien;
    bouton_radio_initialiser(&rad_faible);
    rad_faible.label = "Fassile";
    rad_faible.style.couleur_point = "#c0392b";
    GtkWidget *w_faible = bouton_radio_creer(&rad_faible);

    bouton_radio_initialiser(&rad_moyen);
    rad_moyen.label = "Moyen";
    rad_moyen.style.couleur_point = "#f39c12";
    bouton_radio_set_groupe(&rad_moyen, GTK_CHECK_BUTTON(w_faible));
    GtkWidget *w_moyen = bouton_radio_creer(&rad_moyen);

    bouton_radio_initialiser(&rad_bien);
    rad_bien.label = "difficil";
    rad_bien.style.couleur_point = "#27ae60";
    bouton_radio_set_groupe(&rad_bien, GTK_CHECK_BUTTON(w_faible));
    GtkWidget *w_bien = bouton_radio_creer(&rad_bien);

    conteneur_ajouter(&box_connaissances, w_faible);
    conteneur_ajouter(&box_connaissances, w_moyen);
    conteneur_ajouter(&box_connaissances, w_bien);
    conteneur_ajouter(&left_box, p_box_conn);

    // !checkbox
    static Texte txt_checkbox;
    texte_initialiser(&txt_checkbox);
    txt_checkbox.texte = "vos commentaires sur gtk+";
    txt_checkbox.gras = TRUE;
    txt_checkbox.taille_police = 11;
    GtkWidget *w_txt_checkbox = texte_creer(&txt_checkbox);
    conteneur_ajouter(&left_box, w_txt_checkbox);

    static Conteneur box_commentaires;
    conteneur_initialiser(&box_commentaires);
    box_commentaires.orientation = CONTENEUR_HORIZONTAL;
    box_commentaires.espacement = 20;
    box_commentaires.bordure_largeur = 3;
    box_commentaires.bordure_couleur = "#081516";
    box_commentaires.bordure_rayon = 2;
    box_commentaires.padding.haut = 6;
    box_commentaires.padding.bas = 6;
    box_commentaires.padding.gauche = 10;
    box_commentaires.padding.droite = 10;
    box_commentaires.couleur_fond = "#bbe3ed";
    box_commentaires.align_x = ALIGNEMENT_DEBUT;
    box_commentaires.align_y = ALIGNEMENT_DEBUT;
    GtkWidget *p_box_comm = conteneur_creer(&box_commentaires);

    static BoutonChecklist chk_conv, chk_int, chk_facile, chk_optimise;
    bouton_checklist_initialiser(&chk_conv);
    chk_conv.label = "Convivial";
    GtkWidget *w_conv = bouton_checklist_creer(&chk_conv);

    bouton_checklist_initialiser(&chk_int);
    chk_int.label = "Intéressant";
    chk_int.etat = CHECKLIST_CHECKED; // sélectionné !
    GtkWidget *w_int = bouton_checklist_creer(&chk_int);

    bouton_checklist_initialiser(&chk_facile);
    chk_facile.label = "Facile à utiliser";
    GtkWidget *w_facile = bouton_checklist_creer(&chk_facile);

    bouton_checklist_initialiser(&chk_optimise);
    chk_optimise.label = "Optimise le travail";
    GtkWidget *w_optimise = bouton_checklist_creer(&chk_optimise);

    conteneur_ajouter(&box_commentaires, w_conv);
    conteneur_ajouter(&box_commentaires, w_int);
    conteneur_ajouter(&box_commentaires, w_facile);
    conteneur_ajouter(&box_commentaires, w_optimise);
    conteneur_ajouter(&left_box, p_box_comm);

    // !!!! right conteneur
    // !radio
    static Texte txt_radio_genre;
    texte_initialiser(&txt_radio_genre);
    txt_radio_genre.texte = "genre";
    txt_radio_genre.gras = TRUE;
    txt_radio_genre.taille_police = 11;
    GtkWidget *w_txt_radio_genre = texte_creer(&txt_radio_genre);
    conteneur_ajouter(&right, w_txt_radio_genre);

    static Conteneur box_genre;
    conteneur_initialiser(&box_genre);
    box_genre.orientation = CONTENEUR_VERTICAL;
    box_genre.espacement = 20;
    box_genre.bordure_largeur = 3;
    box_genre.bordure_couleur = "#081516";
    box_genre.bordure_rayon = 2;
    box_genre.padding.haut = 6;
    box_genre.padding.bas = 6;
    box_genre.padding.gauche = 10;
    box_genre.padding.droite = 10;
    box_genre.align_x = ALIGNEMENT_DEBUT;
    box_genre.align_y = ALIGNEMENT_DEBUT;
    GtkWidget *p_box_genre = conteneur_creer(&box_genre);

    static BoutonRadio homme, femme;
    bouton_radio_initialiser(&homme);
    homme.label = "homme";
    homme.style.couleur_point = "#c0392b";
    GtkWidget *w_homme = bouton_radio_creer(&homme);

    bouton_radio_initialiser(&femme);
    femme.label = "femme";
    femme.style.couleur_point = "#f39c12";
    bouton_radio_set_groupe(&femme, GTK_CHECK_BUTTON(w_homme));
    GtkWidget *w_femme = bouton_radio_creer(&femme);

    conteneur_ajouter(&box_genre, w_homme);
    conteneur_ajouter(&box_genre, w_femme);
    conteneur_ajouter(&right, p_box_genre);

    static Texte txt_id;
    texte_initialiser(&txt_id);
    txt_id.texte = "identifiant";
    txt_id.gras = TRUE;
    txt_id.taille_police = 11;
    GtkWidget *w_txt_id = texte_creer(&txt_id);
    conteneur_ajouter(&right, w_txt_id);

    ChampTexte ct2;
    champ_texte_initialiser(&ct2);
    ct2.placeholder = g_strdup("Entrez votre email...");
    ct2.max_length = 50;
    ct2.type = CHAMP_TEXTE_TYPE_EMAIL;
    ct2.sensitive = TRUE;
    ct2.editable = TRUE;
    ct2.on_change = on_texte_change;
    ct2.size.width = 200;
    ct2.size.height = 35;
    ct2.style.epaisseur_bordure = 1;
    ct2.style.couleur_bordure = "#cccccc";
    ct2.style.rayon_arrondi = 4;
    ct2.style.italique = FALSE;
    ct2.style.taille_texte_px = 12;
    ct2.user_data = NULL;

    GtkWidget *p_ct2 = champ_texte_creer(&ct2);
    conteneur_ajouter(&right, p_ct2);

    static Texte txt_pass;
    texte_initialiser(&txt_pass);
    txt_pass.texte = "mote de passe";
    txt_pass.gras = TRUE;
    txt_pass.taille_police = 11;
    GtkWidget *w_txt_pass = texte_creer(&txt_pass);
    conteneur_ajouter(&right, w_txt_pass);

    ChampMotDePasse ct3;
    champ_motdepasse_initialiser(&ct3);
    ct3.placeholder = g_strdup("Entrez votre password...");
    ct3.max_length = 50;
    ct3.sensitive = TRUE;
    ct3.size.width = 200;
    ct3.on_change = on_texte_change;
    ct3.size.height = 35;
    ct3.style.epaisseur_bordure = 1;
    ct3.style.couleur_bordure = "#cccccc";
    ct3.style.rayon_arrondi = 4;
    ct3.style.italique = FALSE;
    ct3.style.taille_texte_px = 12;
    ct3.user_data = NULL;

    GtkWidget *p_ct3 = champ_motdepasse_creer(&ct3);
    conteneur_ajouter(&right, p_ct3);

    static BoutonChecklist chk_afficher_pass;
    bouton_checklist_initialiser(&chk_afficher_pass);
    chk_afficher_pass.label = "afficher mot de passe";
    GtkWidget *w_pass = bouton_checklist_creer(&chk_afficher_pass);
    conteneur_ajouter(&right, w_pass);

    /* 6. Choix de date */
    static Texte txt_date;
    texte_initialiser(&txt_date);
    txt_date.texte = "Choix de date";
    txt_date.gras = TRUE;
    txt_date.taille_police = 11;
    conteneur_ajouter(&right, texte_creer(&txt_date));

    static Conteneur box_date;
    conteneur_initialiser(&box_date);
    box_date.orientation = CONTENEUR_HORIZONTAL;
    box_date.espacement = 15;
    box_date.bordure_largeur = 1;
    box_date.bordure_couleur = "#7f8c8d";
    box_date.bordure_rayon = 2;
    box_date.padding.haut = 8;
    box_date.padding.bas = 8;
    box_date.padding.gauche = 12;
    box_date.padding.droite = 12;
    box_date.align_x = ALIGNEMENT_DEBUT;
    box_date.align_y = ALIGNEMENT_DEBUT;
    GtkWidget *p_box_date = conteneur_creer(&box_date);

    static ChampSelect nb_jour;
    champ_select_initialiser(&nb_jour);
    nb_jour.size.width = 200;
    GtkWidget *w_jour = champ_select_creer(&nb_jour);
    char buf[8];
    for (int i = 1; i < 32; ++i)
    {
        snprintf(buf, sizeof(buf), "%d", i);
        champ_select_add_item(&nb_jour, buf);
    }
    champ_select_set_index(&nb_jour, 0);

    static ChampSelect nb_mois;
    champ_select_initialiser(&nb_mois);
    nb_mois.size.width = 200;
    GtkWidget *w_mois = champ_select_creer(&nb_mois);
    const char *months[] = {
        "Janvier", "Février", "Mars", "Avril", "Mai", "Juin",
        "Juillet", "Août", "Septembre", "Octobre", "Novembre", "Décembre"};
    for (int i = 0; i < 12; ++i)
        champ_select_add_item(&nb_mois, months[i]);
    champ_select_set_index(&nb_mois, 0);

    static ChampSelect nb_annee;
    champ_select_initialiser(&nb_annee);
    nb_annee.size.width = 200;
    GtkWidget *w_annee = champ_select_creer(&nb_annee);
    const char *year[] = {
        "2000", "2001", "2002", "2003", "2004", "2005",
        "2006", "2007", "2008", "2009", "2010", "2011"};
    for (int i = 0; i < 12; ++i)
        champ_select_add_item(&nb_annee, year[i]);
    champ_select_set_index(&nb_annee, 0);

    conteneur_ajouter(&box_date, gtk_label_new("Jour"));
    conteneur_ajouter(&box_date, w_jour);
    conteneur_ajouter(&box_date, gtk_label_new("mois"));
    conteneur_ajouter(&box_date, w_mois);
    conteneur_ajouter(&box_date, gtk_label_new("année"));
    conteneur_ajouter(&box_date, w_annee);
    conteneur_ajouter(&right, p_box_date);

    /* 9. Boutons OK et Quitter */
    static Conteneur box_buttons;
    conteneur_initialiser(&box_buttons);
    box_buttons.orientation = CONTENEUR_HORIZONTAL;
    box_buttons.espacement = 30;
    box_buttons.align_x = ALIGNEMENT_CENTRE;
    GtkWidget *p_box_btns = conteneur_creer(&box_buttons);

    static Bouton btn_ok, btn_quitter;
    bouton_initialiser(&btn_ok);
    btn_ok.texte = "_OK";
    btn_ok.taille.mode = TAILLE_FIXE;
    btn_ok.taille.largeur = 200;
    btn_ok.taille.hauteur = 45;
    btn_ok.style.gras = TRUE;
    btn_ok.style.taille_texte_px = 16;
    btn_ok.on_clic = on_ok_clicked;
    btn_ok.user_data = &g_app_ctx; // Passer le contexte applicatif global
    GtkWidget *w_ok = bouton_creer(&btn_ok);

    bouton_initialiser(&btn_quitter);
    btn_quitter.texte = "_Quitter"; // Mnémonique 'Q'
    btn_quitter.taille.mode = TAILLE_FIXE;
    btn_quitter.taille.largeur = 200;
    btn_quitter.taille.hauteur = 45;
    btn_quitter.style.gras = TRUE;
    btn_quitter.style.taille_texte_px = 16;
    btn_quitter.on_clic = action_quitter;
    btn_quitter.user_data = window; // Passer directement la fenêtre
    GtkWidget *w_quit = bouton_creer(&btn_quitter);

    conteneur_ajouter(&box_buttons, w_ok);
    conteneur_ajouter(&box_buttons, w_quit);
    conteneur_ajouter(&right, p_box_btns);

    //------     ------    ------ partie fenetre -------    --------   -----

    // 4. Present the window
    gtk_window_present(GTK_WINDOW(window));
    xml_export_window(window, "interface.txt");
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("fr.exam.exam", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
