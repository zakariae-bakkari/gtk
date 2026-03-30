#include "../headers/champ_texte.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ====================== HELPERS ERREUR ======================

/**
 * Affiche le label d'erreur avec le message donné et passe l'entry en état error.
 */
static void champ_texte_show_error(ChampTexte *cfg, const char *message)
{
   gtk_widget_add_css_class(cfg->widget, "error");

   if (cfg->show_error_label)
   {
      gtk_label_set_text(GTK_LABEL(cfg->label_erreur), message);
      gtk_widget_set_visible(cfg->label_erreur, TRUE);
   }

   if (cfg->on_invalid)
      cfg->on_invalid(cfg->widget, message, cfg->user_data);
}

/**
 * Masque le label d'erreur et retire la classe CSS error de l'entry.
 */
static void champ_texte_clear_error(ChampTexte *cfg)
{
   gtk_widget_remove_css_class(cfg->widget, "error");

   if (cfg->show_error_label)
   {
      gtk_label_set_text(GTK_LABEL(cfg->label_erreur), "");
      gtk_widget_set_visible(cfg->label_erreur, FALSE);
   }
}

// ====================== CSS ======================

static void champ_texte_apply_css(ChampTexte *cfg)
{
   if (!cfg || !cfg->widget || !cfg->id_css)
      return;

   GtkCssProvider *provider = gtk_css_provider_new();
   char css[2048];
   char border_css[128] = "";
   char font_css[256] = "";

   if (cfg->style.epaisseur_bordure > 0)
   {
      snprintf(border_css, sizeof(border_css),
               "  border: %dpx solid %s;\n",
               cfg->style.epaisseur_bordure,
               cfg->style.couleur_bordure ? cfg->style.couleur_bordure : "transparent");
   }

   if (cfg->style.gras || cfg->style.italique || cfg->style.taille_texte_px > 0)
   {
      char size_buf[64] = "";
      if (cfg->style.taille_texte_px > 0)
         snprintf(size_buf, sizeof(size_buf), "  font-size: %dpx;\n", cfg->style.taille_texte_px);

      snprintf(font_css, sizeof(font_css),
               "  font-weight: %s;\n"
               "  font-style: %s;\n"
               "%s",
               cfg->style.gras ? "bold" : "normal",
               cfg->style.italique ? "italic" : "normal",
               size_buf);
   }

   /* Style du label d'erreur */
   int err_size = cfg->erreur_taille_px > 0 ? cfg->erreur_taille_px : 11;
   const char *err_color = cfg->erreur_couleur ? cfg->erreur_couleur : "#e74c3c";

   snprintf(css, sizeof(css),
            /* --- Entry normal --- */
            "entry#%s {\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "%s"
            "  border-radius: %dpx;\n"
            "  padding: 6px 10px;\n"
            "%s"
            "}\n"
            /* --- Entry en erreur --- */
            "entry#%s.error {\n"
            "  border: 1px solid %s;\n"
            "  background: %s;\n"
            "}\n"
            /* --- Label d'erreur --- */
            "label#%s_erreur {\n"
            "  color: %s;\n"
            "  font-size: %dpx;\n"
            "  margin-top: 2px;\n"
            "  margin-left: 2px;\n"
            "}\n",
            /* entry normal */
            cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,
            font_css,
            /* entry error */
            cfg->id_css,
            cfg->style.couleur_bordure_error ? cfg->style.couleur_bordure_error : "#e74c3c",
            cfg->style.bg_error ? cfg->style.bg_error : "#fff1f2",
            /* label erreur */
            cfg->id_css,
            err_color,
            err_size);

   gtk_css_provider_load_from_string(provider, css);

   /* Appliquer sur l'entry */
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);

   /* Appliquer sur le label d'erreur */
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->label_erreur),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);

   g_object_unref(provider);
}

// ====================== VALIDATION ======================

