#ifndef CHAMP_TEXTE_H
#define CHAMP_TEXTE_H

#include <gtk/gtk.h>

/*
 * ChampTexte : composant réutilisable GTK4
 * - Bloc complet = root (Entry + Label d’erreur)
 * - Validation : required + regex
 * - Style configurable
 * - Callbacks personnalisables
 */

// ====================== CALLBACKS ======================

typedef void (*ChampOnChange)(GtkEditable *editable, gpointer user_data);
typedef void (*ChampOnActivate)(GtkEntry *entry, gpointer user_data);
typedef void (*ChampOnInvalid)(GtkWidget *widget, const char *message, gpointer user_data);

// ====================== STYLE ======================

typedef struct
{
    char *bg_normal;
    char *fg_normal;

    char *placeholder_color;

    int   epaisseur_bordure;     // 0 = défaut
    char *couleur_bordure;

    int   rayon_arrondi;         // 0 = défaut

    gboolean gras;
    gboolean italique;

    int   taille_texte_px;       // 0 = défaut

    // Etat erreur
    char *couleur_bordure_error; // ex: "#ff3b30"
    char *bg_error;              // ex: "#fff1f2"

} ChampTexteStyle;

// ====================== STRUCTURE PRINCIPALE ======================

typedef struct
{
    // Widgets
    GtkWidget *root;        // conteneur principal (GtkBox)
    GtkWidget *entry;       // GtkEntry
    GtkWidget *error_label; // message d’erreur sous le champ

    // Classe CSS appliquée à entry
    char *css_class;

    // Contraintes & contenu
    int   max_length;       // 0 = illimité
    char *texte;            // valeur initiale
    char *placeholder;      // texte aide visuelle

    // Validation
    gboolean required;
    char   *regex_pattern;  // pattern texte
    GRegex *regex;          // regex compilée

    // Etat & comportement
    gboolean sensitive;     // actif/inactif

    // Style
    ChampTexteStyle style;

    // Callbacks
    ChampOnChange   on_change;
    ChampOnActivate on_activate;
    ChampOnInvalid  on_invalid;
    gpointer        user_data;

    // Etat erreur courant
    gboolean invalid;
    char   *error_message;

} ChampTexte;


// ====================== API ======================

// Création / destruction
ChampTexte* champtexte_new(const char *css_class);
void        champtexte_free(ChampTexte *ct);

// Récupérer le widget principal à insérer dans l’UI
GtkWidget*  champtexte_widget(ChampTexte *ct);

// ----- Setters / Getters -----

const char* champtexte_get_text(ChampTexte *ct);
void        champtexte_set_text(ChampTexte *ct, const char *text);

void        champtexte_set_placeholder(ChampTexte *ct, const char *ph);

void        champtexte_set_max_length(ChampTexte *ct, int max_len);

void        champtexte_set_required(ChampTexte *ct, gboolean required);

gboolean    champtexte_set_regex(ChampTexte *ct, const char *pattern, GError **err);

void        champtexte_set_sensitive(ChampTexte *ct, gboolean sensitive);

// ----- Style -----

void        champtexte_set_style(ChampTexte *ct, const ChampTexteStyle *style);
void        champtexte_apply_style(ChampTexte *ct);

// ----- Validation -----

gboolean    champtexte_validate(ChampTexte *ct);

void        champtexte_set_invalid(ChampTexte *ct, const char *message);
void        champtexte_clear_invalid(ChampTexte *ct);
void champtexte_set_size(ChampTexte *ct, int width, int height);

// ----- Callbacks -----

void champtexte_set_callbacks(
    ChampTexte      *ct,
    ChampOnChange    on_change,
    ChampOnActivate  on_activate,
    ChampOnInvalid   on_invalid,
    gpointer         user_data
);

#endif // CHAMP_TEXTE_H