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

   Menu *menu_h = g_new0(Menu, 1);
   menu_initialiser(menu_h);
   menu_h->id_css = "menu_principal";
   menu_h->orientation = MENU_HORIZONTAL;
   menu_h->espacement = 4;
   menu_h->on_click = on_menu_click;

   /* Item 1 : Accueil (simple, actif par défaut) */
   MenuItem *i_accueil = g_new0(MenuItem, 1);
   i_accueil->id = "accueil";
   i_accueil->texte = "Accueil";
   i_accueil->nom_icone = "go-home-symbolic";
   i_accueil->type = MENU_ITEM_NORMAL;
   i_accueil->tooltip = g_strdup("Retour à l'accueil");
   menu_ajouter_item(menu_h, i_accueil);

   /* Item 2 : Fichier → sous-menu vertical */
   MenuItem *i_fichier = g_new0(MenuItem, 1);
   i_fichier->id = "fichier";
   i_fichier->texte = "Fichier";
   i_fichier->nom_icone = "document-open-symbolic";
   i_fichier->type = MENU_ITEM_NORMAL;
   i_fichier->sous_menu_orientation = MENU_VERTICAL;

   MenuItem *si_nouveau = g_new0(MenuItem, 1);
   si_nouveau->id = "fichier_nouveau";
   si_nouveau->texte = "Nouveau";
   si_nouveau->nom_icone = "document-new-symbolic";
   si_nouveau->type = MENU_ITEM_NORMAL;

   MenuItem *si_ouvrir = g_new0(MenuItem, 1);
   si_ouvrir->id = "fichier_ouvrir";
   si_ouvrir->texte = "Ouvrir";
   si_ouvrir->nom_icone = "document-open-symbolic";
   si_ouvrir->type = MENU_ITEM_NORMAL;

   MenuItem *si_sep1 = g_new0(MenuItem, 1);
   si_sep1->type = MENU_ITEM_SEPARATEUR;

   MenuItem *si_sauv = g_new0(MenuItem, 1);
   si_sauv->id = "fichier_sauv";
   si_sauv->texte = "Sauvegarder";
   si_sauv->nom_icone = "document-save-symbolic";
   si_sauv->type = MENU_ITEM_NORMAL;

   MenuItem *si_sauv_as = g_new0(MenuItem, 1);
   si_sauv_as->id = "fichier_sauv_as";
   si_sauv_as->texte = "Sauvegarder sous…";
   si_sauv_as->nom_icone = "document-save-as-symbolic";
   si_sauv_as->type = MENU_ITEM_NORMAL;

   MenuItem *si_sep2 = g_new0(MenuItem, 1);
   si_sep2->type = MENU_ITEM_SEPARATEUR;

   MenuItem *si_quitter = g_new0(MenuItem, 1);
   si_quitter->id = "fichier_quitter";
   si_quitter->texte = "Quitter";
   si_quitter->nom_icone = "application-exit-symbolic";
   si_quitter->type = MENU_ITEM_NORMAL;

   menu_item_ajouter_sous_item(i_fichier, si_nouveau);
   menu_item_ajouter_sous_item(i_fichier, si_ouvrir);
   menu_item_ajouter_sous_item(i_fichier, si_sep1);
   menu_item_ajouter_sous_item(i_fichier, si_sauv);
   menu_item_ajouter_sous_item(i_fichier, si_sauv_as);
   menu_item_ajouter_sous_item(i_fichier, si_sep2);
   menu_item_ajouter_sous_item(i_fichier, si_quitter);
   menu_ajouter_item(menu_h, i_fichier);

   /* Item 3 : Edition → sous-menu vertical */
   MenuItem *i_edition = g_new0(MenuItem, 1);
   i_edition->id = "edition";
   i_edition->texte = "Édition";
   i_edition->nom_icone = "edit-symbolic";
   i_edition->type = MENU_ITEM_NORMAL;
   i_edition->sous_menu_orientation = MENU_VERTICAL;

   MenuItem *si_couper = g_new0(MenuItem, 1);
   si_couper->id = "edition_couper";
   si_couper->texte = "Couper";
   si_couper->nom_icone = "edit-cut-symbolic";
   si_couper->type = MENU_ITEM_NORMAL;

   MenuItem *si_copier = g_new0(MenuItem, 1);
   si_copier->id = "edition_copier";
   si_copier->texte = "Copier";
   si_copier->nom_icone = "edit-copy-symbolic";
   si_copier->type = MENU_ITEM_NORMAL;

   MenuItem *si_coller = g_new0(MenuItem, 1);
   si_coller->id = "edition_coller";
   si_coller->texte = "Coller";
   si_coller->nom_icone = "edit-paste-symbolic";
   si_coller->type = MENU_ITEM_NORMAL;

   MenuItem *si_sep3 = g_new0(MenuItem, 1);
   si_sep3->type = MENU_ITEM_SEPARATEUR;

   MenuItem *si_annuler = g_new0(MenuItem, 1);
   si_annuler->id = "edition_annuler";
   si_annuler->texte = "Annuler";
   si_annuler->nom_icone = "edit-undo-symbolic";
   si_annuler->type = MENU_ITEM_DESACTIVE;

   MenuItem *si_retablir = g_new0(MenuItem, 1);
   si_retablir->id = "edition_retablir";
   si_retablir->texte = "Rétablir";
   si_retablir->nom_icone = "edit-redo-symbolic";
   si_retablir->type = MENU_ITEM_DESACTIVE;

   menu_item_ajouter_sous_item(i_edition, si_couper);
   menu_item_ajouter_sous_item(i_edition, si_copier);
   menu_item_ajouter_sous_item(i_edition, si_coller);
   menu_item_ajouter_sous_item(i_edition, si_sep3);
   menu_item_ajouter_sous_item(i_edition, si_annuler);
   menu_item_ajouter_sous_item(i_edition, si_retablir);
   menu_ajouter_item(menu_h, i_edition);

   /* Item 4 : Affichage → sous-menu HORIZONTAL (cas mixte) */
   MenuItem *i_affichage = g_new0(MenuItem, 1);
   i_affichage->id = "affichage";
   i_affichage->texte = "Affichage";
   i_affichage->nom_icone = "video-display-symbolic";
   i_affichage->type = MENU_ITEM_NORMAL;
   i_affichage->sous_menu_orientation = MENU_HORIZONTAL;

   MenuItem *si_zoom_in = g_new0(MenuItem, 1);
   si_zoom_in->id = "zoom_in";
   si_zoom_in->texte = "Zoom +";
   si_zoom_in->nom_icone = "zoom-in-symbolic";
   si_zoom_in->type = MENU_ITEM_NORMAL;

   MenuItem *si_zoom_out = g_new0(MenuItem, 1);
   si_zoom_out->id = "zoom_out";
   si_zoom_out->texte = "Zoom −";
   si_zoom_out->nom_icone = "zoom-out-symbolic";
   si_zoom_out->type = MENU_ITEM_NORMAL;

   MenuItem *si_zoom_fit = g_new0(MenuItem, 1);
   si_zoom_fit->id = "zoom_fit";
   si_zoom_fit->texte = "Ajuster";
   si_zoom_fit->nom_icone = "zoom-fit-best-symbolic";
   si_zoom_fit->type = MENU_ITEM_NORMAL;

   menu_item_ajouter_sous_item(i_affichage, si_zoom_in);
   menu_item_ajouter_sous_item(i_affichage, si_zoom_out);
   menu_item_ajouter_sous_item(i_affichage, si_zoom_fit);
   menu_ajouter_item(menu_h, i_affichage);

   /* Séparateur dans la barre principale */
   MenuItem *sep_h = g_new0(MenuItem, 1);
   sep_h->type = MENU_ITEM_SEPARATEUR;
   menu_ajouter_item(menu_h, sep_h);

   /* Item 5 : À propos (désactivé) */
   MenuItem *i_aide = g_new0(MenuItem, 1);
   i_aide->id = "aide";
   i_aide->texte = "Aide";
   i_aide->nom_icone = "help-about-symbolic";
   i_aide->type = MENU_ITEM_NORMAL;
   menu_ajouter_item(menu_h, i_aide);

   GtkWidget *w_menu_h = menu_creer(menu_h);
   menu_set_item_actif(menu_h, "accueil");
   g_signal_connect_swapped(w_menu_h, "destroy", G_CALLBACK(menu_free), menu_h);
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
   Menu *menu_v = g_new0(Menu, 1);
   menu_initialiser(menu_v);
   menu_v->id_css = "menu_sidebar";
   menu_v->orientation = MENU_VERTICAL;
   menu_v->espacement = 2;
   menu_v->size.width = 200;
   menu_v->on_click = on_menu_click;

   /* Couleurs claires pour contraster */
   g_free(menu_v->style.bg_barre);
   g_free(menu_v->style.bg_item_hover);
   g_free(menu_v->style.bg_item_actif);
   menu_v->style.bg_barre = g_strdup("#1a252f");
   menu_v->style.bg_item_hover = g_strdup("#2c3e50");
   menu_v->style.bg_item_actif = g_strdup("#e74c3c");

   MenuItem *v_tableau = g_new0(MenuItem, 1);
   v_tableau->id = "tableau_bord";
   v_tableau->texte = "Tableau de bord";
   v_tableau->nom_icone = "view-grid-symbolic";
   v_tableau->type = MENU_ITEM_NORMAL;

   MenuItem *v_utilisateurs = g_new0(MenuItem, 1);
   v_utilisateurs->id = "utilisateurs";
   v_utilisateurs->texte = "Utilisateurs";
   v_utilisateurs->nom_icone = "system-users-symbolic";
   v_utilisateurs->type = MENU_ITEM_NORMAL;
   v_utilisateurs->sous_menu_orientation = MENU_VERTICAL;

   MenuItem *v_util_liste = g_new0(MenuItem, 1);
   v_util_liste->id = "util_liste";
   v_util_liste->texte = "Liste";
   v_util_liste->nom_icone = "view-list-symbolic";
   v_util_liste->type = MENU_ITEM_NORMAL;

   MenuItem *v_util_ajouter = g_new0(MenuItem, 1);
   v_util_ajouter->id = "util_ajouter";
   v_util_ajouter->texte = "Ajouter";
   v_util_ajouter->nom_icone = "list-add-symbolic";
   v_util_ajouter->type = MENU_ITEM_NORMAL;

   MenuItem *v_util_groupes = g_new0(MenuItem, 1);
   v_util_groupes->id = "util_groupes";
   v_util_groupes->texte = "Groupes";
   v_util_groupes->nom_icone = "folder-symbolic";
   v_util_groupes->type = MENU_ITEM_NORMAL;

   menu_item_ajouter_sous_item(v_utilisateurs, v_util_liste);
   menu_item_ajouter_sous_item(v_utilisateurs, v_util_ajouter);
   menu_item_ajouter_sous_item(v_utilisateurs, v_util_groupes);

   MenuItem *v_rapports = g_new0(MenuItem, 1);
   v_rapports->id = "rapports";
   v_rapports->texte = "Rapports";
   v_rapports->nom_icone = "x-office-spreadsheet-symbolic";
   v_rapports->type = MENU_ITEM_NORMAL;
   v_rapports->sous_menu_orientation = MENU_HORIZONTAL;

   MenuItem *v_rap_jour = g_new0(MenuItem, 1);
   v_rap_jour->id = "rap_jour";
   v_rap_jour->texte = "Jour";
   v_rap_jour->type = MENU_ITEM_NORMAL;

   MenuItem *v_rap_semaine = g_new0(MenuItem, 1);
   v_rap_semaine->id = "rap_semaine";
   v_rap_semaine->texte = "Semaine";
   v_rap_semaine->type = MENU_ITEM_NORMAL;

   MenuItem *v_rap_mois = g_new0(MenuItem, 1);
   v_rap_mois->id = "rap_mois";
   v_rap_mois->texte = "Mois";
   v_rap_mois->type = MENU_ITEM_NORMAL;

   menu_item_ajouter_sous_item(v_rapports, v_rap_jour);
   menu_item_ajouter_sous_item(v_rapports, v_rap_semaine);
   menu_item_ajouter_sous_item(v_rapports, v_rap_mois);

   MenuItem *v_sep = g_new0(MenuItem, 1);
   v_sep->type = MENU_ITEM_SEPARATEUR;

   MenuItem *v_parametres = g_new0(MenuItem, 1);
   v_parametres->id = "parametres";
   v_parametres->texte = "Paramètres";
   v_parametres->nom_icone = "preferences-system-symbolic";
   v_parametres->type = MENU_ITEM_NORMAL;

   MenuItem *v_deconnexion = g_new0(MenuItem, 1);
   v_deconnexion->id = "deconnexion";
   v_deconnexion->texte = "Déconnexion";
   v_deconnexion->nom_icone = "system-log-out-symbolic";
   v_deconnexion->type = MENU_ITEM_NORMAL;

   menu_ajouter_item(menu_v, v_tableau);
   menu_ajouter_item(menu_v, v_utilisateurs);
   menu_ajouter_item(menu_v, v_rapports);
   menu_ajouter_item(menu_v, v_sep);
   menu_ajouter_item(menu_v, v_parametres);
   menu_ajouter_item(menu_v, v_deconnexion);

   GtkWidget *w_menu_v = menu_creer(menu_v);
   menu_set_item_actif(menu_v, "tableau_bord");
   g_signal_connect_swapped(w_menu_v, "destroy", G_CALLBACK(menu_free), menu_v);
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

   Menu *menu_clair = g_new0(Menu, 1);
   menu_initialiser(menu_clair);
   menu_clair->id_css = "menu_clair";
   menu_clair->orientation = MENU_HORIZONTAL;
   menu_clair->espacement = 4;
   menu_clair->on_click = on_menu_click;

   /* Thème clair */
   g_free(menu_clair->style.bg_barre);
   g_free(menu_clair->style.fg_item);
   g_free(menu_clair->style.bg_item_hover);
   g_free(menu_clair->style.fg_item_hover);
   g_free(menu_clair->style.bg_item_actif);
   g_free(menu_clair->style.couleur_separateur);
   g_free(menu_clair->style.bg_popover);
   menu_clair->style.bg_barre = g_strdup("#ffffff");
   menu_clair->style.fg_item = g_strdup("#2c3e50");
   menu_clair->style.bg_item_hover = g_strdup("#f0f2f5");
   menu_clair->style.fg_item_hover = g_strdup("#2c3e50");
   menu_clair->style.bg_item_actif = g_strdup("#3498db");
   menu_clair->style.couleur_separateur = g_strdup("#dde1e7");
   menu_clair->style.bg_popover = g_strdup("#ffffff");
   menu_clair->style.epaisseur_bordure = 1;
   g_free(menu_clair->style.couleur_bordure);
   menu_clair->style.couleur_bordure = g_strdup("#dde1e7");

   MenuItem *c_accueil = g_new0(MenuItem, 1);
   c_accueil->id = "c_accueil";
   c_accueil->texte = "Accueil";
   c_accueil->nom_icone = "go-home-symbolic";
   c_accueil->type = MENU_ITEM_NORMAL;
   menu_ajouter_item(menu_clair, c_accueil);

   MenuItem *c_produits = g_new0(MenuItem, 1);
   c_produits->id = "c_produits";
   c_produits->texte = "Produits";
   c_produits->nom_icone = "applications-symbolic";
   c_produits->type = MENU_ITEM_NORMAL;
   menu_ajouter_item(menu_clair, c_produits);

   MenuItem *c_services = g_new0(MenuItem, 1);
   c_services->id = "c_services";
   c_services->texte = "Services";
   c_services->type = MENU_ITEM_NORMAL;
   c_services->sous_menu_orientation = MENU_VERTICAL;

   MenuItem *c_svc_web = g_new0(MenuItem, 1);
   c_svc_web->id = "c_svc_web";
   c_svc_web->texte = "Web";
   c_svc_web->nom_icone = "web-browser-symbolic";
   c_svc_web->type = MENU_ITEM_NORMAL;

   MenuItem *c_svc_mobile = g_new0(MenuItem, 1);
   c_svc_mobile->id = "c_svc_mobile";
   c_svc_mobile->texte = "Mobile";
   c_svc_mobile->nom_icone = "phone-symbolic";
   c_svc_mobile->type = MENU_ITEM_NORMAL;

   MenuItem *c_svc_cloud = g_new0(MenuItem, 1);
   c_svc_cloud->id = "c_svc_cloud";
   c_svc_cloud->texte = "Cloud";
   c_svc_cloud->nom_icone = "cloud-symbolic";
   c_svc_cloud->type = MENU_ITEM_NORMAL;

   menu_item_ajouter_sous_item(c_services, c_svc_web);
   menu_item_ajouter_sous_item(c_services, c_svc_mobile);
   menu_item_ajouter_sous_item(c_services, c_svc_cloud);
   menu_ajouter_item(menu_clair, c_services);

   MenuItem *sep_clair = g_new0(MenuItem, 1);
   sep_clair->type = MENU_ITEM_SEPARATEUR;
   menu_ajouter_item(menu_clair, sep_clair);

   MenuItem *c_contact = g_new0(MenuItem, 1);
   c_contact->id = "c_contact";
   c_contact->texte = "Contact";
   c_contact->nom_icone = "mail-send-symbolic";
   c_contact->type = MENU_ITEM_NORMAL;
   menu_ajouter_item(menu_clair, c_contact);

   MenuItem *c_desactive = g_new0(MenuItem, 1);
   c_desactive->id = "c_desactive";
   c_desactive->texte = "Désactivé";
   c_desactive->type = MENU_ITEM_DESACTIVE;
   menu_ajouter_item(menu_clair, c_desactive);

   GtkWidget *w_menu_clair = menu_creer(menu_clair);
   gtk_widget_set_margin_start(w_menu_clair, 20);
   gtk_widget_set_margin_end(w_menu_clair, 20);
   menu_set_item_actif(menu_clair, "c_accueil");
   g_signal_connect_swapped(w_menu_clair, "destroy", G_CALLBACK(menu_free), menu_clair);
   conteneur_ajouter(&main_ct, w_menu_clair);

   // ========== AFFICHAGE ==========
   printf("[OK] Tous les menus créés avec succès.\n");
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
