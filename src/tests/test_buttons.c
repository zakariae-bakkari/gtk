#include <gtk/gtk.h>
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/bouton.h"
#include "../../widgets/headers/bouton_radio.h"
#include "../../widgets/headers/bouton_checklist.h"
#include <stdio.h>

// Callback functions for button testing
static void on_simple_button_click(GtkWidget *widget, gpointer user_data)
{
    printf("Simple button clicked!\n");
    gtk_button_set_label(GTK_BUTTON(widget), "Clicked!");
}

static void on_styled_button_click(GtkWidget *widget, gpointer user_data)
{
    printf("Styled button clicked!\n");
    // Change button style after click
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, "button { background: #4CAF50; color: white; }");

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider(context,
                                  GTK_STYLE_PROVIDER(provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

static void on_disabled_button_toggle(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *target_button = GTK_WIDGET(user_data);
    gboolean is_sensitive = gtk_widget_get_sensitive(target_button);
    gtk_widget_set_sensitive(target_button, !is_sensitive);

    const char *new_label = is_sensitive ? "Enable Button" : "Disable Button";
    gtk_button_set_label(GTK_BUTTON(widget), new_label);

    printf("Button %s\n", is_sensitive ? "disabled" : "enabled");
}

static void on_radio_button_changed(GtkCheckButton *button, gpointer user_data)
{
    if (gtk_check_button_get_active(button)) {
        const char *label = gtk_button_get_label(GTK_BUTTON(button));
        printf("Radio button selected: %s\n", label);
    }
}

static void on_checkbox_toggled(GtkCheckButton *button, gpointer user_data)
{
    gboolean is_active = gtk_check_button_get_active(button);
    const char *label = gtk_button_get_label(GTK_BUTTON(button));
    printf("Checkbox '%s' %s\n", label, is_active ? "checked" : "unchecked");
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
    // 1. Configuration de la fenêtre
    Fenetre fenetre_config;
    fenetre_initialiser(&fenetre_config);
    fenetre_config.title = "Test des Boutons - Démo Complète";
    fenetre_config.taille.width = 900;
    fenetre_config.taille.height = 700;

    // Activer le défilement vertical pour la fenêtre
    fenetre_set_scrollable(&fenetre_config, SCROLL_VERTICAL);

    GtkWidget *window = fenetre_creer(&fenetre_config);
    gtk_application_add_window(app, GTK_WINDOW(window));

    // Conteneur principal avec défilement
    Conteneur conteneur_principal;
    conteneur_initialiser(&conteneur_principal);
    conteneur_principal.orientation = CONTENEUR_VERTICAL;
    conteneur_principal.espacement = 25;
    conteneur_principal.padding.haut = 20;
    conteneur_principal.padding.bas = 20;
    conteneur_principal.padding.gauche = 20;
    conteneur_principal.padding.droite = 20;

    GtkWidget *main_container = conteneur_creer(&conteneur_principal);

    // ===== SECTION 1: BOUTONS SIMPLES =====
    GtkWidget *title1 = gtk_label_new("BOUTONS SIMPLES");
    gtk_label_set_markup(GTK_LABEL(title1), "<b><big>BOUTONS SIMPLES</big></b>");
    conteneur_ajouter(&conteneur_principal, title1);

    Conteneur section1;
    conteneur_initialiser(&section1);
    section1.orientation = CONTENEUR_HORIZONTAL;
    section1.espacement = 15;
    section1.couleur_fond = "#E3F2FD";
    section1.bordure_largeur = 2;
    section1.bordure_couleur = "#2196F3";
    section1.bordure_rayon = 8;
    section1.padding.haut = 15;
    section1.padding.bas = 15;
    section1.padding.gauche = 15;
    section1.padding.droite = 15;

    GtkWidget *container1 = conteneur_creer(&section1);

    // Bouton simple
    Bouton btn_simple;
    bouton_initialiser(&btn_simple);
    btn_simple.texte = "Bouton Simple";
    btn_simple.on_clic = on_simple_button_click;

    GtkWidget *simple_button = bouton_creer(&btn_simple);
    conteneur_ajouter(&section1, simple_button);

    // Bouton avec style personnalisé
    Bouton btn_styled;
    bouton_initialiser(&btn_styled);
    btn_styled.texte = "Bouton Stylé";
    btn_styled.on_clic = on_styled_button_click;
    btn_styled.style.bg_normal = "#FF5722";
    btn_styled.style.fg_normal = "white";
    btn_styled.style.epaisseur_bordure = 2;
    btn_styled.style.couleur_bordure = "#D32F2F";
    btn_styled.style.rayon_arrondi = 15;
    btn_styled.style.gras = true;

    GtkWidget *styled_button = bouton_creer(&btn_styled);
    conteneur_ajouter(&section1, styled_button);

    // Bouton désactivé par défaut
    Bouton btn_disabled;
    bouton_initialiser(&btn_disabled);
    btn_disabled.texte = "Bouton Désactivé";
    btn_disabled.est_actif = false;

    GtkWidget *disabled_button = bouton_creer(&btn_disabled);
    conteneur_ajouter(&section1, disabled_button);

    // Bouton pour activer/désactiver
    Bouton btn_toggle;
    bouton_initialiser(&btn_toggle);
    btn_toggle.texte = "Enable Button";
    btn_toggle.on_clic = on_disabled_button_toggle;
    btn_toggle.user_data = disabled_button;

    GtkWidget *toggle_button = bouton_creer(&btn_toggle);
    conteneur_ajouter(&section1, toggle_button);

    conteneur_ajouter(&conteneur_principal, container1);

    // ===== SECTION 2: BOUTONS RADIO =====
    GtkWidget *title2 = gtk_label_new("BOUTONS RADIO");
    gtk_label_set_markup(GTK_LABEL(title2), "<b><big>BOUTONS RADIO</big></b>");
    conteneur_ajouter(&conteneur_principal, title2);

    Conteneur section2;
    conteneur_initialiser(&section2);
    section2.orientation = CONTENEUR_VERTICAL;
    section2.espacement = 10;
    section2.couleur_fond = "#F3E5F5";
    section2.bordure_largeur = 2;
    section2.bordure_couleur = "#9C27B0";
    section2.bordure_rayon = 8;
    section2.padding.haut = 15;
    section2.padding.bas = 15;
    section2.padding.gauche = 15;
    section2.padding.droite = 15;

    GtkWidget *container2 = conteneur_creer(&section2);


    // Instructions à l'utilisateur
    GtkWidget *instructions = gtk_label_new("Instructions: Testez tous les boutons pour voir leurs différents comportements!");
    gtk_label_set_markup(GTK_LABEL(instructions), "<i>Instructions: Testez tous les boutons pour voir leurs différents comportements!</i>");
    conteneur_ajouter(&conteneur_principal, instructions);

    // Attacher le conteneur principal à la fenêtre
    if (fenetre_config.scroll_mode != SCROLL_NONE && fenetre_config.scroll_widget) {
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fenetre_config.scroll_widget), main_container);
    } else {
        gtk_window_set_child(GTK_WINDOW(window), main_container);
    }

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("com.example.test.buttons", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
