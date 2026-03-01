#ifndef CHAMP_MOTDEPASSE_H
#define CHAMP_MOTDEPASSE_H

#include <gtk/gtk.h>
#include "common.h"

/**
 * Politique de validation pour le mot de passe
 */
typedef struct
{
   int min_len;            // Longueur minimale (0 = aucune)
   gboolean require_digit; // Exige au moins un chiffre
   gboolean require_upper; // Exige au moins une majuscule
   gboolean require_symbol; // Exige au moins un symbole
} ChampPasswordPolicy;

typedef struct
{
   /**
    * IMPORTANT : c'est `container` qu'il faut ajouter au widget parent,
    * pas `widget`. `container` est un GtkBox vertical qui empile :
    *   1. cfg->widget      (GtkPasswordEntry)
    *   2. cfg->label_erreur (GtkLabel, caché quand tout est valide)
    */
   GtkWidget *container;    // GtkBox vertical — à passer à conteneur_ajouter()
   GtkWidget *widget;       // GtkPasswordEntry — pour lire/écrire la valeur
   GtkWidget *label_erreur; // GtkLabel du message d'erreur

   char *id_css;      // ID CSS

   // Contenu / contraintes
   char *placeholder;
   int max_length; // 0 = illimité
   gboolean required;
   ChampPasswordPolicy policy;

   // Comportement
   gboolean reveal_toggle; // afficher/masquer bouton
   gboolean sensitive;     // actif/inactif

   // Taille du widget
   WidgetSize size;

   // Style (utilise la structure commune)
   WidgetStyle style;

   // Style du label d'erreur (laisser à NULL/0 pour les valeurs par défaut)
   gboolean show_error_label; // Afficher le label d'erreur sous le champ (défaut : TRUE)
   char *erreur_couleur;      // Couleur du texte d'erreur  (défaut : "#e74c3c")
   int erreur_taille_px;      // Taille de police en px      (défaut : 11px)

   // Événements
   WidgetOnChange on_change;
   WidgetOnActivate on_activate;
   WidgetOnInvalid on_invalid;
   gpointer user_data;
} ChampMotDePasse;

void champ_motdepasse_initialiser(ChampMotDePasse *cfg);
GtkWidget *champ_motdepasse_creer(ChampMotDePasse *cfg);

const char *champ_motdepasse_get_texte(ChampMotDePasse *cfg);
void champ_motdepasse_set_texte(ChampMotDePasse *cfg, const char *texte);
void champ_motdepasse_set_placeholder(ChampMotDePasse *cfg, const char *ph);
void champ_motdepasse_set_max_length(ChampMotDePasse *cfg, int max_len);
void champ_motdepasse_set_required(ChampMotDePasse *cfg, gboolean required);
void champ_motdepasse_set_reveal_toggle(ChampMotDePasse *cfg, gboolean reveal);
void champ_motdepasse_set_policy(ChampMotDePasse *cfg, ChampPasswordPolicy policy);
void champ_motdepasse_set_size(ChampMotDePasse *cfg, int width, int height);
void champ_motdepasse_free(ChampMotDePasse *cfg);

#endif // CHAMP_MOTDEPASSE_H
