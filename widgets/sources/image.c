#include "../headers/image.h"
#include <stdlib.h>
#include <string.h>

// ====================== CSS ======================

static void image_apply_css(Image *cfg)
{
   if (!cfg || !cfg->widget || !cfg->id_css)
      return;

   GtkCssProvider *provider = gtk_css_provider_new();
   char css[1024];
   char border_css[128] = "";

   // Construction locale du CSS de bordure sans allocation dynamique.
   if (cfg->style.couleur_bordure && cfg->style.epaisseur_bordure > 0)
   {
      snprintf(border_css, sizeof(border_css),
               "  border: %dpx solid %s;\n",
               cfg->style.epaisseur_bordure,
               cfg->style.couleur_bordure);
            }

   int leg_size = cfg->legende_taille_px > 0 ? cfg->legende_taille_px : 11;
   const char *leg_color = cfg->legende_couleur ? cfg->legende_couleur : "#7f8c8d";

   snprintf(css, sizeof(css),
            "picture#%s {\n"
            "  border-radius: %dpx;\n"
            "%s"
            "}\n"
            "label#%s_legende {\n"
            "  color: %s;\n"
            "  font-size: %dpx;\n"
            "  margin-top: 4px;\n"
            "}\n",
            cfg->id_css,
            cfg->rayon_arrondi,
            border_css,
            cfg->id_css,
            leg_color,
            leg_size);

   gtk_css_provider_load_from_string(provider, css);

   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);

   if (cfg->label_legende)
      gtk_style_context_add_provider(
          gtk_widget_get_style_context(cfg->label_legende),
          GTK_STYLE_PROVIDER(provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);

   g_object_unref(provider);
}


static GtkAlign widget_align_to_gtk(WidgetAlignment align)
{
   switch (align)
   {
   case WIDGET_ALIGN_CENTER:
      return GTK_ALIGN_CENTER;
   case WIDGET_ALIGN_END:
      return GTK_ALIGN_END;
   case WIDGET_ALIGN_FILL:
      return GTK_ALIGN_FILL;
   default:
      return GTK_ALIGN_START;
   }
}

static void image_load_source(Image *cfg)
{
   if (!cfg || !cfg->widget)
      return;

   switch (cfg->source_type)
   {
   case IMAGE_SOURCE_FILE:
      if (cfg->file_path)
         gtk_picture_set_filename(GTK_PICTURE(cfg->widget), cfg->file_path);
      break;

   case IMAGE_SOURCE_ICON_NAME:
      if (cfg->icon_name)
      {
         GtkIconTheme *theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
         int icon_size = (cfg->width > 0) ? cfg->width : 48;
         GtkIconPaintable *paintable = gtk_icon_theme_lookup_icon(
             theme, cfg->icon_name, NULL, icon_size, 1,
             GTK_TEXT_DIR_NONE, 0);
         if (paintable)
         {
            gtk_picture_set_paintable(GTK_PICTURE(cfg->widget), GDK_PAINTABLE(paintable));
            g_object_unref(paintable);
         }
      }
      break;

   case IMAGE_SOURCE_PIXBUF:
      if (cfg->pixbuf)
      {
         GdkTexture *texture = gdk_texture_new_for_pixbuf(cfg->pixbuf);
         gtk_picture_set_paintable(GTK_PICTURE(cfg->widget), GDK_PAINTABLE(texture));
         g_object_unref(texture);
      }
      break;
   }
}


void image_initialiser(Image *cfg)
{
   if (!cfg)
      return;
   memset(cfg, 0, sizeof(Image));

   cfg->id_css = malloc(strlen("image") + 1);
   strcpy(cfg->id_css, "image");
   cfg->source_type = IMAGE_SOURCE_FILE;
   cfg->file_path = NULL;
   cfg->icon_name = NULL;
   cfg->pixbuf = NULL;
   cfg->legende = NULL;
   cfg->width = 0;
   cfg->height = 0;
   cfg->fit_mode = IMAGE_FIT_CONTAIN;
   cfg->can_shrink = TRUE;
   cfg->sensitive = TRUE;
   cfg->halign = WIDGET_ALIGN_START;
   cfg->legende_couleur = NULL;
   cfg->legende_taille_px = 0;
   cfg->rayon_arrondi = 0;

   widget_style_init(&cfg->style);
}

