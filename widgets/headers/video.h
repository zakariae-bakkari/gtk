#ifndef VIDEO_H
#define VIDEO_H

#include <gtk/gtk.h>
#include "common.h"

typedef struct
{
   GtkWidget *container;
   GtkWidget *widget;
   GtkWidget *controls;
   GtkWidget *label_legende;
   GtkWidget *label_erreur;

   char *id_css;
   char *file_path;
   char *resource_path;
   char *legende;
   char *legende_couleur;

   int width;
   int height;
   int legende_taille_px;
   int rayon_arrondi;

   gboolean autoplay;
   gboolean loop;
   gboolean controles;
   gboolean sensitive;

   WidgetAlignment halign;
   WidgetStyle style;
} Video;

void video_initialiser(Video *cfg);
GtkWidget *video_creer(Video *cfg);
void video_free(Video *cfg);

void video_set_from_file(Video *cfg, const char *file_path);
void video_set_from_resource(Video *cfg, const char *resource_path);
void video_set_size(Video *cfg, int width, int height);
void video_set_legende(Video *cfg, const char *legende);
void video_set_halign(Video *cfg, WidgetAlignment halign);

#endif // VIDEO_H
