#include "../headers/champ_motdepasse.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ====================== HELPERS ERREUR ======================

/**
 * Affiche le label d'erreur avec le message donné et passe l'entry en état error.
 */
static void champ_motdepasse_show_error(ChampMotDePasse *cfg, const char *message)
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
static void champ_motdepasse_clear_error(ChampMotDePasse *cfg)
{
   gtk_widget_remove_css_class(cfg->widget, "error");

   if (cfg->show_error_label)
   {
      gtk_label_set_text(GTK_LABEL(cfg->label_erreur), "");
      gtk_widget_set_visible(cfg->label_erreur, FALSE);
   }
}

static void champ_pw_apply_css(ChampMotDePasse *cfg)
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

   /* Style du label d'erreur */
   int err_size = cfg->erreur_taille_px > 0 ? cfg->erreur_taille_px : 11;
   const char *err_color = cfg->erreur_couleur ? cfg->erreur_couleur : "#e74c3c";

   snprintf(css, sizeof(css),
            "entry#%s, passwordentry#%s {\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "%s"
            "  border-radius: %dpx;\n"
            "  padding: 6px 10px;\n"
            "}\n"
            "entry.error, passwordentry.error {\n"
            "  border: 1px solid %s;\n"
            "  background: %s;\n"
            "}\n"
            "label#%s_erreur {\n"
            "  color: %s;\n"
            "  font-size: %dpx;\n"
            "  margin-top: 2px;\n"
            "  margin-left: 2px;\n"
            "}\n",
            cfg->id_css, cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,
            cfg->style.couleur_bordure_error ? cfg->style.couleur_bordure_error : "#e74c3c",
            cfg->style.bg_error ? cfg->style.bg_error : "#fff1f2",
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
   if (cfg->label_erreur)
   {
      gtk_style_context_add_provider(
          gtk_widget_get_style_context(cfg->label_erreur),
          GTK_STYLE_PROVIDER(provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);
   }

   g_object_unref(provider);
}

static gboolean champ_pw_validate(ChampMotDePasse *cfg)
{
   if (!cfg || !cfg->widget)
      return TRUE;

   const char *txt = gtk_editable_get_text(GTK_EDITABLE(cfg->widget));
   if (!txt)
      txt = "";

   // Check required
   if (cfg->required && strlen(txt) == 0)
   {
      champ_motdepasse_show_error(cfg, "Password required");
      return FALSE;
   }

   // Check minimum length
   if (cfg->policy.min_len > 0 && (int)strlen(txt) < cfg->policy.min_len)
   {
      champ_motdepasse_show_error(cfg, "Password too short");
      return FALSE;
   }

   // Check digit requirement
   if (cfg->policy.require_digit)
   {
      gboolean ok = FALSE;
      for (int i = 0; txt[i]; i++)
      {
         if (txt[i] >= '0' && txt[i] <= '9')
         {
            ok = TRUE;
            break;
         }
      }
      if (!ok)
      {
         champ_motdepasse_show_error(cfg, "Password must contain a digit");
         return FALSE;
      }
   }

   // Check uppercase requirement
   if (cfg->policy.require_upper)
   {
      gboolean ok = FALSE;
      for (int i = 0; txt[i]; i++)
      {
         if (txt[i] >= 'A' && txt[i] <= 'Z')
         {
            ok = TRUE;
            break;
         }
      }
      if (!ok)
      {
         champ_motdepasse_show_error(cfg, "Password must contain uppercase letter");
         return FALSE;
      }
   }

   // Check symbol requirement
   if (cfg->policy.require_symbol)
   {
      gboolean ok = FALSE;
      for (int i = 0; txt[i]; i++)
      {
         char c = txt[i];
         if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z') && !(c >= '0' && c <= '9'))
         {
            ok = TRUE;
            break;
         }
      }
      if (!ok)
      {
         champ_motdepasse_show_error(cfg, "Password must contain a symbol");
         return FALSE;
      }
   }

   // All checks passed
   champ_motdepasse_clear_error(cfg);
   return TRUE;
}

static void on_pw_changed(GtkEditable *editable, gpointer user_data)
{
   ChampMotDePasse *cfg = (ChampMotDePasse *)user_data;

   // Enforce max_length by truncating if necessary
   if (cfg->max_length > 0)
   {
      const char *txt = gtk_editable_get_text(editable);
      size_t n = txt ? strlen(txt) : 0;

      if (n > (size_t)cfg->max_length)
      {
         // Truncate the text to max_length
         char *truncated = g_strndup(txt, cfg->max_length);
         g_signal_handlers_block_by_func(editable, on_pw_changed, user_data);
         gtk_editable_set_text(editable, truncated);
         gtk_editable_set_position(GTK_EDITABLE(editable), cfg->max_length); // Set cursor to the end
         g_signal_handlers_unblock_by_func(editable, on_pw_changed, user_data);
         g_free(truncated);
      }
   }

   champ_pw_validate(cfg); // validate on every change to update error state
   if (cfg->on_change)
      cfg->on_change(editable, cfg->user_data);
}

// when user presses Enter
static void on_pw_activate(GtkEntry *entry, gpointer user_data)
{
   ChampMotDePasse *cfg = (ChampMotDePasse *)user_data;
   champ_pw_validate(cfg);
   if (cfg->on_activate)
      cfg->on_activate(entry, cfg->user_data);
}

void champ_motdepasse_initialiser(ChampMotDePasse *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(ChampMotDePasse));
   cfg->id_css = malloc(strlen("champ_pwd") + 1);
   strcpy(cfg->id_css, "champ_pwd");
   cfg->placeholder = NULL;
   cfg->max_length = 0;
   cfg->required = FALSE;
   cfg->policy.min_len = 0;
   cfg->policy.require_digit = FALSE;
   cfg->policy.require_upper = FALSE;
   cfg->policy.require_symbol = FALSE;

   cfg->reveal_toggle = TRUE;
   cfg->sensitive = TRUE;
   cfg->size.width = 0;  // 0 = full width (100%)
   cfg->size.height = 0; // 0 = auto size

   cfg->show_error_label = TRUE; // Afficher le label d'erreur par défaut
   cfg->erreur_couleur = NULL;   // Utilise #e74c3c par défaut
   cfg->erreur_taille_px = 0;    // Utilise 11px par défaut

   // Initialize style using common function
   widget_style_init(&cfg->style);
   // Override defaults for password field
   cfg->style.bg_normal = malloc(strlen("white") + 1);
   strcpy(cfg->style.bg_normal, "white");
   cfg->style.fg_normal = malloc(strlen("#2c3e50") + 1);
   strcpy(cfg->style.fg_normal, "#2c3e50");
   cfg->style.couleur_bordure = malloc(strlen("#bdc3c7") + 1);
   strcpy(cfg->style.couleur_bordure, "#bdc3c7");
   cfg->style.rayon_arrondi = 4;
}

