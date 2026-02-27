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
            "  border: 1px solid #e74c3c;\n"
            "}\n",
            cfg->id_css, cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,
            cfg->id_css, cfg->id_css);

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);
   g_object_unref(provider);
}

static gboolean champ_pw_validate(ChampMotDePasse *cfg)
{
   // verify que cfg est valide et que le widget existe
   if (!cfg || !cfg->widget)
      return false;

   // Get the current text and its length
   const char *txt = gtk_editable_get_text(GTK_EDITABLE(cfg->widget));
   size_t n = txt ? strlen(txt) : 0;

   // Check max_length first
   if (cfg->max_length > 0 && n > (size_t)cfg->max_length)
   {
      gtk_widget_add_css_class(cfg->widget, "error");
      if (cfg->on_invalid)
         cfg->on_invalid(cfg->widget, "password exceeds maximum length", cfg->user_data);
      return FALSE;
   }

   // Then check required
   if (cfg->required && n == 0)
   {
      gtk_widget_add_css_class(cfg->widget, "error");
      if (cfg->on_invalid)
         cfg->on_invalid(cfg->widget, "password is required", cfg->user_data);
      return FALSE;
   }

   // If not empty, check policy constraints
   if (n > 0)
   {
      // Check minimum length
      if (cfg->policy.min_len > 0 && n < (size_t)cfg->policy.min_len)
      {
         gtk_widget_add_css_class(cfg->widget, "error");
         if (cfg->on_invalid)
            cfg->on_invalid(cfg->widget, "password too short", cfg->user_data);
         return FALSE;
      }
      // Check digit requirement
      if (cfg->policy.require_digit)
      {
         bool ok = false;
         for (size_t i = 0; i < n; i++)
         {
            if (isdigit((unsigned char)txt[i]))
            {
               ok = true;
               break;
            }
         }
         // if digit not found
         if (!ok)
         {
            gtk_widget_add_css_class(cfg->widget, "error");
            if (cfg->on_invalid)
               cfg->on_invalid(cfg->widget, "must contain digit", cfg->user_data);
            return FALSE;
         }
      }
      // Check uppercase requirement
      if (cfg->policy.require_upper)
      {
         bool ok = false;
         for (size_t i = 0; i < n; i++)
         {
            if (isupper((unsigned char)txt[i]))
            {
               ok = true;
               break;
            }
         }
         if (!ok)
         {
            gtk_widget_add_css_class(cfg->widget, "error");
            if (cfg->on_invalid)
               cfg->on_invalid(cfg->widget, "must contain uppercase", cfg->user_data);
            return FALSE;
         }
      }
      // Check symbol requirement
      if (cfg->policy.require_symbol)
      {
         bool ok = false;
         for (size_t i = 0; i < n; i++)
         {
            if (!isalnum((unsigned char)txt[i]))
            {
               ok = true;
               break;
            }
         }
         if (!ok)
         {
            gtk_widget_add_css_class(cfg->widget, "error");
            if (cfg->on_invalid)
               cfg->on_invalid(cfg->widget, "must contain symbol", cfg->user_data);
            return FALSE;
         }
      }
   }

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
   cfg->required = false;
   cfg->policy.min_len = 0;
   cfg->policy.require_digit = false;
   cfg->policy.require_upper = false;
   cfg->policy.require_symbol = false;

   cfg->reveal_toggle = true;
   cfg->sensitive = true;

   cfg->style.bg_normal = "white";
   cfg->style.fg_normal = "#2c3e50";
   cfg->style.epaisseur_bordure = 1;
   cfg->style.couleur_bordure = "#bdc3c7";
   cfg->style.rayon_arrondi = 4;
   cfg->style.gras = false;
   cfg->style.italique = false;
   cfg->style.taille_texte_px = 0;
}

GtkWidget *champ_motdepasse_creer(ChampMotDePasse *cfg)
{
   if (!cfg)
      return NULL;

   cfg->widget = gtk_password_entry_new();
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "champ_pwd");

   if (cfg->placeholder)
      g_object_set(cfg->widget, "placeholder-text", cfg->placeholder, NULL);

   gtk_password_entry_set_show_peek_icon(GTK_PASSWORD_ENTRY(cfg->widget), cfg->reveal_toggle);
   gtk_widget_set_sensitive(cfg->widget, cfg->sensitive);

   g_signal_connect(cfg->widget, "changed", G_CALLBACK(on_pw_changed), cfg);
   g_signal_connect(cfg->widget, "activate", G_CALLBACK(on_pw_activate), cfg);

   champ_pw_apply_css(cfg);
   champ_pw_validate(cfg);

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

void champ_motdepasse_set_required(ChampMotDePasse *cfg, bool required)
{
   if (!cfg)
      return;
   cfg->required = required;
   if (cfg->widget)
      champ_pw_validate(cfg);
}

void champ_motdepasse_set_reveal_toggle(ChampMotDePasse *cfg, bool reveal)
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
