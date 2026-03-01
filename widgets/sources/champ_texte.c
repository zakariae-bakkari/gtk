#include "../headers/champ_texte.h"
#include <string.h>
#include <ctype.h>

// ====================== CSS ======================

static void champ_texte_apply_css(ChampTexte *cfg)
{
   if (!cfg || !cfg->widget || !cfg->id_css)
      return;

   GtkCssProvider *provider = gtk_css_provider_new();
   char css[2048];
   char border_css[128] = "";

   if (cfg->style.epaisseur_bordure > 0)
   {
      snprintf(border_css, sizeof(border_css),
               "  border: %dpx solid %s;\n",
               cfg->style.epaisseur_bordure,
               cfg->style.couleur_bordure ? cfg->style.couleur_bordure : "transparent");
   }

   char font_css[128] = "";
   if (cfg->style.gras || cfg->style.italique || cfg->style.taille_texte_px > 0)
   {
      snprintf(font_css, sizeof(font_css),
               "  font-weight: %s;\n"
               "  font-style: %s;\n"
               "%s",
               cfg->style.gras ? "bold" : "normal",
               cfg->style.italique ? "italic" : "normal",
               cfg->style.taille_texte_px > 0
                  ? (char[64]){ [0]=0, [1]=0 } /* placeholder, see below */
                  : "");
      if (cfg->style.taille_texte_px > 0)
      {
         char size_buf[64];
         snprintf(size_buf, sizeof(size_buf),
                  "  font-size: %dpx;\n", cfg->style.taille_texte_px);
         strncat(font_css, size_buf, sizeof(font_css) - strlen(font_css) - 1);
      }
   }

   snprintf(css, sizeof(css),
            "entry#%s {\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "%s"
            "  border-radius: %dpx;\n"
            "  padding: 6px 10px;\n"
            "%s"
            "}\n"
            "entry#%s.error {\n"
            "  border: 1px solid %s;\n"
            "  background: %s;\n"
            "}\n",
            cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,
            font_css,
            cfg->id_css,
            cfg->style.couleur_bordure_error ? cfg->style.couleur_bordure_error : "#e74c3c",
            cfg->style.bg_error ? cfg->style.bg_error : "#fff1f2");

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);
   g_object_unref(provider);
}

// ====================== VALIDATION ======================

