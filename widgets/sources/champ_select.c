#include "../headers/champ_select.h"
#include <string.h>

static void champ_select_apply_css(ChampSelect *cfg)
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
            "dropdown#%s {\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "%s"
            "  border-radius: %dpx;\n"
            "  padding: 4px 8px;\n"
            "}\n",
            cfg->id_css,
            cfg->style.bg_normal ? cfg->style.bg_normal : "white",
            cfg->style.fg_normal ? cfg->style.fg_normal : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi);

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);
   g_object_unref(provider);
}

static gboolean champ_select_validate(GtkDropDown *dd, ChampSelect *cfg)
{
   if (!cfg || !dd)
      return TRUE;
   int sel = (int)gtk_drop_down_get_selected(dd);
   if (cfg->required && sel < 0)
   {
      if (cfg->on_invalid)
         cfg->on_invalid(GTK_WIDGET(dd), "selection required", cfg->user_data);
      return FALSE;
   }
   return TRUE;
}

static void on_dropdown_notify_selected(GObject *obj, GParamSpec *pspec, gpointer user_data)
{
   (void)pspec;
   ChampSelect *cfg = (ChampSelect *)user_data;
   if (!GTK_IS_DROP_DOWN(obj))
   {
      // Defensive: ignore unexpected emitter types
      return;
   }
   GtkDropDown *dd = GTK_DROP_DOWN(obj);
   cfg->selected_index = (int)gtk_drop_down_get_selected(dd);
   champ_select_validate(dd, cfg);
   if (cfg->on_change)
      cfg->on_change(dd, cfg->user_data);
}

void champ_select_initialiser(ChampSelect *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(ChampSelect));
   cfg->id_css = "champ_select";
   cfg->model = NULL;
   cfg->selected_index = -1;
   cfg->required = false;
   cfg->enable_search = false;

   cfg->style.bg_normal = "white";
   cfg->style.fg_normal = "#2c3e50";
   cfg->style.epaisseur_bordure = 1;
   cfg->style.couleur_bordure = "#bdc3c7";
   cfg->style.rayon_arrondi = 4;
   cfg->style.gras = false;
   cfg->style.italique = false;
   cfg->style.taille_texte_px = 0;
}

GtkWidget *champ_select_creer(ChampSelect *cfg)
{
   if (!cfg)
      return NULL;

   if (!cfg->model)
   {
      cfg->model = gtk_string_list_new(NULL);
   }

   cfg->widget = gtk_drop_down_new(G_LIST_MODEL(cfg->model), NULL);
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "champ_select");

   if (cfg->selected_index >= 0)
      gtk_drop_down_set_selected(GTK_DROP_DOWN(cfg->widget), (guint)cfg->selected_index);

   // Listen for selection changes on the dropdown object itself
   g_signal_connect(cfg->widget, "notify::selected", G_CALLBACK(on_dropdown_notify_selected), cfg);

   champ_select_apply_css(cfg);
   champ_select_validate(GTK_DROP_DOWN(cfg->widget), cfg);

   return cfg->widget;
}

void champ_select_add_item(ChampSelect *cfg, const char *label)
{
   if (!cfg)
      return;
   if (!cfg->model)
      cfg->model = gtk_string_list_new(NULL);
   if (label)
      gtk_string_list_append(cfg->model, label);
}

void champ_select_set_items(ChampSelect *cfg, const char **labels, int count)
{
   if (!cfg)
      return;
   if (cfg->model)
      g_object_unref(cfg->model);
   cfg->model = gtk_string_list_new(NULL);
   for (int i = 0; i < count; ++i)
   {
      if (labels[i])
         gtk_string_list_append(cfg->model, labels[i]);
   }
   if (cfg->widget)
      gtk_drop_down_set_model(GTK_DROP_DOWN(cfg->widget), G_LIST_MODEL(cfg->model));
}

int champ_select_get_index(ChampSelect *cfg)
{
   if (!cfg || !cfg->widget)
      return -1;
   return (int)gtk_drop_down_get_selected(GTK_DROP_DOWN(cfg->widget));
}

void champ_select_set_index(ChampSelect *cfg, int index)
{
   if (!cfg || !cfg->widget)
      return;
   gtk_drop_down_set_selected(GTK_DROP_DOWN(cfg->widget), (guint)index);
}

const char *champ_select_get_string(ChampSelect *cfg)
{
   if (!cfg || !cfg->widget || !cfg->model)
      return NULL;
   guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(cfg->widget));
   if (sel == GTK_INVALID_LIST_POSITION)
      return NULL;
   return gtk_string_list_get_string(cfg->model, sel);
}
