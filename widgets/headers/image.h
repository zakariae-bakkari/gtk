#ifndef IMAGE_H
#define IMAGE_H

#include <gtk/gtk.h>
#include "common.h"

/**
 * Mode de redimensionnement de l'image
 */
typedef enum
{
   IMAGE_FIT_NONE,    // Taille originale (pas de redimensionnement)
   IMAGE_FIT_FILL,    // Remplit le conteneur (peut déformer)
   IMAGE_FIT_CONTAIN, // Redimensionne en gardant le ratio (sans rogner)
   IMAGE_FIT_COVER,   // Remplit en gardant le ratio (peut rogner)
} ImageFitMode;

/**
 * Source de l'image
 */
typedef enum
{
   IMAGE_SOURCE_FILE,      // Chemin vers un fichier local
   IMAGE_SOURCE_ICON_NAME, // Nom d'icône système (ex: "image-missing-symbolic")
   IMAGE_SOURCE_PIXBUF,    // GdkPixbuf fourni directement
} ImageSourceType;

typedef struct
{
   /**
    * IMPORTANT : c'est `container` qu'il faut ajouter au widget parent.
    * `container` est un GtkBox vertical qui empile :
    *   1. cfg->widget       (GtkPicture ou GtkImage)
    *   2. cfg->label_legende (GtkLabel, caché si aucune légende)
    */
   GtkWidget *container; // GtkBox vertical — à passer à conteneur_ajouter()
   GtkWidget *widget;    // GtkPicture — l'image elle-même

   GtkWidget *label_legende; // GtkLabel de légende (optionnel)

   char *id_css; // ID CSS appliqué au widget image

   // Source
   ImageSourceType source_type;
   char *file_path;   // Chemin fichier (IMAGE_SOURCE_FILE)
   char *icon_name;   // Nom icône (IMAGE_SOURCE_ICON_NAME)
   GdkPixbuf *pixbuf; // Pixbuf (IMAGE_SOURCE_PIXBUF)

   // Légende
   char *legende; // Texte de légende sous l'image (NULL = aucune)

   // Dimensions (0 = auto)
   int width;
   int height;

   // Comportement
   ImageFitMode fit_mode; // Mode de redimensionnement
   gboolean can_shrink;   // Autorise la réduction sous la taille naturelle
   gboolean sensitive;    // Actif/inactif

   // Alignement
   WidgetAlignment halign; // Alignement horizontal du container

   // Style du container et de la légende
   WidgetStyle style;
   char *legende_couleur; // Couleur du texte de légende (défaut : "#7f8c8d")
   int legende_taille_px; // Taille de police de la légende en px (défaut : 11px)

   // Rayon des coins arrondis de l'image (via CSS)
   int rayon_arrondi;

} Image;

/* Cycle de vie */
void image_initialiser(Image *cfg);
GtkWidget *image_creer(Image *cfg); // Retourne cfg->container
void image_free(Image *cfg);

/* Setters */
void image_set_from_file(Image *cfg, const char *file_path);
void image_set_from_icon_name(Image *cfg, const char *icon_name);
void image_set_from_pixbuf(Image *cfg, GdkPixbuf *pixbuf);
void image_set_size(Image *cfg, int width, int height);
void image_set_fit_mode(Image *cfg, ImageFitMode fit_mode);
void image_set_legende(Image *cfg, const char *legende);
void image_set_sensitive(Image *cfg, gboolean sensitive);
void image_set_halign(Image *cfg, WidgetAlignment halign);

#endif // IMAGE_H