static gboolean champ_texte_validate_internal(ChampTexte *cfg)
{
   if (!cfg || !cfg->widget)
      return TRUE;

   const char *txt = gtk_editable_get_text(GTK_EDITABLE(cfg->widget));
   if (!txt)
      txt = "";

   size_t len = strlen(txt);

   // Vérification : champ requis
   if (cfg->required && len == 0)
   {
      gtk_widget_add_css_class(cfg->widget, "error");
      if (cfg->on_invalid)
         cfg->on_invalid(cfg->widget, "Ce champ est obligatoire", cfg->user_data);
      return FALSE;
   }

   // Si le champ est vide et non requis, pas d'autres vérifications
   if (len == 0)
   {
      gtk_widget_remove_css_class(cfg->widget, "error");
      return TRUE;
   }

   // Vérification : longueur minimale
   if (cfg->policy.min_len > 0 && (int)len < cfg->policy.min_len)
   {
      gtk_widget_add_css_class(cfg->widget, "error");
      if (cfg->on_invalid)
         cfg->on_invalid(cfg->widget, "Texte trop court", cfg->user_data);
      return FALSE;
   }

   // Vérification : longueur maximale (politique)
   if (cfg->policy.max_len > 0 && (int)len > cfg->policy.max_len)
   {
      gtk_widget_add_css_class(cfg->widget, "error");
      if (cfg->on_invalid)
         cfg->on_invalid(cfg->widget, "Texte trop long", cfg->user_data);
      return FALSE;
   }

   // Vérification : pas d'espaces
   if (cfg->policy.no_whitespace)
   {
      for (size_t i = 0; i < len; i++)
      {
         if (isspace((unsigned char)txt[i]))
         {
            gtk_widget_add_css_class(cfg->widget, "error");
            if (cfg->on_invalid)
               cfg->on_invalid(cfg->widget, "Les espaces ne sont pas autorisés", cfg->user_data);
            return FALSE;
         }
      }
   }

   // Vérification : pas de chiffres
   if (cfg->policy.no_digits)
   {
      for (size_t i = 0; i < len; i++)
      {
         if (isdigit((unsigned char)txt[i]))
         {
            gtk_widget_add_css_class(cfg->widget, "error");
            if (cfg->on_invalid)
               cfg->on_invalid(cfg->widget, "Les chiffres ne sont pas autorisés", cfg->user_data);
            return FALSE;
         }
      }
   }

   // Vérification : uniquement des chiffres
   if (cfg->policy.only_digits)
   {
      for (size_t i = 0; i < len; i++)
      {
         if (!isdigit((unsigned char)txt[i]))
         {
            gtk_widget_add_css_class(cfg->widget, "error");
            if (cfg->on_invalid)
               cfg->on_invalid(cfg->widget, "Uniquement des chiffres sont autorisés", cfg->user_data);
            return FALSE;
         }
      }
   }

   // Vérification : type e-mail (validation basique)
   if (cfg->type == CHAMP_TEXTE_TYPE_EMAIL)
   {
      const char *at = strchr(txt, '@');
      if (!at || at == txt || *(at + 1) == '\0' || !strchr(at + 1, '.'))
      {
         gtk_widget_add_css_class(cfg->widget, "error");
         if (cfg->on_invalid)
            cfg->on_invalid(cfg->widget, "Adresse e-mail invalide", cfg->user_data);
         return FALSE;
      }
   }

   // Vérification : type URL (validation basique)
   if (cfg->type == CHAMP_TEXTE_TYPE_URL)
   {
      if (strncmp(txt, "http://", 7) != 0 && strncmp(txt, "https://", 8) != 0)
      {
         gtk_widget_add_css_class(cfg->widget, "error");
         if (cfg->on_invalid)
            cfg->on_invalid(cfg->widget, "URL invalide (doit commencer par http:// ou https://)", cfg->user_data);
         return FALSE;
      }
   }

   // Toutes les vérifications passées
   gtk_widget_remove_css_class(cfg->widget, "error");
   return TRUE;
}

// ====================== CALLBACKS INTERNES ======================

static void on_texte_changed(GtkEditable *editable, gpointer user_data)
{
   ChampTexte *cfg = (ChampTexte *)user_data;

   // Appliquer la limite max_length par troncature si nécessaire
   if (cfg->max_length > 0)
   {
      const char *txt = gtk_editable_get_text(editable);
      size_t n = txt ? strlen(txt) : 0;

      if (n > (size_t)cfg->max_length)
      {
         char *truncated = g_strndup(txt, cfg->max_length);
         g_signal_handlers_block_by_func(editable, on_texte_changed, user_data);
         gtk_editable_set_text(editable, truncated);
         gtk_editable_set_position(GTK_EDITABLE(editable), cfg->max_length);
         g_signal_handlers_unblock_by_func(editable, on_texte_changed, user_data);
         g_free(truncated);
      }
   }

   champ_texte_validate_internal(cfg); // Mettre à jour l'état d'erreur en temps réel
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

   cfg->id_css = "champ_texte";
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

   cfg->size.width = 0;  // 0 = largeur complète (100%)
   cfg->size.height = 0; // 0 = hauteur automatique

   // Initialiser le style via la fonction commune
   widget_style_init(&cfg->style);
   // Surcharger les valeurs par défaut pour le champ texte
   cfg->style.bg_normal = g_strdup("white");
   cfg->style.fg_normal = g_strdup("#2c3e50");
   cfg->style.couleur_bordure = g_strdup("#bdc3c7");
   cfg->style.rayon_arrondi = 4;
}