GtkWidget *champ_motdepasse_creer(ChampMotDePasse *cfg)
{
   if (!cfg)
      return NULL;

   /* --- Container vertical --- */
   cfg->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   gtk_widget_set_hexpand(cfg->container, cfg->size.width == 0 ? TRUE : FALSE);
   gtk_widget_set_halign(cfg->container, cfg->size.width == 0 ? GTK_ALIGN_FILL : GTK_ALIGN_START);

   /* --- GtkPasswordEntry --- */
   cfg->widget = gtk_password_entry_new();
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "champ_pwd");

   if (cfg->placeholder)
      g_object_set(cfg->widget, "placeholder-text", cfg->placeholder, NULL);

   gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(cfg->widget), cfg->reveal_toggle);
   gtk_widget_set_sensitive(cfg->widget, cfg->sensitive);

   // Set size if specified
   if (cfg->size.width > 0 || cfg->size.height > 0)
   {
      gtk_widget_set_size_request(cfg->widget,
                                  cfg->size.width > 0 ? cfg->size.width : -1,
                                  cfg->size.height > 0 ? cfg->size.height : -1);
   }

   // Control widget expansion behavior
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
   snprintf(lbl_id, sizeof(lbl_id), "%s_erreur", cfg->id_css ? cfg->id_css : "champ_pwd");
   gtk_widget_set_name(cfg->label_erreur, lbl_id);

   gtk_label_set_xalign(GTK_LABEL(cfg->label_erreur), 0.0f); // Aligné à gauche
   gtk_label_set_wrap(GTK_LABEL(cfg->label_erreur), TRUE);
   gtk_widget_set_hexpand(cfg->label_erreur, TRUE);
   gtk_widget_set_visible(cfg->label_erreur, FALSE); // Caché par défaut

   /* --- Assemblage --- */
   gtk_box_append(GTK_BOX(cfg->container), cfg->widget);
   gtk_box_append(GTK_BOX(cfg->container), cfg->label_erreur);

   /* --- Signaux --- */
   g_signal_connect(cfg->widget, "changed", G_CALLBACK(on_pw_changed), cfg);
   g_signal_connect(cfg->widget, "activate", G_CALLBACK(on_pw_activate), cfg);

   /* --- CSS & validation initiale --- */
   champ_pw_apply_css(cfg);
   champ_pw_validate(cfg);

   return cfg->container;
}

const char *champ_motdepasse_get_texte(ChampMotDePasse *cfg)
{
   if (!cfg || !cfg->widget)
      return NULL;
   return gtk_editable_get_text(GTK_EDITABLE(cfg->widget));
}

void champ_motdepasse_set_texte(ChampMotDePasse *cfg, const char *texte)
{
   if (!cfg || !cfg->widget)
      return;
   gtk_editable_set_text(GTK_EDITABLE(cfg->widget), texte ? texte : "");
}

void champ_motdepasse_set_placeholder(ChampMotDePasse *cfg, const char *ph)
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
      g_object_set(cfg->widget, "placeholder-text", cfg->placeholder, NULL);
}

void champ_motdepasse_set_max_length(ChampMotDePasse *cfg, int max_len)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->max_length = max_len;
}

void champ_motdepasse_set_required(ChampMotDePasse *cfg, gboolean required)
{
   if (!cfg)
      return;
   cfg->required = required;
   if (cfg->widget)
      champ_pw_validate(cfg);
}

void champ_motdepasse_set_reveal_toggle(ChampMotDePasse *cfg, gboolean reveal)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->reveal_toggle = reveal;
   gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(cfg->widget), reveal);
}

void champ_motdepasse_set_policy(ChampMotDePasse *cfg, ChampPasswordPolicy policy)
{
   if (!cfg)
      return;
   cfg->policy = policy;
   if (cfg->widget)
      champ_pw_validate(cfg);
}

void champ_motdepasse_set_size(ChampMotDePasse *cfg, int width, int height)
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

      // Update expansion behavior
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

void champ_motdepasse_free(ChampMotDePasse *cfg)
{
   if (!cfg)
      return;

   g_free(cfg->id_css);
   cfg->id_css = NULL;

   // Free the placeholder string if it was allocated
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

   // Free the style structure
   widget_style_free(&cfg->style);

   // Clear the structure
   memset(cfg, 0, sizeof(ChampMotDePasse));
}
