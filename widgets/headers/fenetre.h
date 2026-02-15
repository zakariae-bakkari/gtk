#ifndef FENETRE_H
#define FENETRE_H

#include <gtk/gtk.h>
#include <stdbool.h>

/**
 * Énumération pour l'alignement du titre
 */
typedef enum {
    TITRE_ALIGN_GAUCHE = 0, // 0.0
    TITRE_ALIGN_CENTRE = 1, // 0.5
    TITRE_ALIGN_DROITE = 2  // 1.0
} FenetreTitreAlign;

typedef enum {
    WIN_POS_CENTER,
    WIN_POS_MOUSE,
    WIN_POS_CENTER_ON_PARENT,
    WIN_POS_DEFAULT
} FenetrePosition;

typedef enum {
    WIN_TYPE_TOPLEVEL,
    WIN_TYPE_POPUP
} FenetreType;

typedef struct {
    int width;
    int height;
} FenetreTaille;

/**
 * Structure Fenetre Mise à Jour
 */
typedef struct {
    GtkWidget *wind;

    // --- Titre et Apparence ---
    char *title;
    FenetreTitreAlign titre_align; // Nouveau : Position du titre
    char *icon_path;

    // --- Comportement ---
    FenetreType type;
    bool resizable;
    bool demarrer_maximisee;  // Nouveau : Si true, prend toute la taille

    // --- Contrôles de la fenêtre (Symboles) ---
    bool bouton_fermer;       // Nouveau
    bool bouton_agrandir;     // Nouveau
    bool bouton_reduire;      // Nouveau

    // --- Style ---
    FenetreTaille taille;
    char *color_bg;
    char *background_image;
    FenetrePosition position;
    int id;
} Fenetre;

/* --- Prototypes --- */
void fenetre_initialiser(Fenetre *config);
GtkWidget* fenetre_creer(Fenetre *config);

#endif // FENETRE_H
