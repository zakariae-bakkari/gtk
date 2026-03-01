#ifndef FENETRE_H
#define FENETRE_H

#include <gtk/gtk.h>
#include "conteneur.h" // Include conteneur to access ConteneurScroll enum
#include "common.h"    // Include common to access WidgetScrollMode enum

/**
 * Énumération pour l'alignement du titre
 */
typedef enum
{
    TITRE_ALIGN_GAUCHE = 0, // 0.0
    TITRE_ALIGN_CENTRE = 1, // 0.5
    TITRE_ALIGN_DROITE = 2  // 1.0
} FenetreTitreAlign;

typedef enum
{
    WIN_POS_CENTER,
    WIN_POS_MOUSE,
    WIN_POS_CENTER_ON_PARENT,
    WIN_POS_DEFAULT
} FenetrePosition;

typedef enum
{
    WIN_TYPE_TOPLEVEL,
    WIN_TYPE_POPUP
} FenetreType;

typedef struct
{
    int width;
    int height;
} FenetreTaille;

/**
 * Structure Fenetre Mise à Jour
 */
typedef struct
{
    GtkWidget *wind;
    GtkWidget *scroll_widget; // GtkScrolledWindow pour le contenu principal

    // --- Titre et Apparence ---
    char *title;
    FenetreTitreAlign titre_align; // Nouveau : Position du titre
    char *icon_path;               // a tester

    // --- Comportement ---
    FenetreType type; // a tester
    gboolean resizable;
    gboolean demarrer_maximisee; // Nouveau : Si true, prend toute la taille

    // --- Contrôles de la fenêtre (Symboles) ---
    gboolean bouton_fermer;   // Nouveau
    gboolean bouton_agrandir; // Nouveau
    gboolean bouton_reduire;  // Nouveau

    // --- Défilement du contenu principal ---
    WidgetScrollMode scroll_mode; // Mode de défilement (utilise l'enum partagé)
    gboolean scroll_overlay;      // Barres de défilement superposées
    int content_min_width;        // Largeur minimale du contenu (0 = auto)
    int content_min_height;       // Hauteur minimale du contenu (0 = auto)

    // --- Style ---
    FenetreTaille taille;
    char *color_bg;
    char *background_image;
    FenetrePosition position;
    int id;
} Fenetre;

/* --- Prototypes --- */
void fenetre_initialiser(Fenetre *config);
GtkWidget *fenetre_creer(Fenetre *config);

// Window scrolling configuration helpers
void fenetre_set_scrollable(Fenetre *config, WidgetScrollMode mode);
void fenetre_set_scroll_content_size(Fenetre *config, int min_width, int min_height);
void fenetre_set_scroll_overlay(Fenetre *config, gboolean overlay);
GtkWidget *fenetre_get_content_container(Fenetre *config);

#endif // FENETRE_H
