#include "screen_stubs.h"
#include "../../widgets/headers/image.h"
#include "../../widgets/headers/video.h"
#include "../../widgets/headers/fenetre.h"

static GtkWidget *placeholder_with_label(const char *text)
{
   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
   GtkWidget *label = gtk_label_new(text);
   gtk_box_append(GTK_BOX(box), label);
   return box;
}

static void on_start_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   (void)user_data;
   nav_to_bassin();
}

static void on_video_widget_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   Video *vid = (Video *)user_data;
   if (vid)
   {
      video_free(vid);
      g_free(vid);
   }
}

GtkWidget *screen_accueil_create(void)
{
   GtkWidget *overlay = gtk_overlay_new();
   gtk_widget_set_hexpand(overlay, TRUE);
   gtk_widget_set_vexpand(overlay, TRUE);

   // Background Video (using GtkMediaFile + GtkPicture for NO controls)
   GtkMediaStream *stream = gtk_media_file_new_for_filename("resources/images/fond/background.mp4");
   gtk_media_stream_set_loop(stream, TRUE);
   
   GtkWidget *w_vid = gtk_picture_new_for_paintable(GDK_PAINTABLE(stream));
   gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(w_vid), FALSE);
   gtk_widget_set_hexpand(w_vid, TRUE);
   gtk_widget_set_vexpand(w_vid, TRUE);
   gtk_widget_set_halign(w_vid, GTK_ALIGN_FILL);
   gtk_widget_set_valign(w_vid, GTK_ALIGN_FILL);
   
   // Start playback manually
   gtk_media_stream_play(stream);
   
   gtk_overlay_set_child(GTK_OVERLAY(overlay), w_vid);

   // Content box
   GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 40);
   gtk_widget_set_halign(content, GTK_ALIGN_CENTER);
   gtk_widget_set_valign(content, GTK_ALIGN_CENTER);
   gtk_overlay_add_overlay(GTK_OVERLAY(overlay), content);

   // Title Image
   GtkWidget *title_img = gtk_picture_new_for_filename("resources/images/fond/title_home_page-removebg-preview.png");
   gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(title_img), TRUE);
   gtk_widget_set_size_request(title_img, 800, 400);
   gtk_box_append(GTK_BOX(content), title_img);

   // Buttons Box
   GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
   gtk_widget_set_halign(btn_box, GTK_ALIGN_CENTER);
   gtk_box_append(GTK_BOX(content), btn_box);

   // Start Button
   GtkWidget *btn_start = gtk_button_new_with_label("🚀 DEMARRER LA SIMULATION");
   gtk_widget_add_css_class(btn_start, "principal");
   gtk_widget_set_size_request(btn_start, 300, 60);
   g_signal_connect(btn_start, "clicked", G_CALLBACK(on_start_clicked), NULL);
   gtk_box_append(GTK_BOX(btn_box), btn_start);

   // Quit Button
   GtkWidget *btn_quit = gtk_button_new_with_label("❌ QUITTER");
   gtk_widget_add_css_class(btn_quit, "danger");
   gtk_widget_set_size_request(btn_quit, 300, 50);
   g_signal_connect(btn_quit, "clicked", G_CALLBACK(action_quitter), NULL);
   gtk_box_append(GTK_BOX(btn_box), btn_quit);

   return overlay;
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
