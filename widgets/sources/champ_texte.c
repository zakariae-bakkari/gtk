#include "../headers/champ_texte.h"
#include <string.h>
#include <glib.h>

// --- Internal helpers ---
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

   snprintf(css, sizeof(css),
            "entry#%s {\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "%s" // border
            "  border-radius: %dpx;\n"
            "  padding: 6px 10px;\n"
            "}\n"
            "entry#%s:focus {\n"
            "  background-color: %s;\n"
            "}\n"
            "entry#%s text {\n"
            "  color: %s;\n"
            "  font-weight: %s;\n"
            "}\n"
            "entry.error {\n"
            "  border: 1px solid #e74c3c;\n"
            "}\n",
            cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,
            cfg->id_css,
            cfg->style.bg_focus ? cfg->style.bg_focus : (cfg->style.bg_normal ? cfg->style.bg_normal : "white"),
            cfg->id_css,
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            cfg->style.gras ? "bold" : "normal");

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);
   g_object_unref(provider);
}

static gboolean champ_texte_validate_now(ChampTexte *cfg)
{
   if (!cfg || !cfg->widget)
      return TRUE;
   const char *txt = gtk_editable_get_text(GTK_EDITABLE(cfg->widget));

   // required check
   if (cfg->required)
   {
      if (!txt || *txt == '\0')
      {
         gtk_widget_add_css_class(cfg->widget, "error");
         if (cfg->on_invalid)
            cfg->on_invalid(cfg->widget, "value is required", cfg->user_data);
         return FALSE;
      }
   }

   // regex check
   if (cfg->regex_pattern && txt)
   {
      GError *err = NULL;
      GRegex *rx = g_regex_new(cfg->regex_pattern, G_REGEX_OPTIMIZE, 0, &err);
      gboolean ok = TRUE;
      if (rx)
      {
         ok = g_regex_match(rx, txt, 0, NULL);
         g_regex_unref(rx);
      }
      if (!ok)
      {
         gtk_widget_add_css_class(cfg->widget, "error");
         if (cfg->on_invalid)
            cfg->on_invalid(cfg->widget, "invalid format", cfg->user_data);
         return FALSE;
      }
      if (err)
         g_error_free(err);
   }

   gtk_widget_remove_css_class(cfg->widget, "error");
   return TRUE;
}

static void on_changed_forward(GtkEditable *editable, gpointer user_data)
{
   ChampTexte *cfg = (ChampTexte *)user_data;
   if (!cfg) return; // Safety check
   champ_texte_validate_now(cfg);
   if (cfg->on_change)
      cfg->on_change(editable, cfg->user_data);
}

static void on_activate_forward(GtkEntry *entry, gpointer user_data)
{
   ChampTexte *cfg = (ChampTexte *)user_data;
   if (!cfg) return; // Safety check
   champ_texte_validate_now(cfg);
   if (cfg->on_activate)
      cfg->on_activate(entry, cfg->user_data);
}

// --- Public API ---
void champ_texte_initialiser(ChampTexte *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(ChampTexte));
   cfg->id_css = "champ_texte";
   cfg->texte = NULL;
   cfg->placeholder = NULL;
   cfg->max_length = 0; // unlimited
   cfg->required = false;
   cfg->regex_pattern = NULL;
   cfg->read_only = false;
   cfg->sensitive = true;

   cfg->style.bg_normal = "white";
   cfg->style.bg_focus = "#f7f9fc";
   cfg->style.fg_normal = "#2c3e50";
   cfg->style.placeholder_color = "#95a5a6";
   cfg->style.epaisseur_bordure = 1;
   cfg->style.couleur_bordure = "#bdc3c7";
   cfg->style.rayon_arrondi = 4;
   cfg->style.gras = false;
   cfg->style.italique = false;
   cfg->style.taille_texte_px = 0;
}

GtkWidget *champ_texte_creer(ChampTexte *cfg)
{
   if (!cfg)
      return NULL;

   // Copier la configuration pour la stocker avec le widget
   ChampTexte *cfg_copy = g_new0(ChampTexte, 1);
   memcpy(cfg_copy, cfg, sizeof(ChampTexte));

   cfg_copy->widget = gtk_entry_new();
   gtk_widget_set_name(cfg_copy->widget, cfg_copy->id_css ? cfg_copy->id_css : "champ_texte");

   if (cfg_copy->texte)
      gtk_editable_set_text(GTK_EDITABLE(cfg_copy->widget), cfg_copy->texte);
   if (cfg_copy->placeholder)
      gtk_entry_set_placeholder_text(GTK_ENTRY(cfg_copy->widget), cfg_copy->placeholder);
   if (cfg_copy->max_length > 0)
      gtk_entry_set_max_length(GTK_ENTRY(cfg_copy->widget), cfg_copy->max_length);

   gtk_editable_set_editable(GTK_EDITABLE(cfg_copy->widget), !cfg_copy->read_only);
   gtk_widget_set_sensitive(cfg_copy->widget, cfg_copy->sensitive);

   // Associer la copie de la configuration au widget
   g_object_set_data(G_OBJECT(cfg_copy->widget), "champ_texte_cfg", cfg_copy);

   // Signals
   g_signal_connect(cfg_copy->widget, "changed", G_CALLBACK(on_changed_forward), cfg_copy);
   g_signal_connect(cfg_copy->widget, "activate", G_CALLBACK(on_activate_forward), cfg_copy);
   // Libérer la mémoire de la copie lors de la destruction du widget
   g_signal_connect(cfg_copy->widget, "destroy", G_CALLBACK(g_free), cfg_copy);

   // Apply CSS styling
   champ_texte_apply_css(cfg_copy);

   // Initial validation state
   champ_texte_validate_now(cfg_copy);

   // Important: Mettre à jour le pointeur du widget dans la config d'origine
   // pour que l'appelant puisse y accéder s'il en a besoin.
   cfg->widget = cfg_copy->widget;

   return cfg_copy->widget;
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
   if (!cfg || !cfg->widget)
      return;
   gtk_entry_set_placeholder_text(GTK_ENTRY(cfg->widget), ph);
}

void champ_texte_set_max_length(ChampTexte *cfg, int max_len)
{
   if (!cfg || !cfg->widget)
      return;
   gtk_entry_set_max_length(GTK_ENTRY(cfg->widget), max_len);
}

void champ_texte_set_read_only(ChampTexte *cfg, bool ro)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->read_only = ro;
   gtk_editable_set_editable(GTK_EDITABLE(cfg->widget), !ro);
}

void champ_texte_set_required(ChampTexte *cfg, bool required)
{
   if (!cfg)
      return;
   cfg->required = required;
   if (cfg->widget)
      champ_texte_validate_now(cfg);
}

void champ_texte_set_regex(ChampTexte *cfg, const char *pattern)
{
   if (!cfg)
      return;
   cfg->regex_pattern = (char *)pattern;
   if (cfg->widget)
      champ_texte_validate_now(cfg);
}
