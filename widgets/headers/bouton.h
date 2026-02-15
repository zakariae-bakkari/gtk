#ifndef BOUTON_H
#define BOUTON_H

#include <gtk/gtk.h>
#include <stdbool.h>

// --- MACROS DE COULEURS PAR DÉFAUT ---
#define COULEUR_DEFAUT_FOND "#e0e0e0"
#define COULEUR_DEFAUT_TEXTE "#000000"
#define COULEUR_DEFAUT_HOVER "#d0d0d0"

/**
 * Énumération pour le mode de dimensionnement du bouton
 */
typedef enum
{
    TAILLE_AUTO,       // Ajuste automatiquement selon le contenu (défaut)
    TAILLE_FIXE,       // Dimensions exactes spécifiées en largeur/hauteur
    TAILLE_FIT_CONTENT // Ajuste au minimum requis pour le contenu
} BoutonTailleMode;

/**
 * Énumération pour le type de curseur au survol
 */
typedef enum
{
    CURSEUR_DEFAUT,
    CURSEUR_MAIN,    // Pointer (le doigt)
    CURSEUR_AIDE,    // Point d'interrogation
    CURSEUR_ATTENTE, // Sablier/Rond de chargement
    CURSEUR_CROIX    // Interdit
} BoutonCurseur;

/**
 * Énumération pour la position de l'icône par rapport au texte
 */
typedef enum
{
    ICONE_GAUCHE,
    ICONE_DROITE,
    ICONE_HAUT,
    ICONE_BAS,
    ICONE_SEULE // Ignore le texte
} BoutonIconePos;

/**
 * Structure de Dimensions
 *
 * Mode AUTO (par défaut):
 *   - Le bouton s'ajuste automatiquement selon son contenu
 *   - largeur et hauteur sont ignorées
 *
 * Mode FIXE:
 *   - Définir largeur et hauteur explicitement
 *   - Le contenu s'adapte à ces dimensions
 *
 * Mode FIT_CONTENT:
 *   - Le bouton prend la taille minimale requise pour son contenu
 *   - largeur et hauteur peuvent définir les dimensions minimales
 */
typedef struct
{
    BoutonTailleMode mode; // Mode de dimensionnement
    int largeur;           // Largeur en pixels (-1 pour ignorer en mode AUTO)
    int hauteur;           // Hauteur en pixels (-1 pour ignorer en mode AUTO)
    int largeur_min;       // Largeur minimale (mode FIT_CONTENT)
    int hauteur_min;       // Hauteur minimale (mode FIT_CONTENT)
} BoutonTaille;

/**
 * Structure de Style (Normal, Survol, Actif)
 */
typedef struct
{
    // État Normal
    char *bg_normal;
    char *fg_normal; // Couleur texte

    // État Survol (Hover)
    char *bg_hover;
    char *fg_hover;

    // Bordures
    int rayon_arrondi; // Border-radius
    int epaisseur_bordure;
    char *couleur_bordure;

    // Police
    bool gras;
    bool italique;
    int taille_texte_px; // 0 = défaut
} BoutonStyle;

/**
 * Définition du pointeur de fonction pour l'événement "clic"
 */
typedef void (*BoutonAction)(GtkWidget *widget, gpointer data);

/**
 * Structure Principale BOUTON
 */
typedef struct
{
    GtkWidget *widget; // Le widget GtkButton
    char *id_css;      // ID unique pour le CSS (obligatoire pour le hover)

    // --- Contenu ---
    char *texte;
    char *nom_icone; // Nom de l'icône (ex: "mail-send-symbolic")
    BoutonIconePos pos_icone;
    int espacement_icone; // Espace entre icone et texte

    // --- Apparence ---
    BoutonTaille taille;
    BoutonStyle style;
    BoutonCurseur curseur;

    // --- Comportement ---
    bool est_actif; // Sensible aux clics (Sensitive)
    char *tooltip;  // Texte d'infobulle au survol

    // --- Événements ---
    BoutonAction on_clic; // Fonction à appeler au clic
    gpointer user_data;   // Données à passer à la fonction

} Bouton;

/* -------------------------------------------------------------------------
 * Prototypes
 * ------------------------------------------------------------------------- */

void bouton_initialiser(Bouton *config);
GtkWidget *bouton_creer(Bouton *config);

// Fonction helper pour changer le texte dynamiquement après création
void bouton_set_texte(Bouton *config, const char *nouveau_texte);

/**
 * Change la taille du bouton dynamiquement après sa création
 * Mode AUTO : Le bouton s'ajuste automatiquement au contenu
 * Mode FIXE : Le bouton prend les dimensions exactes spécifiées
 * Mode FIT_CONTENT : Le bouton prend la taille minimale requise pour le contenu
 * @param config : Structure du bouton
 * @param mode : Mode de dimensionnement (AUTO, FIXE, FIT_CONTENT)
 * @param largeur : Largeur en pixels (-1 pour ignorer)
 * @param hauteur : Hauteur en pixels (-1 pour ignorer)
 */
void bouton_set_taille(Bouton *config, BoutonTailleMode mode, int largeur, int hauteur);

/**
 * Change uniquement la largeur du bouton
 * Utile pour les boutons avec hauteur automatique
 * @param config : Structure du bouton
 * @param largeur : Largeur en pixels
 */
void bouton_set_largeur(Bouton *config, int largeur);

/**
 * Change uniquement la hauteur du bouton
 * Utile pour les boutons avec largeur automatique
 * @param config : Structure du bouton
 * @param hauteur : Hauteur en pixels
 */
void bouton_set_hauteur(Bouton *config, int hauteur);

#endif // BOUTON_H
