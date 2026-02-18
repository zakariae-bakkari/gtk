#include "../headers/champ_nombre.h"
#include <string.h>

static void champ_nombre_apply_css(ChampNombre *cfg)
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
            "spinbutton#%s {\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "%s"
            "  border-radius: %dpx;\n"
            "  padding: 2px 6px;\n"
            "}\n"
            "spinbutton#%s:focus {\n"
            "  background-color: %s;\n"
            "}\n",
            cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,
            cfg->id_css,
            cfg->style.bg_focus ? cfg->style.bg_focus : (cfg->style.bg_normal ? cfg->style.bg_normal : "white"));

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);
   g_object_unref(provider);
}

static void on_spin_value_changed(GtkSpinButton *spin, gpointer user_data)
{
   ChampNombre *cfg = (ChampNombre *)user_data;
   double v = gtk_spin_button_get_value(spin);
   if (v < cfg->min)
      gtk_spin_button_set_value(spin, cfg->min);
   else if (v > cfg->max)
      gtk_spin_button_set_value(spin, cfg->max);
   if (cfg->on_change)
      cfg->on_change(spin, cfg->user_data);
}

void champ_nombre_initialiser(ChampNombre *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(ChampNombre));
   cfg->id_css = "champ_nombre";
   cfg->min = 0.0;
   cfg->max = 100.0;
   cfg->step = 1.0;
   cfg->digits = 0;
   cfg->wrap = FALSE;
   cfg->valeur = 0.0;
   cfg->required = FALSE;

   cfg->style.bg_normal = "white";
   cfg->style.bg_focus = "#f7f9fc";
   cfg->style.fg_normal = "#2c3e50";
   cfg->style.epaisseur_bordure = 1;
   cfg->style.couleur_bordure = "#bdc3c7";
   cfg->style.rayon_arrondi = 4;
   cfg->style.gras = FALSE;
   cfg->style.italique = FALSE;
   cfg->style.taille_texte_px = 0;
}

GtkWidget *champ_nombre_creer(ChampNombre *cfg)
{
   if (!cfg)
      return NULL;

   GtkAdjustment *adj = gtk_adjustment_new(cfg->valeur, cfg->min, cfg->max, cfg->step, cfg->step * 10, 0);
   cfg->widget = gtk_spin_button_new(adj, cfg->step, cfg->digits);

   gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(cfg->widget), cfg->wrap);
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "champ_nombre");

   g_signal_connect(cfg->widget, "value-changed", G_CALLBACK(on_spin_value_changed), cfg);

   champ_nombre_apply_css(cfg);

   return cfg->widget;
}

double champ_nombre_get_valeur(ChampNombre *cfg)
{
   if (!cfg || !cfg->widget)
      return 0.0;
   return gtk_spin_button_get_value(GTK_SPIN_BUTTON(cfg->widget));
}

void champ_nombre_set_valeur(ChampNombre *cfg, double v)
{
   if (!cfg || !cfg->widget)
      return;
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(cfg->widget), v);
}

void champ_nombre_set_bornes(ChampNombre *cfg, double min, double max)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->min = min;
   cfg->max = max;
   GtkAdjustment *adj = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(cfg->widget));
   gtk_adjustment_set_lower(adj, min);
   gtk_adjustment_set_upper(adj, max);
}

void champ_nombre_set_step(ChampNombre *cfg, double step)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->step = step;
   gtk_spin_button_set_increments(GTK_SPIN_BUTTON(cfg->widget), step, step * 10);
}

void champ_nombre_set_digits(ChampNombre *cfg, guint digits)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->digits = digits;
   gtk_spin_button_set_digits(GTK_SPIN_BUTTON(cfg->widget), digits);
}

void champ_nombre_set_wrap(ChampNombre *cfg, gboolean wrap)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->wrap = wrap;
   gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(cfg->widget), wrap);
}
