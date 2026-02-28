#include "../headers/champ_texte.h"
#include <string.h>
#include <glib.h>

// -------------------- Helpers --------------------
static void safe_set_str(char **dst, const char *src)
{
    g_free(*dst);
    *dst = src ? g_strdup(src) : NULL;
}

static gboolean is_empty_trim(const char *s)
{
    if (!s) return TRUE;
    while (*s) {
        if (!g_ascii_isspace(*s)) return FALSE;
        s++;
    }
    return TRUE;
}

// CSS minimal :
// - style normal (bg + border + radius + font)
// - placeholder color
// - invalid : bordure rouge + fond rouge léger
// - label erreur en rouge
static char *build_css_minimal(ChampTexte *ct)
{
    const char *cls = (ct->css_class && ct->css_class[0]) ? ct->css_class : "champtexte";

    const char *bg = ct->style.bg_normal ? ct->style.bg_normal : "#ffffff";
    const char *fg = ct->style.fg_normal ? ct->style.fg_normal : "#111111";
    const char *ph = ct->style.placeholder_color ? ct->style.placeholder_color : "#8a8a8a";

    int border = (ct->style.epaisseur_bordure > 0) ? ct->style.epaisseur_bordure : 1;
    const char *bcol = ct->style.couleur_bordure ? ct->style.couleur_bordure : "#cfcfcf";

    int radius = (ct->style.rayon_arrondi > 0) ? ct->style.rayon_arrondi : 8;

    const char *bcol_err = ct->style.couleur_bordure_error ? ct->style.couleur_bordure_error : "#ff3b30";
    const char *bg_err   = ct->style.bg_error ? ct->style.bg_error : "#fff1f2";

    const char *weight = ct->style.gras ? "700" : "400";
    const char *fstyle = ct->style.italique ? "italic" : "normal";

    char font_rule[64] = {0};
    if (ct->style.taille_texte_px > 0) {
        g_snprintf(font_rule, sizeof(font_rule), "font-size: %dpx;", ct->style.taille_texte_px);
    }

    GString *s = g_string_new(NULL);

    // Entry normal
    g_string_append_printf(s,
        ".%s {"
        " background: %s;"
        " color: %s;"
        " border: %dpx solid %s;"
        " border-radius: %dpx;"
        " padding: 8px;"
        " font-weight: %s;"
        " font-style: %s;"
        " %s"
        "}\n",
        cls, bg, fg, border, bcol, radius, weight, fstyle, font_rule
    );

    // Placeholder GTK4
    g_string_append_printf(s,
        ".%s placeholder { color: %s; }\n",
        cls, ph
    );

    // Invalid = border rouge + bg rouge clair
    g_string_append_printf(s,
        ".%s.invalid {"
        " border: %dpx solid %s;"
        " background: %s;"
        "}\n",
        cls, border, bcol_err, bg_err
    );

    // Error label rouge
    g_string_append(s, ".ct-error { color: #ff3b30; font-size: 12px; }\n");

    return g_string_free(s, FALSE);
}

// -------------------- Gestion erreur --------------------
void champtexte_set_invalid(ChampTexte *ct, const char *message)
{
    if (!ct) return;

    ct->invalid = TRUE;
    safe_set_str(&ct->error_message, message ? message : "Valeur invalide");

    gtk_label_set_text(GTK_LABEL(ct->error_label), ct->error_message);
    gtk_widget_set_visible(ct->error_label, TRUE);

    gtk_widget_add_css_class(ct->entry, "invalid");

    if (ct->on_invalid)
        ct->on_invalid(ct->entry, ct->error_message, ct->user_data);
}

void champtexte_clear_invalid(ChampTexte *ct)
{
    if (!ct) return;

    ct->invalid = FALSE;

    gtk_widget_remove_css_class(ct->entry, "invalid");
    gtk_label_set_text(GTK_LABEL(ct->error_label), "");
    gtk_widget_set_visible(ct->error_label, FALSE);

    g_free(ct->error_message);
    ct->error_message = NULL;
}

// -------------------- Validation --------------------
gboolean champtexte_validate(ChampTexte *ct)
{
    if (!ct) return FALSE;

    const char *value = gtk_editable_get_text(GTK_EDITABLE(ct->entry));

    if (ct->required && is_empty_trim(value)) {
        champtexte_set_invalid(ct, "Ce champ est obligatoire");
        return FALSE;
    }

    if (ct->regex) {
        if (!g_regex_match(ct->regex, value ? value : "", 0, NULL)) {
            champtexte_set_invalid(ct, "Format invalide");
            return FALSE;
        }
    }

    champtexte_clear_invalid(ct);
    return TRUE;
}

// -------------------- Signals internes --------------------
static void on_changed(GtkEditable *editable, gpointer user_data)
{
    ChampTexte *ct = (ChampTexte*)user_data;

    // UX : quand on tape, on enlève l’erreur
    champtexte_clear_invalid(ct);

    if (ct->on_change)
        ct->on_change(editable, ct->user_data);
}

static void on_activate(GtkEntry *entry, gpointer user_data)
{
    ChampTexte *ct = (ChampTexte*)user_data;

    // Enter => valider d'abord
    (void)champtexte_validate(ct);

    if (ct->on_activate)
        ct->on_activate(entry, ct->user_data);
}

static void on_focus_leave(GtkEventControllerFocus *controller, gpointer user_data)
{
    (void)controller;
    ChampTexte *ct = (ChampTexte*)user_data;

    // Perte de focus => valider
    (void)champtexte_validate(ct);
}

