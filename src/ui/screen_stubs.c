#include "screen_stubs.h"

static GtkWidget *placeholder_with_label(const char *text)
{
   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
   GtkWidget *label = gtk_label_new(text);
   gtk_box_append(GTK_BOX(box), label);
   return box;
}

GtkWidget *screen_accueil_create(void)
{
   return placeholder_with_label("Écran accueil (placeholder)");
}

GtkWidget *screen_createur_create(void)
{
   return placeholder_with_label("Écran createur (placeholder)");
}

GtkWidget *screen_predateur_create(void)
{
   return placeholder_with_label("Écran prédateur (placeholder)");
}

GtkWidget *screen_survie_create(void)
{
   return placeholder_with_label("Écran survie (placeholder)");
}
