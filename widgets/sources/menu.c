#include "../headers/menu.h"
#include <string.h>
#include <stdlib.h>

// ====================== FORWARD DECLARATIONS ======================

static void menu_apply_css(Menu *cfg);
static GtkWidget *menu_construire_item(Menu *cfg, MenuItem *item);

// ====================== CSS ======================

static void menu_apply_css(Menu *cfg)
{
   if (!cfg || !cfg->widget || !cfg->id_css)
      return;

   GtkCssProvider *provider = gtk_css_provider_new();
   char css[4096];
   char border_css[128] = "";
   char font_css[128] = "";
   char font_size[64] = "";

   if (cfg->style.epaisseur_bordure > 0)
      snprintf(border_css, sizeof(border_css),
               "  border: %dpx solid %s;\n",
               cfg->style.epaisseur_bordure,
               cfg->style.couleur_bordure ? cfg->style.couleur_bordure : "transparent");

   if (cfg->style.taille_texte_px > 0)
      snprintf(font_size, sizeof(font_size),
               "  font-size: %dpx;\n", cfg->style.taille_texte_px);

   snprintf(font_css, sizeof(font_css),
            "  font-weight: %s;\n%s",
            cfg->style.gras ? "bold" : "normal",
            font_size);

   snprintf(css, sizeof(css),
            /* --- Barre principale --- */
            "box#%s {\n"
            "  background-color: %s;\n"
            "%s"
            "  border-radius: %dpx;\n"
            "  padding: 4px;\n"
            "}\n"

            /* --- Item normal --- */
            "box#%s > button {\n"
            "  background-image: none;\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "  border: none;\n"
            "  box-shadow: none;\n"
            "  border-radius: %dpx;\n"
            "  padding: 6px 14px;\n"
            "  outline: none;\n"
            "%s"
            "}\n"

            /* --- Item au survol --- */
            "box#%s > button:hover {\n"
            "  background-image: none;\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "}\n"

            /* --- Item actif --- */
            "box#%s > button.actif {\n"
            "  background-image: none;\n"
            "  background-color: %s;\n"
            "  color: %s;\n"
            "}\n"

            /* --- Item désactivé --- */
            "box#%s > button:disabled {\n"
            "  opacity: 0.45;\n"
            "}\n"

            /* --- Label dans les items --- */
            "box#%s > button label {\n"
            "  color: inherit;\n"
            "}\n"

            /* --- Séparateur --- */
            "box#%s > separator {\n"
            "  background-color: %s;\n"
            "  min-width: 1px;\n"
            "  min-height: 1px;\n"
            "  margin: 4px 2px;\n"
            "}\n"

            /* --- Popover (sous-menu) --- */
            "popover > contents {\n"
            "  background-color: %s;\n"
            "  padding: 4px;\n"
            "  border-radius: 6px;\n"
            "}\n",

            /* barre */
            cfg->id_css,
            cfg->style.bg_barre ? cfg->style.bg_barre : "#2c3e50",
            border_css,
            cfg->style.rayon_arrondi,

            /* item normal */
            cfg->id_css,
            cfg->style.bg_item ? cfg->style.bg_item : "transparent",
            cfg->style.fg_item ? cfg->style.fg_item : "#ecf0f1",
            cfg->style.rayon_item,
            font_css,

            /* item hover */
            cfg->id_css,
            cfg->style.bg_item_hover ? cfg->style.bg_item_hover : "#3d5166",
            cfg->style.fg_item_hover ? cfg->style.fg_item_hover : "#ffffff",

            /* item actif */
            cfg->id_css,
            cfg->style.bg_item_actif ? cfg->style.bg_item_actif : "#2980b9",
            cfg->style.fg_item_hover ? cfg->style.fg_item_hover : "#ffffff",

            /* item désactivé */
            cfg->id_css,

            /* label */
            cfg->id_css,

            /* séparateur */
            cfg->id_css,
            cfg->style.couleur_separateur ? cfg->style.couleur_separateur : "#4a6278",

            /* popover */
            cfg->style.bg_popover ? cfg->style.bg_popover : "#2c3e50");

   gtk_css_provider_load_from_string(provider, css);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);

   /* Appliquer aussi au niveau global pour cibler les popovers et sous-boîtes */
   gtk_style_context_add_provider_for_display(
       gtk_widget_get_display(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);

   g_object_unref(provider);
}

// ====================== HELPERS INTERNES ======================

