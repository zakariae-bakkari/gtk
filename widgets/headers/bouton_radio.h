//
// Created by admin on 14-Feb-26.
//

#ifndef GTK_BUTTON_RADIO_H
#define GTK_BUTTON_RADIO_H

#include <gtk/gtk.h>

/**
 * Énumération pour la position du label par rapport au bouton radio
 */
typedef enum
{
   RADIO_LABEL_DROITE,
   RADIO_LABEL_GAUCHE
} BoutonRadioLabelPos;

/**
 * Structure de Style pour le bouton radio
 */
typedef struct
{
   char *couleur_texte;       // Couleur du label
   char *couleur_texte_hover; // Couleur au survol
   char *couleur_point;       // Couleur du point du radio
   int taille_texte_px;       // Taille de la police (0 = défaut)
   gboolean gras;             // Texte en gras
} BoutonRadioStyle;

/**
 * Définition du pointeur de fonction pour l'événement "toggled"
 */
typedef void (*BoutonRadioAction)(GtkCheckButton *widget, gpointer data);

/**
 * Structure Principale BOUTON_RADIO
 *
 * En GTK4, les boutons radio sont implémentés en utilisant GtkCheckButton
 * avec la fonction gtk_check_button_set_group() pour lier les widgets
 */
typedef struct
{
   GtkWidget *widget; // Le widget GtkCheckButton (en tant que radio)
   char *id_css;      // ID unique pour le CSS

   // --- Contenu ---
   char *label;                   // Texte du bouton radio
   BoutonRadioLabelPos pos_label; // Position du label

   // --- État ---
   gboolean est_actif; // Si true, ce bouton est sélectionné à la création
   gboolean sensible;  // Sensible aux clics (Sensitive)

   // --- Apparence ---
   BoutonRadioStyle style;
   char *tooltip; // Texte d'infobulle

   // --- Groupe Radio ---
   GtkCheckButton *group_leader; // Pointeur au premier CheckButton du groupe

   // --- Événements ---
   BoutonRadioAction on_toggled; // Fonction à appeler au changement de sélection
   gpointer user_data;           // Données à passer à la fonction

} BoutonRadio;

/* -------------------------------------------------------------------------
 * Prototypes
 * ------------------------------------------------------------------------- */

/**
 * Initialise une structure BoutonRadio avec les valeurs par défaut
 * @param config : Pointeur sur la structure à initialiser
 */
void bouton_radio_initialiser(BoutonRadio *config);

/**
 * Crée et retourne un widget GtkCheckButton configuré comme radio
 * En GTK4, les radios sont des CheckButtons groupés via gtk_check_button_set_group()
 * @param config : Pointeur sur la structure de configuration
 * @return : Le widget GtkCheckButton créé
 */
GtkWidget *bouton_radio_creer(BoutonRadio *config);

/**
 * Lie un bouton radio à un groupe
 * @param config : Pointeur sur la structure BoutonRadio à ajouter au groupe
 * @param group_leader : Pointeur au premier CheckButton du groupe (leader)
 */
void bouton_radio_set_groupe(BoutonRadio *config, GtkCheckButton *group_leader);

/**
 * Définit le bouton radio comme sélectionné
 * @param config : Pointeur sur la structure BoutonRadio
 * @param actif : true pour sélectionner, false pour désélectionner
 */
void bouton_radio_set_actif(BoutonRadio *config, gboolean actif);

/**
 * Récupère l'état du bouton radio
 * @param config : Pointeur sur la structure BoutonRadio
 * @return : true si sélectionné, false sinon
 */
gboolean bouton_radio_est_actif(BoutonRadio *config);

#endif // GTK_BUTTON_RADIO_H