static gboolean champ_texte_validate_internal(ChampTexte *cfg)
{
   char empty_txt[1] = "";

   if (!cfg || !cfg->widget || !cfg->label_erreur)
      return TRUE;

   const char *txt = gtk_editable_get_text(GTK_EDITABLE(cfg->widget));
   if (!txt)
      txt = empty_txt;

   size_t len = strlen(txt);

   /* Champ requis */
   if (cfg->required && len == 0)
   {
      champ_texte_show_error(cfg, "Ce champ est obligatoire");
      return FALSE;
   }

   /* Champ vide + non requis → OK, pas d'autre vérification */
   if (len == 0)
   {
      champ_texte_clear_error(cfg);
      return TRUE;
   }

   /* Longueur minimale */
   if (cfg->policy.min_len > 0 && (int)len < cfg->policy.min_len)
   {
      char msg[128];
      snprintf(msg, sizeof(msg), "Minimum %d caractères requis", cfg->policy.min_len);
      champ_texte_show_error(cfg, msg);
      return FALSE;
   }

   /* Longueur maximale (politique) */
   if (cfg->policy.max_len > 0 && (int)len > cfg->policy.max_len)
   {
      char msg[128];
      snprintf(msg, sizeof(msg), "Maximum %d caractères autorisés", cfg->policy.max_len);
      champ_texte_show_error(cfg, msg);
      return FALSE;
   }

   /* Pas d'espaces */
   if (cfg->policy.no_whitespace)
   {
      for (size_t i = 0; i < len; i++)
      {
         if (isspace((unsigned char)txt[i]))
         {
            champ_texte_show_error(cfg, "Les espaces ne sont pas autorisés");
            return FALSE;
         }
      }
   }

   /* Pas de chiffres */
   if (cfg->policy.no_digits)
   {
      for (size_t i = 0; i < len; i++)
      {
         if (isdigit((unsigned char)txt[i]))
         {
            champ_texte_show_error(cfg, "Les chiffres ne sont pas autorisés");
            return FALSE;
         }
      }
   }

   /* Uniquement des chiffres */
   if (cfg->policy.only_digits)
   {
      for (size_t i = 0; i < len; i++)
      {
         if (!isdigit((unsigned char)txt[i]))
         {
            champ_texte_show_error(cfg, "Uniquement des chiffres sont autorisés");
            return FALSE;
         }
      }
   }

   /* Validation e-mail */
   if (cfg->type == CHAMP_TEXTE_TYPE_EMAIL)
   {
      const char *at = strchr(txt, '@');
      if (!at || at == txt || *(at + 1) == '\0' || !strchr(at + 1, '.'))
      {
         champ_texte_show_error(cfg, "Adresse e-mail invalide");
         return FALSE;
      }
   }

   /* Validation URL */
   if (cfg->type == CHAMP_TEXTE_TYPE_URL)
   {
      if (strncmp(txt, "http://", 7) != 0 && strncmp(txt, "https://", 8) != 0)
      {
         champ_texte_show_error(cfg, "L'URL doit commencer par http:// ou https://");
         return FALSE;
      }
   }

   /* Tout est valide */
   champ_texte_clear_error(cfg);
   return TRUE;
}

// ====================== CALLBACKS INTERNES ======================

static void on_texte_changed(GtkEditable *editable, gpointer user_data)
{
   ChampTexte *cfg = (ChampTexte *)user_data;

   /* Troncature max_length */
   if (cfg->max_length > 0)
   {
      const char *txt = gtk_editable_get_text(editable);
      size_t n = txt ? strlen(txt) : 0;

      if (n > (size_t)cfg->max_length)
      {
         char *truncated = malloc(strlen(txt) + 1);
         if (!truncated)
            return;
         strcpy(truncated, txt);
         truncated[cfg->max_length] = '\0';
         g_signal_handlers_block_by_func(editable, on_texte_changed, user_data);
         gtk_editable_set_text(editable, truncated);
         gtk_editable_set_position(GTK_EDITABLE(editable), cfg->max_length);
         g_signal_handlers_unblock_by_func(editable, on_texte_changed, user_data);
         g_free(truncated);
      }
   }

   champ_texte_validate_internal(cfg);
   if (cfg->on_change)
      cfg->on_change(editable, cfg->user_data);
}

static void on_texte_activate(GtkEntry *entry, gpointer user_data)
{
   ChampTexte *cfg = (ChampTexte *)user_data;
   champ_texte_validate_internal(cfg);
   if (cfg->on_activate)
      cfg->on_activate(entry, cfg->user_data);
}

// ====================== FONCTIONS PUBLIQUES ======================

void champ_texte_initialiser(ChampTexte *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(ChampTexte));

   cfg->id_css = malloc(strlen("champ_texte") + 1);
   strcpy(cfg->id_css, "champ_texte");
   cfg->placeholder = NULL;
   cfg->max_length = 0;
   cfg->required = FALSE;
   cfg->type = CHAMP_TEXTE_TYPE_TEXT;

   cfg->policy.min_len = 0;
   cfg->policy.max_len = 0;
   cfg->policy.pattern = NULL;
   cfg->policy.no_whitespace = FALSE;
   cfg->policy.no_digits = FALSE;
   cfg->policy.only_digits = FALSE;

   cfg->sensitive = TRUE;
   cfg->editable = TRUE;
   cfg->icon_primary = NULL;
   cfg->icon_secondary = NULL;

   cfg->size.width = 0;
   cfg->size.height = 0;

   cfg->show_error_label = TRUE; // Afficher le label d'erreur par défaut
   cfg->erreur_couleur = NULL;   // Utilise #e74c3c par défaut
   cfg->erreur_taille_px = 0;    // Utilise 11px par défaut

   widget_style_init(&cfg->style);
   cfg->style.bg_normal = malloc(strlen("white") + 1);
   strcpy(cfg->style.bg_normal, "white");
   cfg->style.fg_normal = malloc(strlen("#2c3e50") + 1);
   strcpy(cfg->style.fg_normal, "#2c3e50");
   cfg->style.couleur_bordure = malloc(strlen("#bdc3c7") + 1);
   strcpy(cfg->style.couleur_bordure, "#bdc3c7");
   cfg->style.rayon_arrondi = 4;
}

