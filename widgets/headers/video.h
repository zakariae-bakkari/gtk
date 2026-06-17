#ifndef VIDEO_H
#define VIDEO_H

#include "common.h"

typedef struct
{
   Widget container;
   Widget widget;
   Widget controls;
   Widget label_legende;
   Widget label_erreur;

   char *id_css;
   char *file_path;
   char *resource_path;
   char *legende;
   char *legende_couleur;

   int width;
   int height;
   int legende_taille_px;
   int rayon_arrondi;

   bool autoplay;
   bool loop;
   bool controles;
   bool sensitive;

   WidgetAlignment halign;
   WidgetStyle style;
} Video;

void video_initialiser(Video *cfg);
Widget video_creer(Video *cfg);
void video_free(Video *cfg);

void video_set_from_file(Video *cfg, const char *file_path);
void video_set_from_resource(Video *cfg, const char *resource_path);
void video_set_size(Video *cfg, int width, int height);
void video_set_legende(Video *cfg, const char *legende);
void video_set_halign(Video *cfg, WidgetAlignment halign);

#endif // VIDEO_H
