#ifndef CONTENEUR_H
#define CONTENEUR_H

#include <gtk/gtk.h>
#include <stdbool.h>

/**
 * Énumération pour l'orientation (Vertical/Horizontal)
 */
typedef enum {
    CONTENEUR_VERTICAL,
    CONTENEUR_HORIZONTAL
} ConteneurOrientation;

/**
 * Structure Conteneur
 * Représente une GtkBox avec ses propriétés
 */
typedef struct {
    GtkWidget *widget;          // Le widget GTK (GtkBox)

    // --- Configuration ---
    ConteneurOrientation orientation;
    int espacement;             // Espace entre les éléments (pixels)
    bool homogene;              // Si true, tous les enfants ont la même taille

    // --- Style & Dimensions ---
    char *id_css;               // Pour le stylage CSS (ex: "mon_menu")
    char *couleur_fond;         // Couleur de fond (ex: "white", "#333")

    // --- Marges (Espace autour du conteneur) ---
    int marge_haut;
    int marge_bas;
    int marge_gauche;
    int marge_droite;

} Conteneur;

/* -------------------------------------------------------------------------
 * Prototypes (Format: module_action)
 * ------------------------------------------------------------------------- */

/**
 * Initialise la structure avec les valeurs par défaut.
 * @param config : Pointeur vers la structure Conteneur
 */
void conteneur_initialiser(Conteneur *config);

/**
 * Crée le widget GtkBox configuré.
 * @param config : La structure Conteneur
 * @return GtkWidget* : Le widget créé
 */
GtkWidget* conteneur_creer(Conteneur *config);

/**
 * Ajoute un widget enfant dans le conteneur.
 * @param config : Le conteneur parent
 * @param enfant : Le widget à ajouter (Bouton, Label, etc.)
 */
void conteneur_ajouter(Conteneur *config, GtkWidget *enfant);

#endif // CONTENEUR_H
