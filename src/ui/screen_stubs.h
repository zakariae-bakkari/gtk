#ifndef SCREEN_STUBS_H
#define SCREEN_STUBS_H

#include <gtk/gtk.h>

/* Simple placeholder screens used so main can build while only bassin is implemented */
GtkWidget *screen_accueil_create(void);
GtkWidget *screen_createur_create(void);
GtkWidget *screen_predateur_create(void);
GtkWidget *screen_survie_create(void);

#endif // SCREEN_STUBS_H