// -------------------- API --------------------
ChampTexte* champtexte_new(const char *css_class)
{
    ChampTexte *ct = g_malloc0(sizeof(ChampTexte));

    // Widgets
    ct->root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    ct->entry = gtk_entry_new();
    ct->error_label = gtk_label_new("");

    gtk_label_set_xalign(GTK_LABEL(ct->error_label), 0.0f);
    gtk_widget_set_visible(ct->error_label, FALSE);
    gtk_widget_add_css_class(ct->error_label, "ct-error");

    gtk_box_append(GTK_BOX(ct->root), ct->entry);
    gtk_box_append(GTK_BOX(ct->root), ct->error_label);

    // css_class
    safe_set_str(&ct->css_class, (css_class && css_class[0]) ? css_class : "champtexte");
    gtk_widget_add_css_class(ct->entry, ct->css_class);

    // defaults
    ct->max_length = 0;
    ct->required = FALSE;
    ct->sensitive = TRUE;

    // signals
    g_signal_connect(ct->entry, "changed", G_CALLBACK(on_changed), ct);
    g_signal_connect(ct->entry, "activate", G_CALLBACK(on_activate), ct);

    GtkEventController *focus = gtk_event_controller_focus_new();
    g_signal_connect(focus, "leave", G_CALLBACK(on_focus_leave), ct);
    gtk_widget_add_controller(ct->entry, focus);

    return ct;
}

void champtexte_free(ChampTexte *ct)
{
    if (!ct) return;

    // Attention: widgets sont détruits par GTK via la hiérarchie UI.
    // Ici on libère seulement ce qu’on a alloué nous-même.
    g_free(ct->css_class);

    g_free(ct->texte);
    g_free(ct->placeholder);

    g_free(ct->regex_pattern);
    if (ct->regex) g_regex_unref(ct->regex);

    g_free(ct->error_message);

    // style strings
    g_free(ct->style.bg_normal);
    g_free(ct->style.fg_normal);
    g_free(ct->style.placeholder_color);
    g_free(ct->style.couleur_bordure);
    g_free(ct->style.couleur_bordure_error);
    g_free(ct->style.bg_error);

    g_free(ct);
}

GtkWidget* champtexte_widget(ChampTexte *ct)
{
    return ct ? ct->root : NULL;
}

// -------------------- Get/Set --------------------
const char* champtexte_get_text(ChampTexte *ct)
{
    if (!ct) return "";
    return gtk_editable_get_text(GTK_EDITABLE(ct->entry));
}

void champtexte_set_text(ChampTexte *ct, const char *text)
{
    if (!ct) return;
    safe_set_str(&ct->texte, text);
    gtk_editable_set_text(GTK_EDITABLE(ct->entry), text ? text : "");
}

void champtexte_set_placeholder(ChampTexte *ct, const char *ph)
{
    if (!ct) return;
    safe_set_str(&ct->placeholder, ph);
    gtk_entry_set_placeholder_text(GTK_ENTRY(ct->entry), ph ? ph : "");
}

void champtexte_set_max_length(ChampTexte *ct, int max_len)
{
    if (!ct) return;
    ct->max_length = max_len;
    gtk_entry_set_max_length(GTK_ENTRY(ct->entry), max_len);
}

void champtexte_set_required(ChampTexte *ct, gboolean required)
{
    if (!ct) return;
    ct->required = required;
}

gboolean champtexte_set_regex(ChampTexte *ct, const char *pattern, GError **err)
{
    if (!ct) return FALSE;

    if (ct->regex) { g_regex_unref(ct->regex); ct->regex = NULL; }
    safe_set_str(&ct->regex_pattern, pattern);

    if (!pattern || pattern[0] == '\0')
        return TRUE;

    ct->regex = g_regex_new(pattern, 0, 0, err);
    return ct->regex != NULL;
}

void champtexte_set_sensitive(ChampTexte *ct, gboolean sensitive)
{
    if (!ct) return;
    ct->sensitive = sensitive;
    gtk_widget_set_sensitive(ct->entry, sensitive);
}

void champtexte_set_style(ChampTexte *ct, const ChampTexteStyle *style)
{
    if (!ct || !style) return;

    safe_set_str(&ct->style.bg_normal, style->bg_normal);
    safe_set_str(&ct->style.fg_normal, style->fg_normal);
    safe_set_str(&ct->style.placeholder_color, style->placeholder_color);

    ct->style.epaisseur_bordure = style->epaisseur_bordure;
    safe_set_str(&ct->style.couleur_bordure, style->couleur_bordure);

    ct->style.rayon_arrondi = style->rayon_arrondi;

    ct->style.gras = style->gras;
    ct->style.italique = style->italique;
    ct->style.taille_texte_px = style->taille_texte_px;

    safe_set_str(&ct->style.couleur_bordure_error, style->couleur_bordure_error);
    safe_set_str(&ct->style.bg_error, style->bg_error);
}

void champtexte_apply_style(ChampTexte *ct)
{
    if (!ct) return;

    char *css = build_css_minimal(ct);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, css);

    GdkDisplay *display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(
        display,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    g_object_unref(provider);
    g_free(css);
}
void champtexte_set_size(ChampTexte *ct, int width, int height)
{
    if (!ct) return;
    gtk_widget_set_size_request(ct->entry, width, height);
}

void champtexte_set_callbacks(
    ChampTexte      *ct,
    ChampOnChange    on_change,
    ChampOnActivate  on_activate,
    ChampOnInvalid   on_invalid,
    gpointer         user_data
){
    if (!ct) return;
    ct->on_change = on_change;
    ct->on_activate = on_activate;
    ct->on_invalid = on_invalid;
    ct->user_data = user_data;
}