/**
 * Données passées au callback de clic d'un item.
 */
typedef struct
{
   Menu *menu;
   MenuItem *item;
} MenuClickData;

static void on_item_clicked(GtkButton *button, gpointer user_data)
{
   MenuClickData *d = (MenuClickData *)user_data;
   if (!d)
      return;

   /* Si l'item a un sous-menu, ouvrir/fermer le popover */
   if (d->item->popover)
   {
      if (gtk_widget_get_visible(d->item->popover))
         gtk_popover_popdown(GTK_POPOVER(d->item->popover));
      else
         gtk_popover_popup(GTK_POPOVER(d->item->popover));
      return;
   }

   /* Sinon déclencher le callback utilisateur */
   if (d->menu->on_click)
      d->menu->on_click(d->item->id, d->menu->user_data);
}

/**
 * Construit récursivement le GtkBox d'un sous-menu dans un GtkPopover.
 */
static GtkWidget *menu_construire_sous_box(Menu *cfg, MenuItem *parent)
{
   GtkOrientation orient =
       (parent->sous_menu_orientation == MENU_VERTICAL)
           ? GTK_ORIENTATION_VERTICAL
           : GTK_ORIENTATION_HORIZONTAL;

   GtkWidget *box = gtk_box_new(orient, cfg->espacement);

   for (int i = 0; i < parent->nb_sous_items; i++)
   {
      MenuItem *child = parent->sous_items[i];

      if (child->type == MENU_ITEM_SEPARATEUR)
      {
         GtkWidget *sep = gtk_separator_new(
             orient == GTK_ORIENTATION_VERTICAL
                 ? GTK_ORIENTATION_HORIZONTAL
                 : GTK_ORIENTATION_VERTICAL);
         gtk_box_append(GTK_BOX(box), sep);
         child->widget = sep;
         continue;
      }

      /* Créer le bouton du sous-item */
      GtkWidget *btn = gtk_button_new();
      GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
      gtk_widget_set_halign(inner, GTK_ALIGN_START);

      if (child->nom_icone)
      {
         GtkWidget *img = gtk_image_new_from_icon_name(child->nom_icone);
         gtk_box_append(GTK_BOX(inner), img);
      }
      gtk_box_append(GTK_BOX(inner), gtk_label_new(child->texte));

      if (child->nb_sous_items > 0)
      {
         // Si le parent est vertical, ses enfants ouvrent à droite → flèche droite
         // Si le parent est horizontal, ses enfants ouvrent en bas → flèche bas
         const char *arrow = (parent->sous_menu_orientation == MENU_VERTICAL)
             ? "pan-end-symbolic"    // →
             : "pan-down-symbolic";  // ↓
         GtkWidget *arrow_img = gtk_image_new_from_icon_name(arrow);
         gtk_box_append(GTK_BOX(inner), arrow_img);
      }
      gtk_button_set_child(GTK_BUTTON(btn), inner);

      if (child->tooltip)
         gtk_widget_set_tooltip_text(btn, child->tooltip);

      gtk_widget_set_sensitive(btn, child->type != MENU_ITEM_DESACTIVE);
      gtk_widget_set_hexpand(btn, orient == GTK_ORIENTATION_VERTICAL ? TRUE : FALSE);

      /* Sous-sous-menu récursif */
      if (child->nb_sous_items > 0)
      {
         child->popover = gtk_popover_new();
         child->sous_box = menu_construire_sous_box(cfg, child);
         gtk_popover_set_child(GTK_POPOVER(child->popover), child->sous_box);
         gtk_widget_set_parent(child->popover, btn);
         gtk_popover_set_has_arrow(GTK_POPOVER(child->popover), TRUE);

         // Position selon l'orientation du PARENT (pas du cfg global)
         GtkPositionType pos = (orient == GTK_ORIENTATION_VERTICAL)
    ? GTK_POS_RIGHT
    : GTK_POS_BOTTOM;
         gtk_popover_set_position(GTK_POPOVER(child->popover), pos);
      }

      MenuClickData *d = g_new0(MenuClickData, 1);
      d->menu = cfg;
      d->item = child;
      g_signal_connect(btn, "clicked", G_CALLBACK(on_item_clicked), d);
      g_signal_connect_swapped(btn, "destroy", G_CALLBACK(g_free), d);

      gtk_box_append(GTK_BOX(box), btn);
      child->widget = btn;
   }

   return box;
}

/**
 * Construit le widget GTK d'un item de premier niveau.
 */