/**
 * Crée le widget et retourne cfg->container (GtkBox vertical).
 * C'est ce container qu'il faut ajouter au widget parent, pas cfg->widget.
 *
 * Structure interne du container :
 *   GtkBox (vertical, spacing=0)
 *   ├── GtkEntry   (cfg->widget)
 *   └── GtkLabel   (cfg->label_erreur, caché par défaut)
 */
GtkWidget *champ_texte_creer(ChampTexte *cfg)
{
   if (!cfg)
      return NULL;

   /* --- Container vertical --- */
   cfg->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   gtk_widget_set_hexpand(cfg->container, cfg->size.width == 0 ? TRUE : FALSE);
   gtk_widget_set_halign(cfg->container, cfg->size.width == 0 ? GTK_ALIGN_FILL : GTK_ALIGN_START);

   /* --- GtkEntry --- */
   cfg->widget = gtk_entry_new();
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "champ_texte");

   if (cfg->placeholder)
      gtk_entry_set_placeholder_text(GTK_ENTRY(cfg->widget), cfg->placeholder);

   /* Type / purpose clavier */
   switch (cfg->type)
   {
   case CHAMP_TEXTE_TYPE_EMAIL:
      gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_EMAIL);
      break;
   case CHAMP_TEXTE_TYPE_URL:
      gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_URL);
      break;
   default:
      gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_FREE_FORM);
      break;
   }

   /* Icônes */
   if (cfg->icon_primary)
      gtk_entry_set_icon_from_icon_name(GTK_ENTRY(cfg->widget),
                                        GTK_ENTRY_ICON_PRIMARY, cfg->icon_primary);
   if (cfg->icon_secondary)
      gtk_entry_set_icon_from_icon_name(GTK_ENTRY(cfg->widget),
                                        GTK_ENTRY_ICON_SECONDARY, cfg->icon_secondary);

   gtk_editable_set_editable(GTK_EDITABLE(cfg->widget), cfg->editable);
   gtk_widget_set_sensitive(cfg->widget, cfg->sensitive);

   /* Taille de l'entry */
   if (cfg->size.width > 0 || cfg->size.height > 0)
      gtk_widget_set_size_request(cfg->widget,
                                  cfg->size.width > 0 ? cfg->size.width : -1,
                                  cfg->size.height > 0 ? cfg->size.height : -1);

   if (cfg->size.width > 0)
   {
      gtk_widget_set_hexpand(cfg->widget, FALSE);
      gtk_widget_set_halign(cfg->widget, GTK_ALIGN_START);
   }
   else
   {
      gtk_widget_set_hexpand(cfg->widget, TRUE);
      gtk_widget_set_halign(cfg->widget, GTK_ALIGN_FILL);
   }

   gtk_widget_set_vexpand(cfg->widget, FALSE);
   gtk_widget_set_valign(cfg->widget, GTK_ALIGN_START);

   /* --- Label d'erreur --- */
   cfg->label_erreur = gtk_label_new("");

   /* ID CSS = "<id_css>_erreur" pour le ciblage CSS */
   char lbl_id[256];
   snprintf(lbl_id, sizeof(lbl_id), "%s_erreur", cfg->id_css ? cfg->id_css : "champ_texte");
   gtk_widget_set_name(cfg->label_erreur, lbl_id);

   gtk_label_set_xalign(GTK_LABEL(cfg->label_erreur), 0.0f); // Aligné à gauche
   gtk_label_set_wrap(GTK_LABEL(cfg->label_erreur), TRUE);
   gtk_widget_set_hexpand(cfg->label_erreur, TRUE);
   gtk_widget_set_visible(cfg->label_erreur, FALSE); // Caché par défaut

   /* --- Assemblage --- */
   gtk_box_append(GTK_BOX(cfg->container), cfg->widget);
   gtk_box_append(GTK_BOX(cfg->container), cfg->label_erreur);

   /* --- Signaux --- */
   g_signal_connect(cfg->widget, "changed", G_CALLBACK(on_texte_changed), cfg);
   g_signal_connect(cfg->widget, "activate", G_CALLBACK(on_texte_activate), cfg);

   /* --- CSS & validation initiale --- */
   champ_texte_apply_css(cfg);
   champ_texte_validate_internal(cfg);

   return cfg->container;
}

