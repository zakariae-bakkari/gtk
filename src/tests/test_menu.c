#include <gtk/gtk.h>
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/menu.h"
#include <stdio.h>

// ====================== CALLBACK UNIQUE ======================

static void on_menu_click(const char *id, gpointer user_data)
{
   printf("[MENU] Item cliqué : '%s'\n", id);
}

// ====================== ACTIVATE ======================

static void on_activate(GtkApplication *app, gpointer user_data)
{
   // ========== FENETRE ==========
   Fenetre fenetre;
   fenetre_initialiser(&fenetre);
   fenetre.app = app;
   fenetre.title = "Test Menu — Démo Complète";
   fenetre.taille.width = 900;
   fenetre.taille.height = 700;
   fenetre.scroll_mode = SCROLL_VERTICAL;

   GtkWidget *window = fenetre_creer(&fenetre);
   gtk_application_add_window(app, GTK_WINDOW(window));

   // ========== CONTENEUR PRINCIPAL ==========
   Conteneur main_ct;
   conteneur_initialiser(&main_ct);
   main_ct.orientation = CONTENEUR_VERTICAL;
   main_ct.espacement = 30;
   main_ct.padding.haut = 0;
   main_ct.padding.bas = 20;
   main_ct.padding.gauche = 0;
   main_ct.padding.droite = 0;
   main_ct.couleur_fond = "#f0f2f5";

   GtkWidget *main_box = conteneur_creer(&main_ct);

   if (fenetre.scroll_mode != SCROLL_NONE && fenetre.scroll_widget)
      gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fenetre.scroll_widget), main_box);
   else
      gtk_window_set_child(GTK_WINDOW(window), main_box);

   // =====================================================================
   // SECTION 1 — Menu HORIZONTAL (barre de navigation) avec sous-menus VERTICAUX
   // =====================================================================
   GtkWidget *lbl1 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl1), "<b><big>MENU HORIZONTAL — sous-menus verticaux</big></b>");
   gtk_widget_set_margin_top(lbl1, 20);
   gtk_widget_set_margin_start(lbl1, 20);
   gtk_label_set_xalign(GTK_LABEL(lbl1), 0.0f);
   conteneur_ajouter(&main_ct, lbl1);

   Menu menu_h;
   menu_initialiser(&menu_h);
   menu_h.id_css = "menu_principal";
   menu_h.orientation = MENU_HORIZONTAL;
   menu_h.espacement = 4;
   menu_h.on_click = on_menu_click;

   /* Item 1 : Accueil (simple, actif par défaut) */
   MenuItem *i_accueil = menu_item_creer("accueil", "Accueil",
                                         "go-home-symbolic", MENU_ITEM_NORMAL);
   i_accueil->tooltip = g_strdup("Retour à l'accueil");
   menu_ajouter_item(&menu_h, i_accueil);

   /* Item 2 : Fichier → sous-menu vertical */
   MenuItem *i_fichier = menu_item_creer("fichier", "Fichier",
                                         "document-open-symbolic", MENU_ITEM_NORMAL);
   i_fichier->sous_menu_orientation = MENU_VERTICAL;

   MenuItem *si_nouveau = menu_item_creer("fichier_nouveau", "Nouveau", "document-new-symbolic", MENU_ITEM_NORMAL);
   MenuItem *si_ouvrir = menu_item_creer("fichier_ouvrir", "Ouvrir", "document-open-symbolic", MENU_ITEM_NORMAL);
   MenuItem *si_sep1 = menu_item_separateur();
   MenuItem *si_sauv = menu_item_creer("fichier_sauv", "Sauvegarder", "document-save-symbolic", MENU_ITEM_NORMAL);
   MenuItem *si_sauv_as = menu_item_creer("fichier_sauv_as", "Sauvegarder sous…", "document-save-as-symbolic", MENU_ITEM_NORMAL);
   MenuItem *si_sep2 = menu_item_separateur();
   MenuItem *si_quitter = menu_item_creer("fichier_quitter", "Quitter", "application-exit-symbolic", MENU_ITEM_NORMAL);

   menu_item_ajouter_sous_item(i_fichier, si_nouveau);
   menu_item_ajouter_sous_item(i_fichier, si_ouvrir);
   menu_item_ajouter_sous_item(i_fichier, si_sep1);
   menu_item_ajouter_sous_item(i_fichier, si_sauv);
   menu_item_ajouter_sous_item(i_fichier, si_sauv_as);
   menu_item_ajouter_sous_item(i_fichier, si_sep2);
   menu_item_ajouter_sous_item(i_fichier, si_quitter);
   menu_ajouter_item(&menu_h, i_fichier);

   /* Item 3 : Edition → sous-menu vertical */
   MenuItem *i_edition = menu_item_creer("edition", "Édition",
                                         "edit-symbolic", MENU_ITEM_NORMAL);
   i_edition->sous_menu_orientation = MENU_VERTICAL;

   MenuItem *si_couper = menu_item_creer("edition_couper", "Couper", "edit-cut-symbolic", MENU_ITEM_NORMAL);
   MenuItem *si_copier = menu_item_creer("edition_copier", "Copier", "edit-copy-symbolic", MENU_ITEM_NORMAL);
   MenuItem *si_coller = menu_item_creer("edition_coller", "Coller", "edit-paste-symbolic", MENU_ITEM_NORMAL);
   MenuItem *si_sep3 = menu_item_separateur();
   MenuItem *si_annuler = menu_item_creer("edition_annuler", "Annuler", "edit-undo-symbolic", MENU_ITEM_DESACTIVE);
   MenuItem *si_retablir = menu_item_creer("edition_retablir", "Rétablir", "edit-redo-symbolic", MENU_ITEM_DESACTIVE);

   menu_item_ajouter_sous_item(i_edition, si_couper);
   menu_item_ajouter_sous_item(i_edition, si_copier);
   menu_item_ajouter_sous_item(i_edition, si_coller);
   menu_item_ajouter_sous_item(i_edition, si_sep3);
   menu_item_ajouter_sous_item(i_edition, si_annuler);
   menu_item_ajouter_sous_item(i_edition, si_retablir);
   menu_ajouter_item(&menu_h, i_edition);

   /* Item 4 : Affichage → sous-menu HORIZONTAL (cas mixte) */
   MenuItem *i_affichage = menu_item_creer("affichage", "Affichage",
                                           "video-display-symbolic", MENU_ITEM_NORMAL);
   i_affichage->sous_menu_orientation = MENU_HORIZONTAL;

   MenuItem *si_zoom_in = menu_item_creer("zoom_in", "Zoom +", "zoom-in-symbolic", MENU_ITEM_NORMAL);
   MenuItem *si_zoom_out = menu_item_creer("zoom_out", "Zoom −", "zoom-out-symbolic", MENU_ITEM_NORMAL);
   MenuItem *si_zoom_fit = menu_item_creer("zoom_fit", "Ajuster", "zoom-fit-best-symbolic", MENU_ITEM_NORMAL);

   menu_item_ajouter_sous_item(i_affichage, si_zoom_in);
   menu_item_ajouter_sous_item(i_affichage, si_zoom_out);
   menu_item_ajouter_sous_item(i_affichage, si_zoom_fit);
   menu_ajouter_item(&menu_h, i_affichage);

   /* Séparateur dans la barre principale */
   menu_ajouter_item(&menu_h, menu_item_separateur());

   /* Item 5 : À propos (désactivé) */
   MenuItem *i_aide = menu_item_creer("aide", "Aide",
                                      "help-about-symbolic", MENU_ITEM_NORMAL);
   menu_ajouter_item(&menu_h, i_aide);

   GtkWidget *w_menu_h = menu_creer(&menu_h);
   menu_set_item_actif(&menu_h, "accueil");
   conteneur_ajouter(&main_ct, w_menu_h);

   // =====================================================================
   // SECTION 2 — Démo côte à côte : menu VERTICAL + contenu
   // =====================================================================
   GtkWidget *lbl2 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl2), "<b><big>MENU VERTICAL — sous-menus horizontaux et verticaux</big></b>");
   gtk_widget_set_margin_start(lbl2, 20);
   gtk_label_set_xalign(GTK_LABEL(lbl2), 0.0f);
   conteneur_ajouter(&main_ct, lbl2);

   /* Conteneur horizontal : sidebar + zone de contenu */
   Conteneur row;
   conteneur_initialiser(&row);
   row.orientation = CONTENEUR_HORIZONTAL;
   row.espacement = 0;
   row.padding.gauche = 20;
   row.padding.droite = 20;
   GtkWidget *row_box = conteneur_creer(&row);
   conteneur_ajouter(&main_ct, row_box);

   /* --- Menu vertical (sidebar) --- */
   Menu menu_v;
   menu_initialiser(&menu_v);
   menu_v.id_css = "menu_sidebar";
   menu_v.orientation = MENU_VERTICAL;
   menu_v.espacement = 2;
   menu_v.size.width = 200;
   menu_v.on_click = on_menu_click;

   /* Couleurs claires pour contraster */
   g_free(menu_v.style.bg_barre);
   g_free(menu_v.style.bg_item_hover);
   g_free(menu_v.style.bg_item_actif);
   menu_v.style.bg_barre = g_strdup("#1a252f");
   menu_v.style.bg_item_hover = g_strdup("#2c3e50");
   menu_v.style.bg_item_actif = g_strdup("#e74c3c");

   MenuItem *v_tableau = menu_item_creer("tableau_bord", "Tableau de bord",
                                         "view-grid-symbolic", MENU_ITEM_NORMAL);
   MenuItem *v_utilisateurs = menu_item_creer("utilisateurs", "Utilisateurs",
                                              "system-users-symbolic", MENU_ITEM_NORMAL);
   /* Sous-menu vertical des utilisateurs */
   v_utilisateurs->sous_menu_orientation = MENU_VERTICAL;
   menu_item_ajouter_sous_item(v_utilisateurs,
                               menu_item_creer("util_liste", "Liste", "view-list-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(v_utilisateurs,
                               menu_item_creer("util_ajouter", "Ajouter", "list-add-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(v_utilisateurs,
                               menu_item_creer("util_groupes", "Groupes", "folder-symbolic", MENU_ITEM_NORMAL));

   MenuItem *v_rapports = menu_item_creer("rapports", "Rapports",
                                          "x-office-spreadsheet-symbolic", MENU_ITEM_NORMAL);
   /* Sous-menu HORIZONTAL des rapports */
   v_rapports->sous_menu_orientation = MENU_HORIZONTAL;
   menu_item_ajouter_sous_item(v_rapports,
                               menu_item_creer("rap_jour", "Jour", NULL, MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(v_rapports,
                               menu_item_creer("rap_semaine", "Semaine", NULL, MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(v_rapports,
                               menu_item_creer("rap_mois", "Mois", NULL, MENU_ITEM_NORMAL));

   MenuItem *v_sep = menu_item_separateur();

   MenuItem *v_parametres = menu_item_creer("parametres", "Paramètres",
                                            "preferences-system-symbolic", MENU_ITEM_NORMAL);
   MenuItem *v_deconnexion = menu_item_creer("deconnexion", "Déconnexion",
                                             "system-log-out-symbolic", MENU_ITEM_NORMAL);

   menu_ajouter_item(&menu_v, v_tableau);
   menu_ajouter_item(&menu_v, v_utilisateurs);
   menu_ajouter_item(&menu_v, v_rapports);
   menu_ajouter_item(&menu_v, v_sep);
   menu_ajouter_item(&menu_v, v_parametres);
   menu_ajouter_item(&menu_v, v_deconnexion);

   GtkWidget *w_menu_v = menu_creer(&menu_v);
   menu_set_item_actif(&menu_v, "tableau_bord");
   gtk_box_append(GTK_BOX(row_box), w_menu_v);

   /* --- Zone de contenu à droite --- */
   Conteneur zone;
   conteneur_initialiser(&zone);
   zone.orientation = CONTENEUR_VERTICAL;
   zone.espacement = 10;
   zone.padding.haut = 20;
   zone.padding.gauche = 20;
   zone.couleur_fond = "#ffffff";
   zone.bordure_largeur = 1;
   zone.bordure_couleur = "#dde1e7";
   zone.bordure_rayon = 8;
   GtkWidget *zone_box = conteneur_creer(&zone);
   gtk_widget_set_hexpand(zone_box, TRUE);

   GtkWidget *zone_titre = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(zone_titre),
                        "<b>Zone de contenu</b>\n"
                        "<small>Cliquez sur les items du menu pour voir les callbacks dans la console.</small>");
   gtk_label_set_xalign(GTK_LABEL(zone_titre), 0.0f);
   gtk_label_set_wrap(GTK_LABEL(zone_titre), TRUE);
   conteneur_ajouter(&zone, zone_titre);

   gtk_box_append(GTK_BOX(row_box), zone_box);

   // =====================================================================
   // SECTION 3 — Menu horizontal style clair
   // =====================================================================
   GtkWidget *lbl3 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl3), "<b><big>MENU HORIZONTAL — thème clair</big></b>");
   gtk_widget_set_margin_start(lbl3, 20);
   gtk_label_set_xalign(GTK_LABEL(lbl3), 0.0f);
   conteneur_ajouter(&main_ct, lbl3);

   Menu menu_clair;
   menu_initialiser(&menu_clair);
   menu_clair.id_css = "menu_clair";
   menu_clair.orientation = MENU_HORIZONTAL;
   menu_clair.espacement = 4;
   menu_clair.on_click = on_menu_click;

   /* Thème clair */
   g_free(menu_clair.style.bg_barre);
   g_free(menu_clair.style.fg_item);
   g_free(menu_clair.style.bg_item_hover);
   g_free(menu_clair.style.fg_item_hover);
   g_free(menu_clair.style.bg_item_actif);
   g_free(menu_clair.style.couleur_separateur);
   g_free(menu_clair.style.bg_popover);
   menu_clair.style.bg_barre = g_strdup("#ffffff");
   menu_clair.style.fg_item = g_strdup("#2c3e50");
   menu_clair.style.bg_item_hover = g_strdup("#f0f2f5");
   menu_clair.style.fg_item_hover = g_strdup("#2c3e50");
   menu_clair.style.bg_item_actif = g_strdup("#3498db");
   menu_clair.style.couleur_separateur = g_strdup("#dde1e7");
   menu_clair.style.bg_popover = g_strdup("#ffffff");
   menu_clair.style.epaisseur_bordure = 1;
   g_free(menu_clair.style.couleur_bordure);
   menu_clair.style.couleur_bordure = g_strdup("#dde1e7");

   menu_ajouter_item(&menu_clair, menu_item_creer("c_accueil", "Accueil", "go-home-symbolic", MENU_ITEM_NORMAL));
   menu_ajouter_item(&menu_clair, menu_item_creer("c_produits", "Produits", "applications-symbolic", MENU_ITEM_NORMAL));

   MenuItem *c_services = menu_item_creer("c_services", "Services", NULL, MENU_ITEM_NORMAL);
   c_services->sous_menu_orientation = MENU_VERTICAL;
   menu_item_ajouter_sous_item(c_services, menu_item_creer("c_svc_web", "Web", "web-browser-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(c_services, menu_item_creer("c_svc_mobile", "Mobile", "phone-symbolic", MENU_ITEM_NORMAL));
   menu_item_ajouter_sous_item(c_services, menu_item_creer("c_svc_cloud", "Cloud", "cloud-symbolic", MENU_ITEM_NORMAL));
   menu_ajouter_item(&menu_clair, c_services);

   menu_ajouter_item(&menu_clair, menu_item_separateur());
   menu_ajouter_item(&menu_clair, menu_item_creer("c_contact", "Contact", "mail-send-symbolic", MENU_ITEM_NORMAL));
   menu_ajouter_item(&menu_clair, menu_item_creer("c_desactive", "Désactivé", NULL, MENU_ITEM_DESACTIVE));

   GtkWidget *w_menu_clair = menu_creer(&menu_clair);
   gtk_widget_set_margin_start(w_menu_clair, 20);
   gtk_widget_set_margin_end(w_menu_clair, 20);
   menu_set_item_actif(&menu_clair, "c_accueil");
   conteneur_ajouter(&main_ct, w_menu_clair);

   // ========== AFFICHAGE ==========
   printf("[OK] Tous les menus créés avec succès.\n");
   printf("[OK] Menu horizontal  : %d items\n", menu_h.nb_items);
   printf("[OK] Menu vertical    : %d items\n", menu_v.nb_items);
   printf("[OK] Menu clair       : %d items\n", menu_clair.nb_items);

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
