#include "../headers/dialog.h"
#include <string.h>
#include <stdlib.h>

// ====================== FORWARD DECLARATIONS ======================

static void dialog_apply_css(Dialog *cfg);
static void dialog_construire_footer(Dialog *cfg);

// ====================== HELPERS INTERNES ======================

/**
 * Données passées aux callbacks de boutons
 */
typedef struct
{
   Dialog *dialog;
   int reponse_id;
} DialogClickData;

static void on_bouton_clicked(GtkButton *button, gpointer user_data)
{
   DialogClickData *d = (DialogClickData *)user_data;
   if (!d || !d->dialog)
      return;

   if (d->dialog->on_reponse)
      d->dialog->on_reponse(d->reponse_id, d->dialog->user_data);

   dialog_fermer(d->dialog);
}

static void on_close_request(GtkWindow *window, gpointer user_data)
{
   Dialog *cfg = (Dialog *)user_data;
   if (cfg && cfg->on_reponse)
      cfg->on_reponse(DIALOG_REPONSE_FERMER, cfg->user_data);
}

// ====================== CSS ======================

static void dialog_apply_css(Dialog *cfg)
{
   if (!cfg || !cfg->window || !cfg->id_css)
      return;

   GtkCssProvider *provider = gtk_css_provider_new();
   char css[4096];

   char border_css[128] = "";
   if (cfg->style.epaisseur_bordure > 0)
      snprintf(border_css, sizeof(border_css),
               "border: %dpx solid %s;\n",
               cfg->style.epaisseur_bordure,
               cfg->style.couleur_bordure ? cfg->style.couleur_bordure : "#cccccc");

   char titre_size[64] = "";
   if (cfg->style.taille_titre > 0)
      snprintf(titre_size, sizeof(titre_size),
               "font-size: %dpx;\n", cfg->style.taille_titre);

   snprintf(css, sizeof(css),
            /* Fenêtre */
            "window#%s {\n"
            "  background-color: %s;\n"
            "  border-radius: %dpx;\n"
            "  %s"
            "}\n"

            /* En-tête */
            "window#%s .dialog-header {\n"
            "  background-color: %s;\n"
            "  padding: 16px 20px 12px 20px;\n"
            "  border-radius: %dpx %dpx 0 0;\n"
            "}\n"

            /* Titre dans l'en-tête */
            "window#%s .dialog-header label {\n"
            "  color: %s;\n"
            "  font-weight: %s;\n"
            "  %s"
            "}\n"

            /* Corps */
            "window#%s .dialog-corps {\n"
            "  background-color: %s;\n"
            "  padding: 16px 20px;\n"
            "}\n"

            /* Texte du corps */
            "window#%s .dialog-corps label {\n"
            "  color: %s;\n"
            "}\n"

            /* Pied de page */
            "window#%s .dialog-footer {\n"
            "  background-color: %s;\n"
            "  padding: 10px 16px;\n"
            "  border-radius: 0 0 %dpx %dpx;\n"
            "}\n"

            /* Bouton principal */
            "window#%s .dialog-footer button.principal {\n"
            "  background-image: none;\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "  border: none;\n"
            "  box-shadow: none;\n"
            "  border-radius: 6px;\n"
            "  padding: 8px 20px;\n"
            "  font-weight: bold;\n"
            "}\n"
            "window#%s .dialog-footer button.principal:hover {\n"
            "  opacity: 0.88;\n"
            "}\n"

            /* Bouton secondaire */
            "window#%s .dialog-footer button.secondaire {\n"
            "  background-image: none;\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "  border: 1px solid #cccccc;\n"
            "  box-shadow: none;\n"
            "  border-radius: 6px;\n"
            "  padding: 8px 20px;\n"
            "}\n"
            "window#%s .dialog-footer button.secondaire:hover {\n"
            "  opacity: 0.88;\n"
            "}\n",

            /* window */
            cfg->id_css,
            cfg->style.bg_corps ? cfg->style.bg_corps : "#ffffff",
            cfg->style.rayon_arrondi,
            border_css,

            /* header */
            cfg->id_css,
            cfg->style.bg_header ? cfg->style.bg_header : "#3498db",
            cfg->style.rayon_arrondi, cfg->style.rayon_arrondi,

            /* header label */
            cfg->id_css,
            cfg->style.fg_header ? cfg->style.fg_header : "#ffffff",
            cfg->style.titre_gras ? "bold" : "normal",
            titre_size,

            /* corps */
            cfg->id_css,
            cfg->style.bg_corps ? cfg->style.bg_corps : "#ffffff",

            /* corps label */
            cfg->id_css,
            cfg->style.fg_corps ? cfg->style.fg_corps : "#2c3e50",

            /* footer */
            cfg->id_css,
            cfg->style.bg_footer ? cfg->style.bg_footer : "#f5f6fa",
            cfg->style.rayon_arrondi, cfg->style.rayon_arrondi,

            /* bouton principal */
            cfg->id_css,
            cfg->style.bg_bouton_principal ? cfg->style.bg_bouton_principal : "#3498db",
            cfg->style.fg_bouton_principal ? cfg->style.fg_bouton_principal : "#ffffff",
            cfg->id_css,

            /* bouton secondaire */
            cfg->id_css,
            cfg->style.bg_bouton_secondaire ? cfg->style.bg_bouton_secondaire : "#ecf0f1",
            cfg->style.fg_bouton_secondaire ? cfg->style.fg_bouton_secondaire : "#2c3e50",
            cfg->id_css);

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider_for_display(
       gtk_widget_get_display(cfg->window),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);

   g_object_unref(provider);
}

