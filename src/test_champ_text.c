#include "../widgets/headers/champ_texte.h"
#include "../widgets/headers/conteneur.h"
#include <gtk/gtk.h>
#include <stdio.h>

// ---------------- Callbacks de test ----------------
static void cb_change(GtkEditable *editable, gpointer user_data)
{
    const char *name = (const char*)user_data;
    printf("[on_change] %s -> '%s'\n", name, gtk_editable_get_text(editable));
}

static void cb_activate(GtkEntry *entry, gpointer user_data)
{
    const char *name = (const char*)user_data;
    printf("[on_activate] %s (ENTER) -> '%s'\n", name, gtk_editable_get_text(GTK_EDITABLE(entry)));
}

static void cb_invalid(GtkWidget *widget, const char *message, gpointer user_data)
{
    (void)widget;
    const char *name = (const char*)user_data;
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

    // -------- Style commun (minimal) --------
    ChampTexteStyle st = {0};
    st.bg_normal = "#ffffff";
    st.fg_normal = "#111111";
    st.placeholder_color = "#888888";

    st.epaisseur_bordure = 2;
    st.couleur_bordure = "#cfcfcf";
    st.rayon_arrondi = 12;

    st.gras = FALSE;
    st.italique = FALSE;
    st.taille_texte_px = 14;

    // erreur
    st.couleur_bordure_error = "#ff3b30";
    st.bg_error = "#fff1f2";

    // ======================================================
    // 1) EMAIL : required + regex (format email)
    // ======================================================
    add_label(box, "1) Email (required + regex). Quitte le champ (click ailleurs) ou ENTER pour valider.");

    ChampTexte *email = champtexte_new("ct-email");
    champtexte_set_placeholder(email, "ex: abc@gmail.com");
    champtexte_set_required(email, TRUE);

    GError *err = NULL;
    if (!champtexte_set_regex(email, "^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$", &err)) {
        printf("[REGEX ERROR] EMAIL: %s\n", err ? err->message : "unknown");
        g_clear_error(&err);
    }

    champtexte_set_style(email, &st);
    champtexte_apply_style(email);

    champtexte_set_callbacks(email, cb_change, cb_activate, cb_invalid, (gpointer)"EMAIL");

    // taille (largeur)
    gtk_widget_set_size_request(email->entry, 420, -1);

    conteneur_ajouter(&main_container, champtexte_widget(email));

    // ======================================================
    // 2) CIN / chiffres : max_length + required + regex {8}
    // ======================================================
    add_label(box, "2) CIN (8 chiffres) : max_length=8 + required + regex ^[0-9]{8}$");

    ChampTexte *cin = champtexte_new("ct-cin");
    champtexte_set_placeholder(cin, "8 chiffres (ex: 12345678)");
    champtexte_set_max_length(cin, 8);
    champtexte_set_required(cin, TRUE);

    if (!champtexte_set_regex(cin, "^[0-9]{8}$", &err)) {
        printf("[REGEX ERROR] CIN: %s\n", err ? err->message : "unknown");
        g_clear_error(&err);
    }

    champtexte_set_style(cin, &st);
    champtexte_apply_style(cin);

    champtexte_set_callbacks(cin, cb_change, cb_activate, cb_invalid, (gpointer)"CIN");

    gtk_widget_set_size_request(cin->entry, 300, -1);

    conteneur_ajouter(&main_container, champtexte_widget(cin));

    // ======================================================
    // 3) TEXTE INITIAL + required=false (pas d’erreur si vide)
    // ======================================================
    add_label(box, "3) Texte initial + required=FALSE (pas d'erreur si vide).");

    ChampTexte *initial = champtexte_new("ct-initial");
    champtexte_set_text(initial, "Texte initial");
    champtexte_set_placeholder(initial, "placeholder (visible seulement si vide)");
    champtexte_set_required(initial, FALSE); // même si vide => pas d’erreur

    champtexte_set_style(initial, &st);
    champtexte_apply_style(initial);

    champtexte_set_callbacks(initial, cb_change, cb_activate, cb_invalid, (gpointer)"INITIAL");

    gtk_widget_set_size_request(initial->entry, 420, -1);

    conteneur_ajouter(&main_container, champtexte_widget(initial));

    // ======================================================
    // 4) SENSITIVE = FALSE (désactivé)
    // ======================================================
    add_label(box, "4) Champ désactivé (sensitive=FALSE).");

    ChampTexte *disabled = champtexte_new("ct-disabled");
    champtexte_set_text(disabled, "Je suis désactivé");
    champtexte_set_placeholder(disabled, "Disabled");
    champtexte_set_sensitive(disabled, FALSE); // grisé, pas d'interaction

    champtexte_set_style(disabled, &st);
    champtexte_apply_style(disabled);

    champtexte_set_callbacks(disabled, cb_change, cb_activate, cb_invalid, (gpointer)"DISABLED");

    gtk_widget_set_size_request(disabled->entry, 420, -1);

    conteneur_ajouter(&main_container, champtexte_widget(disabled));

    // ======================================================
    // 5) TEST MANUEL invalid + error_message (pour vérifier visuellement)
    // ======================================================
    add_label(box, "5) Force invalid au démarrage (test invalid + error_message).");

    ChampTexte *forced = champtexte_new("ct-forced");
    champtexte_set_placeholder(forced, "Ce champ est forcé en erreur au démarrage");
    champtexte_set_required(forced, FALSE);

    champtexte_set_style(forced, &st);
    champtexte_apply_style(forced);

    champtexte_set_callbacks(forced, cb_change, cb_activate, cb_invalid, (gpointer)"FORCED");

    gtk_widget_set_size_request(forced->entry, 420, -1);

    // Forcer une erreur pour tester invalid + label + style rouge
    champtexte_set_invalid(forced, "Erreur forcée (test)");

    conteneur_ajouter(&main_container, champtexte_widget(forced));

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