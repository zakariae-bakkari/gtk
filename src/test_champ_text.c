#include "../widgets/headers/champ_texte.h"
#include "../widgets/headers/conteneur.h"
#include <gtk/gtk.h>
#include <stdio.h>

// ---------------- Callbacks de test ----------------
static void cb_change(GtkEditable *editable, gpointer user_data)
{
    const char *name = (const char *)user_data;
    printf("[on_change] %s -> '%s'\n", name, gtk_editable_get_text(editable));
}

static void cb_activate(GtkEntry *entry, gpointer user_data)
{
    const char *name = (const char *)user_data;
    printf("[on_activate] %s (ENTER) -> '%s'\n", name, gtk_editable_get_text(GTK_EDITABLE(entry)));
}

static void cb_invalid(GtkWidget *widget, const char *message, gpointer user_data)
{
    (void)widget;
    const char *name = (const char *)user_data;
    printf("[on_invalid] %s -> %s\n", name, message);
}

// Petit helper pour ajouter un titre à gauche
static void add_label(GtkWidget *parent_box, const char *text)
{
    GtkWidget *lbl = gtk_label_new(text);
    gtk_label_set_xalign(GTK_LABEL(lbl), 0.0f);
    gtk_box_append(GTK_BOX(parent_box), lbl);
}