// ====================== COULEURS PAR TYPE ======================

static void dialog_appliquer_couleurs_type(Dialog *cfg)
{
   g_free(cfg->style.bg_header);
   g_free(cfg->style.bg_bouton_principal);

   switch (cfg->type)
   {
   case DIALOG_SUCCES:
      cfg->style.bg_header = g_strdup("#27ae60");
      cfg->style.bg_bouton_principal = g_strdup("#27ae60");
      break;
   case DIALOG_AVERTISSEMENT:
      cfg->style.bg_header = g_strdup("#e67e22");
      cfg->style.bg_bouton_principal = g_strdup("#e67e22");
      break;
   case DIALOG_ERREUR:
      cfg->style.bg_header = g_strdup("#e74c3c");
      cfg->style.bg_bouton_principal = g_strdup("#e74c3c");
      break;
   case DIALOG_INFO:
   default:
      cfg->style.bg_header = g_strdup("#3498db");
      cfg->style.bg_bouton_principal = g_strdup("#3498db");
      break;
   }
}

// ====================== ICÔNE PAR TYPE ======================

static const char *dialog_icone_par_type(DialogType type)
{
   switch (type)
   {
   case DIALOG_SUCCES:
      return "emblem-ok-symbolic";
   case DIALOG_AVERTISSEMENT:
      return "dialog-warning-symbolic";
   case DIALOG_ERREUR:
      return "dialog-error-symbolic";
   case DIALOG_INFO:
      return "dialog-information-symbolic";
   default:
      return NULL;
   }
}

// ====================== CONSTRUCTION DU FOOTER ======================

