#ifndef MENU_H
#define MENU_H

#include <gtk/gtk.h>
#include "common.h"

#define MENU_MAX_ITEMS 64  // Nombre max d'items par niveau
#define MENU_MAX_NIVEAUX 8 // Profondeur maximale des sous-menus

// ====================== ENUMS ======================

/**
 * Orientation du menu principal et des sous-menus
 */
typedef enum
{
   MENU_HORIZONTAL, // Barre de navigation horizontale
   MENU_VERTICAL    // Menu latéral vertical
} MenuOrientation;

/**
 * Type d'un item de menu
 */
typedef enum
{
   MENU_ITEM_NORMAL,     // Item cliquable standard
   MENU_ITEM_SEPARATEUR, // Séparateur visuel (ligne)
   MENU_ITEM_DESACTIVE   // Item non cliquable (grisé)
} MenuItemType;

// ====================== CALLBACKS ======================

/**
 * Callback déclenché quand un item est cliqué
 * @param id   : identifiant de l'item (cfg->id)
 * @param data : user_data passé à la structure Menu
 */
typedef void (*MenuOnClick)(const char *id, gpointer data);

// ====================== STRUCTURES ======================

/* Déclaration anticipée pour les sous-menus récursifs */
typedef struct MenuItem MenuItem;
typedef struct Menu Menu;

/**
 * Structure d'un item de menu (peut contenir un sous-menu)
 *
 * Un item SANS sous-menu → bouton cliquable
 * Un item AVEC sous-menu → bouton qui ouvre un GtkPopover contenant un sous-menu
 */
struct MenuItem
{
   char *id;          // Identifiant unique (pour le callback)
   char *texte;       // Libellé affiché
   char *nom_icone;   // Nom d'icône GTK (NULL = aucune)
   char *tooltip;     // Info-bulle (NULL = aucune)
   MenuItemType type; // Normal / Séparateur / Désactivé

   /* Sous-menu (NULL = pas de sous-menu) */
   MenuOrientation sous_menu_orientation; // Orientation du sous-menu enfant
   MenuItem **sous_items;                 // Tableau de pointeurs d'items enfants
   int nb_sous_items;                     // Nombre d'items dans sous_items

   /* Widgets GTK créés (remplis par menu_creer) */
   GtkWidget *widget;   // GtkButton ou GtkSeparator
   GtkWidget *popover;  // GtkPopover (si sous-menu)
   GtkWidget *sous_box; // GtkBox interne du popover
};

/**
 * Style du menu et de ses items
 */
typedef struct
{
   /* Barre de menu */
   char *bg_barre;        // Couleur de fond de la barre
   char *couleur_bordure; // Couleur de bordure de la barre
   int epaisseur_bordure; // Épaisseur de la bordure (0 = aucune)
   int rayon_arrondi;     // Arrondi de la barre

   /* Items normaux */
   char *bg_item;       // Fond d'un item
   char *fg_item;       // Couleur texte d'un item
   char *bg_item_hover; // Fond au survol
   char *fg_item_hover; // Texte au survol
   char *bg_item_actif; // Fond item actif/sélectionné
   int rayon_item;      // Arrondi des items
   int taille_texte_px; // Taille de police (0 = défaut)
   gboolean gras;       // Texte en gras

   /* Séparateur */
   char *couleur_separateur; // Couleur du séparateur

   /* Popover / sous-menu */
   char *bg_popover; // Fond du popover
} MenuStyle;

/**
 * Structure principale MENU
 *
 * Structure interne :
 *   GtkBox (cfg->widget, orientation principale)
 *   ├── GtkButton  item 1  (cfg->items[0]->widget)
 *   ├── GtkSeparator item 2
 *   ├── GtkButton  item 3
 *   │     └── GtkPopover (cfg->items[2]->popover)
 *   │           └── GtkBox (cfg->items[2]->sous_box)
 *   │                 ├── GtkButton sous-item 1
 *   │                 └── GtkButton sous-item 2
 *   └── ...
 */
struct Menu
{
   GtkWidget *widget; // GtkBox principal (à ajouter au parent)
   char *id_css;      // ID CSS unique

   MenuOrientation orientation; // Orientation de la barre principale

   /* Items */
   MenuItem **items; // Tableau de pointeurs d'items
   int nb_items;     // Nombre d'items

   /* Style */
   MenuStyle style;

   /* Taille de la barre */
   WidgetSize size;

   /* Espacement entre items */
   int espacement;

   /* Événements */
   MenuOnClick on_click; // Callback unique pour tous les items
   gpointer user_data;
};

// ====================== PROTOTYPES ======================

/* --- Gestion des items --- */
MenuItem *menu_item_creer(const char *id, const char *texte,
                          const char *nom_icone, MenuItemType type);
MenuItem *menu_item_separateur(void);
void menu_item_ajouter_sous_item(MenuItem *parent, MenuItem *enfant);
void menu_item_free(MenuItem *item);

/* --- Menu principal --- */
void menu_initialiser(Menu *cfg);
GtkWidget *menu_creer(Menu *cfg);
void menu_ajouter_item(Menu *cfg, MenuItem *item);

/* --- Setters --- */
void menu_set_orientation(Menu *cfg, MenuOrientation orientation);
void menu_set_size(Menu *cfg, int width, int height);
void menu_set_espacement(Menu *cfg, int espacement);
void menu_set_item_actif(Menu *cfg, const char *id);
void menu_set_item_sensitive(Menu *cfg, const char *id, gboolean sensitive);

/* --- Libération mémoire --- */
void menu_free(Menu *cfg);

#endif // MENU_H
