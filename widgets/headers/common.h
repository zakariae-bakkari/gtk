#ifndef COMMON_H
#define COMMON_H

#include <gtk/gtk.h>

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
   gboolean gras;         // texte en gras
   gboolean italique;     // texte en italique
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
typedef void (*WidgetOnChange)(GtkEditable *editable, gpointer user_data);

/**
 * Callback déclenché quand l'utilisateur appuie sur Entrée
 */
typedef void (*WidgetOnActivate)(GtkEntry *entry, gpointer user_data);

/**
 * Callback déclenché quand une validation échoue
 */
typedef void (*WidgetOnInvalid)(GtkWidget *widget, const char *message, gpointer user_data);

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

#endif // COMMON_H