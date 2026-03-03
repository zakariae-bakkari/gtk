#include <gtk/gtk.h>
#include "image.h"

static void activate(GtkApplication *app, gpointer user_data)
{
    // ── Fenêtre principale ──────────────────────────────────────────
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Test — Widget Image");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 500);

    // Scroll global
    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_window_set_child(GTK_WINDOW(window), scroll);

    // Conteneur principal vertical
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), vbox);

    // ── Titre ───────────────────────────────────────────────────────
    GtkWidget *titre = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(titre), "<b><big>Tests du widget Image</big></b>");
    gtk_box_append(GTK_BOX(vbox), titre);

    // ── Test 1 : Depuis un fichier local ────────────────────────────
    GtkWidget *lbl1 = gtk_label_new("1. Image depuis un fichier (300×200, CONTAIN, rayon=12)");
    gtk_label_set_xalign(GTK_LABEL(lbl1), 0.0f);
    gtk_box_append(GTK_BOX(vbox), lbl1);

    Image img1;
    image_initialiser(&img1);
    img1.id_css = "img_fichier";
    img1.width = 300;
    img1.height = 200;
    img1.fit_mode = IMAGE_FIT_CONTAIN;
    img1.rayon_arrondi = 12;
    img1.legende = "Logo zcode — mode CONTAIN";
    img1.legende_taille_px = 12;
    image_set_from_file(&img1, "resources/images/zcode.png");
    gtk_box_append(GTK_BOX(vbox), image_creer(&img1));

    // ── Séparateur ──────────────────────────────────────────────────
    gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // ── Test 2 : Mode COVER ─────────────────────────────────────────
    GtkWidget *lbl2 = gtk_label_new("2. Même fichier (200×200, COVER, rayon=100 → cercle)");
    gtk_label_set_xalign(GTK_LABEL(lbl2), 0.0f);
    gtk_box_append(GTK_BOX(vbox), lbl2);

    Image img2;
    image_initialiser(&img2);
    img2.id_css = "img_cover";
    img2.width = 200;
    img2.height = 200;
    img2.fit_mode = IMAGE_FIT_COVER;
    img2.rayon_arrondi = 100;
    img2.legende = "Mode COVER — coins arrondis 100px";
    img2.legende_couleur = "#8e44ad";
    image_set_from_file(&img2, "resources/images/zcode.png");
    gtk_box_append(GTK_BOX(vbox), image_creer(&img2));

    // ── Séparateur ──────────────────────────────────────────────────
    gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // ── Test 3 : Mode FILL ──────────────────────────────────────────
    GtkWidget *lbl3 = gtk_label_new("3. Mode FILL (400×120) — peut déformer");
    gtk_label_set_xalign(GTK_LABEL(lbl3), 0.0f);
    gtk_box_append(GTK_BOX(vbox), lbl3);

    Image img3;
    image_initialiser(&img3);
    img3.id_css = "img_fill";
    img3.width = 400;
    img3.height = 120;
    img3.fit_mode = IMAGE_FIT_FILL;
    img3.legende = "Mode FILL";
    image_set_from_file(&img3, "resources/images/zcode.png");
    gtk_box_append(GTK_BOX(vbox), image_creer(&img3));

    // ── Séparateur ──────────────────────────────────────────────────
    gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // ── Test 4 : Icône système ──────────────────────────────────────
    GtkWidget *lbl4 = gtk_label_new("4. Source ICON_NAME (48px, sans légende)");
    gtk_label_set_xalign(GTK_LABEL(lbl4), 0.0f);
    gtk_box_append(GTK_BOX(vbox), lbl4);

    Image img4;
    image_initialiser(&img4);
    img4.id_css = "img_icon";
    img4.width = 48;
    img4.height = 48;
    img4.halign = WIDGET_ALIGN_START;
    image_set_from_icon_name(&img4, "image-missing-symbolic");
    gtk_box_append(GTK_BOX(vbox), image_creer(&img4));

    // ── Séparateur ──────────────────────────────────────────────────
    gtk_box_append(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

    // ── Test 5 : Setter dynamique après création ────────────────────
    GtkWidget *lbl5 = gtk_label_new("5. Setter dynamique : image_set_legende() après image_creer()");
    gtk_label_set_xalign(GTK_LABEL(lbl5), 0.0f);
    gtk_box_append(GTK_BOX(vbox), lbl5);

    Image img5;
    image_initialiser(&img5);
    img5.id_css = "img_dynamic";
    img5.width = 250;
    img5.height = 150;
    img5.fit_mode = IMAGE_FIT_CONTAIN;
    img5.rayon_arrondi = 6;
    img5.style.epaisseur_bordure = 2;
    img5.style.couleur_bordure = "#3498db";
    image_set_from_file(&img5, "resources/images/zcode.png");
    GtkWidget *w5 = image_creer(&img5);

    // Légende ajoutée dynamiquement après création
    image_set_legende(&img5, "Légende ajoutée après image_creer() — bordure bleue");
    gtk_box_append(GTK_BOX(vbox), w5);

    // ── Afficher ────────────────────────────────────────────────────
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("com.example.test_image",
                                              G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}