static void dialog_construire_footer(Dialog *cfg)
{
   /* Vider le footer existant */
   GtkWidget *child;
   while ((child = gtk_widget_get_first_child(cfg->box_footer)) != NULL)
      gtk_box_remove(GTK_BOX(cfg->box_footer), child);

   /* Choisir les boutons selon le preset ou la liste personnalisée */
   if (cfg->boutons_preset != DIALOG_BOUTONS_PERSONNALISES)
   {
      /* Définitions statiques des presets */
      typedef struct
      {
         const char *texte;
         int id;
         gboolean principal;
      } BtnDef;

      static const BtnDef ok[] = {{"OK", DIALOG_REPONSE_OK, TRUE}};
      static const BtnDef ok_ann[] = {{"Annuler", DIALOG_REPONSE_ANNULER, FALSE},
                                      {"OK", DIALOG_REPONSE_OK, TRUE}};
      static const BtnDef oui_non[] = {{"Non", DIALOG_REPONSE_NON, FALSE},
                                       {"Oui", DIALOG_REPONSE_OUI, TRUE}};
      static const BtnDef oui_non_ann[] = {{"Annuler", DIALOG_REPONSE_ANNULER, FALSE},
                                           {"Non", DIALOG_REPONSE_NON, FALSE},
                                           {"Oui", DIALOG_REPONSE_OUI, TRUE}};

      const BtnDef *defs = NULL;
      int count = 0;

      switch (cfg->boutons_preset)
      {
      case DIALOG_BOUTONS_OK:
         defs = ok;
         count = 1;
         break;
      case DIALOG_BOUTONS_OK_ANNULER:
         defs = ok_ann;
         count = 2;
         break;
      case DIALOG_BOUTONS_OUI_NON:
         defs = oui_non;
         count = 2;
         break;
      case DIALOG_BOUTONS_OUI_NON_ANNULER:
         defs = oui_non_ann;
         count = 3;
         break;
      default:
         break;
      }

      for (int i = 0; i < count; i++)
      {
         GtkWidget *btn = gtk_button_new_with_label(defs[i].texte);
         gtk_widget_add_css_class(btn, defs[i].principal ? "principal" : "secondaire");

         DialogClickData *d = g_new0(DialogClickData, 1);
         d->dialog = cfg;
         d->reponse_id = defs[i].id;
         g_signal_connect(btn, "clicked", G_CALLBACK(on_bouton_clicked), d);
         g_signal_connect_swapped(btn, "destroy", G_CALLBACK(g_free), d);

         gtk_box_append(GTK_BOX(cfg->box_footer), btn);
      }
   }
   else
   {
      /* Boutons personnalisés */
      for (int i = 0; i < cfg->nb_boutons; i++)
      {
         DialogBoutonConfig *bc = cfg->boutons[i];
         GtkWidget *btn;

         if (bc->nom_icone)
         {
            btn = gtk_button_new();
            GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
            gtk_box_append(GTK_BOX(inner), gtk_image_new_from_icon_name(bc->nom_icone));
            gtk_box_append(GTK_BOX(inner), gtk_label_new(bc->texte));
            gtk_button_set_child(GTK_BUTTON(btn), inner);
         }
         else
         {
            btn = gtk_button_new_with_label(bc->texte);
         }

         gtk_widget_add_css_class(btn, bc->principal ? "principal" : "secondaire");

         DialogClickData *d = g_new0(DialogClickData, 1);
         d->dialog = cfg;
         d->reponse_id = bc->reponse_id;
         g_signal_connect(btn, "clicked", G_CALLBACK(on_bouton_clicked), d);
         g_signal_connect_swapped(btn, "destroy", G_CALLBACK(g_free), d);

         gtk_box_append(GTK_BOX(cfg->box_footer), btn);
      }
   }
}

// ====================== FONCTIONS PUBLIQUES ======================

void dialog_initialiser(Dialog *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(Dialog));

   cfg->id_css = g_strdup("dialog");
   cfg->type = DIALOG_INFO;
   cfg->boutons_preset = DIALOG_BOUTONS_OK;
   cfg->modal = TRUE;
   cfg->fermeture_croix = TRUE;

   cfg->taille.width = 420;
   cfg->taille.height = 0; /* hauteur automatique */

   /* Style par défaut */
   cfg->style.bg_header = g_strdup("#3498db");
   cfg->style.fg_header = g_strdup("#ffffff");
   cfg->style.taille_titre = 15;
   cfg->style.titre_gras = TRUE;
   cfg->style.bg_corps = g_strdup("#ffffff");
   cfg->style.fg_corps = g_strdup("#2c3e50");
   cfg->style.bg_footer = g_strdup("#f5f6fa");
   cfg->style.rayon_arrondi = 10;
   cfg->style.epaisseur_bordure = 0;
   cfg->style.couleur_bordure = g_strdup("#cccccc");
   cfg->style.bg_bouton_principal = g_strdup("#3498db");
   cfg->style.fg_bouton_principal = g_strdup("#ffffff");
   cfg->style.bg_bouton_secondaire = g_strdup("#ecf0f1");
   cfg->style.fg_bouton_secondaire = g_strdup("#2c3e50");
}

