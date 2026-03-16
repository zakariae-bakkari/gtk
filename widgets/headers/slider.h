#ifndef SLIDER_H
#define SLIDER_H

#include <gtk/gtk.h>
#include "common.h"

/**
 * Orientation du slider
 */
typedef enum
{
   SLIDER_HORIZONTAL,
   SLIDER_VERTICAL
} SliderOrientation;

/**
 * Position des marques (ticks) sur le slider
 */
typedef enum
{
   SLIDER_MARQUES_AUCUNE,  // Pas de marques
   SLIDER_MARQUES_DESSUS,  // Au-dessus (horizontal) / à gauche (vertical)
   SLIDER_MARQUES_DESSOUS, // En-dessous (horizontal) / à droite (vertical)
   SLIDER_MARQUES_LES_DEUX // Des deux côtés
} SliderMarquesPos;

/**
 * Callback déclenché quand la valeur du slider change
 */
typedef void (*SliderOnChange)(GtkRange *range, double valeur, gpointer user_data);

/**
 * Structure principale SLIDER (GtkScale)
 */
typedef struct
{
   GtkWidget *widget;       // GtkScale
   GtkWidget *container;    // GtkBox vertical (scale + label_valeur)
   GtkWidget *label_valeur; // GtkLabel affichant la valeur courante (optionnel)

   char *id_css; // ID CSS unique

   // Bornes et valeur
   double min;
   double max;
   double step;
   double valeur; // Valeur initiale
   guint digits;  // Nombre de décimales affichées (0 = entier)

   // Orientation
   SliderOrientation orientation;

   // Comportement
   gboolean afficher_valeur; // Afficher la valeur directement sur le slider
   gboolean afficher_label;  // Afficher un label séparé sous/à côté du slider
   gboolean inverser;        // Inverser la direction (max à gauche/haut)
   gboolean sensitive;       // Actif / inactif

   // Marques
   SliderMarquesPos marques_pos; // Position des marques
   double marques_step;          // Intervalle entre les marques (0 = aucune)

   // Taille
   WidgetSize size;

   // Style
   WidgetStyle style;

   // Événements
   SliderOnChange on_change;
   gpointer user_data;

} Slider;

/* -------------------------------------------------------------------------
 * Prototypes
 * ------------------------------------------------------------------------- */

void slider_initialiser(Slider *cfg);
GtkWidget *slider_creer(Slider *cfg);

// Getter
double slider_get_valeur(Slider *cfg);

// Setters
void slider_set_valeur(Slider *cfg, double valeur);
void slider_set_bornes(Slider *cfg, double min, double max);
void slider_set_step(Slider *cfg, double step);
void slider_set_digits(Slider *cfg, guint digits);
void slider_set_orientation(Slider *cfg, SliderOrientation orientation);
void slider_set_inverser(Slider *cfg, gboolean inverser);
void slider_set_sensitive(Slider *cfg, gboolean sensitive);
void slider_set_afficher_valeur(Slider *cfg, gboolean afficher);
void slider_set_marques(Slider *cfg, SliderMarquesPos pos, double step);
void slider_set_size(Slider *cfg, int width, int height);

void slider_free(Slider *cfg);

#endif // SLIDER_H
