#include "../../widgets/headers/champ_texte.h"
#include "../../widgets/headers/conteneur.h"
#include <gtk/gtk.h>
#include <stdio.h>

// -------------------- Enhanced Callback Test Functions ----------------
static void cb_change(GtkEditable *editable, gpointer user_data)
{
   const char *name = (const char *)user_data;
   printf("[CHANGE] %s -> '%s'\n", name, gtk_editable_get_text(editable));
}

static void cb_activate(GtkEntry *entry, gpointer user_data)
{
   const char *name = (const char *)user_data;
   printf("[ACTIVATE] %s (ENTER) -> '%s'\n", name, gtk_editable_get_text(GTK_EDITABLE(entry)));
}

static void cb_invalid(GtkWidget *widget, const char *message, gpointer user_data)
{
   (void)widget;
   const char *name = (const char *)user_data;
   printf("[INVALID] %s -> %s\n", name, message);
}

static void cb_valid(GtkWidget *widget, const char *message, gpointer user_data)
{
   (void)widget;
   const char *name = (const char *)user_data;
   printf("[VALID] %s -> %s\n", name, message);
}

static void cb_focus_in(GtkEditable *editable, gpointer user_data)
{
   const char *name = (const char *)user_data;
   printf("[FOCUS_IN] %s obtained focus\n", name);
}

static void cb_focus_out(GtkEditable *editable, gpointer user_data)
{
   const char *name = (const char *)user_data;
   printf("[FOCUS_OUT] %s lost focus -> '%s'\n", name, gtk_editable_get_text(editable));
}

static void cb_text_insert(GtkEditable *editable, gpointer user_data)
{
   const char *name = (const char *)user_data;
   printf("[TEXT_INSERT] %s inserting text\n", name);
}

static void cb_text_delete(GtkEditable *editable, gpointer user_data)
{
   const char *name = (const char *)user_data;
   printf("[TEXT_DELETE] %s deleting text\n", name);
}

static void activate(GtkApplication *app, gpointer user_data)
{
   (void)user_data;

   GtkWidget *window = gtk_application_window_new(app);
   gtk_window_set_title(GTK_WINDOW(window), "Test Enhanced ChampTexte Callbacks");
   gtk_window_set_default_size(GTK_WINDOW(window), 720, 400);

   // -------- Conteneur principal --------
   Conteneur main_container;
   conteneur_initialiser(&main_container);
   main_container.orientation = CONTENEUR_VERTICAL;
   main_container.align_x = ALIGNEMENT_CENTRE;
   main_container.align_y = ALIGNEMENT_CENTRE;
   main_container.espacement = 20;

   GtkWidget *main_box = conteneur_creer(&main_container);
   gtk_window_set_child(GTK_WINDOW(window), main_box);

   // ======================================================
   // TEST 1: All callbacks using new individual setters
   // ======================================================
   GtkWidget *label1 = gtk_label_new("Test Field 1 - Individual Callback Setters:");
   gtk_label_set_xalign(GTK_LABEL(label1), 0.0f);
   gtk_box_append(GTK_BOX(main_box), label1);

   ChampTexte cfg1;
   champtexte_initialiser(&cfg1);
   cfg1.css_class = g_strdup("test-field-1");
   cfg1.placeholder = g_strdup("Type here to test all callbacks");
   cfg1.required = TRUE;

   // Set style
   cfg1.style.bg_normal = g_strdup("#f8f9fa");
   cfg1.style.epaisseur_bordure = 2;
   cfg1.style.couleur_bordure = g_strdup("#007bff");
   cfg1.style.rayon_arrondi = 8;

   GtkWidget *widget1 = champtexte_creer(&cfg1);

   // Set callbacks individually
   champtexte_set_callback_change(&cfg1, cb_change, (gpointer) "FIELD1");
   champtexte_set_callback_activate(&cfg1, cb_activate, (gpointer) "FIELD1");
   champtexte_set_callback_invalid(&cfg1, cb_invalid, (gpointer) "FIELD1");
   champtexte_set_callback_valid(&cfg1, cb_valid, (gpointer) "FIELD1");
   champtexte_set_callback_focus_in(&cfg1, cb_focus_in, (gpointer) "FIELD1");
   champtexte_set_callback_focus_out(&cfg1, cb_focus_out, (gpointer) "FIELD1");
   champtexte_set_callback_text_insert(&cfg1, cb_text_insert, (gpointer) "FIELD1");
   champtexte_set_callback_text_delete(&cfg1, cb_text_delete, (gpointer) "FIELD1");

   gtk_widget_set_size_request(cfg1.entry, 400, -1);
   gtk_box_append(GTK_BOX(main_box), widget1);

   // ======================================================
   // TEST 2: All callbacks using champtexte_set_all_callbacks
   // ======================================================
   GtkWidget *label2 = gtk_label_new("Test Field 2 - All Callbacks Set At Once:");
   gtk_label_set_xalign(GTK_LABEL(label2), 0.0f);
   gtk_box_append(GTK_BOX(main_box), label2);

   ChampTexte cfg2;
   champtexte_initialiser(&cfg2);
   cfg2.css_class = g_strdup("test-field-2");
   cfg2.placeholder = g_strdup("Email validation test");
   cfg2.required = TRUE;

   // Set email regex
   GError *err = NULL;
   if (!champtexte_set_regex(&cfg2, "^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$", &err))
   {
      printf("[REGEX ERROR] %s\n", err ? err->message : "unknown");
      g_clear_error(&err);
   }

   // Set style
   cfg2.style.bg_normal = g_strdup("#fff3cd");
   cfg2.style.epaisseur_bordure = 1;
   cfg2.style.couleur_bordure = g_strdup("#ffc107");
   cfg2.style.rayon_arrondi = 12;

   GtkWidget *widget2 = champtexte_creer(&cfg2);

   // Set all callbacks at once
   champtexte_set_all_callbacks(&cfg2,
                                cb_change,      // on_change
                                cb_activate,    // on_activate
                                cb_invalid,     // on_invalid
                                cb_valid,       // on_valid
                                cb_focus_in,    // on_focus_in
                                cb_focus_out,   // on_focus_out
                                cb_text_insert, // on_text_insert
                                cb_text_delete, // on_text_delete
                                (gpointer) "FIELD2");

   gtk_widget_set_size_request(cfg2.entry, 400, -1);
   gtk_box_append(GTK_BOX(main_box), widget2);

   // ======================================================
   // Instructions
   // ======================================================
   GtkWidget *instructions = gtk_label_new(
       "Instructions:\n"
       "• Type in fields to see change/insert/delete callbacks\n"
       "• Click between fields to see focus in/out callbacks\n"
       "• Press Enter to see activate callbacks\n"
       "• Field 1: Required field (try leaving empty)\n"
       "• Field 2: Email validation (try invalid email)\n"
       "• Check console for all callback messages");
   gtk_label_set_justify(GTK_LABEL(instructions), GTK_JUSTIFY_LEFT);
   gtk_box_append(GTK_BOX(main_box), instructions);

   gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
   GtkApplication *app = gtk_application_new("org.gtk.test_champ_texte_callbacks", G_APPLICATION_DEFAULT_FLAGS);
   g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

   int status = g_application_run(G_APPLICATION(app), argc, argv);
   g_object_unref(app);

   return status;
}