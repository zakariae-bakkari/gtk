#include "screen_home.h"
#include "../../widgets/headers/image.h"
#include "../../widgets/headers/video.h"
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/bouton.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/texte.h"

static void on_custom_bouton_destroy(Widget widget, void *data)
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

static void on_start_clicked(Widget btn, void *user_data)
{
   (void)btn;
   (void)user_data;
   nav_to_bassin();
}

static void on_video_widget_destroy(Widget widget, void *user_data)
{
   (void)widget;
   Video *vid = (Video *)user_data;
   if (vid)
   {
      video_free(vid);
      free(vid);
   }
}

Widget screen_accueil_create(void)
{
   Widget overlay = widget_creer_overlay();
   widget_set_hexpand(overlay, true);
   widget_set_vexpand(overlay, true);

   // Background Video (using GtkMediaFile + GtkPicture for NO controls)
   void* stream = widget_media_stream_new_from_file("resources/images/fond/background.mp4");
   widget_media_stream_set_loop(stream, true);
   
   Widget w_vid = widget_picture_new_for_paintable(stream);
   widget_picture_set_keep_aspect_ratio(w_vid, false);
   widget_set_hexpand(w_vid, true);
   widget_set_vexpand(w_vid, true);
   widget_set_halign_fill(w_vid);
   widget_set_valign_fill(w_vid);
   
   // Start playback manually
   widget_media_stream_play(stream);
   
   widget_overlay_set_child(overlay, w_vid);

   // Content box
   Conteneur c_content;
   conteneur_initialiser(&c_content);
   c_content.orientation = CONTENEUR_VERTICAL;
   c_content.espacement = 40;
   c_content.align_x = ALIGNEMENT_CENTRE;
   c_content.align_y = ALIGNEMENT_CENTRE;
   Widget content = conteneur_creer(&c_content);
   widget_overlay_add_overlay(overlay, content);

   // Title Image
   Widget title_img = widget_creer_picture_depuis_fichier("resources/images/fond/title_home_page-removebg-preview.png");
   widget_picture_set_keep_aspect_ratio(title_img, true);
   widget_set_size(title_img, 800, 400);
   conteneur_ajouter(&c_content, title_img);

   // Buttons Box
   Conteneur c_btn_box;
   conteneur_initialiser(&c_btn_box);
   c_btn_box.orientation = CONTENEUR_VERTICAL;
   c_btn_box.espacement = 15;
   c_btn_box.align_x = ALIGNEMENT_CENTRE;
   Widget btn_box = conteneur_creer(&c_btn_box);
   conteneur_ajouter(&c_content, btn_box);

   // Start Button
   Bouton *b_start = calloc(1, sizeof(Bouton));
   bouton_initialiser(b_start);
   free(b_start->texte);
   b_start->texte = strdup("🚀 DEMARRER LA SIMULATION");
   free(b_start->id_css);
   b_start->id_css = strdup("home_btn_start");
   bouton_appliquer_preset(b_start, BOUTON_STYLE_SUGGESTED);
   b_start->taille.mode = TAILLE_FIXE;
   b_start->taille.largeur = 300;
   b_start->taille.hauteur = 60;
   b_start->on_clic = (BoutonAction)on_start_clicked;
   Widget btn_start = bouton_creer(b_start);
   widget_connect_destroy_signal(btn_start, on_custom_bouton_destroy, b_start);
   conteneur_ajouter(&c_btn_box, btn_start);

   // Quit Button
   Bouton *b_quit = calloc(1, sizeof(Bouton));
   bouton_initialiser(b_quit);
   free(b_quit->texte);
   b_quit->texte = strdup("❌ QUITTER");
   free(b_quit->id_css);
   b_quit->id_css = strdup("home_btn_quit");
   bouton_appliquer_preset(b_quit, BOUTON_STYLE_DESTRUCTIVE);
   b_quit->taille.mode = TAILLE_FIXE;
   b_quit->taille.largeur = 300;
   b_quit->taille.hauteur = 50;
   b_quit->on_clic = (BoutonAction)action_quitter;
   Widget btn_quit = bouton_creer(b_quit);
   widget_connect_destroy_signal(btn_quit, on_custom_bouton_destroy, b_quit);
   conteneur_ajouter(&c_btn_box, btn_quit);

   return overlay;
}
