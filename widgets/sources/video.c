#include "../headers/video.h"
#include <stdlib.h>
#include <string.h>

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

static void video_update_error_state(Video *cfg)
{
   GtkMediaStream *stream;
   const GError *error;
   char tooltip[1024];

   if (!cfg || !cfg->widget)
      return;

   stream = gtk_video_get_media_stream(GTK_VIDEO(cfg->widget));
   error = stream ? gtk_media_stream_get_error(stream) : NULL;

   if (error)
   {
      snprintf(tooltip, sizeof(tooltip), "Lecture video impossible : %s", error->message);
      gtk_widget_set_tooltip_text(cfg->widget, tooltip);

      if (cfg->label_erreur)
      {
         gtk_label_set_text(GTK_LABEL(cfg->label_erreur), tooltip);
         gtk_widget_set_visible(cfg->label_erreur, TRUE);
      }

      fprintf(stderr, "[VIDEO] %s\n", tooltip);
      return;
   }

   gtk_widget_set_tooltip_text(cfg->widget, NULL);
   if (cfg->label_erreur)
      gtk_widget_set_visible(cfg->label_erreur, FALSE);
}

static void on_video_error_notify(GObject *object, GParamSpec *pspec, gpointer data)
{
   (void)object;
   (void)pspec;
   video_update_error_state((Video *)data);
}

static void video_bind_stream(Video *cfg)
{
   GtkMediaStream *stream;

   if (!cfg || !cfg->widget)
      return;

   stream = gtk_video_get_media_stream(GTK_VIDEO(cfg->widget));
   if (!stream)
      return;

   g_signal_connect(stream, "notify::error", G_CALLBACK(on_video_error_notify), cfg);
   video_update_error_state(cfg);
}