const char *champ_texte_get_texte(ChampTexte *cfg)
{
   if (!cfg || !cfg->widget)
      return NULL;
   return gtk_editable_get_text(GTK_EDITABLE(cfg->widget));
}

void champ_texte_set_texte(ChampTexte *cfg, const char *texte)
{
   if (!cfg || !cfg->widget)
      return;
   gtk_editable_set_text(GTK_EDITABLE(cfg->widget), texte ? texte : "");
}

void champ_texte_set_placeholder(ChampTexte *cfg, const char *ph)
{
   if (!cfg)
      return;
   g_free(cfg->placeholder);
   if (ph)
   {
      cfg->placeholder = malloc(strlen(ph) + 1);
      strcpy(cfg->placeholder, ph);
   }
   else
   {
      cfg->placeholder = NULL;
   }
   if (cfg->widget)
      gtk_entry_set_placeholder_text(GTK_ENTRY(cfg->widget), cfg->placeholder);
}

void champ_texte_set_max_length(ChampTexte *cfg, int max_len)
{
   if (!cfg)
      return;
   cfg->max_length = max_len;
}

void champ_texte_set_required(ChampTexte *cfg, gboolean required)
{
   if (!cfg)
      return;
   cfg->required = required;
   if (cfg->widget)
      champ_texte_validate_internal(cfg);
}

void champ_texte_set_editable(ChampTexte *cfg, gboolean editable)
{
   if (!cfg)
      return;
   cfg->editable = editable;
   if (cfg->widget)
      gtk_editable_set_editable(GTK_EDITABLE(cfg->widget), editable);
}

void champ_texte_set_type(ChampTexte *cfg, ChampTexteType type)
{
   if (!cfg)
      return;
   cfg->type = type;
   if (cfg->widget)
   {
      switch (type)
      {
      case CHAMP_TEXTE_TYPE_EMAIL:
         gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_EMAIL);
         break;
      case CHAMP_TEXTE_TYPE_URL:
         gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_URL);
         break;
      default:
         gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_FREE_FORM);
         break;
      }
      champ_texte_validate_internal(cfg);
   }
}

void champ_texte_set_policy(ChampTexte *cfg, ChampTextePolicy policy)
{
   if (!cfg)
      return;
   cfg->policy = policy;
   if (cfg->widget)
      champ_texte_validate_internal(cfg);
}

void champ_texte_set_size(ChampTexte *cfg, int width, int height)
{
   if (!cfg)
      return;
   cfg->size.width = width;
   cfg->size.height = height;
   if (cfg->widget)
   {
      gtk_widget_set_size_request(cfg->widget,
                                  width > 0 ? width : -1,
                                  height > 0 ? height : -1);
      if (width > 0)
      {
         gtk_widget_set_hexpand(cfg->widget, FALSE);
         gtk_widget_set_halign(cfg->widget, GTK_ALIGN_START);
         gtk_widget_set_hexpand(cfg->container, FALSE);
         gtk_widget_set_halign(cfg->container, GTK_ALIGN_START);
      }
      else
      {
         gtk_widget_set_hexpand(cfg->widget, TRUE);
         gtk_widget_set_halign(cfg->widget, GTK_ALIGN_FILL);
         gtk_widget_set_hexpand(cfg->container, TRUE);
         gtk_widget_set_halign(cfg->container, GTK_ALIGN_FILL);
      }
   }
}

void champ_texte_set_icons(ChampTexte *cfg, const char *icon_primary, const char *icon_secondary)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->icon_primary = icon_primary;
   cfg->icon_secondary = icon_secondary;
   gtk_entry_set_icon_from_icon_name(GTK_ENTRY(cfg->widget),
                                     GTK_ENTRY_ICON_PRIMARY, icon_primary);
   gtk_entry_set_icon_from_icon_name(GTK_ENTRY(cfg->widget),
                                     GTK_ENTRY_ICON_SECONDARY, icon_secondary);
}

gboolean champ_texte_valider(ChampTexte *cfg)
{
   return champ_texte_validate_internal(cfg);
}

void champ_texte_free(ChampTexte *cfg)
{
   if (!cfg)
      return;

   g_free(cfg->id_css);
   cfg->id_css = NULL;

   if (cfg->placeholder)
   {
      g_free(cfg->placeholder);
      cfg->placeholder = NULL;
   }
   if (cfg->erreur_couleur)
   {
      g_free(cfg->erreur_couleur);
      cfg->erreur_couleur = NULL;
   }

   widget_style_free(&cfg->style);
   memset(cfg, 0, sizeof(ChampTexte));
}
