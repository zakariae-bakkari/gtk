#ifndef TEXTE_H
#define TEXTE_H

#include <gtk/gtk.h>
#include "common.h"

/**
 * Types de heading pour le texte (comme HTML h1, h2, etc.)
 */
typedef enum
{
   TEXTE_NORMAL,   // Texte normal
   TEXTE_H1,       // Titre de niveau 1 (le plus grand)
   TEXTE_H2,       // Titre de niveau 2
   TEXTE_H3,       // Titre de niveau 3
   TEXTE_H4,       // Titre de niveau 4
   TEXTE_H5,       // Titre de niveau 5
   TEXTE_H6,       // Titre de niveau 6 (le plus petit)
   TEXTE_SUBTITLE, // Sous-titre
   TEXTE_CAPTION   // Légende/caption
} TexteType;

/**
 * Alignement du texte
 */
typedef enum
{
   TEXTE_ALIGN_LEFT,   // Aligné à gauche
   TEXTE_ALIGN_CENTER, // Centré
   TEXTE_ALIGN_RIGHT,  // Aligné à droite
   TEXTE_ALIGN_JUSTIFY // Justifié
} TexteAlignement;

/**
 * Décoration du texte
 */
typedef enum
{
   TEXTE_DECORATION_NONE,         // Aucune décoration
   TEXTE_DECORATION_UNDERLINE,    // Souligné
   TEXTE_DECORATION_OVERLINE,     // Ligne au-dessus
   TEXTE_DECORATION_STRIKETHROUGH // Barré
} TexteDecoration;

/**
 * Structure pour les dimensions du texte
 */
typedef struct
{
   int largeur; // Largeur maximale (-1 = auto)
   int hauteur; // Hauteur (-1 = auto)
} TexteDimensions;

/**
 * Structure pour les marges du texte
 */
typedef struct
{
   int haut;
   int bas;
   int gauche;
   int droite;
} TexteMarges;

/**
 * Structure complète pour le widget texte
 */
typedef struct
{
   GtkWidget *widget; // Le widget GTK (GtkLabel)

   // --- Contenu ---
   char *texte;         // Le texte à afficher
   char *texte_markup;  // Texte avec markup Pango (optionnel)
   gboolean use_markup; // Utiliser le markup Pango

   // --- Type et style ---
   TexteType type;             // Type de heading
   TexteAlignement alignement; // Alignement du texte
   TexteDecoration decoration; // Décoration du texte

   // --- Propriétés de police ---
   char *famille_police; // Famille de police (ex: "Arial", "Times")
   int taille_police;    // Taille de police en points (0 = défaut)
   gboolean gras;        // Texte en gras
   gboolean italique;    // Texte en italique

   // --- Couleurs ---
   char *couleur_texte; // Couleur du texte
   char *couleur_fond;  // Couleur de fond

   // --- Dimensions et positionnement ---
   TexteDimensions taille;         // Dimensions du widget
   TexteMarges marges;             // Marges extérieures
   WidgetAlignment align_widget_x; // Alignement du widget horizontalement
   WidgetAlignment align_widget_y; // Alignement du widget verticalement

   // --- Comportement ---
   gboolean selectable; // Le texte peut être sélectionné
   gboolean wrap;       // Retour à la ligne automatique
   int wrap_width;      // Largeur pour le retour à la ligne (-1 = auto)
   gboolean ellipsize;  // Utiliser des ellipses (...)

   // --- Style personnalisé ---
   WidgetStyle style; // Style commun
   char *id_css;      // ID CSS personnalisé

   // --- Bordure ---
   int bordure_largeur;   // Épaisseur de la bordure
   char *bordure_couleur; // Couleur de la bordure
   int bordure_rayon;     // Rayon des coins arrondis

} Texte;

/* -------------------------------------------------------------------------
 * Prototypes des fonctions
 * ------------------------------------------------------------------------- */

/**
 * Initialise une structure Texte avec des valeurs par défaut
 */
void texte_initialiser(Texte *config);

/**
 * Crée le widget GTK Label avec la configuration donnée
 */
GtkWidget *texte_creer(Texte *config);

/**
 * Met à jour le texte affiché
 */
void texte_set_text(Texte *config, const char *nouveau_texte);

/**
 * Met à jour le texte avec markup Pango
 */
void texte_set_markup(Texte *config, const char *markup);

/**
 * Change le type de heading
 */
void texte_set_type(Texte *config, TexteType type);

/**
 * Change l'alignement du texte
 */
void texte_set_alignement(Texte *config, TexteAlignement alignement);

/**
 * Configure la police
 */
void texte_set_police(Texte *config, const char *famille, int taille, gboolean gras, gboolean italique);

/**
 * Configure les couleurs
 */
void texte_set_couleurs(Texte *config, const char *couleur_texte, const char *couleur_fond);

/**
 * Active/désactive le retour à la ligne
 */
void texte_set_wrap(Texte *config, gboolean wrap, int width);

/**
 * Configure la sélectabilité du texte
 */
void texte_set_selectable(Texte *config, gboolean selectable);

#endif // TEXTE_H