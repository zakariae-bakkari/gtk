#include <gtk/gtk.h>
#include "../widgets/headers/fenetre.h"
#include "../widgets/headers/conteneur.h"
#include "../widgets/headers/champ_motdepasse.h"
#include "../widgets/headers/champ_nombre.h"
#include <stdio.h>

/* --- Callbacks pour les evenements --- */

static void on_input_text_changed(GtkEditable *editable, gpointer data)
{
    const char *id = (const char *)data;
    printf("[INPUT] changed: %s -> '%s'\n", id, gtk_editable_get_text(editable));
}

static void on_input_text_activate(GtkEntry *entry, gpointer data)
{
    const char *id = (const char *)data;
    printf("[INPUT] activate (submit): %s -> '%s'\n", id, gtk_editable_get_text(GTK_EDITABLE(entry)));
}

static void on_input_number_changed(GtkSpinButton *spin, gpointer data)
{
    const char *id = (const char *)data;
    double value = gtk_spin_button_get_value(spin);
    printf("[INPUT] number changed: %s -> %.2f\n", id, value);
}

static void on_input_invalid(GtkWidget *widget, const char *message, gpointer data)
{
    const char *id = (const char *)data;
    printf("[INPUT][INVALID] %s: %s\n", id, message);
}

/* --- Fonction principale de creation de l'interface --- */

static void activate(GtkApplication *app, gpointer user_data)
{
    printf("GTK4 Password Input Test\n");

    /* ========== FENETRE PRINCIPALE ========== */
    Fenetre main_window;
    fenetre_initialiser(&main_window);
    main_window.title = "Password Input Test";
    main_window.taille.width = 400;
    main_window.taille.height = 300;
    main_window.color_bg = "#f5f5f5";
    main_window.icon_path = "application-x-executable-symbolic";
    main_window.titre_align = TITRE_ALIGN_GAUCHE;
    main_window.bouton_agrandir = false;

    GtkWidget *window = fenetre_creer(&main_window);
    gtk_window_set_application(GTK_WINDOW(window), app);

    /* ========== CONTENEUR PRINCIPAL (Vertical) ========== */
    Conteneur main_container;
    conteneur_initialiser(&main_container);
    main_container.orientation = CONTENEUR_VERTICAL;
    main_container.espacement = 15;
    main_container.marges.haut = 20;
    main_container.marges.bas = 20;
    main_container.marges.gauche = 20;
    main_container.marges.droite = 20;
    main_container.couleur_fond = "#ffffff";
    main_container.enfants_hexpand = false;

    GtkWidget *main_box = conteneur_creer(&main_container);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    /* ========== PASSWORD INPUT (ChampMotDePasse) ========== */
    GtkWidget *lbl_pwd = gtk_label_new_with_mnemonic("_Password:");

    ChampMotDePasse *cpw = g_new0(ChampMotDePasse, 1);
    champ_motdepasse_initialiser(cpw);

    // All attributes
    cpw->id_css = "input_pwd";
    cpw->placeholder = "Enter a strong password";
    cpw->max_length = 5;
    cpw->required = true;
    cpw->policy.min_len = 1;
    cpw->policy.require_digit = false;
    cpw->policy.require_upper = false;
    cpw->policy.require_symbol = false;
    cpw->reveal_toggle = true;
    cpw->sensitive = true;
    cpw->style.epaisseur_bordure = 2;
    cpw->style.couleur_bordure = "blue";
    cpw->style.rayon_arrondi = 50;
    cpw->style.gras = true;
    cpw->style.italique = false;
    cpw->style.taille_texte_px = 14;
    cpw->on_change = on_input_text_changed;
    cpw->on_activate = on_input_text_activate;
    cpw->on_invalid = on_input_invalid;
    cpw->user_data = "password";

    GtkWidget *w_pwd = champ_motdepasse_creer(cpw);
    g_signal_connect(w_pwd, "destroy", G_CALLBACK(g_free), cpw);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_pwd), w_pwd);

    conteneur_ajouter(&main_container, lbl_pwd);
    conteneur_ajouter(&main_container, w_pwd);

    /* ========== NUMBER INPUT (ChampNombre) ========== */
    GtkWidget *lbl_num = gtk_label_new_with_mnemonic("_Number:");

    ChampNombre *cn = g_new0(ChampNombre, 1);
    champ_nombre_initialiser(cn);

    // All attributes
    cn->id_css = "input_number";
    cn->min = 0;
    cn->max = 20;
    cn->step = 1;
    cn->digits = 0;
    cn->wrap = true;
    cn->valeur = 10;
    cn->required = true;
    cn->style.epaisseur_bordure = 2;
    cn->style.couleur_bordure = "green";
    cn->style.rayon_arrondi = 50;
    cn->style.gras = true;
    cn->style.italique = false;
    cn->style.taille_texte_px = 14;
    cn->on_change = on_input_number_changed;
    cn->on_invalid = on_input_invalid;
    cn->user_data = "number";

    GtkWidget *w_num = champ_nombre_creer(cn);
    g_signal_connect(w_num, "destroy", G_CALLBACK(g_free), cn);
    gtk_label_set_mnemonic_widget(GTK_LABEL(lbl_num), w_num);

    conteneur_ajouter(&main_container, lbl_num);
    conteneur_ajouter(&main_container, w_num);

    /* ========== AFFICHAGE DE LA FENETRE ========== */
    printf("[OK] Password and number inputs created successfully!\n");
    printf("[OK] Window size: %dx%d pixels\n\n", main_window.taille.width, main_window.taille.height);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("org.zcode.password.test", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
