#include <gtk/gtk.h>
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/dialog.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ====================== STRUCTURES DE DONNÉES POUR CALLBACKS ======================

typedef struct
{
   GtkWindow *win;
   DialogType type;
   const char *titre;
   const char *msg;
} BtnTypeData;
typedef struct
{
   GtkWindow *win;
   DialogBoutons preset;
   const char *titre;
} BtnPresetData;

// ====================== FORWARD DECLARATIONS ======================

static void on_reponse(int reponse, gpointer user_data);
static void show_typed_cb(GtkButton *btn, gpointer data);
static void show_preset_cb(GtkButton *btn, gpointer data);
static void show_custom_cb(GtkButton *btn, gpointer data);
static void show_contenu_cb(GtkButton *btn, gpointer data);
static void show_theme_cb(GtkButton *btn, gpointer data);

// ====================== CALLBACK RÉPONSE ======================

static void on_reponse(int reponse, gpointer user_data)
{
   const char *label = (const char *)user_data;
   char r_str[16];
   strcpy(r_str, "INCONNUE");
   switch (reponse)
   {
   case DIALOG_REPONSE_OK:
      strcpy(r_str, "OK");
      break;
   case DIALOG_REPONSE_ANNULER:
      strcpy(r_str, "ANNULER");
      break;
   case DIALOG_REPONSE_OUI:
      strcpy(r_str, "OUI");
      break;
   case DIALOG_REPONSE_NON:
      strcpy(r_str, "NON");
      break;
   case DIALOG_REPONSE_FERMER:
      strcpy(r_str, "FERMER");
      break;
   default:
      break;
   }
   printf("[DIALOG] '%s' → réponse : %s (%d)\n", label ? label : "?", r_str, reponse);
}

// ====================== CALLBACKS STATIQUES ======================

static void show_typed_cb(GtkButton *btn, gpointer data)
{
   BtnTypeData *info = (BtnTypeData *)data;
   Dialog *cfg = g_new0(Dialog, 1);
   dialog_initialiser(cfg);
   cfg->parent = info->win;
   cfg->type = info->type;
    cfg->titre = malloc(strlen(info->titre) + 1);
    strcpy(cfg->titre, info->titre);
    cfg->message = malloc(strlen(info->msg) + 1);
    strcpy(cfg->message, info->msg);
   cfg->boutons_preset = DIALOG_BOUTONS_OK;
   cfg->on_reponse = on_reponse;
   cfg->user_data = (gpointer)info->titre;
   dialog_creer(cfg);
   dialog_afficher(cfg);
   g_signal_connect_swapped(cfg->window, "destroy", G_CALLBACK(dialog_free), cfg);
}

static void show_preset_cb(GtkButton *btn, gpointer data)
{
   BtnPresetData *info = (BtnPresetData *)data;
   Dialog *cfg = g_new0(Dialog, 1);
   dialog_initialiser(cfg);
   cfg->parent = info->win;
   cfg->type = DIALOG_INFO;
    cfg->titre = malloc(strlen(info->titre) + 1);
    strcpy(cfg->titre, info->titre);
    cfg->message = malloc(strlen("Choisissez une réponse parmi les boutons ci-dessous.\n"
                                 "Le résultat s'affichera dans la console.") + 1);
    strcpy(cfg->message, "Choisissez une réponse parmi les boutons ci-dessous.\n"
                         "Le résultat s'affichera dans la console.");
   cfg->boutons_preset = info->preset;
   cfg->on_reponse = on_reponse;
   cfg->user_data = (gpointer)info->titre;
   dialog_creer(cfg);
   dialog_afficher(cfg);
   g_signal_connect_swapped(cfg->window, "destroy", G_CALLBACK(dialog_free), cfg);
}

