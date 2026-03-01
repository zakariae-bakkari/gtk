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
            "spinbutton.error {\n"
            "  border: %dpx solid %s;\n"
            "  background: %s;\n"
            "}\n",
            cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,
            cfg->style.epaisseur_bordure > 0 ? cfg->style.epaisseur_bordure : 1,
            cfg->style.couleur_bordure_error ? cfg->style.couleur_bordure_error : "#e74c3c",
            cfg->style.bg_error ? cfg->style.bg_error : "#fff1f2");

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);
   g_object_unref(provider);
}

static gboolean champ_nombre_validate(ChampNombre *cfg)
{
   if (!cfg || !cfg->widget)
      return FALSE;

   double val = gtk_spin_button_get_value(GTK_SPIN_BUTTON(cfg->widget));

   // Check if value is within bounds (this should already be enforced by GtkSpinButton)
   if (val < cfg->min || val > cfg->max)
   {
      gtk_widget_add_css_class(cfg->widget, "error");
      if (cfg->on_invalid)
         cfg->on_invalid(cfg->widget, "value out of range", cfg->user_data);
      return FALSE;
   }

   // For numeric fields, we could add more validation rules here if needed
   // For now, GtkSpinButton already handles most validation

   gtk_widget_remove_css_class(cfg->widget, "error");
   return TRUE;
}

static void on_spin_value_changed(GtkSpinButton *spin, gpointer user_data)
{
   ChampNombre *cfg = (ChampNombre *)user_data;
   double v = gtk_spin_button_get_value(spin);

   // Ensure value stays within bounds
   if (v < cfg->min)
      gtk_spin_button_set_value(spin, cfg->min);
   else if (v > cfg->max)
      gtk_spin_button_set_value(spin, cfg->max);

   champ_nombre_validate(cfg);

   if (cfg->on_change)
      cfg->on_change(GTK_EDITABLE(spin), cfg->user_data);
}

static void on_spin_activate(GtkEntry *entry, gpointer user_data)
{
   ChampNombre *cfg = (ChampNombre *)user_data;
   champ_nombre_validate(cfg);
   if (cfg->on_activate)
      cfg->on_activate(entry, cfg->user_data);
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
   cfg->size.width = 0;  // 0 = full width (100%)
   cfg->size.height = 0; // 0 = auto size

   // Initialize style using common function
   widget_style_init(&cfg->style);
   // Override defaults for number field
   cfg->style.bg_normal = g_strdup("white");
   cfg->style.fg_normal = g_strdup("#2c3e50");
   cfg->style.couleur_bordure = g_strdup("#bdc3c7");
   cfg->style.couleur_bordure_error = g_strdup("#e74c3c");
   cfg->style.bg_error = g_strdup("#fff1f2");
   cfg->style.rayon_arrondi = 4;
}

GtkWidget *champ_nombre_creer(ChampNombre *cfg)
{
   if (!cfg)
      return NULL;

   GtkAdjustment *adj = gtk_adjustment_new(cfg->valeur, cfg->min, cfg->max, cfg->step, cfg->step * 10, 0);
   cfg->widget = gtk_spin_button_new(adj, cfg->step, cfg->digits);

   gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(cfg->widget), cfg->wrap);
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "champ_nombre");

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

   // Spin buttons typically don't need vertical expansion
   gtk_widget_set_vexpand(cfg->widget, FALSE);
   gtk_widget_set_valign(cfg->widget, GTK_ALIGN_START);

   g_signal_connect(cfg->widget, "value-changed", G_CALLBACK(on_spin_value_changed), cfg);
   g_signal_connect(cfg->widget, "activate", G_CALLBACK(on_spin_activate), cfg);

   champ_nombre_apply_css(cfg);
   champ_nombre_validate(cfg);

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

void champ_nombre_set_callbacks(ChampNombre *cfg, WidgetOnChange on_change, WidgetOnActivate on_activate, WidgetOnInvalid on_invalid, gpointer user_data)
{
   if (!cfg)
      return;
   cfg->on_change = on_change;
   cfg->on_activate = on_activate;
   cfg->on_invalid = on_invalid;
   cfg->user_data = user_data;
}

void champ_nombre_set_style(ChampNombre *cfg, const WidgetStyle *style)
{
   if (!cfg || !style)
      return;
   widget_style_copy(&cfg->style, style);
}

void champ_nombre_apply_style(ChampNombre *cfg)
{
   if (!cfg)
      return;
   champ_nombre_apply_css(cfg);
}

void champ_nombre_set_size(ChampNombre *cfg, int width, int height)
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

void champ_nombre_free(ChampNombre *cfg)
{
   if (!cfg)
      return;

   // Free the style structure
   widget_style_free(&cfg->style);

   // Clear the structure
   memset(cfg, 0, sizeof(ChampNombre));
}
