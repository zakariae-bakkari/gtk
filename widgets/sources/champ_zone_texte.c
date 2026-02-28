#include "../headers/champ_zone_texte.h"
#include <string.h>

static void champ_zt_apply_css(ChampZoneTexte *cfg)
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
            "textview#%s {\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "%s"
            "  border-radius: %dpx;\n"
            "  padding: 6px 8px;\n"
            "}\n"
            "textview#%s text {\n"
            "  color: %s;\n"
            "  font-weight: %s;\n"
            "}\n"
            "textview.error {\n"
            "  border: 1px solid %s;\n"
            "  background: %s;\n"
            "}\n",
            cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,
            cfg->id_css,
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            cfg->style.gras ? "bold" : "normal",
            cfg->style.couleur_bordure_error ? cfg->style.couleur_bordure_error : "#e74c3c",
            cfg->style.bg_error ? cfg->style.bg_error : "#fff1f2");

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);
   g_object_unref(provider);
}

static gboolean champ_zt_validate_now(ChampZoneTexte *cfg)
{
   if (!cfg || !cfg->widget)
      return TRUE;
   GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cfg->widget));
   GtkTextIter start, end;
   gtk_text_buffer_get_start_iter(buf, &start);
   gtk_text_buffer_get_end_iter(buf, &end);
   char *txt = gtk_text_buffer_get_text(buf, &start, &end, FALSE);

   gboolean ok = TRUE;
   if (cfg->required)
   {
      if (!txt || *txt == '\0')
         ok = FALSE;
   }
   if (cfg->max_length > 0 && txt)
   {
      if ((int)strlen(txt) > cfg->max_length)
         ok = FALSE;
   }

   if (!ok)
   {
      gtk_widget_add_css_class(cfg->widget, "error");
      if (cfg->on_invalid)
         cfg->on_invalid(cfg->widget, "invalid value", cfg->user_data);
   }
   else
   {
      gtk_widget_remove_css_class(cfg->widget, "error");
   }

   if (txt)
      g_free(txt);
   return ok;
}

static void on_textbuffer_changed(GtkTextBuffer *buffer, gpointer user_data)
{
   ChampZoneTexte *cfg = (ChampZoneTexte *)user_data;
   (void)buffer;
   champ_zt_validate_now(cfg);
   if (cfg->on_change)
      cfg->on_change(buffer, cfg->user_data);
}

void champ_zone_texte_initialiser(ChampZoneTexte *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(ChampZoneTexte));
   cfg->id_css = "champ_zone_texte";
   cfg->texte = NULL;
   cfg->max_length = 0;
   cfg->wrap_word = true;
   cfg->sensitive = true;
   cfg->required = false;

   // Initialize style using common function
   widget_style_init(&cfg->style);
   // Override defaults for text area
   cfg->style.bg_normal = g_strdup("white");
   cfg->style.fg_normal = g_strdup("#2c3e50");
   cfg->style.couleur_bordure = g_strdup("#bdc3c7");
   cfg->style.rayon_arrondi = 4;
}

GtkWidget *champ_zone_texte_creer(ChampZoneTexte *cfg)
{
   if (!cfg)
      return NULL;

   cfg->widget = gtk_text_view_new();
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "champ_zone_texte");

   GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cfg->widget));
   if (cfg->texte)
      gtk_text_buffer_set_text(buf, cfg->texte, -1);

   gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cfg->widget), cfg->wrap_word ? GTK_WRAP_WORD : GTK_WRAP_CHAR);
   gtk_widget_set_sensitive(cfg->widget, cfg->sensitive);

   g_signal_connect(buf, "changed", G_CALLBACK(on_textbuffer_changed), cfg);

   champ_zt_apply_css(cfg);
   champ_zt_validate_now(cfg);

   return cfg->widget;
}

char *champ_zone_texte_get_texte(ChampZoneTexte *cfg)
{
   if (!cfg || !cfg->widget)
      return NULL;
   GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cfg->widget));
   GtkTextIter s, e;
   gtk_text_buffer_get_start_iter(buf, &s);
   gtk_text_buffer_get_end_iter(buf, &e);
   return gtk_text_buffer_get_text(buf, &s, &e, FALSE); // caller must g_free
}

void champ_zone_texte_set_texte(ChampZoneTexte *cfg, const char *texte)
{
   if (!cfg || !cfg->widget)
      return;
   GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cfg->widget));
   gtk_text_buffer_set_text(buf, texte ? texte : "", -1);
}

void champ_zone_texte_set_max_length(ChampZoneTexte *cfg, int max_len)
{
   if (!cfg)
      return;
   cfg->max_length = max_len;
   if (cfg->widget)
      champ_zt_validate_now(cfg);
}

void champ_zone_texte_set_wrap_word(ChampZoneTexte *cfg, bool wrap_word)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->wrap_word = wrap_word;
   gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cfg->widget), wrap_word ? GTK_WRAP_WORD : GTK_WRAP_CHAR);
}

void champ_zone_texte_set_sensitive(ChampZoneTexte *cfg, bool sensitive)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->sensitive = sensitive;
   gtk_widget_set_sensitive(cfg->widget, sensitive);
}

void champ_zone_texte_set_required(ChampZoneTexte *cfg, bool required)
{
   if (!cfg)
      return;
   cfg->required = required;
   if (cfg->widget)
      champ_zt_validate_now(cfg);
}