static void show_custom_cb(GtkButton *btn, gpointer data)
{
   GtkWindow *parent = (GtkWindow *)data;
   Dialog *cfg = g_new0(Dialog, 1);
   dialog_initialiser(cfg);
   cfg->parent = parent;
   cfg->type = DIALOG_AVERTISSEMENT;
    cfg->titre = malloc(strlen("Supprimer le fichier ?") + 1);
    strcpy(cfg->titre, "Supprimer le fichier ?");
    cfg->message = malloc(strlen("Cette action supprimera définitivement le fichier sélectionné.\n"
                                 "Voulez-vous le déplacer dans la corbeille ou le supprimer ?") + 1);
    strcpy(cfg->message, "Cette action supprimera définitivement le fichier sélectionné.\n"
                         "Voulez-vous le déplacer dans la corbeille ou le supprimer ?");
   cfg->on_reponse = on_reponse;
   cfg->user_data = malloc(strlen("custom") + 1);
   strcpy(cfg->user_data, "custom");

   dialog_ajouter_bouton(cfg, "Annuler", "process-stop-symbolic", DIALOG_REPONSE_ANNULER, FALSE);
   dialog_ajouter_bouton(cfg, "Corbeille", "user-trash-symbolic", 10, FALSE);
   dialog_ajouter_bouton(cfg, "Supprimer", "edit-delete-symbolic", DIALOG_REPONSE_OK, TRUE);

   dialog_creer(cfg);
   dialog_afficher(cfg);
   g_signal_connect_swapped(cfg->window, "destroy", G_CALLBACK(dialog_free), cfg);
}

static void show_contenu_cb(GtkButton *btn, gpointer data)
{
   GtkWindow *parent = (GtkWindow *)data;
   Dialog *cfg = g_new0(Dialog, 1);
   dialog_initialiser(cfg);
   cfg->parent = parent;
   cfg->type = DIALOG_PERSONNALISE;
    cfg->titre = malloc(strlen("Renommer le fichier") + 1);
    strcpy(cfg->titre, "Renommer le fichier");
    cfg->message = malloc(strlen("Entrez le nouveau nom du fichier :") + 1);
    strcpy(cfg->message, "Entrez le nouveau nom du fichier :");
   cfg->boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
   cfg->on_reponse = on_reponse;
   cfg->user_data = malloc(strlen("contenu") + 1);
   strcpy(cfg->user_data, "contenu");

   /* Widget personnalisé : champ de saisie */
   GtkWidget *entry = gtk_entry_new();
   gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "nouveau_nom.txt");
   gtk_editable_set_text(GTK_EDITABLE(entry), "document.txt");
   cfg->widget_contenu = entry;

   /* Couleur indigo */
   g_free(cfg->style.bg_header);
   g_free(cfg->style.bg_bouton_principal);
    cfg->style.bg_header = malloc(strlen("#5c6bc0") + 1);
    strcpy(cfg->style.bg_header, "#5c6bc0");
    cfg->style.bg_bouton_principal = malloc(strlen("#5c6bc0") + 1);
    strcpy(cfg->style.bg_bouton_principal, "#5c6bc0");

   dialog_creer(cfg);
   dialog_afficher(cfg);
   g_signal_connect_swapped(cfg->window, "destroy", G_CALLBACK(dialog_free), cfg);
}

static void show_theme_cb(GtkButton *btn, gpointer data)
{
   GtkWindow *parent = (GtkWindow *)data;
   Dialog *cfg = g_new0(Dialog, 1);
   dialog_initialiser(cfg);
   cfg->parent = parent;
   cfg->type = DIALOG_PERSONNALISE;
    cfg->titre = malloc(strlen("Thème sombre") + 1);
    strcpy(cfg->titre, "Thème sombre");
    cfg->message = malloc(strlen("Voici un dialog avec un thème entièrement personnalisé.\n"
                                 "Toutes les couleurs sont définies manuellement.") + 1);
    strcpy(cfg->message, "Voici un dialog avec un thème entièrement personnalisé.\n"
                         "Toutes les couleurs sont définies manuellement.");
   cfg->boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
   cfg->on_reponse = on_reponse;
   cfg->user_data = malloc(strlen("theme") + 1);
   strcpy(cfg->user_data, "theme");
   cfg->taille.width = 480;

   /* Thème sombre */
   g_free(cfg->style.bg_header);
   g_free(cfg->style.fg_header);
   g_free(cfg->style.bg_corps);
   g_free(cfg->style.fg_corps);
   g_free(cfg->style.bg_footer);
   g_free(cfg->style.bg_bouton_principal);
   g_free(cfg->style.fg_bouton_principal);
   g_free(cfg->style.bg_bouton_secondaire);
   g_free(cfg->style.fg_bouton_secondaire);
   g_free(cfg->style.couleur_bordure);

    cfg->style.bg_header = malloc(strlen("#1a1a2e") + 1);
    strcpy(cfg->style.bg_header, "#1a1a2e");
    cfg->style.fg_header = malloc(strlen("#e94560") + 1);
    strcpy(cfg->style.fg_header, "#e94560");
    cfg->style.bg_corps = malloc(strlen("#16213e") + 1);
    strcpy(cfg->style.bg_corps, "#16213e");
    cfg->style.fg_corps = malloc(strlen("#a8b2d8") + 1);
    strcpy(cfg->style.fg_corps, "#a8b2d8");
    cfg->style.bg_footer = malloc(strlen("#0f3460") + 1);
    strcpy(cfg->style.bg_footer, "#0f3460");
    cfg->style.bg_bouton_principal = malloc(strlen("#e94560") + 1);
    strcpy(cfg->style.bg_bouton_principal, "#e94560");
    cfg->style.fg_bouton_principal = malloc(strlen("#ffffff") + 1);
    strcpy(cfg->style.fg_bouton_principal, "#ffffff");
    cfg->style.bg_bouton_secondaire = malloc(strlen("#1a1a2e") + 1);
    strcpy(cfg->style.bg_bouton_secondaire, "#1a1a2e");
    cfg->style.fg_bouton_secondaire = malloc(strlen("#a8b2d8") + 1);
    strcpy(cfg->style.fg_bouton_secondaire, "#a8b2d8");
   cfg->style.rayon_arrondi = 12;
   cfg->style.epaisseur_bordure = 1;
    cfg->style.couleur_bordure = malloc(strlen("#e94560") + 1);
    strcpy(cfg->style.couleur_bordure, "#e94560");

   dialog_creer(cfg);
   dialog_afficher(cfg);
   g_signal_connect_swapped(cfg->window, "destroy", G_CALLBACK(dialog_free), cfg);
}

