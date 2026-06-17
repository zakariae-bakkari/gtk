#include "screen_home.h"
#include "../../widgets/headers/image.h"
#include "../../widgets/headers/video.h"
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/bouton.h"

static void on_custom_bouton_destroy(GtkWidget *widget, gpointer data)
{
   (void)widget;
   Bouton *b = data;
   if (b) {
      if (b->texte) free(b->texte);
      if (b->id_css) free(b->id_css);
      if (b->nom_icone) free(b->nom_icone);
      if (b->tooltip) free(b->tooltip);
      if (b->style.bg_normal) free(b->style.bg_normal);
      if (b->style.bg_hover) free(b->style.bg_hover);
      if (b->style.fg_normal) free(b->style.fg_normal);
      if (b->style.fg_hover) free(b->style.fg_hover);
      if (b->style.couleur_bordure) free(b->style.couleur_bordure);
      free(b);
   }
}

static GtkWidget *placeholder_with_label(const char *text)
{
   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
   GtkWidget *label = gtk_label_new(text);
   gtk_box_append(GTK_BOX(box), label);
   return box;
}

static void on_start_clicked(GtkWidget *btn, gpointer user_data)
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
   Bouton *b_start = g_new0(Bouton, 1);
   bouton_initialiser(b_start);
   g_free(b_start->texte);
   b_start->texte = strdup("🚀 DEMARRER LA SIMULATION");
   g_free(b_start->id_css);
   b_start->id_css = strdup("home_btn_start");
   bouton_appliquer_preset(b_start, BOUTON_STYLE_SUGGESTED);
   b_start->taille.mode = TAILLE_FIXE;
   b_start->taille.largeur = 300;
   b_start->taille.hauteur = 60;
   b_start->on_clic = (BoutonAction)on_start_clicked;
   GtkWidget *btn_start = bouton_creer(b_start);
   g_signal_connect(btn_start, "destroy", G_CALLBACK(on_custom_bouton_destroy), b_start);
   gtk_box_append(GTK_BOX(btn_box), btn_start);

   // Quit Button
   Bouton *b_quit = g_new0(Bouton, 1);
   bouton_initialiser(b_quit);
   g_free(b_quit->texte);
   b_quit->texte = strdup("❌ QUITTER");
   g_free(b_quit->id_css);
   b_quit->id_css = strdup("home_btn_quit");
   bouton_appliquer_preset(b_quit, BOUTON_STYLE_DESTRUCTIVE);
   b_quit->taille.mode = TAILLE_FIXE;
   b_quit->taille.largeur = 300;
   b_quit->taille.hauteur = 50;
   b_quit->on_clic = (BoutonAction)action_quitter;
   GtkWidget *btn_quit = bouton_creer(b_quit);
   g_signal_connect(btn_quit, "destroy", G_CALLBACK(on_custom_bouton_destroy), b_quit);
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
