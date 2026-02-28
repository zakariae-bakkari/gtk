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
    if (!s)
        return TRUE;
    while (*s)
    {
        if (!g_ascii_isspace(*s))
            return FALSE;
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

    int border = (ct->style.epaisseur_bordure > 0) ? ct->style.epaisseur_bordure : 1;
    const char *bcol = ct->style.couleur_bordure ? ct->style.couleur_bordure : "#cfcfcf";

    int radius = (ct->style.rayon_arrondi > 0) ? ct->style.rayon_arrondi : 8;

    const char *bcol_err = ct->style.couleur_bordure_error ? ct->style.couleur_bordure_error : "#ff3b30";
    const char *bg_err = ct->style.bg_error ? ct->style.bg_error : "#fff1f2";

    const char *weight = ct->style.gras ? "700" : "400";
    const char *fstyle = ct->style.italique ? "italic" : "normal";

    char font_rule[64] = {0};
    if (ct->style.taille_texte_px > 0)
    {
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
                           cls, bg, fg, border, bcol, radius, weight, fstyle, font_rule);

    // Placeholder GTK4
    g_string_append_printf(s,
                           ".%s placeholder { color: %s; }\n",
                           cls, "#8a8a8a");

    // Invalid = border rouge + bg rouge clair
    g_string_append_printf(s,
                           ".%s.invalid {"
                           " border: %dpx solid %s;"
                           " background: %s;"
                           "}\n",
                           cls, border, bcol_err, bg_err);

    // Error label rouge
    g_string_append(s, ".ct-error { color: #ff3b30; font-size: 12px; }\n");

    return g_string_free(s, FALSE);
}