// ====================== ACTIVATE ======================

static void on_activate(GtkApplication *app, gpointer user_data)
{
   // ========== FENETRE ==========
   Fenetre fenetre;
   fenetre_initialiser(&fenetre);
   fenetre.title = malloc(strlen("Test Dialog — Démo Complète") + 1);
   strcpy(fenetre.title, "Test Dialog — Démo Complète");
   fenetre.taille.width = 860;
   fenetre.taille.height = 620;
   fenetre.scroll_mode = SCROLL_VERTICAL;

   GtkWidget *window = fenetre_creer(&fenetre);
   gtk_application_add_window(app, GTK_WINDOW(window));

   // ========== CONTENEUR PRINCIPAL ==========
   Conteneur main_ct;
   conteneur_initialiser(&main_ct);
   main_ct.orientation = CONTENEUR_VERTICAL;
   main_ct.espacement = 24;
   main_ct.padding.haut = 24;
   main_ct.padding.bas = 24;
   main_ct.padding.gauche = 32;
   main_ct.padding.droite = 32;
   main_ct.couleur_fond = malloc(strlen("#f0f2f5") + 1);
   strcpy(main_ct.couleur_fond, "#f0f2f5");

   GtkWidget *main_box = conteneur_creer(&main_ct);

   if (fenetre.scroll_mode != SCROLL_NONE && fenetre.scroll_widget)
      gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fenetre.scroll_widget), main_box);
   else
      gtk_window_set_child(GTK_WINDOW(window), main_box);

   // =====================================================================
   // SECTION 1 — Types de dialog (Info, Succès, Avertissement, Erreur)
   // =====================================================================
   GtkWidget *lbl1 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl1), "<b><big>TYPES DE DIALOG</big></b>");
   gtk_label_set_xalign(GTK_LABEL(lbl1), 0.0f);
   conteneur_ajouter(&main_ct, lbl1);

   Conteneur row1;
   conteneur_initialiser(&row1);
   row1.orientation = CONTENEUR_HORIZONTAL;
   row1.espacement = 12;
   GtkWidget *row1_box = conteneur_creer(&row1);
   conteneur_ajouter(&main_ct, row1_box);

   /* Helper macro pour créer un bouton icône+texte */