static GtkWidget *menu_construire_item(Menu *cfg, MenuItem *item)
{
   /* Séparateur */
   if (item->type == MENU_ITEM_SEPARATEUR)
   {
      GtkOrientation sep_orient =
          (cfg->orientation == MENU_HORIZONTAL)
              ? GTK_ORIENTATION_VERTICAL
              : GTK_ORIENTATION_HORIZONTAL;
      item->widget = gtk_separator_new(sep_orient);
      return item->widget;
   }

   /* Bouton */
   GtkWidget *btn = gtk_button_new();

   GtkWidget *inner = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
   gtk_widget_set_halign(inner, GTK_ALIGN_CENTER);

   if (item->nom_icone)
   {
      GtkWidget *img = gtk_image_new_from_icon_name(item->nom_icone);
      gtk_box_append(GTK_BOX(inner), img);
   }

   GtkWidget *lbl = gtk_label_new(item->texte);
   gtk_box_append(GTK_BOX(inner), lbl);

   /* Indicateur de sous-menu (flèche) */
   if (item->nb_sous_items > 0)
   {
      int orient;
      const char *arrow = (orient == GTK_ORIENTATION_VERTICAL)
                             ? "pan-end-symbolic"   // enfants empilés verticalement → flèche droite →
                             : "pan-down-symbolic"; // enfants côte à côte horizontalement → flèche bas ↓
      GtkWidget *arrow_img = gtk_image_new_from_icon_name(arrow);
      gtk_box_append(GTK_BOX(inner), arrow_img);
   }

   gtk_button_set_child(GTK_BUTTON(btn), inner);

   if (item->tooltip)
      gtk_widget_set_tooltip_text(btn, item->tooltip);

   gtk_widget_set_sensitive(btn, item->type != MENU_ITEM_DESACTIVE);

   /* Expansion selon l'orientation de la barre */
   if (cfg->orientation == MENU_VERTICAL)
   {
      gtk_widget_set_hexpand(btn, TRUE);
      gtk_widget_set_halign(btn, GTK_ALIGN_FILL);
   }

   /* Sous-menu via GtkPopover */
   if (item->nb_sous_items > 0)
   {
      item->popover = gtk_popover_new();
      item->sous_box = menu_construire_sous_box(cfg, item);
      gtk_popover_set_child(GTK_POPOVER(item->popover), item->sous_box);
      gtk_widget_set_parent(item->popover, btn);
      gtk_popover_set_has_arrow(GTK_POPOVER(item->popover), TRUE);

      /* Position du popover selon l'orientation */
      // ✅ CORRECTION — position basée sur sous_menu_orientation de l'ITEM lui-même
      if (item->sous_menu_orientation == MENU_VERTICAL)
         gtk_popover_set_position(GTK_POPOVER(item->popover), GTK_POS_BOTTOM);
      else
         gtk_popover_set_position(GTK_POPOVER(item->popover), GTK_POS_RIGHT);
   }

   MenuClickData *d = g_new0(MenuClickData, 1);
   d->menu = cfg;
   d->item = item;
   g_signal_connect(btn, "clicked", G_CALLBACK(on_item_clicked), d);
   g_signal_connect_swapped(btn, "destroy", G_CALLBACK(g_free), d);

   item->widget = btn;
   return btn;
}

// ====================== GESTION DES ITEMS ======================

MenuItem *menu_item_creer(const char *id, const char *texte,
                          const char *nom_icone, MenuItemType type)
{
   MenuItem *item = g_new0(MenuItem, 1);
   if (id)
   {
      item->id = malloc(strlen(id) + 1);
      strcpy(item->id, id);
   }
   else
   {
      item->id = NULL;
   }

   if (texte)
   {
      item->texte = malloc(strlen(texte) + 1);
      strcpy(item->texte, texte);
   }
   else
   {
      item->texte = NULL;
   }

   if (nom_icone)
   {
      item->nom_icone = malloc(strlen(nom_icone) + 1);
      strcpy(item->nom_icone, nom_icone);
   }
   else
   {
      item->nom_icone = NULL;
   }

   item->type = type;
   item->tooltip = NULL;
   item->sous_items = NULL;
   item->nb_sous_items = 0;
   item->sous_menu_orientation = MENU_VERTICAL; /* sous-menu vertical par défaut */
   item->widget = NULL;
   item->popover = NULL;
   item->sous_box = NULL;
   return item;
}

