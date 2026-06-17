#include "../headers/common.h"
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

// ====================== FONCTIONS UTILITAIRES ======================

void widget_style_init(WidgetStyle *style)
{
   if (!style)
      return;

   memset(style, 0, sizeof(WidgetStyle));

   // Valeurs par défaut
   style->bg_normal = NULL;
   style->fg_normal = NULL;
   style->epaisseur_bordure = 1;
   style->couleur_bordure = NULL;
   style->rayon_arrondi = 8;
   style->gras = false;
   style->italique = false;
   style->taille_texte_px = 0;
   style->couleur_bordure_error = NULL;
   style->bg_error = NULL;
}

void widget_style_free(WidgetStyle *style)
{
   if (!style)
      return;

   g_free(style->bg_normal);
   g_free(style->fg_normal);
   g_free(style->couleur_bordure);
   g_free(style->couleur_bordure_error);
   g_free(style->bg_error);

   // Réinitialiser à zéro pour éviter les double-free
   memset(style, 0, sizeof(WidgetStyle));
}

void widget_style_copy(WidgetStyle *dest, const WidgetStyle *src)
{
   if (!dest || !src)
      return;

   // Libérer l'ancien contenu de dest
   widget_style_free(dest);

   // Copier les valeurs
   if (src->bg_normal)
   {
      dest->bg_normal = malloc(strlen(src->bg_normal) + 1);
      strcpy(dest->bg_normal, src->bg_normal);
   }
   else
   {
      dest->bg_normal = NULL;
   }

   if (src->fg_normal)
   {
      dest->fg_normal = malloc(strlen(src->fg_normal) + 1);
      strcpy(dest->fg_normal, src->fg_normal);
   }
   else
   {
      dest->fg_normal = NULL;
   }
   dest->epaisseur_bordure = src->epaisseur_bordure;
   if (src->couleur_bordure)
   {
      dest->couleur_bordure = malloc(strlen(src->couleur_bordure) + 1);
      strcpy(dest->couleur_bordure, src->couleur_bordure);
   }
   else
   {
      dest->couleur_bordure = NULL;
   }
   dest->rayon_arrondi = src->rayon_arrondi;
   dest->gras = src->gras;
   dest->italique = src->italique;
   dest->taille_texte_px = src->taille_texte_px;
   if (src->couleur_bordure_error)
   {
      dest->couleur_bordure_error = malloc(strlen(src->couleur_bordure_error) + 1);
      strcpy(dest->couleur_bordure_error, src->couleur_bordure_error);
   }
   else
   {
      dest->couleur_bordure_error = NULL;
   }

   if (src->bg_error)
   {
      dest->bg_error = malloc(strlen(src->bg_error) + 1);
      strcpy(dest->bg_error, src->bg_error);
   }
   else
   {
      dest->bg_error = NULL;
   }
}