static void video_apply_css(Video *cfg)
{
   if (!cfg || !cfg->widget || !cfg->id_css)
      return;

   GtkCssProvider *provider = gtk_css_provider_new();
   char css[1024];
   char border_css[128] = "";
   const char *leg_color = cfg->legende_couleur ? cfg->legende_couleur : "#7f8c8d";
   int leg_size = cfg->legende_taille_px > 0 ? cfg->legende_taille_px : 11;

   if (cfg->style.couleur_bordure && cfg->style.epaisseur_bordure > 0)
   {
      snprintf(border_css, sizeof(border_css),
               "  border: %dpx solid %s;\n",
               cfg->style.epaisseur_bordure,
               cfg->style.couleur_bordure);
   }

   snprintf(css, sizeof(css),
            "#%s {\n"
            "  border-radius: %dpx;\n"
            "%s"
            "}\n"
            "#%s_legende {\n"
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

   snprintf(css + strlen(css), sizeof(css) - strlen(css),
            "#%s_erreur {\n"
            "  color: #c0392b;\n"
            "  font-size: 11px;\n"
            "  margin-top: 4px;\n"
            "}\n",
            cfg->id_css);

   gtk_css_provider_load_from_string(provider, css);

   gtk_style_context_add_provider(
       gtk_widget_get_style_context(cfg->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_USER);

   if (cfg->controls)
      gtk_style_context_add_provider(
          gtk_widget_get_style_context(cfg->controls),
          GTK_STYLE_PROVIDER(provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);

   if (cfg->label_legende)
      gtk_style_context_add_provider(
          gtk_widget_get_style_context(cfg->label_legende),
          GTK_STYLE_PROVIDER(provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);

   if (cfg->label_erreur)
      gtk_style_context_add_provider(
          gtk_widget_get_style_context(cfg->label_erreur),
          GTK_STYLE_PROVIDER(provider),
          GTK_STYLE_PROVIDER_PRIORITY_USER);

   g_object_unref(provider);
}

static void video_load_source(Video *cfg)
{
   if (!cfg || !cfg->widget)
      return;

   if (cfg->file_path)
      gtk_video_set_filename(GTK_VIDEO(cfg->widget), cfg->file_path);
   else if (cfg->resource_path)
      gtk_video_set_resource(GTK_VIDEO(cfg->widget), cfg->resource_path);

   video_bind_stream(cfg);
}

void video_initialiser(Video *cfg)
{
   if (!cfg)
      return;

   memset(cfg, 0, sizeof(Video));

   cfg->id_css = malloc(strlen("video") + 1);
   strcpy(cfg->id_css, "video");
   cfg->autoplay = TRUE;
   cfg->loop = TRUE;
   cfg->controles = FALSE;
   cfg->sensitive = TRUE;
   cfg->halign = WIDGET_ALIGN_START;

   widget_style_init(&cfg->style);
}

GtkWidget *video_creer(Video *cfg)
{
   char lbl_id[256];

   if (!cfg)
      return NULL;

   cfg->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   gtk_widget_set_halign(cfg->container, widget_align_to_gtk(cfg->halign));
   gtk_widget_set_hexpand(cfg->container, cfg->halign == WIDGET_ALIGN_FILL);

   cfg->widget = gtk_video_new();
   gtk_widget_set_name(cfg->widget, cfg->id_css ? cfg->id_css : "video");
   gtk_video_set_autoplay(GTK_VIDEO(cfg->widget), cfg->autoplay);
   gtk_video_set_loop(GTK_VIDEO(cfg->widget), cfg->loop);
   gtk_widget_set_sensitive(cfg->widget, cfg->sensitive);

   if (cfg->width > 0 || cfg->height > 0)
      gtk_widget_set_size_request(cfg->widget,
                                  cfg->width > 0 ? cfg->width : -1,
                                  cfg->height > 0 ? cfg->height : -1);

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

   video_load_source(cfg);
   gtk_box_append(GTK_BOX(cfg->container), cfg->widget);

   cfg->label_legende = gtk_label_new(cfg->legende ? cfg->legende : "");
   snprintf(lbl_id, sizeof(lbl_id), "%s_legende", cfg->id_css ? cfg->id_css : "video");
   gtk_widget_set_name(cfg->label_legende, lbl_id);
   gtk_label_set_xalign(GTK_LABEL(cfg->label_legende), 0.5f);
   gtk_label_set_wrap(GTK_LABEL(cfg->label_legende), TRUE);
   gtk_widget_set_hexpand(cfg->label_legende, TRUE);
   gtk_widget_set_visible(cfg->label_legende, cfg->legende != NULL);
   gtk_box_append(GTK_BOX(cfg->container), cfg->label_legende);

   cfg->label_erreur = gtk_label_new("");
   snprintf(lbl_id, sizeof(lbl_id), "%s_erreur", cfg->id_css ? cfg->id_css : "video");
   gtk_widget_set_name(cfg->label_erreur, lbl_id);
   gtk_label_set_xalign(GTK_LABEL(cfg->label_erreur), 0.0f);
   gtk_label_set_wrap(GTK_LABEL(cfg->label_erreur), TRUE);
   gtk_widget_set_hexpand(cfg->label_erreur, TRUE);
   gtk_widget_set_visible(cfg->label_erreur, FALSE);
   gtk_box_append(GTK_BOX(cfg->container), cfg->label_erreur);

   video_apply_css(cfg);
   video_update_error_state(cfg);
   return cfg->container;
}

void video_free(Video *cfg)
{
   if (!cfg)
      return;

   g_free(cfg->id_css);
   g_free(cfg->file_path);
   g_free(cfg->resource_path);
   g_free(cfg->legende);
   g_free(cfg->legende_couleur);
   widget_style_free(&cfg->style);

   memset(cfg, 0, sizeof(Video));
}

void video_set_from_file(Video *cfg, const char *file_path)
{
   if (!cfg)
      return;

   g_free(cfg->file_path);
   cfg->file_path = NULL;

   if (file_path)
   {
      cfg->file_path = malloc(strlen(file_path) + 1);
      strcpy(cfg->file_path, file_path);
   }

   g_free(cfg->resource_path);
   cfg->resource_path = NULL;

   if (cfg->widget)
      video_load_source(cfg);
}

void video_set_from_resource(Video *cfg, const char *resource_path)
{
   if (!cfg)
      return;

   g_free(cfg->resource_path);
   cfg->resource_path = NULL;

   if (resource_path)
   {
      cfg->resource_path = malloc(strlen(resource_path) + 1);
      strcpy(cfg->resource_path, resource_path);
   }

   g_free(cfg->file_path);
   cfg->file_path = NULL;

   if (cfg->widget)
      video_load_source(cfg);
}

void video_set_size(Video *cfg, int width, int height)
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

void video_set_legende(Video *cfg, const char *legende)
{
   if (!cfg)
      return;

   g_free(cfg->legende);
   cfg->legende = NULL;

   if (legende)
   {
      cfg->legende = malloc(strlen(legende) + 1);
      strcpy(cfg->legende, legende);
   }

   if (cfg->label_legende)
   {
      gtk_label_set_text(GTK_LABEL(cfg->label_legende), cfg->legende ? cfg->legende : "");
      gtk_widget_set_visible(cfg->label_legende, cfg->legende != NULL);
   }
}

void video_set_halign(Video *cfg, WidgetAlignment halign)
{
   if (!cfg)
      return;

   cfg->halign = halign;
   if (cfg->container)
      gtk_widget_set_halign(cfg->container, widget_align_to_gtk(halign));
}