// -------------------- Gestion erreur --------------------
void champtexte_set_invalid(ChampTexte *ct, const char *message)
{
    if (!ct)
        return;

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
    if (!ct)
        return;

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
    if (!ct)
        return FALSE;

    const char *value = gtk_editable_get_text(GTK_EDITABLE(ct->entry));

    if (ct->required && is_empty_trim(value))
    {
        champtexte_set_invalid(ct, "Ce champ est obligatoire");
        return FALSE;
    }

    if (ct->regex)
    {
        if (!g_regex_match(ct->regex, value ? value : "", 0, NULL))
        {
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
    ChampTexte *ct = (ChampTexte *)user_data;

    // UX : quand on tape, on enlève l'erreur
    champtexte_clear_invalid(ct);

    if (ct->on_change)
        ct->on_change(editable, ct->user_data);
}

static void on_activate(GtkEntry *entry, gpointer user_data)
{
    ChampTexte *ct = (ChampTexte *)user_data;

    // Enter => valider d'abord
    gboolean is_valid = champtexte_validate(ct);

    // Déclencher le callback approprié
    if (is_valid && ct->on_valid)
        ct->on_valid((GtkWidget *)entry, "Valid", ct->user_data);

    if (ct->on_activate)
        ct->on_activate(entry, ct->user_data);
}

static void on_focus_leave(GtkEventControllerFocus *controller, gpointer user_data)
{
    (void)controller;
    ChampTexte *ct = (ChampTexte *)user_data;

    // Perte de focus => valider
    gboolean is_valid = champtexte_validate(ct);

    // Déclencher le callback approprié
    if (is_valid && ct->on_valid)
        ct->on_valid(ct->entry, "Valid", ct->user_data);

    // Callback focus out
    if (ct->on_focus_out)
        ct->on_focus_out(GTK_EDITABLE(ct->entry), ct->user_data);
}

static void on_focus_enter(GtkEventControllerFocus *controller, gpointer user_data)
{
    (void)controller;
    ChampTexte *ct = (ChampTexte *)user_data;

    // Callback focus in
    if (ct->on_focus_in)
        ct->on_focus_in(GTK_EDITABLE(ct->entry), ct->user_data);
}

static void on_text_insert(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer user_data)
{
    (void)text;
    (void)length;
    (void)position;
    ChampTexte *ct = (ChampTexte *)user_data;

    if (ct->on_text_insert)
        ct->on_text_insert(editable, ct->user_data);
}

static void on_text_delete(GtkEditable *editable, gint start_pos, gint end_pos, gpointer user_data)
{
    (void)start_pos;
    (void)end_pos;
    ChampTexte *ct = (ChampTexte *)user_data;

    if (ct->on_text_delete)
        ct->on_text_delete(editable, ct->user_data);
}

// -------------------- API --------------------

void champtexte_initialiser(ChampTexte *cfg)
{
    if (!cfg)
        return;

    memset(cfg, 0, sizeof(ChampTexte));

    // Default CSS class
    cfg->id_css = NULL; // Will be set in champtexte_creer

    // Default constraints & content
    cfg->max_length = 0;
    cfg->texte = NULL;
    cfg->placeholder = NULL;

    // Default validation
    cfg->required = FALSE;
    cfg->regex_pattern = NULL;
    cfg->regex = NULL;

    // Default behavior
    cfg->sensitive = TRUE;

    // Initialize style using common function
    widget_style_init(&cfg->style);

    // Default callbacks
    cfg->on_change = NULL;
    cfg->on_activate = NULL;
    cfg->on_invalid = NULL;
    cfg->user_data = NULL;

    // Default error state
    cfg->invalid = FALSE;
    cfg->error_message = NULL;
}

GtkWidget *champtexte_creer(ChampTexte *cfg)
{
    if (!cfg)
        return NULL;

    // Create widgets
    cfg->root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    cfg->entry = gtk_entry_new();
    cfg->error_label = gtk_label_new("");

    gtk_label_set_xalign(GTK_LABEL(cfg->error_label), 0.0f);
    gtk_widget_set_visible(cfg->error_label, FALSE);
    gtk_widget_add_css_class(cfg->error_label, "ct-error");

    gtk_box_append(GTK_BOX(cfg->root), cfg->entry);
    gtk_box_append(GTK_BOX(cfg->root), cfg->error_label);

    // Set CSS class
    const char *css_class = cfg->id_css ? cfg->id_css : "champtexte";
    safe_set_str(&cfg->id_css, css_class);
    gtk_widget_add_css_class(cfg->entry, cfg->id_css);

    // Apply initial settings
    if (cfg->placeholder)
        gtk_entry_set_placeholder_text(GTK_ENTRY(cfg->entry), cfg->placeholder);

    if (cfg->texte)
        gtk_editable_set_text(GTK_EDITABLE(cfg->entry), cfg->texte);

    if (cfg->max_length > 0)
        gtk_entry_set_max_length(GTK_ENTRY(cfg->entry), cfg->max_length);

    gtk_widget_set_sensitive(cfg->entry, cfg->sensitive);

    // Connect signals
    g_signal_connect(cfg->entry, "changed", G_CALLBACK(on_changed), cfg);
    g_signal_connect(cfg->entry, "activate", G_CALLBACK(on_activate), cfg);

    GtkEventController *focus = gtk_event_controller_focus_new();
    g_signal_connect(focus, "leave", G_CALLBACK(on_focus_leave), cfg);
    gtk_widget_add_controller(cfg->entry, focus);

    // Apply CSS styling
    char *css = build_css_minimal(cfg);
    if (css)
    {
        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_css_provider_load_from_string(provider, css);

        GdkDisplay *display = gdk_display_get_default();
        gtk_style_context_add_provider_for_display(
            display,
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER);

        g_object_unref(provider);
        g_free(css);
    }

    return cfg->root;
}

ChampTexte *champtexte_new(const char *css_class)
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
    if (!ct)
        return;

    // Attention: widgets sont détruits par GTK via la hiérarchie UI.
    // Ici on libère seulement ce qu'on a alloué nous-même.
    g_free(ct->css_class);

    g_free(ct->texte);
    g_free(ct->placeholder);

    g_free(ct->regex_pattern);
    if (ct->regex)
        g_regex_unref(ct->regex);

    g_free(ct->error_message);

    // Free style using common function
    widget_style_free(&ct->style);

    g_free(ct);
}

GtkWidget *champtexte_widget(ChampTexte *ct)
{
    return ct ? ct->root : NULL;
}

// -------------------- Get/Set --------------------
const char *champtexte_get_text(ChampTexte *ct)
{
    if (!ct)
        return "";
    return gtk_editable_get_text(GTK_EDITABLE(ct->entry));
}

void champtexte_set_text(ChampTexte *ct, const char *text)
{
    if (!ct)
        return;
    safe_set_str(&ct->texte, text);
    gtk_editable_set_text(GTK_EDITABLE(ct->entry), text ? text : "");
}

void champtexte_set_placeholder(ChampTexte *ct, const char *ph)
{
    if (!ct)
        return;
    safe_set_str(&ct->placeholder, ph);
    gtk_entry_set_placeholder_text(GTK_ENTRY(ct->entry), ph ? ph : "");
}

void champtexte_set_max_length(ChampTexte *ct, int max_len)
{
    if (!ct)
        return;
    ct->max_length = max_len;
    gtk_entry_set_max_length(GTK_ENTRY(ct->entry), max_len);
}

void champtexte_set_required(ChampTexte *ct, gboolean required)
{
    if (!ct)
        return;
    ct->required = required;
}

gboolean champtexte_set_regex(ChampTexte *ct, const char *pattern, GError **err)
{
    if (!ct)
        return FALSE;

    if (ct->regex)
    {
        g_regex_unref(ct->regex);
        ct->regex = NULL;
    }
    safe_set_str(&ct->regex_pattern, pattern);

    if (!pattern || pattern[0] == '\0')
        return TRUE;

    ct->regex = g_regex_new(pattern, 0, 0, err);
    return ct->regex != NULL;
}

void champtexte_set_sensitive(ChampTexte *ct, gboolean sensitive)
{
    if (!ct)
        return;
    ct->sensitive = sensitive;
    gtk_widget_set_sensitive(ct->entry, sensitive);
}

void champtexte_apply_style(ChampTexte *ct)
{
    if (!ct)
        return;

    char *css = build_css_minimal(ct);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, css);

    GdkDisplay *display = gdk_display_get_default();
    gtk_style_context_add_provider_for_display(
        display,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    g_object_unref(provider);
    g_free(css);
}

void champtexte_set_size(ChampTexte *ct, int width, int height)
{
    if (!ct)
        return;
    gtk_widget_set_size_request(ct->entry, width, height);
}

void champtexte_set_callbacks(
    ChampTexte *ct,
    WidgetOnChange on_change,
    WidgetOnActivate on_activate,
    WidgetOnInvalid on_invalid,
    gpointer user_data)
{
    if (!ct)
        return;
    ct->on_change = on_change;
    ct->on_activate = on_activate;
    ct->on_invalid = on_invalid;
    ct->user_data = user_data;
}

// -------------------- New Callback API Functions --------------------

void champtexte_set_callback_change(ChampTexte *ct, WidgetOnChange callback, gpointer user_data)
{
    if (!ct)
        return;
    ct->on_change = callback;
    ct->user_data = user_data;
}

void champtexte_set_callback_activate(ChampTexte *ct, WidgetOnActivate callback, gpointer user_data)
{
    if (!ct)
        return;
    ct->on_activate = callback;
    ct->user_data = user_data;
}

void champtexte_set_callback_invalid(ChampTexte *ct, WidgetOnInvalid callback, gpointer user_data)
{
    if (!ct)
        return;
    ct->on_invalid = callback;
    ct->user_data = user_data;
}

void champtexte_set_callback_valid(ChampTexte *ct, WidgetOnInvalid callback, gpointer user_data)
{
    if (!ct)
        return;
    ct->on_valid = callback;
    ct->user_data = user_data;
}

void champtexte_set_callback_focus_in(ChampTexte *ct, WidgetOnChange callback, gpointer user_data)
{
    if (!ct)
        return;
    ct->on_focus_in = callback;
    ct->user_data = user_data;
}

void champtexte_set_callback_focus_out(ChampTexte *ct, WidgetOnChange callback, gpointer user_data)
{
    if (!ct)
        return;
    ct->on_focus_out = callback;
    ct->user_data = user_data;
}

void champtexte_set_callback_text_insert(ChampTexte *ct, WidgetOnChange callback, gpointer user_data)
{
    if (!ct)
        return;
    ct->on_text_insert = callback;
    ct->user_data = user_data;

    // Connect the signal if not already connected
    g_signal_connect(ct->entry, "insert-text", G_CALLBACK(on_text_insert), ct);
}

void champtexte_set_callback_text_delete(ChampTexte *ct, WidgetOnChange callback, gpointer user_data)
{
    if (!ct)
        return;
    ct->on_text_delete = callback;
    ct->user_data = user_data;

    // Connect the signal if not already connected
    g_signal_connect(ct->entry, "delete-text", G_CALLBACK(on_text_delete), ct);
}

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
    gpointer user_data)
{
    if (!ct)
        return;

    ct->on_change = on_change;
    ct->on_activate = on_activate;
    ct->on_invalid = on_invalid;
    ct->on_valid = on_valid;
    ct->on_focus_in = on_focus_in;
    ct->on_focus_out = on_focus_out;
    ct->on_text_insert = on_text_insert;
    ct->on_text_delete = on_text_delete;
    ct->user_data = user_data;

    // Connect additional signals if needed
    if (on_text_insert)
    {
        g_signal_connect(ct->entry, "insert-text", G_CALLBACK(on_text_insert), ct);
    }
    if (on_text_delete)
    {
        g_signal_connect(ct->entry, "delete-text", G_CALLBACK(on_text_delete), ct);
    }

    // Add focus controller for focus events if not already added
    if (on_focus_in)
    {
        GtkEventController *focus_enter = gtk_event_controller_focus_new();
        g_signal_connect(focus_enter, "enter", G_CALLBACK(on_focus_enter), ct);
        gtk_widget_add_controller(ct->entry, focus_enter);
    }
}