GtkWidget *champ_texte_creer(ChampTexte *cfg)
{
   if (!cfg)
      return NULL;

   cfg->widget = gtk_entry_new(); // Crée un GtkEntry standard
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "champ_texte");

   if (cfg->placeholder)
      gtk_entry_set_placeholder_text(GTK_ENTRY(cfg->widget), cfg->placeholder);

   // Appliquer le type via le purpose de l'entrée
   switch (cfg->type)
   {
   case CHAMP_TEXTE_TYPE_EMAIL:
      gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_EMAIL);
      break;
   case CHAMP_TEXTE_TYPE_URL:
      gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_URL);
      break;
   case CHAMP_TEXTE_TYPE_SEARCH:
      gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_FREE_FORM);
      break;
   default:
      gtk_entry_set_input_purpose(GTK_ENTRY(cfg->widget), GTK_INPUT_PURPOSE_FREE_FORM);
      break;
   }

   // Icônes optionnelles
   if (cfg->icon_primary)
      gtk_entry_set_icon_from_icon_name(GTK_ENTRY(cfg->widget),
                                        GTK_ENTRY_ICON_PRIMARY,
                                        cfg->icon_primary);
   if (cfg->icon_secondary)
      gtk_entry_set_icon_from_icon_name(GTK_ENTRY(cfg->widget),
                                        GTK_ENTRY_ICON_SECONDARY,
                                        cfg->icon_secondary);

   gtk_editable_set_editable(GTK_EDITABLE(cfg->widget), cfg->editable);
   gtk_widget_set_sensitive(cfg->widget, cfg->sensitive);

   // Appliquer la taille si spécifiée
   if (cfg->size.width > 0 || cfg->size.height > 0)
   {
      gtk_widget_set_size_request(cfg->widget,
                                  cfg->size.width > 0 ? cfg->size.width : -1,
                                  cfg->size.height > 0 ? cfg->size.height : -1);
   }

   // Comportement d'expansion
   if (cfg->size.width > 0)
   {
      gtk_widget_set_hexpand(cfg->widget, FALSE);
      gtk_widget_set_halign(cfg->widget, GTK_ALIGN_START);
   }
   else
   {
      // width = 0 → largeur complète
      gtk_widget_set_hexpand(cfg->widget, TRUE);
      gtk_widget_set_halign(cfg->widget, GTK_ALIGN_FILL);
   }

   gtk_widget_set_vexpand(cfg->widget, FALSE);
   gtk_widget_set_valign(cfg->widget, GTK_ALIGN_START);

   // Connecter les signaux
   g_signal_connect(cfg->widget, "changed", G_CALLBACK(on_texte_changed), cfg);
   g_signal_connect(cfg->widget, "activate", G_CALLBACK(on_texte_activate), cfg);

   champ_texte_apply_css(cfg);          // Appliquer le style CSS
   champ_texte_validate_internal(cfg);  // Valider pour afficher l'état initial

   return cfg->widget;
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
   if (cfg->widget)
      gtk_entry_set_placeholder_text(GTK_ENTRY(cfg->widget), ph);
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
      }
      else
      {
         gtk_widget_set_hexpand(cfg->widget, TRUE);
         gtk_widget_set_halign(cfg->widget, GTK_ALIGN_FILL);
      }
   }
}

void champ_texte_set_icons(ChampTexte *cfg, const char *icon_primary, const char *icon_secondary)
{
   if (!cfg)
      return;
   cfg->icon_primary = icon_primary;
   cfg->icon_secondary = icon_secondary;
   if (cfg->widget)
   {
      gtk_entry_set_icon_from_icon_name(GTK_ENTRY(cfg->widget),
                                        GTK_ENTRY_ICON_PRIMARY,
                                        icon_primary);
      gtk_entry_set_icon_from_icon_name(GTK_ENTRY(cfg->widget),
                                        GTK_ENTRY_ICON_SECONDARY,
                                        icon_secondary);
   }
}

gboolean champ_texte_valider(ChampTexte *cfg)
{
   return champ_texte_validate_internal(cfg);
}

void champ_texte_free(ChampTexte *cfg)
{
   if (!cfg)
      return;

   if (cfg->placeholder)
   {
      g_free(cfg->placeholder);
      cfg->placeholder = NULL;
   }

   widget_style_free(&cfg->style);
   memset(cfg, 0, sizeof(ChampTexte));
}