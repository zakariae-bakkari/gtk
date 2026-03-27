#include "../../widgets/headers/champ_nombre.h"
#include "../../widgets/headers/conteneur.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ---------------- Callbacks de test ----------------
static void cb_change(GtkEditable *editable, gpointer user_data)
{
   const char *name = (const char *)user_data;
   // For GtkSpinButton, we need to cast to get the value
   GtkSpinButton *spin = GTK_SPIN_BUTTON(editable);
   double value = gtk_spin_button_get_value(spin);
   printf("[on_change] %s -> %.2f\n", name, value);
}

static void cb_activate(GtkEntry *entry, gpointer user_data)
{
   const char *name = (const char *)user_data;
   GtkSpinButton *spin = GTK_SPIN_BUTTON(entry);
   double value = gtk_spin_button_get_value(spin);
   printf("[on_activate] %s (ENTER) -> %.2f\n", name, value);
}

static void cb_invalid(GtkWidget *widget, const char *message, gpointer user_data)
{
   (void)widget;
   const char *name = (const char *)user_data;
   printf("[on_invalid] %s -> %s\n", name, message);
}

// Helper pour ajouter un titre
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
   gtk_window_set_title(GTK_WINDOW(window), "Test ChampNombre avec common.h");
   gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

   // Conteneur principal
   Conteneur main_container;
   conteneur_initialiser(&main_container);
   main_container.orientation = CONTENEUR_VERTICAL;
   main_container.align_x = ALIGNEMENT_CENTRE;
   main_container.align_y = ALIGNEMENT_CENTRE;
   main_container.espacement = 15;

   GtkWidget *main_box = conteneur_creer(&main_container);
   gtk_window_set_child(GTK_WINDOW(window), main_box);

   // Style commun utilisant WidgetStyle de common.h
   WidgetStyle st = {0};
   widget_style_init(&st);
   st.bg_normal = malloc(strlen("#ffffff") + 1);
   strcpy(st.bg_normal, "#ffffff");
   st.fg_normal = malloc(strlen("#2c3e50") + 1);
   strcpy(st.fg_normal, "#2c3e50");
   st.epaisseur_bordure = 2;
   st.couleur_bordure = malloc(strlen("#3498db") + 1);
   strcpy(st.couleur_bordure, "#3498db");
   st.rayon_arrondi = 8;
   st.couleur_bordure_error = malloc(strlen("#e74c3c") + 1);
   strcpy(st.couleur_bordure_error, "#e74c3c");
   st.bg_error = malloc(strlen("#fff1f2") + 1);
   strcpy(st.bg_error, "#fff1f2");

   // ======================================================
   // 1) Champ nombre entier (0-100)
   // ======================================================
   add_label(main_box, "1) Nombre entier (0-100):");

   ChampNombre nombre1;
   champ_nombre_initialiser(&nombre1);
   g_free(nombre1.id_css);
   nombre1.id_css = malloc(strlen("nombre1") + 1);
   strcpy(nombre1.id_css, "nombre1");
   nombre1.min = 0.0;
   nombre1.max = 100.0;
   nombre1.step = 1.0;
   nombre1.digits = 0;
   nombre1.valeur = 50.0;

   champ_nombre_set_style(&nombre1, &st);
   GtkWidget *widget1 = champ_nombre_creer(&nombre1);
   champ_nombre_apply_style(&nombre1);
   champ_nombre_set_callbacks(&nombre1, cb_change, cb_activate, cb_invalid, (gpointer) "ENTIER");

   gtk_widget_set_size_request(widget1, 200, -1);
   conteneur_ajouter(&main_container, widget1);

   // ======================================================
   // 2) Champ nombre décimal (-10.0 à +10.0, 2 décimales)
   // ======================================================
   add_label(main_box, "2) Nombre décimal (-10.0 à +10.0, 2 décimales):");

   ChampNombre nombre2;
   champ_nombre_initialiser(&nombre2);
   g_free(nombre2.id_css);
   nombre2.id_css = malloc(strlen("nombre2") + 1);
   strcpy(nombre2.id_css, "nombre2");
   nombre2.min = -10.0;
   nombre2.max = 10.0;
   nombre2.step = 0.1;
   nombre2.digits = 2;
   nombre2.valeur = 0.0;

   champ_nombre_set_style(&nombre2, &st);
   GtkWidget *widget2 = champ_nombre_creer(&nombre2);
   champ_nombre_apply_style(&nombre2);
   champ_nombre_set_callbacks(&nombre2, cb_change, cb_activate, cb_invalid, (gpointer) "DECIMAL");

   gtk_widget_set_size_request(widget2, 200, -1);
   conteneur_ajouter(&main_container, widget2);

   // ======================================================
   // 3) Champ avec wrap activé (0-10, boucle)
   // ======================================================
   add_label(main_box, "3) Avec wrap (0-10, boucle après 10 -> 0):");

   ChampNombre nombre3;
   champ_nombre_initialiser(&nombre3);
   g_free(nombre3.id_css);
   nombre3.id_css = malloc(strlen("nombre3") + 1);
   strcpy(nombre3.id_css, "nombre3");
   nombre3.min = 0.0;
   nombre3.max = 10.0;
   nombre3.step = 1.0;
   nombre3.digits = 0;
   nombre3.wrap = TRUE;
   nombre3.valeur = 5.0;

   champ_nombre_set_style(&nombre3, &st);
   GtkWidget *widget3 = champ_nombre_creer(&nombre3);
   champ_nombre_apply_style(&nombre3);
   champ_nombre_set_callbacks(&nombre3, cb_change, cb_activate, cb_invalid, (gpointer) "WRAP");

   gtk_widget_set_size_request(widget3, 200, -1);
   conteneur_ajouter(&main_container, widget3);

   add_label(main_box, "Regarde la console : callbacks s'affichent lors des changements.");

   // Libérer le style temporaire
   widget_style_free(&st);

   gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
   GtkApplication *app = gtk_application_new("org.gtk.test_champ_nombre", G_APPLICATION_DEFAULT_FLAGS);
   g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

   int status = g_application_run(G_APPLICATION(app), argc, argv);
   g_object_unref(app);

   return status;
}
