#ifndef SCREEN_HOME_H
#define SCREEN_HOME_H

#include <gtk/gtk.h>

/* Home and placeholder screens */
GtkWidget *screen_accueil_create(void);
GtkWidget *screen_createur_create(void);
GtkWidget *screen_predateur_create(void);
GtkWidget *screen_survie_create(void);

/* Navigation functions (defined in main.c) */
void nav_to_accueil(void);
void nav_to_bassin(void);
void nav_to_createur(void);
void nav_to_predateur(void);
void nav_to_survie(void);

#endif // SCREEN_HOME_H