void widget_charger_css(const char *chemin_css)
{
   if (!chemin_css)
      return;

   GtkCssProvider *css_provider = gtk_css_provider_new();
   gtk_css_provider_load_from_path(css_provider, chemin_css);
   gtk_style_context_add_provider_for_display(
       gdk_display_get_default(),
       GTK_STYLE_PROVIDER(css_provider),
       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   g_object_unref(css_provider);
}

Widget widget_creer_separateur(int orientation)
{
   GtkOrientation or = (orientation == 1) ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL;
   // Note: in conteneur.h, CONTENEUR_HORIZONTAL = 1, CONTENEUR_VERTICAL = 0
   return gtk_separator_new(or);
}

/* ====================== WIDGET ABSTRACTION WRAPPERS ====================== */

void widget_set_hexpand(Widget w, bool expand)
{
   if (w) gtk_widget_set_hexpand(w, expand ? TRUE : FALSE);
}

void widget_set_vexpand(Widget w, bool expand)
{
   if (w) gtk_widget_set_vexpand(w, expand ? TRUE : FALSE);
}

void widget_set_halign_fill(Widget w)
{
   if (w) gtk_widget_set_halign(w, GTK_ALIGN_FILL);
}

void widget_set_valign_fill(Widget w)
{
   if (w) gtk_widget_set_valign(w, GTK_ALIGN_FILL);
}

void widget_set_size(Widget w, int width, int height)
{
   if (w) gtk_widget_set_size_request(w, width, height);
}

void widget_set_visible(Widget w, bool visible)
{
   if (w) gtk_widget_set_visible(w, visible ? TRUE : FALSE);
}

void widget_set_sensitive(Widget w, bool sensitive)
{
   if (w) gtk_widget_set_sensitive(w, sensitive ? TRUE : FALSE);
}

void widget_set_can_target(Widget w, bool targetable)
{
   if (w) gtk_widget_set_can_target(w, targetable ? TRUE : FALSE);
}

Widget widget_creer_overlay(void)
{
   return gtk_overlay_new();
}

void widget_overlay_set_child(Widget overlay, Widget child)
{
   if (overlay) gtk_overlay_set_child(GTK_OVERLAY(overlay), child);
}

void widget_overlay_add_overlay(Widget overlay, Widget child)
{
   if (overlay) gtk_overlay_add_overlay(GTK_OVERLAY(overlay), child);
}

Widget widget_creer_fixed(void)
{
   return gtk_fixed_new();
}

void widget_fixed_set_overflow_hidden(Widget fixed)
{
   if (fixed) gtk_widget_set_overflow(fixed, GTK_OVERFLOW_HIDDEN);
}

Widget widget_creer_drawing_area(void)
{
   return gtk_drawing_area_new();
}

static void gtk_draw_func_callback(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data)
{
    WidgetDrawFunc user_cb = g_object_get_data(G_OBJECT(drawing_area), "user_draw_func");
    if (user_cb)
    {
        user_cb(drawing_area, cr, width, height, user_data);
    }
}

void widget_drawing_area_set_draw_func(Widget da, WidgetDrawFunc draw_func, void *user_data)
{
   if (da)
   {
      g_object_set_data(G_OBJECT(da), "user_draw_func", draw_func);
      gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(da), gtk_draw_func_callback, user_data, NULL);
   }
}

Widget widget_creer_picture(void)
{
   return gtk_picture_new();
}

Widget widget_creer_picture_depuis_fichier(const char *filename)
{
   return gtk_picture_new_for_filename(filename);
}

void widget_picture_set_keep_aspect_ratio(Widget pic, bool keep)
{
   if (pic) gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(pic), keep ? TRUE : FALSE);
}

void widget_picture_set_filename(Widget pic, const char *filename)
{
   if (pic) gtk_picture_set_filename(GTK_PICTURE(pic), filename);
}

void* widget_media_stream_new_from_file(const char *filename)
{
   return gtk_media_file_new_for_filename(filename);
}

void widget_media_stream_set_loop(void* stream, bool loop)
{
   if (stream) gtk_media_stream_set_loop(GTK_MEDIA_STREAM(stream), loop ? TRUE : FALSE);
}

void widget_media_stream_play(void* stream)
{
   if (stream) gtk_media_stream_play(GTK_MEDIA_STREAM(stream));
}

void widget_media_stream_pause(void* stream)
{
   if (stream) gtk_media_stream_pause(GTK_MEDIA_STREAM(stream));
}

void widget_media_stream_free(void* stream)
{
   if (stream) g_object_unref(stream);
}

Widget widget_picture_new_for_paintable(void* stream)
{
   return gtk_picture_new_for_paintable(GDK_PAINTABLE(stream));
}

void widget_picture_set_paintable(Widget pic, void* stream)
{
   if (pic) gtk_picture_set_paintable(GTK_PICTURE(pic), GDK_PAINTABLE(stream));
}

void widget_add_key_controller(Widget w, void *on_pressed, void *on_released, void *user_data)
{
   if (!w) return;
   GtkEventController *key_controller = gtk_event_controller_key_new();
   if (on_pressed)
      g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_pressed), user_data);
   if (on_released)
      g_signal_connect(key_controller, "key-released", G_CALLBACK(on_released), user_data);
   gtk_widget_add_controller(w, key_controller);
}

void widget_connect_destroy_signal(Widget w, void *on_destroy, void *user_data)
{
   if (w && on_destroy)
      g_signal_connect(w, "destroy", G_CALLBACK(on_destroy), user_data);
}

unsigned int widget_timer_add(unsigned int interval_ms, void *callback, void *user_data)
{
   return g_timeout_add(interval_ms, (GSourceFunc)callback, user_data);
}

void widget_timer_remove(unsigned int timer_id)
{
   if (timer_id > 0)
      g_source_remove(timer_id);
}
