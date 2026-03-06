#ifndef DIALOG_H
#define DIALOG_H

#include <gtk/gtk.h>
#include "common.h"

// ====================== ENUMS ======================

/**
 * Type de dialog — détermine l'icône et la couleur de l'en-tête
 */
typedef enum
{
   DIALOG_INFO,          // Informationnel (bleu)
   DIALOG_SUCCES,        // Succès (vert)
   DIALOG_AVERTISSEMENT, // Avertissement (orange)
   DIALOG_ERREUR,        // Erreur (rouge)
   DIALOG_PERSONNALISE   // Aucune icône/couleur prédéfinie
} DialogType;

/**
 * Boutons prédéfinis pour les dialogs courants
 */
typedef enum
{
   DIALOG_BOUTONS_OK,              // Un seul bouton "OK"
   DIALOG_BOUTONS_OK_ANNULER,      // "OK" + "Annuler"
   DIALOG_BOUTONS_OUI_NON,         // "Oui" + "Non"
   DIALOG_BOUTONS_OUI_NON_ANNULER, // "Oui" + "Non" + "Annuler"
   DIALOG_BOUTONS_PERSONNALISES    // Boutons ajoutés manuellement
} DialogBoutons;

/**
 * Réponse retournée quand l'utilisateur ferme le dialog
 */
typedef enum
{
   DIALOG_REPONSE_OK = 1,
   DIALOG_REPONSE_ANNULER = 2,
   DIALOG_REPONSE_OUI = 3,
   DIALOG_REPONSE_NON = 4,
   DIALOG_REPONSE_FERMER = 5 // Fermeture via la croix
} DialogReponse;

// ====================== CALLBACKS ======================

/**
 * Callback déclenché quand l'utilisateur répond au dialog
 * @param reponse  : identifiant de la réponse (DialogReponse ou id personnalisé)
 * @param user_data : données utilisateur
 */
typedef void (*DialogOnReponse)(int reponse, gpointer user_data);

// ====================== STRUCTURES ======================

/**
 * Style visuel du dialog
 */
typedef struct
{
   /* En-tête */
   char *bg_header;  // Couleur de fond de l'en-tête
   char *fg_header;  // Couleur du texte du titre
   int taille_titre; // Taille du titre en px (0 = défaut)
   gboolean titre_gras;

   /* Corps */
   char *bg_corps; // Couleur de fond du corps
   char *fg_corps; // Couleur du texte du corps

   /* Pied de page */
   char *bg_footer; // Couleur de fond du pied de page

   /* Bordure / arrondi */
   int rayon_arrondi;
   int epaisseur_bordure;
   char *couleur_bordure;

   /* Bouton principal (confirmer/ok) */
   char *bg_bouton_principal;
   char *fg_bouton_principal;
   char *bg_bouton_secondaire;
   char *fg_bouton_secondaire;
} DialogStyle;

/**
 * Bouton personnalisé
 */
typedef struct
{
   char *texte;        // Libellé
   char *nom_icone;    // Icône (NULL = aucune)
   int reponse_id;     // Identifiant retourné dans le callback
   gboolean principal; // TRUE → style "bouton principal"
} DialogBoutonConfig;

/**
 * Structure principale DIALOG
 *
 * Hiérarchie GTK créée par dialog_creer() :
 *   GtkWindow (modal)
 *   └── GtkBox vertical
 *         ├── GtkBox  header  (titre + icône)
 *         ├── GtkSeparator
 *         ├── GtkBox  corps   (message + widget_contenu optionnel)
 *         ├── GtkSeparator
 *         └── GtkBox  footer  (boutons)
 */
typedef struct
{
   /* Widgets GTK (remplis par dialog_creer) */
   GtkWidget *window;     // GtkWindow modal
   GtkWidget *box_header; // Boîte de l'en-tête
   GtkWidget *box_corps;  // Boîte du corps
   GtkWidget *box_footer; // Boîte des boutons

   /* Référence à la fenêtre parente */
   GtkWindow *parent;

   /* Identification CSS */
   char *id_css;

   /* Contenu */
   DialogType type;
   char *titre;
   char *message;             // Texte du corps (NULL = aucun)
   GtkWidget *widget_contenu; // Widget personnalisé dans le corps (NULL = aucun)

   /* Boutons */
   DialogBoutons boutons_preset; // Boutons prédéfinis
   DialogBoutonConfig **boutons; // Tableau de boutons personnalisés
   int nb_boutons;

   /* Style */
   DialogStyle style;

   /* Taille */
   WidgetSize taille;

   /* Comportement */
   gboolean modal;           // TRUE par défaut
   gboolean fermeture_croix; // Afficher le bouton de fermeture (TRUE par défaut)

   /* Événements */
   DialogOnReponse on_reponse;
   gpointer user_data;
} Dialog;

// ====================== PROTOTYPES ======================

/* --- Initialisation / création --- */
void dialog_initialiser(Dialog *cfg);
GtkWidget *dialog_creer(Dialog *cfg);

/* --- Boutons personnalisés --- */
void dialog_ajouter_bouton(Dialog *cfg, const char *texte,
                           const char *nom_icone, int reponse_id,
                           gboolean principal);

/* --- Raccourcis (dialogs fréquents) --- */
void dialog_afficher_info(GtkWindow *parent, const char *titre,
                          const char *message, DialogOnReponse cb, gpointer data);
void dialog_afficher_erreur(GtkWindow *parent, const char *titre,
                            const char *message, DialogOnReponse cb, gpointer data);
void dialog_afficher_avertissement(GtkWindow *parent, const char *titre,
                                   const char *message, DialogOnReponse cb, gpointer data);
void dialog_afficher_confirmation(GtkWindow *parent, const char *titre,
                                  const char *message, DialogOnReponse cb, gpointer data);

/* --- Setters dynamiques --- */
void dialog_set_titre(Dialog *cfg, const char *titre);
void dialog_set_message(Dialog *cfg, const char *message);
void dialog_set_contenu(Dialog *cfg, GtkWidget *widget);

/* --- Affichage / fermeture --- */
void dialog_afficher(Dialog *cfg);
void dialog_fermer(Dialog *cfg);

/* --- Libération mémoire --- */
void dialog_free(Dialog *cfg);

#endif // DIALOG_H
