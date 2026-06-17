#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef void* Widget;

/*
 * common.h : structures et définitions partagées entre tous les widgets
 *
 * Ce fichier contient :
 * - Structures de style communes
 * - Callbacks communes
 * - Enums et constantes partagées
 */

// ====================== STRUCTURES DE STYLE ======================

/**
 * Structure de style commune pour tous les widgets de saisie
 * Utilisée par : ChampTexte, ChampMotDePasse, ChampNombre, ChampSelect, etc.
 */
typedef struct
{
   char *bg_normal;       // couleur de fond normale
   char *fg_normal;       // couleur du texte normale
   int epaisseur_bordure; // épaisseur bordure (0 = défaut)
   char *couleur_bordure; // couleur de la bordure
   int rayon_arrondi;     // rayon des coins arrondis
   bool gras;             // texte en gras
   bool italique;         // texte en italique
   int taille_texte_px;   // taille du texte en pixels (0 = défaut)

   // États d'erreur (optionnels)
   char *couleur_bordure_error; // couleur bordure en cas d'erreur
   char *bg_error;              // couleur de fond en cas d'erreur
} WidgetStyle;

/**
 * Structure de taille commune pour tous les widgets
 * width et height en pixels (0 = auto/100% selon le contexte)
 */
typedef struct
{
   int width;  // largeur en pixels (0 = 100% largeur)
   int height; // hauteur en pixels (0 = auto)
} WidgetSize;

// ====================== CALLBACKS COMMUNES ======================

/**
 * Callback déclenché quand le contenu d'un widget change
 */
typedef void (*WidgetOnChange)(Widget widget, void *user_data);

/**
 * Callback déclenché quand l'utilisateur appuie sur Entrée
 */
typedef void (*WidgetOnActivate)(Widget widget, void *user_data);

/**
 * Callback déclenché quand une validation échoue
 */
typedef void (*WidgetOnInvalid)(Widget widget, const char *message, void *user_data);

// ====================== ENUMS ET CONSTANTES ======================

/**
 * États de validation pour les widgets
 */
typedef enum
{
   WIDGET_VALID,
   WIDGET_INVALID,
   WIDGET_PENDING
} WidgetValidationState;

/**
 * Types d'alignement pour les widgets
 */
typedef enum
{
   WIDGET_ALIGN_START,
   WIDGET_ALIGN_CENTER,
   WIDGET_ALIGN_END,
   WIDGET_ALIGN_FILL
} WidgetAlignment;

/**
 * Mode de défilement pour les conteneurs et fenêtres
 */
typedef enum
{
   SCROLL_NONE,       // Pas de défilement
   SCROLL_HORIZONTAL, // Défilement horizontal uniquement
   SCROLL_VERTICAL,   // Défilement vertical uniquement
   SCROLL_BOTH        // Défilement horizontal et vertical
} WidgetScrollMode;

// ====================== FONCTIONS UTILITAIRES ======================

/**
 * Initialise une structure WidgetStyle avec des valeurs par défaut
 */
void widget_style_init(WidgetStyle *style);

/**
 * Libère la mémoire allouée dans une structure WidgetStyle
 */
void widget_style_free(WidgetStyle *style);

/**
 * Copie une structure WidgetStyle
 */
void widget_style_copy(WidgetStyle *dest, const WidgetStyle *src);

/**
 * Charge un fichier CSS globalement pour l'application sans appeler de fonction GTK directement
 */
void widget_charger_css(const char *chemin_css);

/**
 * Crée un séparateur horizontal ou vertical sous forme de GtkWidget* sans appeler GTK en direct
 */
Widget widget_creer_separateur(int orientation);

/* ====================== WIDGET ABSTRACTION WRAPPERS ====================== */

// Layout & Sizing
void widget_set_hexpand(Widget w, bool expand);
void widget_set_vexpand(Widget w, bool expand);
void widget_set_halign_fill(Widget w);
void widget_set_valign_fill(Widget w);
void widget_set_size(Widget w, int width, int height);
void widget_set_visible(Widget w, bool visible);
void widget_set_sensitive(Widget w, bool sensitive);
void widget_set_can_target(Widget w, bool targetable);

// Overlay layout
Widget widget_creer_overlay(void);
void widget_overlay_set_child(Widget overlay, Widget child);
void widget_overlay_add_overlay(Widget overlay, Widget child);

// Fixed layout
Widget widget_creer_fixed(void);
void widget_fixed_set_overflow_hidden(Widget fixed);

// Drawing area
typedef void (*WidgetDrawFunc)(Widget drawing_area, void *cr, int width, int height, void *user_data);
Widget widget_creer_drawing_area(void);
void widget_drawing_area_set_draw_func(Widget da, WidgetDrawFunc draw_func, void *user_data);

// Picture
Widget widget_creer_picture(void);
Widget widget_creer_picture_depuis_fichier(const char *filename);
void widget_picture_set_keep_aspect_ratio(Widget pic, bool keep);
void widget_picture_set_filename(Widget pic, const char *filename);

// Media Streams / Video (returns void* to hide GtkMediaStream*)
void* widget_media_stream_new_from_file(const char *filename);
void widget_media_stream_set_loop(void* stream, bool loop);
void widget_media_stream_play(void* stream);
void widget_media_stream_pause(void* stream);
void widget_media_stream_free(void* stream);
Widget widget_picture_new_for_paintable(void* stream);
void widget_picture_set_paintable(Widget pic, void* stream);

// Events & Key Controllers
void widget_add_key_controller(Widget w, void *on_pressed, void *on_released, void *user_data);
void widget_connect_destroy_signal(Widget w, void *on_destroy, void *user_data);

// Timer utilities
unsigned int widget_timer_add(unsigned int interval_ms, void *callback, void *user_data);
void widget_timer_remove(unsigned int timer_id);

#endif // COMMON_H
