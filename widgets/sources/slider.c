#include "../headers/slider.h"
#include <stdlib.h>
#include <string.h>

// ====================== CSS ======================

static void slider_apply_css(Slider *cfg)
{
   if (!cfg || !cfg->widget || !cfg->id_css)
      return;

   GtkCssProvider *provider = gtk_css_provider_new();
   char css[2048];

   const char *trough_color = cfg->style.couleur_bordure ? cfg->style.couleur_bordure : "#bdc3c7";
   const char *highlight_color = cfg->style.bg_normal ? cfg->style.bg_normal : "#3498db";
   const char *handle_color = cfg->style.fg_normal ? cfg->style.fg_normal : "#2980b9";

   snprintf(css, sizeof(css),
            /* --- Piste (fond) --- */
            "scale#%s trough {\n"
            "  background-color: %s;\n"
            "  border-radius: %dpx;\n"
            "  min-height: 4px;\n"
            "  min-width:  4px;\n"
            "}\n"
            /* --- Partie remplie (highlight) --- */
            "scale#%s trough highlight {\n"
            "  background-color: %s;\n"
            "  border-radius: %dpx;\n"
            "}\n"
            /* --- Curseur (handle) --- */
            "scale#%s slider {\n"
            "  background-color: %s;\n"
            "  border-radius: 50%%;\n"
            "  min-width:  16px;\n"
            "  min-height: 16px;\n"
            "  border: 2px solid %s;\n"
            "}\n"
            /* --- Curseur au survol --- */
            "scale#%s slider:hover {\n"
            "  background-color: %s;\n"
            "}\n"
            /* --- Label valeur --- */
            "label#%s_valeur {\n"
            "  color: %s;\n"
            "  font-size: %dpx;\n"
            "  margin-top: 2px;\n"
            "}\n",
            /* trough */
            cfg->id_css, trough_color, cfg->style.rayon_arrondi,
            /* highlight */
            cfg->id_css, highlight_color, cfg->style.rayon_arrondi,
            /* slider handle */
            cfg->id_css, handle_color, trough_color,
            /* slider hover */
            cfg->id_css, highlight_color,
            /* label valeur */
            cfg->id_css,
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            cfg->style.taille_texte_px > 0 ? cfg->style.taille_texte_px : 11);

   gtk_css_provider_load_from_string(provider, css);

   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);

   if (cfg->label_valeur)
      gtk_style_context_add_provider(
          gtk_widget_get_style_context(cfg->label_valeur),
          GTK_STYLE_PROVIDER(provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);

   g_object_unref(provider);
}

// ====================== HELPERS ======================

static void slider_apply_marques(Slider *cfg)
{
   if (!cfg || !cfg->widget)
      return;

   gtk_scale_clear_marks(GTK_SCALE(cfg->widget));

   if (cfg->marques_step <= 0.0 || cfg->marques_pos == SLIDER_MARQUES_AUCUNE)
      return;

   GtkPositionType pos;
   switch (cfg->marques_pos)
   {
   case SLIDER_MARQUES_DESSUS:
      pos = GTK_POS_TOP;
      break;
   case SLIDER_MARQUES_DESSOUS:
      pos = GTK_POS_BOTTOM;
      break;
   default:
      pos = GTK_POS_BOTTOM;
      break;
   }

   for (double v = cfg->min; v <= cfg->max + 1e-9; v += cfg->marques_step)
   {
      if (cfg->marques_pos == SLIDER_MARQUES_LES_DEUX)
      {
         gtk_scale_add_mark(GTK_SCALE(cfg->widget), v, GTK_POS_TOP, NULL);
         gtk_scale_add_mark(GTK_SCALE(cfg->widget), v, GTK_POS_BOTTOM, NULL);
      }
      else
      {
         gtk_scale_add_mark(GTK_SCALE(cfg->widget), v, pos, NULL);
      }
   }
}

static void slider_update_label(Slider *cfg, double valeur)
{
   if (!cfg || !cfg->label_valeur || !cfg->afficher_label)
      return;

   char buf[64];
   if (cfg->digits == 0)
      snprintf(buf, sizeof(buf), "%.0f", valeur);
   else
      snprintf(buf, sizeof(buf), "%.*f", (int)cfg->digits, valeur);

   gtk_label_set_text(GTK_LABEL(cfg->label_valeur), buf);
}

// ====================== CALLBACKS INTERNES ======================

static void on_slider_value_changed(GtkRange *range, gpointer user_data)
{
   Slider *cfg = (Slider *)user_data;
   double val = gtk_range_get_value(range);

   slider_update_label(cfg, val);

   if (cfg->on_change)
      cfg->on_change(range, val, cfg->user_data);
}

// ====================== FONCTIONS PUBLIQUES ======================

void slider_initialiser(Slider *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(Slider));

   cfg->id_css = malloc(strlen("slider") + 1);
   strcpy(cfg->id_css, "slider");
   cfg->min = 0.0;
   cfg->max = 100.0;
   cfg->step = 1.0;
   cfg->valeur = 0.0;
   cfg->digits = 0;

   cfg->orientation = SLIDER_HORIZONTAL;
   cfg->afficher_valeur = TRUE;
   cfg->afficher_label = FALSE;
   cfg->inverser = FALSE;
   cfg->sensitive = TRUE;

   cfg->marques_pos = SLIDER_MARQUES_AUCUNE;
   cfg->marques_step = 0.0;

   cfg->size.width = 0;
   cfg->size.height = 0;

   widget_style_init(&cfg->style);
   cfg->style.bg_normal = malloc(strlen("#3498db") + 1);
   strcpy(cfg->style.bg_normal, "#3498db");
   cfg->style.fg_normal = malloc(strlen("#2980b9") + 1);
   strcpy(cfg->style.fg_normal, "#2980b9");
   cfg->style.couleur_bordure = malloc(strlen("#bdc3c7") + 1);
   strcpy(cfg->style.couleur_bordure, "#bdc3c7");
   cfg->style.rayon_arrondi = 4;
}

