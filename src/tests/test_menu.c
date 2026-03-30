#include <gtk/gtk.h>
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/menu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void on_menu_click(const char *id, gpointer user_data)
{
   (void)user_data;
   printf("[MENU] Item clique : '%s'\n", id);
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
   (void)user_data;

   Fenetre fenetre;
   fenetre_initialiser(&fenetre);
   g_free(fenetre.title);
   fenetre.title = malloc(strlen("Test Menu - Demo Complete") + 1);
   strcpy(fenetre.title, "Test Menu - Demo Complete");
   fenetre.taille.width = 900;
   fenetre.taille.height = 700;
   fenetre.scroll_mode = SCROLL_VERTICAL;

   GtkWidget *window = fenetre_creer(&fenetre, app);

   Conteneur main_ct;
   conteneur_initialiser(&main_ct);
   main_ct.orientation = CONTENEUR_VERTICAL;
   main_ct.espacement = 30;
   main_ct.padding.bas = 20;
   g_free(main_ct.couleur_fond);
   main_ct.couleur_fond = malloc(strlen("#f0f2f5") + 1);
   strcpy(main_ct.couleur_fond, "#f0f2f5");

   GtkWidget *main_box = conteneur_creer(&main_ct);
   if (fenetre.scroll_mode != SCROLL_NONE && fenetre.scroll_widget)
      gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fenetre.scroll_widget), main_box);
   else
      gtk_window_set_child(GTK_WINDOW(window), main_box);

   GtkWidget *lbl1 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl1), "<b><big>MENU HORIZONTAL - sous-menus verticaux</big></b>");
   gtk_widget_set_margin_top(lbl1, 20);
   gtk_widget_set_margin_start(lbl1, 20);
   gtk_label_set_xalign(GTK_LABEL(lbl1), 0.0f);
   conteneur_ajouter(&main_ct, lbl1);

   Menu *menu_h = g_new0(Menu, 1);
   menu_initialiser(menu_h);
   g_free(menu_h->id_css);
   menu_h->id_css = malloc(strlen("menu_principal") + 1);
   strcpy(menu_h->id_css, "menu_principal");
   menu_h->orientation = MENU_HORIZONTAL;
   menu_h->espacement = 4;
   menu_h->on_click = on_menu_click;

   MenuItem *i_accueil = menu_item_creer("accueil", "Accueil", "go-home-symbolic", MENU_ITEM_NORMAL);
   i_accueil->tooltip = malloc(strlen("Retour a l'accueil") + 1);
   strcpy(i_accueil->tooltip, "Retour a l'accueil");
   menu_ajouter_item(menu_h, i_accueil);

   MenuItem *i_fichier = menu_item_creer("fichier", "Fichier", "document-open-symbolic", MENU_ITEM_NORMAL);
   i_fichier->sous_menu_orientation = MENU_VERTICAL;
   menu_item_ajouter_sous_item(i_fichier, menu_item_creer("fichier_nouveau", "Nouveau", "document-new-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(i_fichier, menu_item_creer("fichier_ouvrir", "Ouvrir", "document-open-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(i_fichier, menu_item_separateur());
   menu_item_ajouter_sous_item(i_fichier, menu_item_creer("fichier_sauv", "Sauvegarder", "document-save-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(i_fichier, menu_item_creer("fichier_sauv_as", "Sauvegarder sous", "document-save-as-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(i_fichier, menu_item_separateur());
   menu_item_ajouter_sous_item(i_fichier, menu_item_creer("fichier_quitter", "Quitter", "application-exit-symbolic", MENU_ITEM_NORMAL));
   menu_ajouter_item(menu_h, i_fichier);

   MenuItem *i_edition = menu_item_creer("edition", "Edition", "edit-symbolic", MENU_ITEM_NORMAL);
   i_edition->sous_menu_orientation = MENU_VERTICAL;
   menu_item_ajouter_sous_item(i_edition, menu_item_creer("edition_couper", "Couper", "edit-cut-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(i_edition, menu_item_creer("edition_copier", "Copier", "edit-copy-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(i_edition, menu_item_creer("edition_coller", "Coller", "edit-paste-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(i_edition, menu_item_separateur());
   menu_item_ajouter_sous_item(i_edition, menu_item_creer("edition_annuler", "Annuler", "edit-undo-symbolic", MENU_ITEM_DESACTIVE));
   menu_item_ajouter_sous_item(i_edition, menu_item_creer("edition_retablir", "Retablir", "edit-redo-symbolic", MENU_ITEM_DESACTIVE));
   menu_ajouter_item(menu_h, i_edition);

   MenuItem *i_affichage = menu_item_creer("affichage", "Affichage", "video-display-symbolic", MENU_ITEM_NORMAL);
   i_affichage->sous_menu_orientation = MENU_HORIZONTAL;
   menu_item_ajouter_sous_item(i_affichage, menu_item_creer("zoom_in", "Zoom +", "zoom-in-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(i_affichage, menu_item_creer("zoom_out", "Zoom -", "zoom-out-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(i_affichage, menu_item_creer("zoom_fit", "Ajuster", "zoom-fit-best-symbolic", MENU_ITEM_NORMAL));
   menu_ajouter_item(menu_h, i_affichage);

   menu_ajouter_item(menu_h, menu_item_separateur());
   menu_ajouter_item(menu_h, menu_item_creer("aide", "Aide", "help-about-symbolic", MENU_ITEM_NORMAL));

   GtkWidget *w_menu_h = menu_creer(menu_h);
   menu_set_item_actif(menu_h, "accueil");
   g_signal_connect_swapped(w_menu_h, "destroy", G_CALLBACK(menu_free), menu_h);
   conteneur_ajouter(&main_ct, w_menu_h);

   GtkWidget *lbl2 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl2), "<b><big>MENU VERTICAL - sous-menus mixtes</big></b>");
   gtk_widget_set_margin_start(lbl2, 20);
   gtk_label_set_xalign(GTK_LABEL(lbl2), 0.0f);
   conteneur_ajouter(&main_ct, lbl2);

   Conteneur row;
   conteneur_initialiser(&row);
   row.orientation = CONTENEUR_HORIZONTAL;
   row.padding.gauche = 20;
   row.padding.droite = 20;
   GtkWidget *row_box = conteneur_creer(&row);
   conteneur_ajouter(&main_ct, row_box);

   Menu *menu_v = g_new0(Menu, 1);
   menu_initialiser(menu_v);
   g_free(menu_v->id_css);
   menu_v->id_css = malloc(strlen("menu_sidebar") + 1);
   strcpy(menu_v->id_css, "menu_sidebar");
   menu_v->orientation = MENU_VERTICAL;
   menu_v->espacement = 2;
   menu_v->size.width = 200;
   menu_v->on_click = on_menu_click;

   g_free(menu_v->style.bg_barre);
   g_free(menu_v->style.bg_item_hover);
   g_free(menu_v->style.bg_item_actif);
   menu_v->style.bg_barre = malloc(strlen("#1a252f") + 1);
   strcpy(menu_v->style.bg_barre, "#1a252f");
   menu_v->style.bg_item_hover = malloc(strlen("#2c3e50") + 1);
   strcpy(menu_v->style.bg_item_hover, "#2c3e50");
   menu_v->style.bg_item_actif = malloc(strlen("#e74c3c") + 1);
   strcpy(menu_v->style.bg_item_actif, "#e74c3c");

   MenuItem *v_tableau = menu_item_creer("tableau_bord", "Tableau de bord", "view-grid-symbolic", MENU_ITEM_NORMAL);
   MenuItem *v_utilisateurs = menu_item_creer("utilisateurs", "Utilisateurs", "system-users-symbolic", MENU_ITEM_NORMAL);
   v_utilisateurs->sous_menu_orientation = MENU_VERTICAL;
   menu_item_ajouter_sous_item(v_utilisateurs, menu_item_creer("util_liste", "Liste", "view-list-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(v_utilisateurs, menu_item_creer("util_ajouter", "Ajouter", "list-add-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(v_utilisateurs, menu_item_creer("util_groupes", "Groupes", "folder-symbolic", MENU_ITEM_NORMAL));

   MenuItem *v_rapports = menu_item_creer("rapports", "Rapports", "x-office-spreadsheet-symbolic", MENU_ITEM_NORMAL);
   v_rapports->sous_menu_orientation = MENU_HORIZONTAL;
   menu_item_ajouter_sous_item(v_rapports, menu_item_creer("rap_jour", "Jour", NULL, MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(v_rapports, menu_item_creer("rap_semaine", "Semaine", NULL, MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(v_rapports, menu_item_creer("rap_mois", "Mois", NULL, MENU_ITEM_NORMAL));

   menu_ajouter_item(menu_v, v_tableau);
   menu_ajouter_item(menu_v, v_utilisateurs);
   menu_ajouter_item(menu_v, v_rapports);
   menu_ajouter_item(menu_v, menu_item_separateur());
   menu_ajouter_item(menu_v, menu_item_creer("parametres", "Parametres", "preferences-system-symbolic", MENU_ITEM_NORMAL));
   menu_ajouter_item(menu_v, menu_item_creer("deconnexion", "Deconnexion", "system-log-out-symbolic", MENU_ITEM_NORMAL));

   GtkWidget *w_menu_v = menu_creer(menu_v);
   menu_set_item_actif(menu_v, "tableau_bord");
   g_signal_connect_swapped(w_menu_v, "destroy", G_CALLBACK(menu_free), menu_v);
   gtk_box_append(GTK_BOX(row_box), w_menu_v);

   Conteneur zone;
   conteneur_initialiser(&zone);
   zone.orientation = CONTENEUR_VERTICAL;
   zone.espacement = 10;
   zone.padding.haut = 20;
   zone.padding.gauche = 20;
   g_free(zone.couleur_fond);
   zone.couleur_fond = malloc(strlen("#ffffff") + 1);
   strcpy(zone.couleur_fond, "#ffffff");

   GtkWidget *zone_box = conteneur_creer(&zone);
   gtk_widget_set_hexpand(zone_box, TRUE);
   gtk_box_append(GTK_BOX(row_box), zone_box);

   GtkWidget *zone_title = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(zone_title), "<b>Zone de contenu</b>");
   gtk_label_set_xalign(GTK_LABEL(zone_title), 0.0f);
   conteneur_ajouter(&zone, zone_title);
   conteneur_ajouter(&zone, gtk_label_new("Cliquez sur les items du menu pour voir les callbacks dans la console."));

   GtkWidget *lbl3 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl3), "<b><big>MENU HORIZONTAL - theme clair</big></b>");
   gtk_widget_set_margin_start(lbl3, 20);
   gtk_label_set_xalign(GTK_LABEL(lbl3), 0.0f);
   conteneur_ajouter(&main_ct, lbl3);

   Menu *menu_clair = g_new0(Menu, 1);
   menu_initialiser(menu_clair);
   g_free(menu_clair->id_css);
   menu_clair->id_css = malloc(strlen("menu_clair") + 1);
   strcpy(menu_clair->id_css, "menu_clair");
   menu_clair->orientation = MENU_HORIZONTAL;
   menu_clair->espacement = 4;
   menu_clair->on_click = on_menu_click;

   g_free(menu_clair->style.bg_barre);
   g_free(menu_clair->style.fg_item);
   g_free(menu_clair->style.bg_item_hover);
   g_free(menu_clair->style.fg_item_hover);
   g_free(menu_clair->style.bg_item_actif);
   g_free(menu_clair->style.couleur_separateur);
   g_free(menu_clair->style.bg_popover);
   g_free(menu_clair->style.couleur_bordure);
   menu_clair->style.bg_barre = malloc(strlen("#ffffff") + 1);
   strcpy(menu_clair->style.bg_barre, "#ffffff");
   menu_clair->style.fg_item = malloc(strlen("#2c3e50") + 1);
   strcpy(menu_clair->style.fg_item, "#2c3e50");
   menu_clair->style.bg_item_hover = malloc(strlen("#f0f2f5") + 1);
   strcpy(menu_clair->style.bg_item_hover, "#f0f2f5");
   menu_clair->style.fg_item_hover = malloc(strlen("#2c3e50") + 1);
   strcpy(menu_clair->style.fg_item_hover, "#2c3e50");
   menu_clair->style.bg_item_actif = malloc(strlen("#3498db") + 1);
   strcpy(menu_clair->style.bg_item_actif, "#3498db");
   menu_clair->style.couleur_separateur = malloc(strlen("#dde1e7") + 1);
   strcpy(menu_clair->style.couleur_separateur, "#dde1e7");
   menu_clair->style.bg_popover = malloc(strlen("#ffffff") + 1);
   strcpy(menu_clair->style.bg_popover, "#ffffff");
   menu_clair->style.couleur_bordure = malloc(strlen("#dde1e7") + 1);
   strcpy(menu_clair->style.couleur_bordure, "#dde1e7");
   menu_clair->style.epaisseur_bordure = 1;

   menu_ajouter_item(menu_clair, menu_item_creer("c_accueil", "Accueil", "go-home-symbolic", MENU_ITEM_NORMAL));
   menu_ajouter_item(menu_clair, menu_item_creer("c_produits", "Produits", "applications-symbolic", MENU_ITEM_NORMAL));

   MenuItem *c_services = menu_item_creer("c_services", "Services", NULL, MENU_ITEM_NORMAL);
   c_services->sous_menu_orientation = MENU_VERTICAL;
   menu_item_ajouter_sous_item(c_services, menu_item_creer("c_svc_web", "Web", "web-browser-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(c_services, menu_item_creer("c_svc_mobile", "Mobile", "phone-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(c_services, menu_item_creer("c_svc_cloud", "Cloud", "cloud-symbolic", MENU_ITEM_NORMAL));
   menu_ajouter_item(menu_clair, c_services);

   menu_ajouter_item(menu_clair, menu_item_separateur());
   menu_ajouter_item(menu_clair, menu_item_creer("c_contact", "Contact", "mail-send-symbolic", MENU_ITEM_NORMAL));
   menu_ajouter_item(menu_clair, menu_item_creer("c_desactive", "Desactive", NULL, MENU_ITEM_DESACTIVE));

   GtkWidget *w_menu_clair = menu_creer(menu_clair);
   gtk_widget_set_margin_start(w_menu_clair, 20);
   gtk_widget_set_margin_end(w_menu_clair, 20);
   menu_set_item_actif(menu_clair, "c_accueil");
   g_signal_connect_swapped(w_menu_clair, "destroy", G_CALLBACK(menu_free), menu_clair);
   conteneur_ajouter(&main_ct, w_menu_clair);

   printf("[OK] Tous les menus crees avec succes.\n");
   printf("[OK] Menu horizontal  : %d items\n", menu_h->nb_items);
   printf("[OK] Menu vertical    : %d items\n", menu_v->nb_items);
   printf("[OK] Menu clair       : %d items\n", menu_clair->nb_items);

   gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
   GtkApplication *app = gtk_application_new("org.zcode.test.menu", G_APPLICATION_DEFAULT_FLAGS);
   g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

   int status = g_application_run(G_APPLICATION(app), argc, argv);
   g_object_unref(app);
   return status;
}
