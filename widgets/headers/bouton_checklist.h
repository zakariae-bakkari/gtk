//
// Created by admin on 14-Feb-26.
//

#ifndef GTK_BOUTON_CHECKLIST_H
#define GTK_BOUTON_CHECKLIST_H

#include <gtk/gtk.h>
#include <stdbool.h>

/**
 * Énumération pour l'état de la case à cocher
 */
typedef enum
{
   CHECKLIST_UNCHECKED = 0,
   CHECKLIST_CHECKED = 1,
   CHECKLIST_INCONSISTENT = 2 // État partiellement coché (indéterminé)
} BoutonChecklistEtat;

/**
 * Énumération pour la position du label par rapport à la case
 */
typedef enum
{
   CHECKLIST_LABEL_DROITE,
   CHECKLIST_LABEL_GAUCHE
} BoutonChecklistLabelPos;

/**
 * Structure de Style pour la case à cocher
 */
typedef struct
{
   char *couleur_texte;       // Couleur du label
   char *couleur_texte_hover; // Couleur au survol
   int taille_texte_px;       // Taille de la police (0 = défaut)
   bool gras;                 // Texte en gras
} BoutonChecklistStyle;

/**
 * Définition du pointeur de fonction pour l'événement "toggled"
 */
typedef void (*BoutonChecklistAction)(GtkCheckButton *widget, gpointer data);

/**
 * Structure Principale BOUTON_CHECKLIST
 */
typedef struct
{
   GtkWidget *widget; // Le widget GtkCheckButton
   char *id_css;      // ID unique pour le CSS

   // --- Contenu ---
   char *label;                       // Texte de la case à cocher
   BoutonChecklistLabelPos pos_label; // Position du label

   // --- État ---
   BoutonChecklistEtat etat; // État initial (coché ou non)

   // --- Apparence ---
   BoutonChecklistStyle style;
   char *tooltip; // Texte d'infobulle

   // --- Comportement ---
   bool est_actif;    // Sensible aux clics (Sensitive)
   bool inconsistent; // Si true, affiche l'état indéterminé

   // --- Événements ---
   BoutonChecklistAction on_toggled; // Fonction à appeler au changement d'état
   gpointer user_data;               // Données à passer à la fonction

} BoutonChecklist;

/* -------------------------------------------------------------------------
 * Prototypes
 * ------------------------------------------------------------------------- */

/**
 * Initialise une structure BoutonChecklist avec les valeurs par défaut
 * @param config : Pointeur sur la structure à initialiser
 */
void bouton_checklist_initialiser(BoutonChecklist *config);

/**
 * Crée et retourne un widget GtkCheckButton configuré selon la structure
 * @param config : Pointeur sur la structure de configuration
 * @return : Le widget GtkCheckButton créé
 */
GtkWidget *bouton_checklist_creer(BoutonChecklist *config);

/**
 * Change l'état de la case à cocher de manière programmatique
 * @param config : Pointeur sur la structure BoutonChecklist
 * @param etat : Nouvel état (CHECKED, UNCHECKED, INCONSISTENT)
 */
void bouton_checklist_set_etat(BoutonChecklist *config, BoutonChecklistEtat etat);

/**
 * Récupère l'état actuel de la case à cocher
 * @param config : Pointeur sur la structure BoutonChecklist
 * @return : L'état actuel
 */
BoutonChecklistEtat bouton_checklist_get_etat(BoutonChecklist *config);

#endif // GTK_BOUTON_CHECKLIST_H