/**
 * Crée le widget et retourne cfg->container (GtkBox).
 * Structure interne du container :
 *   GtkBox (vertical, spacing=2)
 *   ├── GtkScale       (cfg->widget)
 *   └── GtkLabel       (cfg->label_valeur, caché si afficher_label = FALSE)
 */
GtkWidget *slider_creer(Slider *cfg)
{
   if (!cfg)
      return NULL;

   /* --- Orientation GTK --- */
   GtkOrientation gtk_orient = (cfg->orientation == SLIDER_VERTICAL)
                                   ? GTK_ORIENTATION_VERTICAL
                                   : GTK_ORIENTATION_HORIZONTAL;

   /* --- Container --- */
   cfg->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
   gtk_widget_set_hexpand(cfg->container, cfg->size.width == 0 ? TRUE : FALSE);
   gtk_widget_set_halign(cfg->container, cfg->size.width == 0 ? GTK_ALIGN_FILL : GTK_ALIGN_START);

   /* --- GtkScale --- */
   GtkAdjustment *adj = gtk_adjustment_new(cfg->valeur, cfg->min, cfg->max,
                                           cfg->step, cfg->step * 10, 0);

   cfg->widget = gtk_scale_new(gtk_orient, adj);
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "slider");

   gtk_scale_set_digits(GTK_SCALE(cfg->widget), (int)cfg->digits);
   gtk_scale_set_draw_value(GTK_SCALE(cfg->widget), cfg->afficher_valeur);
   gtk_range_set_inverted(GTK_RANGE(cfg->widget), cfg->inverser);
   gtk_widget_set_sensitive(cfg->widget, cfg->sensitive);

   /* --- Taille --- */
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
   gtk_widget_set_valign(cfg->widget, GTK_ALIGN_CENTER);

   /* --- Marques --- */
   slider_apply_marques(cfg);

   /* --- Label valeur séparé --- */
   cfg->label_valeur = gtk_label_new("");

   char lbl_id[256];
   snprintf(lbl_id, sizeof(lbl_id), "%s_valeur", cfg->id_css ? cfg->id_css : "slider");
   gtk_widget_set_name(cfg->label_valeur, lbl_id);

   gtk_label_set_xalign(GTK_LABEL(cfg->label_valeur), 0.5f);
   gtk_widget_set_hexpand(cfg->label_valeur, TRUE);
   gtk_widget_set_visible(cfg->label_valeur, cfg->afficher_label);
   slider_update_label(cfg, cfg->valeur);

   /* --- Assemblage --- */
   gtk_box_append(GTK_BOX(cfg->container), cfg->widget);
   gtk_box_append(GTK_BOX(cfg->container), cfg->label_valeur);

   /* --- Signal --- */
   g_signal_connect(cfg->widget, "value-changed", G_CALLBACK(on_slider_value_changed), cfg);

   /* --- CSS --- */
   slider_apply_css(cfg);

   return cfg->container;
}

double slider_get_valeur(Slider *cfg)
{
   if (!cfg || !cfg->widget)
      return 0.0;
   return gtk_range_get_value(GTK_RANGE(cfg->widget));
}

void slider_set_valeur(Slider *cfg, double valeur)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->valeur = valeur;
   gtk_range_set_value(GTK_RANGE(cfg->widget), valeur);
   slider_update_label(cfg, valeur);
}

void slider_set_bornes(Slider *cfg, double min, double max)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->min = min;
   cfg->max = max;
   GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(cfg->widget));
   gtk_adjustment_set_lower(adj, min);
   gtk_adjustment_set_upper(adj, max);
   slider_apply_marques(cfg);
}

void slider_set_step(Slider *cfg, double step)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->step = step;
   GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(cfg->widget));
   gtk_adjustment_set_step_increment(adj, step);
   gtk_adjustment_set_page_increment(adj, step * 10);
}

void slider_set_digits(Slider *cfg, guint digits)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->digits = digits;
   gtk_scale_set_digits(GTK_SCALE(cfg->widget), (int)digits);
}

void slider_set_orientation(Slider *cfg, SliderOrientation orientation)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->orientation = orientation;
   gtk_orientable_set_orientation(
       GTK_ORIENTABLE(cfg->widget),
       orientation == SLIDER_VERTICAL ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL);
}

void slider_set_inverser(Slider *cfg, gboolean inverser)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->inverser = inverser;
   gtk_range_set_inverted(GTK_RANGE(cfg->widget), inverser);
}

void slider_set_sensitive(Slider *cfg, gboolean sensitive)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->sensitive = sensitive;
   gtk_widget_set_sensitive(cfg->widget, sensitive);
}

void slider_set_afficher_valeur(Slider *cfg, gboolean afficher)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->afficher_valeur = afficher;
   gtk_scale_set_draw_value(GTK_SCALE(cfg->widget), afficher);
}

void slider_set_marques(Slider *cfg, SliderMarquesPos pos, double step)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->marques_pos = pos;
   cfg->marques_step = step;
   slider_apply_marques(cfg);
}

void slider_set_size(Slider *cfg, int width, int height)
{
   if (!cfg || !cfg->widget)
      return;
   cfg->size.width = width;
   cfg->size.height = height;
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

void slider_free(Slider *cfg)
{
   if (!cfg)
      return;

   g_free(cfg->id_css);
   widget_style_free(&cfg->style);
   memset(cfg, 0, sizeof(Slider));
}