GtkWidget *dialog_creer(Dialog *cfg)
{
   if (!cfg)
      return NULL;

   /* Appliquer les couleurs selon le type */
   if (cfg->type != DIALOG_PERSONNALISE)
      dialog_appliquer_couleurs_type(cfg);

   /* --- Fenêtre --- */
   cfg->window = gtk_window_new();
   gtk_widget_set_name(cfg->window, cfg->id_css ? cfg->id_css : "dialog");

   if (cfg->parent)
      gtk_window_set_transient_for(GTK_WINDOW(cfg->window), cfg->parent);

   gtk_window_set_modal(GTK_WINDOW(cfg->window), cfg->modal);
   gtk_window_set_resizable(GTK_WINDOW(cfg->window), FALSE);
   gtk_window_set_decorated(GTK_WINDOW(cfg->window), FALSE); /* Pas de barre de titre OS */

   int w = cfg->taille.width > 0 ? cfg->taille.width : 420;
   int h = cfg->taille.height > 0 ? cfg->taille.height : -1;
   gtk_widget_set_size_request(cfg->window, w, h);

   /* Signal de fermeture via la croix (gérée manuellement) */
   g_signal_connect(cfg->window, "close-request", G_CALLBACK(on_close_request), cfg);

   /* --- Boîte racine --- */
   GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   gtk_window_set_child(GTK_WINDOW(cfg->window), root);

   /* --- EN-TÊTE --- */
   cfg->box_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
   gtk_widget_add_css_class(cfg->box_header, "dialog-header");
   gtk_widget_set_hexpand(cfg->box_header, TRUE);

   /* Icône de type */
   const char *icone = dialog_icone_par_type(cfg->type);
   if (icone)
   {
      GtkWidget *img = gtk_image_new_from_icon_name(icone);
      gtk_image_set_pixel_size(GTK_IMAGE(img), 22);
      gtk_box_append(GTK_BOX(cfg->box_header), img);
   }

   /* Titre */
   GtkWidget *lbl_titre = gtk_label_new(cfg->titre ? cfg->titre : "");
   gtk_widget_set_hexpand(lbl_titre, TRUE);
   gtk_label_set_xalign(GTK_LABEL(lbl_titre), 0.0f);
   gtk_label_set_wrap(GTK_LABEL(lbl_titre), TRUE);
   gtk_box_append(GTK_BOX(cfg->box_header), lbl_titre);

   /* Bouton de fermeture dans l'en-tête */
   if (cfg->fermeture_croix)
   {
      GtkWidget *btn_close = gtk_button_new_from_icon_name("window-close-symbolic");
      gtk_widget_add_css_class(btn_close, "secondaire");
      DialogClickData *dc = g_new0(DialogClickData, 1);
      dc->dialog = cfg;
      dc->reponse_id = DIALOG_REPONSE_FERMER;
      g_signal_connect(btn_close, "clicked", G_CALLBACK(on_bouton_clicked), dc);
      g_signal_connect_swapped(btn_close, "destroy", G_CALLBACK(g_free), dc);
      gtk_box_append(GTK_BOX(cfg->box_header), btn_close);
   }

   gtk_box_append(GTK_BOX(root), cfg->box_header);

   /* Séparateur header / corps */
   gtk_box_append(GTK_BOX(root), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

   /* --- CORPS --- */
   cfg->box_corps = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
   gtk_widget_add_css_class(cfg->box_corps, "dialog-corps");
   gtk_widget_set_hexpand(cfg->box_corps, TRUE);

   if (cfg->message)
   {
      GtkWidget *lbl_msg = gtk_label_new(cfg->message);
      gtk_label_set_wrap(GTK_LABEL(lbl_msg), TRUE);
      gtk_label_set_xalign(GTK_LABEL(lbl_msg), 0.0f);
      gtk_widget_set_hexpand(lbl_msg, TRUE);
      gtk_box_append(GTK_BOX(cfg->box_corps), lbl_msg);
   }

   if (cfg->widget_contenu)
   {
      gtk_widget_set_hexpand(cfg->widget_contenu, TRUE);
      gtk_box_append(GTK_BOX(cfg->box_corps), cfg->widget_contenu);
   }

   gtk_box_append(GTK_BOX(root), cfg->box_corps);

   /* Séparateur corps / footer */
   gtk_box_append(GTK_BOX(root), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

   /* --- FOOTER --- */
   cfg->box_footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
   gtk_widget_add_css_class(cfg->box_footer, "dialog-footer");
   gtk_widget_set_halign(cfg->box_footer, GTK_ALIGN_END);

   dialog_construire_footer(cfg);
   gtk_box_append(GTK_BOX(root), cfg->box_footer);

   /* CSS */
   dialog_apply_css(cfg);

   return cfg->window;
}

// ====================== BOUTONS PERSONNALISÉS ======================

void dialog_ajouter_bouton(Dialog *cfg, const char *texte,
                           const char *nom_icone, int reponse_id,
                           gboolean principal)
{
   if (!cfg || !texte)
      return;

   cfg->boutons_preset = DIALOG_BOUTONS_PERSONNALISES;

   DialogBoutonConfig *bc = g_new0(DialogBoutonConfig, 1);
   bc->texte = g_strdup(texte);
   bc->nom_icone = nom_icone ? g_strdup(nom_icone) : NULL;
   bc->reponse_id = reponse_id;
   bc->principal = principal;

   cfg->boutons = g_realloc(cfg->boutons,
                            sizeof(DialogBoutonConfig *) * (cfg->nb_boutons + 1));
   cfg->boutons[cfg->nb_boutons] = bc;
   cfg->nb_boutons++;

   /* Reconstruire le footer si le widget existe déjà */
   if (cfg->box_footer)
      dialog_construire_footer(cfg);
}

// ====================== RACCOURCIS ======================

static void dialog_afficher_simple(GtkWindow *parent, const char *titre,
                                   const char *message, DialogType type,
                                   DialogBoutons boutons,
                                   DialogOnReponse cb, gpointer data)
{
   Dialog *cfg = g_new0(Dialog, 1);
   dialog_initialiser(cfg);

   cfg->parent = parent;
   cfg->titre = g_strdup(titre ? titre : "");
   cfg->message = g_strdup(message ? message : "");
   cfg->type = type;
   cfg->boutons_preset = boutons;
   cfg->on_reponse = cb;
   cfg->user_data = data;

   dialog_creer(cfg);
   dialog_afficher(cfg);

   /* cfg sera libéré à la fermeture via le signal destroy */
   g_signal_connect_swapped(cfg->window, "destroy", G_CALLBACK(dialog_free), cfg);
}

void dialog_afficher_info(GtkWindow *parent, const char *titre,
                          const char *message, DialogOnReponse cb, gpointer data)
{
   dialog_afficher_simple(parent, titre, message, DIALOG_INFO,
                          DIALOG_BOUTONS_OK, cb, data);
}

void dialog_afficher_erreur(GtkWindow *parent, const char *titre,
                            const char *message, DialogOnReponse cb, gpointer data)
{
   dialog_afficher_simple(parent, titre, message, DIALOG_ERREUR,
                          DIALOG_BOUTONS_OK, cb, data);
}

void dialog_afficher_avertissement(GtkWindow *parent, const char *titre,
                                   const char *message, DialogOnReponse cb, gpointer data)
{
   dialog_afficher_simple(parent, titre, message, DIALOG_AVERTISSEMENT,
                          DIALOG_BOUTONS_OK, cb, data);
}

void dialog_afficher_confirmation(GtkWindow *parent, const char *titre,
                                  const char *message, DialogOnReponse cb, gpointer data)
{
   dialog_afficher_simple(parent, titre, message, DIALOG_INFO,
                          DIALOG_BOUTONS_OUI_NON, cb, data);
}

// ====================== SETTERS DYNAMIQUES ======================

void dialog_set_titre(Dialog *cfg, const char *titre)
{
   if (!cfg)
      return;
   g_free(cfg->titre);
   cfg->titre = titre ? g_strdup(titre) : NULL;

   if (cfg->box_header)
   {
      /* Le label du titre est le 2e enfant de box_header (après l'icône ou 1er) */
      GtkWidget *child = gtk_widget_get_first_child(cfg->box_header);
      while (child)
      {
         if (GTK_IS_LABEL(child))
         {
            gtk_label_set_text(GTK_LABEL(child), cfg->titre ? cfg->titre : "");
            break;
         }
         child = gtk_widget_get_next_sibling(child);
      }
   }
}

void dialog_set_message(Dialog *cfg, const char *message)
{
   if (!cfg)
      return;
   g_free(cfg->message);
   cfg->message = message ? g_strdup(message) : NULL;

   if (cfg->box_corps)
   {
      GtkWidget *child = gtk_widget_get_first_child(cfg->box_corps);
      while (child)
      {
         if (GTK_IS_LABEL(child))
         {
            gtk_label_set_text(GTK_LABEL(child), cfg->message ? cfg->message : "");
            return;
         }
         child = gtk_widget_get_next_sibling(child);
      }
      /* Aucun label trouvé : en ajouter un */
      if (cfg->message)
      {
         GtkWidget *lbl = gtk_label_new(cfg->message);
         gtk_label_set_wrap(GTK_LABEL(lbl), TRUE);
         gtk_label_set_xalign(GTK_LABEL(lbl), 0.0f);
         gtk_widget_set_hexpand(lbl, TRUE);
         gtk_box_prepend(GTK_BOX(cfg->box_corps), lbl);
      }
   }
}

void dialog_set_contenu(Dialog *cfg, GtkWidget *widget)
{
   if (!cfg || !widget)
      return;

   /* Retirer l'ancien widget_contenu si présent */
   if (cfg->widget_contenu && cfg->box_corps)
      gtk_box_remove(GTK_BOX(cfg->box_corps), cfg->widget_contenu);

   cfg->widget_contenu = widget;

   if (cfg->box_corps)
   {
      gtk_widget_set_hexpand(widget, TRUE);
      gtk_box_append(GTK_BOX(cfg->box_corps), widget);
   }
}

// ====================== AFFICHAGE / FERMETURE ======================

void dialog_afficher(Dialog *cfg)
{
   if (!cfg || !cfg->window)
      return;
   gtk_window_present(GTK_WINDOW(cfg->window));
}

void dialog_fermer(Dialog *cfg)
{
   if (!cfg || !cfg->window)
      return;
   gtk_window_destroy(GTK_WINDOW(cfg->window));
   cfg->window = NULL;
}

// ====================== LIBÉRATION MÉMOIRE ======================

void dialog_free(Dialog *cfg)
{
   if (!cfg)
      return;

   g_free(cfg->id_css);
   g_free(cfg->titre);
   g_free(cfg->message);

   g_free(cfg->style.bg_header);
   g_free(cfg->style.fg_header);
   g_free(cfg->style.bg_corps);
   g_free(cfg->style.fg_corps);
   g_free(cfg->style.bg_footer);
   g_free(cfg->style.couleur_bordure);
   g_free(cfg->style.bg_bouton_principal);
   g_free(cfg->style.fg_bouton_principal);
   g_free(cfg->style.bg_bouton_secondaire);
   g_free(cfg->style.fg_bouton_secondaire);

   for (int i = 0; i < cfg->nb_boutons; i++)
   {
      g_free(cfg->boutons[i]->texte);
      g_free(cfg->boutons[i]->nom_icone);
      g_free(cfg->boutons[i]);
   }
   g_free(cfg->boutons);

   g_free(cfg);
}
