#include <gtk/gtk.h>
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/slider.h"
#include "../../widgets/headers/texte.h"
#include <stdio.h>

// ====================== CALLBACKS ======================

static void on_volume_changed(GtkRange *range, double valeur, gpointer user_data)
{
   printf("[SLIDER] Volume: %.0f%%\n", valeur);
}

static void on_temperature_changed(GtkRange *range, double valeur, gpointer user_data)
{
   printf("[SLIDER] Température: %.1f°C\n", valeur);
}

static void on_opacite_changed(GtkRange *range, double valeur, gpointer user_data)
{
   printf("[SLIDER] Opacité: %.2f\n", valeur);
}

static void on_zoom_changed(GtkRange *range, double valeur, gpointer user_data)
{
   printf("[SLIDER] Zoom: %.0f%%\n", valeur);
}

static void on_vertical_changed(GtkRange *range, double valeur, gpointer user_data)
{
   printf("[SLIDER] Vertical: %.0f\n", valeur);
}

// ====================== HELPERS ======================

/**
 * Crée une section visuellement délimitée avec un titre et un fond coloré.
 */
static Conteneur creer_section(Conteneur *parent,
                               const char *titre,
                               const char *couleur_fond,
                               const char *couleur_bordure,
                               ConteneurOrientation orientation)
{
   GtkWidget *lbl = gtk_label_new(titre);
   char markup[256];
   snprintf(markup, sizeof(markup), "<b><big>%s</big></b>", titre);
   gtk_label_set_markup(GTK_LABEL(lbl), markup);
   conteneur_ajouter(parent, lbl);

   Conteneur section;
   conteneur_initialiser(&section);
   section.orientation = orientation;
   section.espacement = 20;
   section.couleur_fond = (char *)couleur_fond;
   section.bordure_largeur = 2;
   section.bordure_couleur = (char *)couleur_bordure;
   section.bordure_rayon = 8;
   section.padding.haut = 20;
   section.padding.bas = 20;
   section.padding.gauche = 20;
   section.padding.droite = 20;

   conteneur_creer(&section);
   conteneur_ajouter(parent, section.widget);

   return section;
}

// ====================== ACTIVATE ======================

