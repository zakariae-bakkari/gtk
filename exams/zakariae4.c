#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/texte.h"
#include "../widgets/headers/bouton.h"
#include "../widgets/headers/bouton_radio.h"
#include "../widgets/headers/bouton_checklist.h"
#include "../widgets/headers/champ_texte.h"
#include "../widgets/headers/champ_motdepasse.h"
#include "../widgets/headers/champ_nombre.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/champ_zone_texte.h"
#include "../widgets/headers/image.h"
#include "../widgets/headers/video.h"
#include "../widgets/headers/slider.h"
#include "../widgets/headers/menu.h"
#include "../widgets/headers/dialog.h"
#include "../widgets/headers/export_xml.h"
#include "../widgets/headers/xml_parser.h"

static void on_menu_click(const char *id, gpointer data)
{
    (void)data;
    printf("[EXAM1] Item de menu cliqué : %s\n", id);
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    // 1. Initialize and Create the Custom Fenetre with title as file name
    static Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = malloc(strlen("zakariae4") + 1);
    strcpy(fenetre.title, "zakariae4");
    fenetre.taille.width = 800;
    fenetre.taille.height = 600;
    fenetre.scroll_mode = SCROLL_VERTICAL;
    fenetre.scroll_overlay = TRUE;
    fenetre.background_image = "resources/images/zakariae.png";

    fenetre.icon_path = "resources/icons/zcode.png";
    fenetre.ico_path = "resources/icons/zcode.ico";

    GtkWidget *window = fenetre_creer(&fenetre, app);

#ifdef _WIN32
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, &fenetre);
#endif

    // 2. Initialize and Create the Custom Conteneur main_box by default
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
    fenetre_ajouter(&fenetre, p_main_box);

    static Menu main_menu;
    menu_initialiser(&main_menu);
    main_menu.orientation = MENU_VERTICAL; // menu latéral vertical
    main_menu.on_click = on_menu_click;

    MenuItem *mon_choix = menu_item_creer("mon_choix", "choix 1", NULL, MENU_ITEM_NORMAL);
    MenuItem *votre_choix = menu_item_creer("votre_choix", "coix 2 ", NULL, MENU_ITEM_NORMAL);
    MenuItem *leur_choix = menu_item_creer("leur_choix", "choix 3", NULL, MENU_ITEM_NORMAL);

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

    gtk_window_present(GTK_WINDOW(window));

    /* 11. XML Export (après présentation pour assurer la réalisation des widgets) */
    xml_export_window(window, "interface.txt");
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("fr.exam.zakariae4", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