static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Test complet ChampTexte (toutes fonctionnalités)");
    gtk_window_set_default_size(GTK_WINDOW(window), 720, 560);

    // -------- Conteneur principal (ton système) --------
    Conteneur main_container;
    conteneur_initialiser(&main_container);
    main_container.orientation = CONTENEUR_VERTICAL;
    main_container.align_x = ALIGNEMENT_CENTRE;
    main_container.align_y = ALIGNEMENT_CENTRE;
    main_container.espacement = 12;

    GtkWidget *main_box = conteneur_creer(&main_container);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    // On veut mettre des labels au-dessus => on force que main_box est une GtkBox
    // (si ton conteneur_creer ne renvoie pas une GtkBox, dis-moi et j’adapte)
    GtkWidget *box = main_box;

    // ======================================================
    // 1) EMAIL : required + regex (format email)
    // ======================================================
    add_label(box, "1) Email (required + regex). Quitte le champ (click ailleurs) ou ENTER pour valider.");

    ChampTexte email_cfg;
    champtexte_initialiser(&email_cfg);
    email_cfg.id_css = g_strdup("ct-email");
    email_cfg.required = TRUE;
    email_cfg.placeholder = g_strdup("ex: abc@gmail.com");

    // Set style properties using the common WidgetStyle structure
    email_cfg.style.bg_normal = g_strdup("#ffffff");
    email_cfg.style.fg_normal = g_strdup("#111111");
    email_cfg.style.epaisseur_bordure = 2;
    email_cfg.style.couleur_bordure = g_strdup("#cfcfcf");
    email_cfg.style.rayon_arrondi = 12;
    email_cfg.style.gras = FALSE;
    email_cfg.style.italique = FALSE;
    email_cfg.style.taille_texte_px = 14;
    email_cfg.style.couleur_bordure_error = g_strdup("#ff3b30");
    email_cfg.style.bg_error = g_strdup("#fff1f2");

    GError *err = NULL;
    if (!champtexte_set_regex(&email_cfg, "^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$", &err))
    {
        printf("[REGEX ERROR] EMAIL: %s\n", err ? err->message : "unknown");
        g_clear_error(&err);
    }

    email_cfg.on_change = cb_change;
    email_cfg.on_activate = cb_activate;
    email_cfg.on_invalid = cb_invalid;
    email_cfg.user_data = (gpointer) "EMAIL";

    GtkWidget *email_widget = champtexte_creer(&email_cfg);
    gtk_widget_set_size_request(email_cfg.entry, 420, -1);
    conteneur_ajouter(&main_container, email_widget);

    // ======================================================
    // 2) CIN / chiffres : max_length + required + regex {8}
    // ======================================================
    add_label(box, "2) CIN (8 chiffres) : max_length=8 + required + regex ^[0-9]{8}$");

    ChampTexte cin_cfg;
    champtexte_initialiser(&cin_cfg);
    cin_cfg.id_css = g_strdup("ct-cin");
    cin_cfg.placeholder = g_strdup("8 chiffres (ex: 12345678)");
    cin_cfg.max_length = 8;
    cin_cfg.required = TRUE;

    // Set style properties using the common WidgetStyle structure
    cin_cfg.style.bg_normal = g_strdup("#ffffff");
    cin_cfg.style.fg_normal = g_strdup("#111111");
    cin_cfg.style.epaisseur_bordure = 2;
    cin_cfg.style.couleur_bordure = g_strdup("#cfcfcf");
    cin_cfg.style.rayon_arrondi = 12;
    cin_cfg.style.gras = FALSE;
    cin_cfg.style.italique = FALSE;
    cin_cfg.style.taille_texte_px = 14;
    cin_cfg.style.couleur_bordure_error = g_strdup("#ff3b30");
    cin_cfg.style.bg_error = g_strdup("#fff1f2");

    if (!champtexte_set_regex(&cin_cfg, "^[0-9]{8}$", &err))
    {
        printf("[REGEX ERROR] CIN: %s\n", err ? err->message : "unknown");
        g_clear_error(&err);
    }

    cin_cfg.on_change = cb_change;
    cin_cfg.on_activate = cb_activate;
    cin_cfg.on_invalid = cb_invalid;
    cin_cfg.user_data = (gpointer) "CIN";

    GtkWidget *cin_widget = champtexte_creer(&cin_cfg);
    gtk_widget_set_size_request(cin_cfg.entry, 300, -1);
    conteneur_ajouter(&main_container, cin_widget);

    // ======================================================
    // 3) TEXTE INITIAL + required=false (pas d'erreur si vide)
    // ======================================================
    add_label(box, "3) Texte initial + required=FALSE (pas d'erreur si vide).");

    ChampTexte initial_cfg;
    champtexte_initialiser(&initial_cfg);
    initial_cfg.id_css = g_strdup("ct-initial");
    initial_cfg.texte = g_strdup("Texte initial");
    initial_cfg.placeholder = g_strdup("placeholder (visible seulement si vide)");
    initial_cfg.required = FALSE;

    // Set style properties using the common WidgetStyle structure
    initial_cfg.style.bg_normal = g_strdup("#ffffff");
    initial_cfg.style.fg_normal = g_strdup("#111111");
    initial_cfg.style.epaisseur_bordure = 2;
    initial_cfg.style.couleur_bordure = g_strdup("#cfcfcf");
    initial_cfg.style.rayon_arrondi = 12;
    initial_cfg.style.gras = FALSE;
    initial_cfg.style.italique = FALSE;
    initial_cfg.style.taille_texte_px = 14;
    initial_cfg.style.couleur_bordure_error = g_strdup("#ff3b30");
    initial_cfg.style.bg_error = g_strdup("#fff1f2");

    initial_cfg.on_change = cb_change;
    initial_cfg.on_activate = cb_activate;
    initial_cfg.on_invalid = cb_invalid;
    initial_cfg.user_data = (gpointer) "INITIAL";

    GtkWidget *initial_widget = champtexte_creer(&initial_cfg);
    gtk_widget_set_size_request(initial_cfg.entry, 420, -1);
    conteneur_ajouter(&main_container, initial_widget);

    // ======================================================
    // 4) SENSITIVE = FALSE (désactivé)
    // ======================================================
    add_label(box, "4) Champ désactivé (sensitive=FALSE).");

    ChampTexte disabled_cfg;
    champtexte_initialiser(&disabled_cfg);
    disabled_cfg.id_css = g_strdup("ct-disabled");
    disabled_cfg.texte = g_strdup("Je suis désactivé");
    disabled_cfg.placeholder = g_strdup("Disabled");
    disabled_cfg.sensitive = FALSE;

    // Set style properties using the common WidgetStyle structure
    disabled_cfg.style.bg_normal = g_strdup("#ffffff");
    disabled_cfg.style.fg_normal = g_strdup("#111111");
    disabled_cfg.style.epaisseur_bordure = 2;
    disabled_cfg.style.couleur_bordure = g_strdup("#cfcfcf");
    disabled_cfg.style.rayon_arrondi = 12;
    disabled_cfg.style.gras = FALSE;
    disabled_cfg.style.italique = FALSE;
    disabled_cfg.style.taille_texte_px = 14;
    disabled_cfg.style.couleur_bordure_error = g_strdup("#ff3b30");
    disabled_cfg.style.bg_error = g_strdup("#fff1f2");

    disabled_cfg.on_change = cb_change;
    disabled_cfg.on_activate = cb_activate;
    disabled_cfg.on_invalid = cb_invalid;
    disabled_cfg.user_data = (gpointer) "DISABLED";

    GtkWidget *disabled_widget = champtexte_creer(&disabled_cfg);
    gtk_widget_set_size_request(disabled_cfg.entry, 420, -1);
    conteneur_ajouter(&main_container, disabled_widget);

    // ======================================================
    // 5) TEST MANUEL invalid + error_message (pour vérifier visuellement)
    // ======================================================
    add_label(box, "5) Force invalid au démarrage (test invalid + error_message).");

    ChampTexte forced_cfg;
    champtexte_initialiser(&forced_cfg);
    forced_cfg.id_css = g_strdup("ct-forced");
    forced_cfg.placeholder = g_strdup("Ce champ est forcé en erreur au démarrage");
    forced_cfg.required = FALSE;

    // Set style properties using the common WidgetStyle structure
    forced_cfg.style.bg_normal = g_strdup("#ffffff");
    forced_cfg.style.fg_normal = g_strdup("#111111");
    forced_cfg.style.epaisseur_bordure = 2;
    forced_cfg.style.couleur_bordure = g_strdup("#cfcfcf");
    forced_cfg.style.rayon_arrondi = 12;
    forced_cfg.style.gras = FALSE;
    forced_cfg.style.italique = FALSE;
    forced_cfg.style.taille_texte_px = 14;
    forced_cfg.style.couleur_bordure_error = g_strdup("#ff3b30");
    forced_cfg.style.bg_error = g_strdup("#fff1f2");

    forced_cfg.on_change = cb_change;
    forced_cfg.on_activate = cb_activate;
    forced_cfg.on_invalid = cb_invalid;
    forced_cfg.user_data = (gpointer) "FORCED";

    GtkWidget *forced_widget = champtexte_creer(&forced_cfg);
    gtk_widget_set_size_request(forced_cfg.entry, 420, -1);

    // Forcer une erreur pour tester invalid + label + style rouge
    champtexte_set_invalid(&forced_cfg, "Erreur forcée (test)");

    conteneur_ajouter(&main_container, forced_widget);

    // ======================================================
    // Petit hint
    // ======================================================
    add_label(box, "Regarde la console CLion : on_change / on_activate / on_invalid s’affichent.");

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("org.gtk.test_champ_texte_full", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