static void on_activate(GtkApplication *app, gpointer user_data)
{
   // ========== FENETRE ==========
   Fenetre fenetre;
   fenetre_initialiser(&fenetre);
   fenetre.title = "Test Slider — Démo Complète";
   fenetre.taille.width = 800;
   fenetre.taille.height = 750;
   fenetre.scroll_mode = SCROLL_VERTICAL;

   GtkWidget *window = fenetre_creer(&fenetre);
   gtk_application_add_window(app, GTK_WINDOW(window));

   // ========== CONTENEUR PRINCIPAL ==========
   Conteneur main_ct;
   conteneur_initialiser(&main_ct);
   main_ct.orientation = CONTENEUR_VERTICAL;
   main_ct.espacement = 25;
   main_ct.padding.haut = 20;
   main_ct.padding.bas = 20;
   main_ct.padding.gauche = 25;
   main_ct.padding.droite = 25;
   main_ct.couleur_fond = "#f5f6fa";

   GtkWidget *main_box = conteneur_creer(&main_ct);

   if (fenetre.scroll_mode != SCROLL_NONE && fenetre.scroll_widget)
      gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fenetre.scroll_widget), main_box);
   else
      gtk_window_set_child(GTK_WINDOW(window), main_box);

   // =====================================================================
   // SECTION 1 — Slider de base (Volume)
   // =====================================================================
   Conteneur sec1 = creer_section(&main_ct, "SLIDER DE BASE — Volume",
                                  "#E3F2FD", "#2196F3", CONTENEUR_VERTICAL);

   GtkWidget *lbl_vol = gtk_label_new("Volume (0 – 100, pas 1, valeur initiale 50)");
   conteneur_ajouter(&sec1, lbl_vol);

   Slider *sl_volume = g_new0(Slider, 1);
   slider_initialiser(sl_volume);
   sl_volume->id_css = "slider_volume";
   sl_volume->min = 0.0;
   sl_volume->max = 100.0;
   sl_volume->step = 1.0;
   sl_volume->valeur = 50.0;
   sl_volume->digits = 0;
   sl_volume->afficher_valeur = TRUE;
   sl_volume->afficher_label = TRUE;
   sl_volume->on_change = on_volume_changed;

   GtkWidget *w_vol = slider_creer(sl_volume);
   g_signal_connect_swapped(w_vol, "destroy", G_CALLBACK(slider_free), sl_volume);
   g_signal_connect(w_vol, "destroy", G_CALLBACK(g_free), sl_volume);
   conteneur_ajouter(&sec1, w_vol);

   // =====================================================================
   // SECTION 2 — Slider avec décimales (Température)
   // =====================================================================
   Conteneur sec2 = creer_section(&main_ct, "SLIDER DÉCIMALES — Température",
                                  "#FFF3E0", "#FF9800", CONTENEUR_VERTICAL);

   GtkWidget *lbl_temp = gtk_label_new("Température (−20 – 50 °C, pas 0.5, 1 décimale)");
   conteneur_ajouter(&sec2, lbl_temp);

   Slider *sl_temp = g_new0(Slider, 1);
   slider_initialiser(sl_temp);
   sl_temp->id_css = "slider_temperature";
   sl_temp->min = -20.0;
   sl_temp->max = 50.0;
   sl_temp->step = 0.5;
   sl_temp->valeur = 20.0;
   sl_temp->digits = 1;
   sl_temp->afficher_valeur = TRUE;
   sl_temp->afficher_label = TRUE;
   sl_temp->style.bg_normal = g_strdup("#FF9800");
   sl_temp->style.fg_normal = g_strdup("#e65100");
   sl_temp->style.couleur_bordure = g_strdup("#ffe0b2");
   sl_temp->on_change = on_temperature_changed;

   GtkWidget *w_temp = slider_creer(sl_temp);
   g_signal_connect_swapped(w_temp, "destroy", G_CALLBACK(slider_free), sl_temp);
   g_signal_connect(w_temp, "destroy", G_CALLBACK(g_free), sl_temp);
   conteneur_ajouter(&sec2, w_temp);

   // =====================================================================
   // SECTION 3 — Slider avec marques (Zoom)
   // =====================================================================
   Conteneur sec3 = creer_section(&main_ct, "SLIDER AVEC MARQUES — Zoom",
                                  "#F3E5F5", "#9C27B0", CONTENEUR_VERTICAL);

   GtkWidget *lbl_zoom = gtk_label_new("Zoom (50 – 200%, marques tous les 25%)");
   conteneur_ajouter(&sec3, lbl_zoom);

   Slider *sl_zoom = g_new0(Slider, 1);
   slider_initialiser(sl_zoom);
   sl_zoom->id_css = "slider_zoom";
   sl_zoom->min = 50.0;
   sl_zoom->max = 200.0;
   sl_zoom->step = 5.0;
   sl_zoom->valeur = 100.0;
   sl_zoom->digits = 0;
   sl_zoom->afficher_valeur = TRUE;
   sl_zoom->afficher_label = TRUE;
   sl_zoom->marques_pos = SLIDER_MARQUES_DESSOUS;
   sl_zoom->marques_step = 25.0;
   sl_zoom->style.bg_normal = g_strdup("#9C27B0");
   sl_zoom->style.fg_normal = g_strdup("#6A1B9A");
   sl_zoom->style.couleur_bordure = g_strdup("#e1bee7");
   sl_zoom->on_change = on_zoom_changed;

   GtkWidget *w_zoom = slider_creer(sl_zoom);
   g_signal_connect_swapped(w_zoom, "destroy", G_CALLBACK(slider_free), sl_zoom);
   g_signal_connect(w_zoom, "destroy", G_CALLBACK(g_free), sl_zoom);
   conteneur_ajouter(&sec3, w_zoom);

   // =====================================================================
   // SECTION 4 — Slider opacité (0.0 – 1.0) + taille fixe
   // =====================================================================
   Conteneur sec4 = creer_section(&main_ct, "SLIDER TAILLE FIXE — Opacité",
                                  "#E8F5E9", "#4CAF50", CONTENEUR_VERTICAL);

   GtkWidget *lbl_opa = gtk_label_new("Opacité (0.00 – 1.00, largeur fixe 300px, 2 décimales)");
   conteneur_ajouter(&sec4, lbl_opa);

   Slider *sl_opa = g_new0(Slider, 1);
   slider_initialiser(sl_opa);
   sl_opa->id_css = "slider_opacite";
   sl_opa->min = 0.0;
   sl_opa->max = 1.0;
   sl_opa->step = 0.01;
   sl_opa->valeur = 1.0;
   sl_opa->digits = 2;
   sl_opa->size.width = 300;
   sl_opa->afficher_valeur = TRUE;
   sl_opa->afficher_label = TRUE;
   sl_opa->style.bg_normal = g_strdup("#4CAF50");
   sl_opa->style.fg_normal = g_strdup("#2E7D32");
   sl_opa->style.couleur_bordure = g_strdup("#c8e6c9");
   sl_opa->on_change = on_opacite_changed;

   GtkWidget *w_opa = slider_creer(sl_opa);
   g_signal_connect_swapped(w_opa, "destroy", G_CALLBACK(slider_free), sl_opa);
   g_signal_connect(w_opa, "destroy", G_CALLBACK(g_free), sl_opa);
   conteneur_ajouter(&sec4, w_opa);

   // =====================================================================
   // SECTION 5 — Slider vertical + slider inversé + slider désactivé
   // =====================================================================
   Conteneur sec5 = creer_section(&main_ct, "VERTICAL — INVERSÉ — DÉSACTIVÉ",
                                  "#FCE4EC", "#E91E63", CONTENEUR_HORIZONTAL);

   /* --- Vertical --- */
   Conteneur col_vert;
   conteneur_initialiser(&col_vert);
   col_vert.orientation = CONTENEUR_VERTICAL;
   col_vert.espacement = 6;
   conteneur_creer(&col_vert);
   conteneur_ajouter(&sec5, col_vert.widget);

   GtkWidget *lbl_vert = gtk_label_new("Vertical");
   conteneur_ajouter(&col_vert, lbl_vert);

   Slider *sl_vert = g_new0(Slider, 1);
   slider_initialiser(sl_vert);
   sl_vert->id_css = "slider_vertical";
   sl_vert->orientation = SLIDER_VERTICAL;
   sl_vert->min = 0.0;
   sl_vert->max = 100.0;
   sl_vert->step = 1.0;
   sl_vert->valeur = 40.0;
   sl_vert->size.height = 150;
   sl_vert->afficher_valeur = TRUE;
   sl_vert->afficher_label = FALSE;
   sl_vert->style.bg_normal = g_strdup("#E91E63");
   sl_vert->style.fg_normal = g_strdup("#880E4F");
   sl_vert->style.couleur_bordure = g_strdup("#f8bbd0");
   sl_vert->on_change = on_vertical_changed;

   GtkWidget *w_vert = slider_creer(sl_vert);
   g_signal_connect_swapped(w_vert, "destroy", G_CALLBACK(slider_free), sl_vert);
   g_signal_connect(w_vert, "destroy", G_CALLBACK(g_free), sl_vert);
   conteneur_ajouter(&col_vert, w_vert);

   /* --- Inversé --- */
   Conteneur col_inv;
   conteneur_initialiser(&col_inv);
   col_inv.orientation = CONTENEUR_VERTICAL;
   col_inv.espacement = 6;
   conteneur_creer(&col_inv);
   conteneur_ajouter(&sec5, col_inv.widget);

   GtkWidget *lbl_inv = gtk_label_new("Inversé");
   conteneur_ajouter(&col_inv, lbl_inv);

   Slider *sl_inv = g_new0(Slider, 1);
   slider_initialiser(sl_inv);
   sl_inv->id_css = "slider_inverse";
   sl_inv->min = 0.0;
   sl_inv->max = 100.0;
   sl_inv->step = 1.0;
   sl_inv->valeur = 70.0;
   sl_inv->inverser = TRUE;
   sl_inv->afficher_valeur = TRUE;
   sl_inv->afficher_label = TRUE;
   sl_inv->size.width = 180;
   sl_inv->style.bg_normal = g_strdup("#FF5722");
   sl_inv->style.fg_normal = g_strdup("#BF360C");
   sl_inv->style.couleur_bordure = g_strdup("#ffccbc");

   GtkWidget *w_inv = slider_creer(sl_inv);
   g_signal_connect_swapped(w_inv, "destroy", G_CALLBACK(slider_free), sl_inv);
   g_signal_connect(w_inv, "destroy", G_CALLBACK(g_free), sl_inv);
   conteneur_ajouter(&col_inv, w_inv);

   /* --- Désactivé --- */
   Conteneur col_dis;
   conteneur_initialiser(&col_dis);
   col_dis.orientation = CONTENEUR_VERTICAL;
   col_dis.espacement = 6;
   conteneur_creer(&col_dis);
   conteneur_ajouter(&sec5, col_dis.widget);

   GtkWidget *lbl_dis = gtk_label_new("Désactivé");
   conteneur_ajouter(&col_dis, lbl_dis);

   Slider *sl_dis = g_new0(Slider, 1);
   slider_initialiser(sl_dis);
   sl_dis->id_css = "slider_disabled";
   sl_dis->min = 0.0;
   sl_dis->max = 100.0;
   sl_dis->valeur = 30.0;
   sl_dis->sensitive = FALSE;
   sl_dis->afficher_valeur = TRUE;
   sl_dis->afficher_label = TRUE;
   sl_dis->size.width = 180;

   GtkWidget *w_dis = slider_creer(sl_dis);
   g_signal_connect_swapped(w_dis, "destroy", G_CALLBACK(slider_free), sl_dis);
   g_signal_connect(w_dis, "destroy", G_CALLBACK(g_free), sl_dis);
   conteneur_ajouter(&col_dis, w_dis);

   // =====================================================================
   // Pied de page
   // =====================================================================
   GtkWidget *footer = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(footer),
                        "<i>Déplacez les curseurs pour tester les callbacks et les styles.</i>");
   conteneur_ajouter(&main_ct, footer);

   // ========== AFFICHAGE ==========
   printf("[OK] Tous les sliders créés avec succès.\n");
   gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
   GtkApplication *app = gtk_application_new("org.zcode.test.slider", G_APPLICATION_DEFAULT_FLAGS);
   g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

   int status = g_application_run(G_APPLICATION(app), argc, argv);
   g_object_unref(app);
   return status;
}
