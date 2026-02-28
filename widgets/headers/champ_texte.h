#ifndef CHAMP_TEXTE_H
#define CHAMP_TEXTE_H

#include <gtk/gtk.h>
#include "common.h"

/*
 * ChampTexte : composant réutilisable GTK4
 * - Bloc complet = root (Entry + Label d'erreur)
 * - Validation : required + regex
 * - Style configurable
 * - Callbacks personnalisables
 */

// ====================== STRUCTURE PRINCIPALE ======================

typedef struct
{
    // Widgets
    GtkWidget *root;        // conteneur principal (GtkBox)
    GtkWidget *entry;       // GtkEntry
    GtkWidget *error_label; // message d'erreur sous le champ

    // Classe CSS appliquée à entry
    char *id_css;

    // Contraintes & contenu
    int max_length;    // 0 = illimité
    char *texte;       // valeur initiale
    char *placeholder; // texte aide visuelle

    // Validation
    gboolean required;
    char *regex_pattern; // pattern texte
    GRegex *regex;       // regex compilée

    // Etat & comportement
    gboolean sensitive; // actif/inactif

    // Style (utilise la structure commune)
    WidgetStyle style;

    // Callbacks étendues
    WidgetOnChange on_change;     // texte modifié
    WidgetOnActivate on_activate; // Entrée pressée
    WidgetOnInvalid on_invalid;   // validation échouée

    // Nouveaux callbacks
    WidgetOnChange on_focus_in;    // champ obtient le focus
    WidgetOnChange on_focus_out;   // champ perd le focus
    WidgetOnInvalid on_valid;      // validation réussie
    WidgetOnChange on_text_insert; // texte inséré (avant modification)
    WidgetOnChange on_text_delete; // texte supprimé (avant modification)

    gpointer user_data;

    // Etat erreur courant
    gboolean invalid;
    char *error_message;

} ChampTexte;

// ====================== API ======================

// Création / destruction
ChampTexte *champtexte_new(const char *css_class);
void champtexte_free(ChampTexte *ct);

// New pattern matching champ_motdepasse
void champtexte_initialiser(ChampTexte *cfg);
GtkWidget *champtexte_creer(ChampTexte *cfg);

// Récupérer le widget principal à insérer dans l’UI
GtkWidget *champtexte_widget(ChampTexte *ct);

// ----- Setters / Getters -----

const char *champtexte_get_text(ChampTexte *ct);
void champtexte_set_text(ChampTexte *ct, const char *text);

void champtexte_set_placeholder(ChampTexte *ct, const char *ph);

void champtexte_set_max_length(ChampTexte *ct, int max_len);

void champtexte_set_required(ChampTexte *ct, gboolean required);

gboolean champtexte_set_regex(ChampTexte *ct, const char *pattern, GError **err);

void champtexte_set_sensitive(ChampTexte *ct, gboolean sensitive);

// ----- Style -----

void champtexte_apply_style(ChampTexte *ct);

// ----- Validation -----

gboolean champtexte_validate(ChampTexte *ct);

void champtexte_set_invalid(ChampTexte *ct, const char *message);
void champtexte_clear_invalid(ChampTexte *ct);
void champtexte_set_size(ChampTexte *ct, int width, int height);

// ----- Callbacks -----

// Fonction originale (maintenue pour compatibilité) - updated to use common types
void champtexte_set_callbacks(
    ChampTexte *ct,
    WidgetOnChange on_change,
    WidgetOnActivate on_activate,
    WidgetOnInvalid on_invalid,
    gpointer user_data);

// Nouvelles fonctions pour callbacks étendues
void champtexte_set_callback_change(ChampTexte *ct, WidgetOnChange callback, gpointer user_data);
void champtexte_set_callback_activate(ChampTexte *ct, WidgetOnActivate callback, gpointer user_data);
void champtexte_set_callback_invalid(ChampTexte *ct, WidgetOnInvalid callback, gpointer user_data);
void champtexte_set_callback_valid(ChampTexte *ct, WidgetOnInvalid callback, gpointer user_data);
void champtexte_set_callback_focus_in(ChampTexte *ct, WidgetOnChange callback, gpointer user_data);
void champtexte_set_callback_focus_out(ChampTexte *ct, WidgetOnChange callback, gpointer user_data);
void champtexte_set_callback_text_insert(ChampTexte *ct, WidgetOnChange callback, gpointer user_data);
void champtexte_set_callback_text_delete(ChampTexte *ct, WidgetOnChange callback, gpointer user_data);

// Fonction complète pour définir tous les callbacks en une fois
void champtexte_set_all_callbacks(
    ChampTexte *ct,
    WidgetOnChange on_change,
    WidgetOnActivate on_activate,
    WidgetOnInvalid on_invalid,
    WidgetOnInvalid on_valid,
    WidgetOnChange on_focus_in,
    WidgetOnChange on_focus_out,
    WidgetOnChange on_text_insert,
    WidgetOnChange on_text_delete,
    gpointer user_data);

#endif // CHAMP_TEXTE_H