#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/export_xml.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/texte.h"

static void on_menu_click(const char *id, gpointer data)
{
    (void)data;
    printf("[EXAM1] Item de menu cliqué : %s\n", id);
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
    ExportContext ctx;
    (void)user_data;

    // 1. Initialize and Create the Custom Fenetre with title as file name
    static Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = malloc(strlen("Test des fonctions gtk+ tt") + 1);
    strcpy(fenetre.title, "Test des fonctions gtk+ tt");
    fenetre.taille.width = 200;
    fenetre.taille.height = 200;
    fenetre.icon_path = "resources/icons/zcode.png";
    fenetre.ico_path = "resources/icons/zcode.ico";
    fenetre.background_image = "resources/images/image.jpg";
    fenetre.scroll_mode = SCROLL_BOTH;
    fenetre.scroll_overlay = TRUE;

    GtkWidget *window = fenetre_creer(&fenetre, app);

#ifdef _WIN32
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, &fenetre);
#endif

    // 2. Initialize and Create the Custom Conteneur main_box by default
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

    GtkWidget *p_main_box = conteneur_creer(&main_box);
    conteneur_ajouter(&main, p_main_box);

    static Conteneur right;
    conteneur_initialiser(&right);
    right.orientation = CONTENEUR_VERTICAL;
    right.espacement = 15;
    right.padding.haut = 15;
    right.padding.bas = 15;
    right.padding.gauche = 20;
    right.padding.droite = 20;
    right.enfants_hexpand = TRUE;
    right.enfants_vexpand = TRUE;

    GtkWidget *p_right = conteneur_creer(&right);
    conteneur_ajouter(&main, p_right);

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
    GtkWidget *w_txt_connaissances = texte_creer(&txt_connaissances);
    conteneur_ajouter(&main_box, w_txt_connaissances);

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
    rad_faible.label = "Faible";
    rad_faible.style.couleur_point = "#c0392b";
    GtkWidget *w_faible = bouton_radio_creer(&rad_faible);

    bouton_radio_initialiser(&rad_moyen);
    rad_moyen.label = "Moyen";
    rad_moyen.style.couleur_point = "#f39c12";
    bouton_radio_set_groupe(&rad_moyen, GTK_CHECK_BUTTON(w_faible));
    GtkWidget *w_moyen = bouton_radio_creer(&rad_moyen);

    bouton_radio_initialiser(&rad_bien);
    rad_bien.label = "Bien";
    rad_bien.style.couleur_point = "#27ae60";
    bouton_radio_set_groupe(&rad_bien, GTK_CHECK_BUTTON(w_faible));
    GtkWidget *w_bien = bouton_radio_creer(&rad_bien);

    bouton_radio_initialiser(&rad_tres_bien);
    rad_tres_bien.label = "Très Bien";
    rad_tres_bien.style.couleur_point = "#27ae60";
    rad_tres_bien.est_actif = TRUE; // sélectionné !
    bouton_radio_set_groupe(&rad_tres_bien, GTK_CHECK_BUTTON(w_faible));
    GtkWidget *w_tb = bouton_radio_creer(&rad_tres_bien);

    conteneur_ajouter(&box_connaissances, w_faible);
    conteneur_ajouter(&box_connaissances, w_moyen);
    conteneur_ajouter(&box_connaissances, w_bien);
    conteneur_ajouter(&box_connaissances, w_tb);
    conteneur_ajouter(&main_box, p_box_conn);

    // Menu Multi choix -------------------------------------------------------------------
    static Texte commentaires;
    texte_initialiser(&commentaires);
    commentaires.texte = "Vos Commentaires sur gtk+ :";
    commentaires.gras = TRUE;
    commentaires.taille_police = 11;
    GtkWidget *w_txt_commentaires = texte_creer(&commentaires);
    conteneur_ajouter(&main_box, w_txt_commentaires);

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
    conteneur_ajouter(&main_box, p_box_comm);

    // CORONO+A--------------------------------------
    static Texte txt_corona;
    texte_initialiser(&txt_corona);
    txt_corona.texte = "Le corona virus";
    txt_corona.gras = TRUE;
    txt_corona.taille_police = 11;
    GtkWidget *w_txt_corona = texte_creer(&txt_corona);
    conteneur_ajouter(&main_box, w_txt_corona);

    static Conteneur box_corona;
    conteneur_initialiser(&box_corona);
    box_corona.orientation = CONTENEUR_HORIZONTAL;
    box_corona.espacement = 20;
    box_corona.bordure_largeur = 3;
    box_corona.bordure_couleur = "#081516";
    box_corona.bordure_rayon = 2;
    box_corona.padding.haut = 6;
    box_corona.padding.bas = 6;
    box_corona.padding.gauche = 10;
    box_corona.padding.droite = 10;
    box_corona.couleur_fond = "#bbe3ed";
    box_corona.align_x = ALIGNEMENT_DEBUT;
    box_corona.align_y = ALIGNEMENT_DEBUT;
    GtkWidget *p_box_corona = conteneur_creer(&box_corona);

    static BoutonRadio rad_malade, rad_suspect, rad_gueri, rad_sain;
    bouton_radio_initialiser(&rad_malade);
    rad_malade.label = "Malade";
    rad_malade.style.couleur_point = "#c0392b";
    GtkWidget *w_malade = bouton_radio_creer(&rad_malade);

    bouton_radio_initialiser(&rad_suspect);
    rad_suspect.label = "Susoect";
    rad_suspect.style.couleur_point = "#f39c12";
    bouton_radio_set_groupe(&rad_suspect, GTK_CHECK_BUTTON(w_malade));
    GtkWidget *w_suspect = bouton_radio_creer(&rad_suspect);

    bouton_radio_initialiser(&rad_gueri);
    rad_gueri.label = "Gueri";
    rad_gueri.style.couleur_point = "#27ae60";
    bouton_radio_set_groupe(&rad_gueri, GTK_CHECK_BUTTON(w_malade));
    GtkWidget *w_gueri = bouton_radio_creer(&rad_gueri);

    bouton_radio_initialiser(&rad_sain);
    rad_sain.label = "Sain";
    rad_sain.style.couleur_point = "#27ae60";
    rad_sain.est_actif = TRUE; // sélectionné !
    bouton_radio_set_groupe(&rad_sain, GTK_CHECK_BUTTON(w_malade));
    GtkWidget *w_sain = bouton_radio_creer(&rad_sain);

    conteneur_ajouter(&box_corona, w_malade);
    conteneur_ajouter(&box_corona, w_suspect);
    conteneur_ajouter(&box_corona, w_gueri);
    conteneur_ajouter(&box_corona, w_sain);
    conteneur_ajouter(&main_box, p_box_corona);

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
    box_date.align_x=ALIGNEMENT_DEBUT;
    box_date.align_y=ALIGNEMENT_DEBUT;
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
    conteneur_ajouter(&main_box, p_box_date);

    static Texte txt_choix;
    texte_initialiser(&txt_choix);
    txt_choix.texte = "faire un choix";
    txt_choix.gras = TRUE;
    txt_choix.taille_police = 11;
    conteneur_ajouter(&main_box, texte_creer(&txt_choix));

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
    box_choix.align_x=ALIGNEMENT_DEBUT;
    box_choix.align_y=ALIGNEMENT_DEBUT;
    GtkWidget *p_box_choix = conteneur_creer(&box_choix);

    static ChampSelect choix;
    champ_select_initialiser(&choix);
    choix.size.width = 200;
    GtkWidget *w_choix = champ_select_creer(&choix);
    const char *choice[] = {
        "choix1", "choix2", "choix3", "choix4", "choix5"};
    for (int i = 0; i <5; ++i)
        champ_select_add_item(&choix, choice[i]);
    champ_select_set_index(&choix, 0);

    conteneur_ajouter(&box_choix,w_choix);
    conteneur_ajouter(&main_box, p_box_choix);

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