#define BTN_ICONE(icone, texte) ({ \
   GtkWidget *_btn = gtk_button_new(); \
   GtkWidget *_inner = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6); \
   gtk_box_append(GTK_BOX(_inner), gtk_image_new_from_icon_name(icone)); \
   gtk_box_append(GTK_BOX(_inner), gtk_label_new(texte)); \
   gtk_button_set_child(GTK_BUTTON(_btn), _inner); \
   _btn; })

   GtkWidget *w_info = BTN_ICONE("dialog-information-symbolic", "Info");
   GtkWidget *w_succes = BTN_ICONE("emblem-ok-symbolic", "Succès");
   GtkWidget *w_avert = BTN_ICONE("dialog-warning-symbolic", "Avertissement");
   GtkWidget *w_erreur = BTN_ICONE("dialog-error-symbolic", "Erreur");

   gtk_box_append(GTK_BOX(row1_box), w_info);
   gtk_box_append(GTK_BOX(row1_box), w_succes);
   gtk_box_append(GTK_BOX(row1_box), w_avert);
   gtk_box_append(GTK_BOX(row1_box), w_erreur);

   /* Données pour chaque bouton */
   BtnTypeData *d_info = g_new0(BtnTypeData, 1);
   d_info->win = GTK_WINDOW(window);
   d_info->type = DIALOG_INFO;
   d_info->titre = malloc(strlen("Information") + 1);
   strcpy(d_info->titre, "Information");
   d_info->msg = malloc(strlen("Opération effectuée avec succès.\nVous pouvez continuer.") + 1);
   strcpy(d_info->msg, "Opération effectuée avec succès.\nVous pouvez continuer.");

   BtnTypeData *d_succes = g_new0(BtnTypeData, 1);
   d_succes->win = GTK_WINDOW(window);
   d_succes->type = DIALOG_SUCCES;
   d_succes->titre = malloc(strlen("Succès !") + 1);
   strcpy(d_succes->titre, "Succès !");
   d_succes->msg = malloc(strlen("Le fichier a été sauvegardé correctement.") + 1);
   strcpy(d_succes->msg, "Le fichier a été sauvegardé correctement.");

   BtnTypeData *d_avert = g_new0(BtnTypeData, 1);
   d_avert->win = GTK_WINDOW(window);
   d_avert->type = DIALOG_AVERTISSEMENT;
   d_avert->titre = malloc(strlen("Attention") + 1);
   strcpy(d_avert->titre, "Attention");
   d_avert->msg = malloc(strlen("Cette action est irréversible.\nÊtes-vous sûr de vouloir continuer ?") + 1);
   strcpy(d_avert->msg, "Cette action est irréversible.\nÊtes-vous sûr de vouloir continuer ?");

   BtnTypeData *d_erreur = g_new0(BtnTypeData, 1);
   d_erreur->win = GTK_WINDOW(window);
   d_erreur->type = DIALOG_ERREUR;
   d_erreur->titre = malloc(strlen("Erreur critique") + 1);
   strcpy(d_erreur->titre, "Erreur critique");
   d_erreur->msg = malloc(strlen("Impossible d'ouvrir le fichier.\nVérifiez les permissions d'accès.") + 1);
   strcpy(d_erreur->msg, "Impossible d'ouvrir le fichier.\nVérifiez les permissions d'accès.");

   g_signal_connect(w_info, "clicked", G_CALLBACK(show_typed_cb), d_info);
   g_signal_connect(w_succes, "clicked", G_CALLBACK(show_typed_cb), d_succes);
   g_signal_connect(w_avert, "clicked", G_CALLBACK(show_typed_cb), d_avert);
   g_signal_connect(w_erreur, "clicked", G_CALLBACK(show_typed_cb), d_erreur);

   // =====================================================================
   // SECTION 2 — Presets de boutons
   // =====================================================================
   GtkWidget *lbl2 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl2), "<b><big>PRESETS DE BOUTONS</big></b>");
   gtk_label_set_xalign(GTK_LABEL(lbl2), 0.0f);
   conteneur_ajouter(&main_ct, lbl2);

   Conteneur row2;
   conteneur_initialiser(&row2);
   row2.orientation = CONTENEUR_HORIZONTAL;
   row2.espacement = 12;
   GtkWidget *row2_box = conteneur_creer(&row2);
   conteneur_ajouter(&main_ct, row2_box);

   GtkWidget *b_ok = gtk_button_new_with_label("OK seulement");
   GtkWidget *b_ok_ann = gtk_button_new_with_label("OK + Annuler");
   GtkWidget *b_oui_non = gtk_button_new_with_label("Oui / Non");
   GtkWidget *b_oui_non_ann = gtk_button_new_with_label("Oui / Non / Annuler");

   gtk_box_append(GTK_BOX(row2_box), b_ok);
   gtk_box_append(GTK_BOX(row2_box), b_ok_ann);
   gtk_box_append(GTK_BOX(row2_box), b_oui_non);
   gtk_box_append(GTK_BOX(row2_box), b_oui_non_ann);

   BtnPresetData *p_ok = g_new0(BtnPresetData, 1);
   p_ok->win = GTK_WINDOW(window);
   p_ok->preset = DIALOG_BOUTONS_OK;
   p_ok->titre = malloc(strlen("OK seulement") + 1);
   strcpy(p_ok->titre, "OK seulement");

   BtnPresetData *p_ok_ann = g_new0(BtnPresetData, 1);
   p_ok_ann->win = GTK_WINDOW(window);
   p_ok_ann->preset = DIALOG_BOUTONS_OK_ANNULER;
   p_ok_ann->titre = malloc(strlen("OK + Annuler") + 1);
   strcpy(p_ok_ann->titre, "OK + Annuler");

   BtnPresetData *p_oui_non = g_new0(BtnPresetData, 1);
   p_oui_non->win = GTK_WINDOW(window);
   p_oui_non->preset = DIALOG_BOUTONS_OUI_NON;
   p_oui_non->titre = malloc(strlen("Oui / Non") + 1);
   strcpy(p_oui_non->titre, "Oui / Non");

   BtnPresetData *p_oui_non_ann = g_new0(BtnPresetData, 1);
   p_oui_non_ann->win = GTK_WINDOW(window);
   p_oui_non_ann->preset = DIALOG_BOUTONS_OUI_NON_ANNULER;
   p_oui_non_ann->titre = malloc(strlen("Oui / Non / Annuler") + 1);
   strcpy(p_oui_non_ann->titre, "Oui / Non / Annuler");

   g_signal_connect(b_ok, "clicked", G_CALLBACK(show_preset_cb), p_ok);
   g_signal_connect(b_ok_ann, "clicked", G_CALLBACK(show_preset_cb), p_ok_ann);
   g_signal_connect(b_oui_non, "clicked", G_CALLBACK(show_preset_cb), p_oui_non);
   g_signal_connect(b_oui_non_ann, "clicked", G_CALLBACK(show_preset_cb), p_oui_non_ann);

   // =====================================================================
   // SECTION 3 — Boutons personnalisés
   // =====================================================================
   GtkWidget *lbl3 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl3), "<b><big>BOUTONS PERSONNALISÉS</big></b>");
   gtk_label_set_xalign(GTK_LABEL(lbl3), 0.0f);
   conteneur_ajouter(&main_ct, lbl3);

   GtkWidget *b_custom = gtk_button_new_with_label("Dialog personnalisé (3 boutons avec icônes)");
   gtk_widget_set_halign(b_custom, GTK_ALIGN_START);
   conteneur_ajouter(&main_ct, b_custom);
   g_signal_connect(b_custom, "clicked", G_CALLBACK(show_custom_cb), GTK_WINDOW(window));

   // =====================================================================
   // SECTION 4 — Contenu personnalisé (widget dans le corps)
   // =====================================================================
   GtkWidget *lbl4 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl4), "<b><big>CONTENU PERSONNALISÉ (widget dans le corps)</big></b>");
   gtk_label_set_xalign(GTK_LABEL(lbl4), 0.0f);
   conteneur_ajouter(&main_ct, lbl4);

   GtkWidget *b_contenu = gtk_button_new_with_label("Dialog avec champ de saisie");
   gtk_widget_set_halign(b_contenu, GTK_ALIGN_START);
   conteneur_ajouter(&main_ct, b_contenu);
   g_signal_connect(b_contenu, "clicked", G_CALLBACK(show_contenu_cb), GTK_WINDOW(window));

   // =====================================================================
   // SECTION 5 — Thème personnalisé
   // =====================================================================
   GtkWidget *lbl5 = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl5), "<b><big>THÈME PERSONNALISÉ</big></b>");
   gtk_label_set_xalign(GTK_LABEL(lbl5), 0.0f);
   conteneur_ajouter(&main_ct, lbl5);

   GtkWidget *b_theme = gtk_button_new_with_label("Dialog thème sombre");
   gtk_widget_set_halign(b_theme, GTK_ALIGN_START);
   conteneur_ajouter(&main_ct, b_theme);
   g_signal_connect(b_theme, "clicked", G_CALLBACK(show_theme_cb), GTK_WINDOW(window));

   // ========== AFFICHAGE ==========
   printf("[OK] Interface de test dialog créée.\n");
   printf("[OK] Cliquez sur les boutons pour ouvrir les différents types de dialogs.\n");

   gtk_window_present(GTK_WINDOW(window));
}

// ====================== MAIN ======================

int main(int argc, char **argv)
{
   GtkApplication *app = gtk_application_new("org.zcode.test.dialog", G_APPLICATION_DEFAULT_FLAGS);
   g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

   int status = g_application_run(G_APPLICATION(app), argc, argv);
   g_object_unref(app);
   return status;
}
