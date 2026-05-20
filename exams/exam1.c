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

static void on_dialog_reponse(int reponse, gpointer data)
{
    AppContext *ctx = (AppContext *)data;
    if (!ctx || !ctx->window)
        return;

    if (reponse == DIALOG_REPONSE_OUI)
    {
        printf("[EXAM1] Réponse: Oui -> Affichage de l'image de fond.\n");
        fenetre_set_background_image(ctx->window, "resources/images/zakariae.png");
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

/* ── Menu OnClick Callback ───────────────────────────────────────── */
static void on_menu_click(const char *id, gpointer data)
{
    (void)data;
    printf("[EXAM1] Item de menu cliqué : %s\n", id);
}

/* ── Initialisation GTK4 ──────────────────────────────────────────── */

static void on_activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    /* 1. Fenêtre principale */
    static Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = g_strdup("Test des fonctions GTK+");
    fenetre.taille.width = 620;
    fenetre.taille.height = 700;
    fenetre.resizable = TRUE;
    fenetre.scroll_mode = SCROLL_VERTICAL;
    fenetre.scroll_overlay = TRUE;
    fenetre.icon_path = "resources/icons/zcode.png";
    fenetre.ico_path = "resources/icons/zcode.ico";
    GtkWidget *window = fenetre_creer(&fenetre, app);

#ifdef _WIN32
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, &fenetre);
#endif
    /* 2. Conteneur principal */
    static Conteneur main_box;
    conteneur_initialiser(&main_box);
    main_box.orientation = CONTENEUR_VERTICAL;
    main_box.espacement = 15;
    main_box.padding.haut = 15;
    main_box.padding.bas = 15;
    main_box.padding.gauche = 20;
    main_box.padding.droite = 20;
    main_box.enfants_hexpand = TRUE;
    main_box.enfants_vexpand = TRUE;
    GtkWidget *p_main = conteneur_creer(&main_box);
    gtk_window_set_child(GTK_WINDOW(window), p_main);

    /* Initialisation du contexte applicatif */
    g_app_ctx.window = window;
    g_app_ctx.main_box = p_main;

    /* 3. MENU */
    static Menu main_menu;
    menu_initialiser(&main_menu);
    main_menu.orientation = MENU_HORIZONTAL;
    main_menu.on_click = on_menu_click;

    MenuItem *mon_choix = menu_item_creer("mon_choix", "MonChoix", NULL, MENU_ITEM_NORMAL);
    MenuItem *votre_choix = menu_item_creer("votre_choix", "VotreChoix", NULL, MENU_ITEM_NORMAL);
    MenuItem *leur_choix = menu_item_creer("leur_choix", "LeurChoix", NULL, MENU_ITEM_NORMAL);

    votre_choix->sous_menu_orientation = MENU_VERTICAL;
    MenuItem *sous_choix1 = menu_item_creer("sous_choix1", "Sous Choix1", NULL, MENU_ITEM_NORMAL);
    MenuItem *sous_choix2 = menu_item_creer("sous_choix2", "Sous Choix2", NULL, MENU_ITEM_NORMAL);
    MenuItem *sous_choix3 = menu_item_creer("sous_choix3", "Sous Choix3", NULL, MENU_ITEM_NORMAL);

    menu_item_ajouter_sous_item(votre_choix, sous_choix1);
    menu_item_ajouter_sous_item(votre_choix, sous_choix2);
    menu_item_ajouter_sous_item(votre_choix, sous_choix3);

    sous_choix2->sous_menu_orientation = MENU_HORIZONTAL;
    MenuItem *choix221 = menu_item_creer("choix221", "Choix221", NULL, MENU_ITEM_NORMAL);
    MenuItem *choix222 = menu_item_creer("choix222", "Choix222", NULL, MENU_ITEM_NORMAL);
    MenuItem *choix223 = menu_item_creer("choix223", "Choix223", NULL, MENU_ITEM_NORMAL);

    menu_item_ajouter_sous_item(sous_choix2, choix221);
    menu_item_ajouter_sous_item(sous_choix2, choix222);
    menu_item_ajouter_sous_item(sous_choix2, choix223);

    menu_ajouter_item(&main_menu, mon_choix);
    menu_ajouter_item(&main_menu, votre_choix);
    menu_ajouter_item(&main_menu, leur_choix);

    GtkWidget *p_menu = menu_creer(&main_menu);
    conteneur_ajouter(&main_box, p_menu);

    /* 4. Vos connaissances de GTK+ */
    static Texte txt_connaissances;
    texte_initialiser(&txt_connaissances);
    txt_connaissances.texte = "Vos connaissances de GTK+";
    txt_connaissances.gras = TRUE;
    txt_connaissances.taille_police = 11;
    conteneur_ajouter(&main_box, texte_creer(&txt_connaissances));

    static Conteneur box_connaissances;
    conteneur_initialiser(&box_connaissances);
    box_connaissances.orientation = CONTENEUR_HORIZONTAL;
    box_connaissances.espacement = 20;
    box_connaissances.bordure_largeur = 1;
    box_connaissances.bordure_couleur = "#7f8c8d";
    box_connaissances.bordure_rayon = 2;
    box_connaissances.padding.haut = 6;
    box_connaissances.padding.bas = 6;
    box_connaissances.padding.gauche = 10;
    box_connaissances.padding.droite = 10;
    GtkWidget *p_box_conn = conteneur_creer(&box_connaissances);

    static BoutonRadio rad_faible, rad_moyen, rad_bien, rad_tres_bien;
    bouton_radio_initialiser(&rad_faible);
    rad_faible.label = "Faible";
    GtkWidget *w_faible = bouton_radio_creer(&rad_faible);

    bouton_radio_initialiser(&rad_moyen);
    rad_moyen.label = "Moyen";
    bouton_radio_set_groupe(&rad_moyen, GTK_CHECK_BUTTON(w_faible));
    GtkWidget *w_moyen = bouton_radio_creer(&rad_moyen);

    bouton_radio_initialiser(&rad_bien);
    rad_bien.label = "Bien";
    bouton_radio_set_groupe(&rad_bien, GTK_CHECK_BUTTON(w_faible));
    GtkWidget *w_bien = bouton_radio_creer(&rad_bien);

    bouton_radio_initialiser(&rad_tres_bien);
    rad_tres_bien.label = "Très Bien";
    rad_tres_bien.est_actif = TRUE; // sélectionné !
    bouton_radio_set_groupe(&rad_tres_bien, GTK_CHECK_BUTTON(w_faible));
    GtkWidget *w_tres_bien = bouton_radio_creer(&rad_tres_bien);

    conteneur_ajouter(&box_connaissances, w_faible);
    conteneur_ajouter(&box_connaissances, w_moyen);
    conteneur_ajouter(&box_connaissances, w_bien);
    conteneur_ajouter(&box_connaissances, w_tres_bien);
    conteneur_ajouter(&main_box, p_box_conn);

    /* 5. Vos Commentaires sur GTK+ */
    static Texte txt_commentaires;
    texte_initialiser(&txt_commentaires);
    txt_commentaires.texte = "Vos Commentaires sur GTK+";
    txt_commentaires.gras = TRUE;
    txt_commentaires.taille_police = 11;
    conteneur_ajouter(&main_box, texte_creer(&txt_commentaires));

    static Conteneur box_commentaires;
    conteneur_initialiser(&box_commentaires);
    box_commentaires.orientation = CONTENEUR_HORIZONTAL;
    box_commentaires.espacement = 15;
    box_commentaires.bordure_largeur = 1;
    box_commentaires.bordure_couleur = "#7f8c8d";
    box_commentaires.bordure_rayon = 2;
    box_commentaires.padding.haut = 6;
    box_commentaires.padding.bas = 6;
    box_commentaires.padding.gauche = 10;
    box_commentaires.padding.droite = 10;
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
    conteneur_ajouter(&main_box, p_box_comm);

    /* 6. Choix de date */
    static Texte txt_date;
    texte_initialiser(&txt_date);
    txt_date.texte = "Choix de date";
    txt_date.gras = TRUE;
    txt_date.taille_police = 11;
    conteneur_ajouter(&main_box, texte_creer(&txt_date));

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
    GtkWidget *p_box_date = conteneur_creer(&box_date);

    static ChampNombre nb_jour, nb_mois, nb_annee;
    champ_nombre_initialiser(&nb_jour);
    nb_jour.min = 1;
    nb_jour.max = 31;
    nb_jour.valeur = 1;
    nb_jour.size.width = 65;
    GtkWidget *w_jour = champ_nombre_creer(&nb_jour);

    champ_nombre_initialiser(&nb_mois);
    nb_mois.min = 0;
    nb_mois.max = 12;
    nb_mois.valeur = 0;
    nb_mois.size.width = 65;
    GtkWidget *w_mois = champ_nombre_creer(&nb_mois);

    champ_nombre_initialiser(&nb_annee);
    nb_annee.min = 0;
    nb_annee.max = 3000;
    nb_annee.valeur = 0;
    nb_annee.size.width = 80;
    GtkWidget *w_annee = champ_nombre_creer(&nb_annee);

    conteneur_ajouter(&box_date, gtk_label_new("Jour"));
    conteneur_ajouter(&box_date, w_jour);
    conteneur_ajouter(&box_date, gtk_label_new("mois"));
    conteneur_ajouter(&box_date, w_mois);
    conteneur_ajouter(&box_date, gtk_label_new("année"));
    conteneur_ajouter(&box_date, w_annee);
    conteneur_ajouter(&main_box, p_box_date);

    /* 7. Dropdown "Faire un choix" */
    static ChampSelect select_choix;
    champ_select_initialiser(&select_choix);
    select_choix.size.width = 200;
    GtkWidget *w_select = champ_select_creer(&select_choix);
    champ_select_add_item(&select_choix, "Faire un choix");
    champ_select_add_item(&select_choix, "Option A");
    champ_select_add_item(&select_choix, "Option B");
    champ_select_set_index(&select_choix, 0);
    conteneur_ajouter(&main_box, w_select);

    /* 8. Le CoronaVirus */
    static Texte txt_corona;
    texte_initialiser(&txt_corona);
    txt_corona.texte = "Le CoronaVirus";
    txt_corona.gras = TRUE;
    txt_corona.taille_police = 11;
    conteneur_ajouter(&main_box, texte_creer(&txt_corona));

    static Conteneur box_corona;
    conteneur_initialiser(&box_corona);
    box_corona.orientation = CONTENEUR_HORIZONTAL;
    box_corona.espacement = 20;
    box_corona.bordure_largeur = 1;
    box_corona.bordure_couleur = "#7f8c8d";
    box_corona.bordure_rayon = 2;
    box_corona.padding.haut = 6;
    box_corona.padding.bas = 6;
    box_corona.padding.gauche = 10;
    box_corona.padding.droite = 10;
    GtkWidget *p_box_corona = conteneur_creer(&box_corona);

    static BoutonRadio rad_malade, rad_suspect, rad_gueri, rad_sain;
    bouton_radio_initialiser(&rad_malade);
    rad_malade.label = "Malade";
    GtkWidget *w_malade = bouton_radio_creer(&rad_malade);

    bouton_radio_initialiser(&rad_suspect);
    rad_suspect.label = "Suspect";
    bouton_radio_set_groupe(&rad_suspect, GTK_CHECK_BUTTON(w_malade));
    GtkWidget *w_suspect = bouton_radio_creer(&rad_suspect);

    bouton_radio_initialiser(&rad_gueri);
    rad_gueri.label = "Guéri";
    bouton_radio_set_groupe(&rad_gueri, GTK_CHECK_BUTTON(w_malade));
    GtkWidget *w_gueri = bouton_radio_creer(&rad_gueri);

    bouton_radio_initialiser(&rad_sain);
    rad_sain.label = "Sain";
    rad_sain.est_actif = TRUE; // sélectionné !
    bouton_radio_set_groupe(&rad_sain, GTK_CHECK_BUTTON(w_malade));
    GtkWidget *w_sain = bouton_radio_creer(&rad_sain);

    conteneur_ajouter(&box_corona, w_malade);
    conteneur_ajouter(&box_corona, w_suspect);
    conteneur_ajouter(&box_corona, w_gueri);
    conteneur_ajouter(&box_corona, w_sain);
    conteneur_ajouter(&main_box, p_box_corona);

    /* 9. Boutons OK et Quitter */
    static Conteneur box_buttons;
    conteneur_initialiser(&box_buttons);
    box_buttons.orientation = CONTENEUR_HORIZONTAL;
    box_buttons.espacement = 30;
    box_buttons.align_x = ALIGNEMENT_CENTRE;
    GtkWidget *p_box_btns = conteneur_creer(&box_buttons);

    static Bouton btn_ok, btn_quitter;
    bouton_initialiser(&btn_ok);
    btn_ok.texte = "_OK"; // Mnémonique 'O'
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
    conteneur_ajouter(&main_box, p_box_btns);

    /* 10. XML Export Unique Call */
    xml_export_window(window, "interface.txt");

    /* 11. Affichage */
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("fr.exam.exam1", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
