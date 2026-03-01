#include "../headers/champ_motdepasse.h"
#include <string.h>
#include <ctype.h>

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
            "}\n",
            cfg->id_css, cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,
            cfg->style.couleur_bordure_error ? cfg->style.couleur_bordure_error : "#e74c3c",
            cfg->style.bg_error ? cfg->style.bg_error : "#fff1f2");

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);
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
      gtk_widget_add_css_class(cfg->widget, "error");
      if (cfg->on_invalid)
         cfg->on_invalid(cfg->widget, "Password required", cfg->user_data);
      return FALSE;
   }

   // Check minimum length
   if (cfg->policy.min_len > 0 && (int)strlen(txt) < cfg->policy.min_len)
   {
      gtk_widget_add_css_class(cfg->widget, "error");
      if (cfg->on_invalid)
         cfg->on_invalid(cfg->widget, "Password too short", cfg->user_data);
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
         gtk_widget_add_css_class(cfg->widget, "error");
         if (cfg->on_invalid)
            cfg->on_invalid(cfg->widget, "Password must contain a digit", cfg->user_data);
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
         gtk_widget_add_css_class(cfg->widget, "error");
         if (cfg->on_invalid)
            cfg->on_invalid(cfg->widget, "Password must contain uppercase letter", cfg->user_data);
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
         gtk_widget_add_css_class(cfg->widget, "error");
         if (cfg->on_invalid)
            cfg->on_invalid(cfg->widget, "Password must contain a symbol", cfg->user_data);
         return FALSE;
      }
   }

   // All checks passed
   gtk_widget_remove_css_class(cfg->widget, "error");
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
   cfg->id_css = "champ_pwd";
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

   // Initialize style using common function
   widget_style_init(&cfg->style);
   // Override defaults for password field
   cfg->style.bg_normal = g_strdup("white");
   cfg->style.fg_normal = g_strdup("#2c3e50");
   cfg->style.couleur_bordure = g_strdup("#bdc3c7");
   cfg->style.rayon_arrondi = 4;
}

GtkWidget *champ_motdepasse_creer(ChampMotDePasse *cfg)
{
   if (!cfg)
      return NULL;

   cfg->widget = gtk_password_entry_new(); // cree un GtkPasswordEntry
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
      // Fixed width - don't expand
      gtk_widget_set_hexpand(cfg->widget, FALSE);
      gtk_widget_set_halign(cfg->widget, GTK_ALIGN_START);
   }
   else
   {
      // width = 0 means full width - expand to fill container
      gtk_widget_set_hexpand(cfg->widget, TRUE);
      gtk_widget_set_halign(cfg->widget, GTK_ALIGN_FILL);
   }

   // Password entries typically don't need vertical expansion
   gtk_widget_set_vexpand(cfg->widget, FALSE);
   gtk_widget_set_valign(cfg->widget, GTK_ALIGN_START);

   g_signal_connect(cfg->widget, "changed", G_CALLBACK(on_pw_changed), cfg);
   g_signal_connect(cfg->widget, "activate", G_CALLBACK(on_pw_activate), cfg);

   champ_pw_apply_css(cfg); // Appliquer le style CSS
   champ_pw_validate(cfg);  // Valider le champ pour afficher l'état initial

   return cfg->widget;
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
   if (!cfg || !cfg->widget)
      return;
   g_object_set(cfg->widget, "placeholder-text", ph, NULL);
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

   // Free the placeholder string if it was allocated
   if (cfg->placeholder)
   {
      g_free(cfg->placeholder);
      cfg->placeholder = NULL;
   }

   // Free the style structure
   widget_style_free(&cfg->style);

   // Clear the structure
   memset(cfg, 0, sizeof(ChampMotDePasse));
}
