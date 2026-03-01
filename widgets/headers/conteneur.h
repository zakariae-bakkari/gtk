#ifndef CONTENEUR_H
#define CONTENEUR_H

#include <gtk/gtk.h>
#include "common.h" // Include common.h to access WidgetScrollMode

/**
 * Orientation du conteneur
 */
typedef enum
{
    CONTENEUR_VERTICAL,
    CONTENEUR_HORIZONTAL
} ConteneurOrientation;

/**
 * Alignement du conteneur dans son parent
 * (Correspond à GtkAlign : FILL = s'étirer, CENTER = centrer, START = début, END = fin)
 */
typedef enum
{
    ALIGNEMENT_REMPLIR, // Prend tout l'espace disponible (Défaut)
    ALIGNEMENT_DEBUT,   // Calé à gauche ou en haut
    ALIGNEMENT_FIN,     // Calé à droite ou en bas
    ALIGNEMENT_CENTRE   // Centré
} ConteneurAlignement;

/**
 * Structure pour les dimensions (Largeur / Hauteur)
 * Valeur -1 = Taille automatique (par défaut)
 */
typedef struct
{
    int largeur;
    int hauteur;
} ConteneurDimensions;

/**
 * Structure pour les marges (Espace EXTÉRIEUR)
 */
typedef struct
{
    int haut;
    int bas;
    int gauche;
    int droite;
} ConteneurMarges;

/**
 * Structure pour le rembourrage (Padding - Espace INTÉRIEUR)
 */
typedef struct
{
    int haut;
    int bas;
    int gauche;
    int droite;
} ConteneurPadding;

/**
 * Structure Conteneur Complète
 */
typedef struct
{
    GtkWidget *widget;        // Le widget GTK (GtkBox)
    GtkWidget *scroll_widget; // GtkScrolledWindow (si défilable)

    // --- Disposition ---
    ConteneurOrientation orientation;
    int espacement;    // Espace entre les éléments enfants
    gboolean homogene; // Si true, tous les enfants ont la même taille

    // --- Dimensions & Positionnement ---
    ConteneurDimensions taille;  // Largeur et Hauteur forcées
    ConteneurAlignement align_x; // Alignement Horizontal
    ConteneurAlignement align_y; // Alignement Vertical

    // --- Expansion des enfants ---
    gboolean enfants_hexpand; // Si true, les enfants s'étendent horizontalement
    gboolean enfants_vexpand; // Si true, les enfants s'étendent verticalement

    // --- Espacements ---
    ConteneurMarges marges;   // Espace autour de la boite
    ConteneurPadding padding; // Espace à l'intérieur de la boite

    // --- Style ---
    char *id_css;       // ID CSS personnalisé
    char *couleur_fond; // Couleur de fond (ex: "#FFFFFF")

    // --- Bordure (Détail ajouté) ---
    int bordure_largeur;   // Épaisseur bordure (0 = aucune)
    char *bordure_couleur; // Couleur bordure
    int bordure_rayon;     // Arrondi (border-radius)

    // --- Défilement ---
    WidgetScrollMode scroll_mode; // Mode de défilement (utilise l'enum partagé)
    int scroll_min_width;         // Largeur minimale pour la zone défilable (0 = auto)
    int scroll_min_height;        // Hauteur minimale pour la zone défilable (0 = auto)
    gboolean scroll_overlay;      // Utiliser des barres de défilement superposées (true = moderne, false = classique)

} Conteneur;

/* -------------------------------------------------------------------------
 * Prototypes
 * ------------------------------------------------------------------------- */

void conteneur_initialiser(Conteneur *config);
GtkWidget *conteneur_creer(Conteneur *config);
void conteneur_ajouter(Conteneur *config, GtkWidget *enfant);

// Scrolling configuration helpers
void conteneur_set_scrollable(Conteneur *config, WidgetScrollMode mode);
void conteneur_set_scroll_size(Conteneur *config, int min_width, int min_height);
void conteneur_set_scroll_overlay(Conteneur *config, gboolean overlay);

#endif // CONTENEUR_H