GtkWidget *image_creer(Image *cfg)
{
   if (!cfg)
      return NULL;
   /* --- Container vertical --- */
   cfg->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   gtk_widget_set_halign(cfg->container, widget_align_to_gtk(cfg->halign));
   gtk_widget_set_hexpand(cfg->container, cfg->halign == WIDGET_ALIGN_FILL);
   /* --- GtkPicture --- */
   cfg->widget = gtk_picture_new();
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "image");
   /* Fit mode */
   switch (cfg->fit_mode)
   {
   case IMAGE_FIT_FILL:
      gtk_picture_set_content_fit(GTK_PICTURE(cfg->widget), GTK_CONTENT_FIT_FILL);
      break;
   case IMAGE_FIT_COVER:
      gtk_picture_set_content_fit(GTK_PICTURE(cfg->widget), GTK_CONTENT_FIT_COVER);
      break;
   case IMAGE_FIT_NONE:
      gtk_picture_set_content_fit(GTK_PICTURE(cfg->widget), GTK_CONTENT_FIT_SCALE_DOWN);
      break;
   default: /* IMAGE_FIT_CONTAIN */
      gtk_picture_set_content_fit(GTK_PICTURE(cfg->widget), GTK_CONTENT_FIT_CONTAIN);
      break;
   }
   gtk_picture_set_can_shrink(GTK_PICTURE(cfg->widget), cfg->can_shrink);
   gtk_widget_set_sensitive(cfg->widget, cfg->sensitive);
   /* Dimensions */
   if (cfg->width > 0 || cfg->height > 0)
      gtk_widget_set_size_request(cfg->widget, cfg->width > 0 ? cfg->width : -1, cfg->height > 0 ? cfg->height : -1);

   if (cfg->halign == WIDGET_ALIGN_FILL)
   {
      gtk_widget_set_hexpand(cfg->widget, TRUE);
      gtk_widget_set_halign(cfg->widget, GTK_ALIGN_FILL);
   }
   else
   {
      gtk_widget_set_hexpand(cfg->widget, FALSE);
      gtk_widget_set_halign(cfg->widget, widget_align_to_gtk(cfg->halign));
   }
   /* Charger la source */
   image_load_source(cfg);
   /* --- Label de légende --- */
   cfg->label_legende = gtk_label_new(cfg->legende ? cfg->legende : "");
   char lbl_id[256];
   snprintf(lbl_id, sizeof(lbl_id), "%s_legende", cfg->id_css ? cfg->id_css : "image");
   gtk_widget_set_name(cfg->label_legende, lbl_id);
   gtk_label_set_xalign(GTK_LABEL(cfg->label_legende), 0.5f); /* centré sous l'image */
   gtk_label_set_wrap(GTK_LABEL(cfg->label_legende), TRUE);
   gtk_widget_set_hexpand(cfg->label_legende, TRUE);
   gtk_widget_set_visible(cfg->label_legende, cfg->legende != NULL);
   /* --- Assemblage --- */
   gtk_box_append(GTK_BOX(cfg->container), cfg->widget);
   gtk_box_append(GTK_BOX(cfg->container), cfg->label_legende);
   /* --- CSS --- */
   image_apply_css(cfg);
   return cfg->container;
}

void image_free(Image *cfg)
{
   if (!cfg)
      return;

   g_free(cfg->file_path);
   g_free(cfg->icon_name);
   g_free(cfg->legende);
   g_free(cfg->legende_couleur);
   g_free(cfg->id_css);
   widget_style_free(&cfg->style);

   memset(cfg, 0, sizeof(Image));
}


void image_set_from_file(Image *cfg, const char *file_path)
{
   if (!cfg)
      return;
   g_free(cfg->file_path);
   if (file_path)
   {
      cfg->file_path = malloc(strlen(file_path) + 1);
      strcpy(cfg->file_path, file_path);
   }
   else
   {
      cfg->file_path = NULL;
   }
   cfg->source_type = IMAGE_SOURCE_FILE;
   if (cfg->widget)
      image_load_source(cfg);
}

void image_set_from_icon_name(Image *cfg, const char *icon_name)
{
   if (!cfg)
      return;
   g_free(cfg->icon_name);
   if (icon_name)
   {
      cfg->icon_name = malloc(strlen(icon_name) + 1);
      strcpy(cfg->icon_name, icon_name);
   }
   else
   {
      cfg->icon_name = NULL;
   }
   cfg->source_type = IMAGE_SOURCE_ICON_NAME;
   if (cfg->widget)
      image_load_source(cfg);
}

void image_set_from_pixbuf(Image *cfg, GdkPixbuf *pixbuf)
{
   if (!cfg)
      return;
   cfg->pixbuf = pixbuf;
   cfg->source_type = IMAGE_SOURCE_PIXBUF;
   if (cfg->widget)
      image_load_source(cfg);
}

void image_set_size(Image *cfg, int width, int height)
{
   if (!cfg)
      return;
   cfg->width = width;
   cfg->height = height;
   if (cfg->widget)
      gtk_widget_set_size_request(cfg->widget,
                                  width > 0 ? width : -1,
                                  height > 0 ? height : -1);
}

void image_set_fit_mode(Image *cfg, ImageFitMode fit_mode)
{
   if (!cfg)
      return;
   cfg->fit_mode = fit_mode;
   if (!cfg->widget)
      return;

   switch (fit_mode)
   {
   case IMAGE_FIT_FILL:
      gtk_picture_set_content_fit(GTK_PICTURE(cfg->widget), GTK_CONTENT_FIT_FILL);
      break;
   case IMAGE_FIT_COVER:
      gtk_picture_set_content_fit(GTK_PICTURE(cfg->widget), GTK_CONTENT_FIT_COVER);
      break;
   case IMAGE_FIT_NONE:
      gtk_picture_set_content_fit(GTK_PICTURE(cfg->widget), GTK_CONTENT_FIT_SCALE_DOWN);
      break;
   default:
      gtk_picture_set_content_fit(GTK_PICTURE(cfg->widget), GTK_CONTENT_FIT_CONTAIN);
      break;
   }
}

void image_set_legende(Image *cfg, const char *legende)
{
   if (!cfg)
      return;
   g_free(cfg->legende);
   if (legende)
   {
      cfg->legende = malloc(strlen(legende) + 1);
      strcpy(cfg->legende, legende);
   }
   else
   {
      cfg->legende = NULL;
   }
   if (cfg->label_legende)
   {
      gtk_label_set_text(GTK_LABEL(cfg->label_legende), cfg->legende ? cfg->legende : "");
      gtk_widget_set_visible(cfg->label_legende, cfg->legende != NULL);
   }
}

void image_set_sensitive(Image *cfg, gboolean sensitive)
{
   if (!cfg)
      return;
   cfg->sensitive = sensitive;
   if (cfg->widget)
      gtk_widget_set_sensitive(cfg->widget, sensitive);
}

void image_set_halign(Image *cfg, WidgetAlignment halign)
{
   if (!cfg)
      return;
   cfg->halign = halign;
   if (cfg->container)
      gtk_widget_set_halign(cfg->container, widget_align_to_gtk(halign));
}
