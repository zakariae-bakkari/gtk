#include "../widgets/headers/champ_texte.h"
#include <gtk/gtk.h>
#include <stdio.h>

static void cb_change(GtkEditable *editable, gpointer user_data)
{
    (void)user_data;
    printf("[on_change] -> '%s'\n", gtk_editable_get_text(editable));
}

static void cb_activate(GtkEntry *entry, gpointer user_data)
{
    (void)user_data;
    printf("[on_activate] ENTER -> '%s'\n", gtk_editable_get_text(GTK_EDITABLE(entry)));
}

static void cb_invalid(GtkWidget *widget, const char *message, gpointer user_data)
{
    (void)widget;
    (void)user_data;
    printf("[on_invalid] -> %s\n", message);
}

static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Test ChampTexte");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(box, 24);
    gtk_widget_set_margin_end(box, 24);
    gtk_widget_set_margin_top(box, 24);
    gtk_widget_set_margin_bottom(box, 24);
    gtk_window_set_child(GTK_WINDOW(window), box);

    // --- Un seul ChampTexte ---
    ChampTexte email;
    champtexte_initialiser(&email);
    email.id_css = "ct-email";

    champtexte_set_placeholder(&email, "ex: abc@gmail.com");
    champtexte_set_required(&email, TRUE);
    champtexte_set_size(&email, 340, -1);

    champtexte_creer(&email);
    champtexte_apply_style(&email);
    champtexte_set_callbacks(&email, cb_change, cb_activate, cb_invalid, NULL);

    gtk_box_append(GTK_BOX(box), champtexte_widget(&email));

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("org.gtk.test_champ_texte", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}