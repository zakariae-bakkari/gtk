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

/**
 * Structure Fenetre
 * Note: FenetrePosition et FenetreType ont été supprimés car non supportés en GTK4.
 * - Le positionnement de fenêtre est géré par le compositeur (Wayland/Windows).
 * - Les popups GTK4 utilisent GtkPopover, pas un type de fenêtre.
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
    WidgetSize taille;
    char *color_bg;
    char *background_image;
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