MenuItem *menu_item_separateur(void)
{
   return menu_item_creer(NULL, NULL, NULL, MENU_ITEM_SEPARATEUR);
}

void menu_item_ajouter_sous_item(MenuItem *parent, MenuItem *enfant)
{
   if (!parent || !enfant)
      return;

   parent->sous_items = g_realloc(parent->sous_items,
                                  sizeof(MenuItem *) * (parent->nb_sous_items + 1));
   parent->sous_items[parent->nb_sous_items] = enfant;
   parent->nb_sous_items++;
}

void menu_item_free(MenuItem *item)
{
   if (!item)
      return;

   g_free(item->id);
   g_free(item->texte);
   g_free(item->nom_icone);
   g_free(item->tooltip);

   for (int i = 0; i < item->nb_sous_items; i++)
      menu_item_free(item->sous_items[i]);

   g_free(item->sous_items);
   g_free(item);
}

// ====================== FONCTIONS PUBLIQUES ======================

void menu_initialiser(Menu *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(Menu));

   cfg->id_css = malloc(strlen("menu") + 1);
   strcpy(cfg->id_css, "menu");
   cfg->orientation = MENU_HORIZONTAL;
   cfg->espacement = 2;
   cfg->items = NULL;
   cfg->nb_items = 0;

   cfg->size.width = 0;
   cfg->size.height = 0;

   /* Style par défaut — thème sombre */
   cfg->style.bg_barre = malloc(strlen("#2c3e50") + 1);
   strcpy(cfg->style.bg_barre, "#2c3e50");
   cfg->style.couleur_bordure = malloc(strlen("transparent") + 1);
   strcpy(cfg->style.couleur_bordure, "transparent");
   cfg->style.epaisseur_bordure = 0;
   cfg->style.rayon_arrondi = 6;

   cfg->style.bg_item = malloc(strlen("transparent") + 1);
   strcpy(cfg->style.bg_item, "transparent");
   cfg->style.fg_item = malloc(strlen("#ecf0f1") + 1);
   strcpy(cfg->style.fg_item, "#ecf0f1");
   cfg->style.bg_item_hover = malloc(strlen("#3d5166") + 1);
   strcpy(cfg->style.bg_item_hover, "#3d5166");
   cfg->style.fg_item_hover = malloc(strlen("#ffffff") + 1);
   strcpy(cfg->style.fg_item_hover, "#ffffff");
   cfg->style.bg_item_actif = malloc(strlen("#2980b9") + 1);
   strcpy(cfg->style.bg_item_actif, "#2980b9");
   cfg->style.rayon_item = 4;
   cfg->style.taille_texte_px = 0;
   cfg->style.gras = FALSE;

   cfg->style.couleur_separateur = malloc(strlen("#4a6278") + 1);
   strcpy(cfg->style.couleur_separateur, "#4a6278");
   cfg->style.bg_popover = malloc(strlen("#2c3e50") + 1);
   strcpy(cfg->style.bg_popover, "#2c3e50");
}

/**
 * Crée le widget GtkBox principal et tous ses items.
 * Retourne cfg->widget à ajouter au parent.
 *
 * Structure interne :
 *   GtkBox#id_css (orientation principale)
 *   ├── GtkButton   item normal
 *   ├── GtkSeparator
 *   ├── GtkButton   item avec sous-menu
 *   │     └── GtkPopover
 *   │           └── GtkBox (sous_menu_orientation)
 *   │                 ├── GtkButton  sous-item
 *   │                 └── ...
 *   └── ...
 */
GtkWidget *menu_creer(Menu *cfg)
{
   if (!cfg)
      return NULL;

   GtkOrientation orient =
       (cfg->orientation == MENU_VERTICAL)
           ? GTK_ORIENTATION_VERTICAL
           : GTK_ORIENTATION_HORIZONTAL;

   cfg->widget = gtk_box_new(orient, cfg->espacement);
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "menu");

   /* Taille */
   if (cfg->size.width > 0 || cfg->size.height > 0)
      gtk_widget_set_size_request(cfg->widget,
                                  cfg->size.width > 0 ? cfg->size.width : -1,
                                  cfg->size.height > 0 ? cfg->size.height : -1);

   /* Expansion */
   if (cfg->orientation == MENU_HORIZONTAL)
   {
      gtk_widget_set_hexpand(cfg->widget, cfg->size.width == 0 ? TRUE : FALSE);
      gtk_widget_set_halign(cfg->widget, cfg->size.width == 0 ? GTK_ALIGN_FILL : GTK_ALIGN_START);
      gtk_widget_set_vexpand(cfg->widget, FALSE);
      gtk_widget_set_valign(cfg->widget, GTK_ALIGN_START);
   }
   else
   {
      gtk_widget_set_hexpand(cfg->widget, FALSE);
      gtk_widget_set_halign(cfg->widget, GTK_ALIGN_START);
      gtk_widget_set_vexpand(cfg->widget, cfg->size.height == 0 ? TRUE : FALSE);
      gtk_widget_set_valign(cfg->widget, cfg->size.height == 0 ? GTK_ALIGN_FILL : GTK_ALIGN_START);
   }

   /* Construction de tous les items */
   for (int i = 0; i < cfg->nb_items; i++)
   {
      GtkWidget *w = menu_construire_item(cfg, cfg->items[i]);
      gtk_box_append(GTK_BOX(cfg->widget), w);
   }

   /* CSS */
   menu_apply_css(cfg);

   return cfg->widget;
}

void menu_ajouter_item(Menu *cfg, MenuItem *item)
{
   if (!cfg || !item)
      return;

   cfg->items = g_realloc(cfg->items, sizeof(MenuItem *) * (cfg->nb_items + 1));
   cfg->items[cfg->nb_items] = item;
   cfg->nb_items++;

   /* Si le widget existe déjà, ajouter l'item directement */
   if (cfg->widget)
   {
      GtkWidget *w = menu_construire_item(cfg, item);
      gtk_box_append(GTK_BOX(cfg->widget), w);
   }
}

// ====================== SETTERS ======================

void menu_set_orientation(Menu *cfg, MenuOrientation orientation)
{
   if (!cfg)
      return;
   cfg->orientation = orientation;
   if (cfg->widget)
      gtk_orientable_set_orientation(
          GTK_ORIENTABLE(cfg->widget),
          orientation == MENU_VERTICAL
              ? GTK_ORIENTATION_VERTICAL
              : GTK_ORIENTATION_HORIZONTAL);
}

void menu_set_size(Menu *cfg, int width, int height)
{
   if (!cfg)
      return;
   cfg->size.width = width;
   cfg->size.height = height;
   if (cfg->widget)
      gtk_widget_set_size_request(cfg->widget,
                                  width > 0 ? width : -1,
                                  height > 0 ? height : -1);
}

void menu_set_espacement(Menu *cfg, int espacement)
{
   if (!cfg)
      return;
   cfg->espacement = espacement;
   if (cfg->widget)
      gtk_box_set_spacing(GTK_BOX(cfg->widget), espacement);
}

/**
 * Ajoute la classe CSS "actif" à l'item dont l'id correspond,
 * et la retire de tous les autres.
 */
void menu_set_item_actif(Menu *cfg, const char *id)
{
   if (!cfg || !id)
      return;
   for (int i = 0; i < cfg->nb_items; i++)
   {
      MenuItem *item = cfg->items[i];
      if (!item->widget)
         continue;
      if (item->id && strcmp(item->id, id) == 0)
         gtk_widget_add_css_class(item->widget, "actif");
      else
         gtk_widget_remove_css_class(item->widget, "actif");
   }
}

void menu_set_item_sensitive(Menu *cfg, const char *id, gboolean sensitive)
{
   if (!cfg || !id)
      return;
   for (int i = 0; i < cfg->nb_items; i++)
   {
      MenuItem *item = cfg->items[i];
      if (item->id && strcmp(item->id, id) == 0 && item->widget)
      {
         gtk_widget_set_sensitive(item->widget, sensitive);
         return;
      }
   }
}

// ====================== LIBÉRATION MÉMOIRE ======================

void menu_free(Menu *cfg)
{
   if (!cfg)
      return;

   for (int i = 0; i < cfg->nb_items; i++)
      menu_item_free(cfg->items[i]);

   g_free(cfg->items);
   g_free(cfg->id_css);

   g_free(cfg->style.bg_barre);
   g_free(cfg->style.couleur_bordure);
   g_free(cfg->style.bg_item);
   g_free(cfg->style.fg_item);
   g_free(cfg->style.bg_item_hover);
   g_free(cfg->style.fg_item_hover);
   g_free(cfg->style.bg_item_actif);
   g_free(cfg->style.couleur_separateur);
   g_free(cfg->style.bg_popover);

   memset(cfg, 0, sizeof(Menu));
}