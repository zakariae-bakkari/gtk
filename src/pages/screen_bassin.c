#include "screen_bassin.h"
#include "../simulation/bassin_private.h"
#include "../../widgets/headers/bouton.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/texte.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

static void on_root_destroy(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (ui->timer_id > 0)
   {
      widget_timer_remove(ui->timer_id);
      ui->timer_id = 0;
   }
   if (ui->bg_stream)
   {
      widget_media_stream_pause(ui->bg_stream);
      widget_media_stream_free(ui->bg_stream);
      ui->bg_stream = NULL;
   }
}

extern void on_key_pressed(void *controller, unsigned int keyval, unsigned int keycode, unsigned int state, void *user_data);
extern void on_key_released(void *controller, unsigned int keyval, unsigned int keycode, unsigned int state, void *user_data);

// Create Main Aquarium UI Interface
Widget screen_bassin_create(void)
{
   // Load premium CSS styles
   widget_charger_css("resources/bassin.css");

   BassinUI *ui = calloc(1, sizeof(BassinUI));
   ui->simulation_running = true;
   ui->active_tab = 0; // Entités
   ui->elapsed_time = 0;
   ui->simulation_speed = 1.0;

   // Dynamic species loading
   load_species_configs(ui);

   // Default configuration values
   ui->config_fish_size = 0;
   ui->config_bg_path = strdup("resources/images/fond/background_banc.png");
   ui->config_canvas_width = 900;
   ui->config_canvas_height = 600;
   ui->config_hide_health_bar = false;
   ui->config_hide_fish_name = false;
   ui->config_hide_status_bar = false;
   ui->zen_mode = false;
   ui->sidebar_was_visible = true;

   // Default customizable shortcuts
   ui->shortcut_play = strdup("Ctrl+P");
   ui->shortcut_zen = strdup("Ctrl+H");
   ui->shortcut_debug = strdup("Ctrl+D");
   ui->shortcut_settings = strdup("Ctrl+S");
   ui->shortcut_add = strdup("Ctrl+N");
   ui->shortcut_sidebar = strdup("Ctrl+B");
   ui->shortcut_restart = strdup("Ctrl+R");
   ui->shortcut_food = strdup("Ctrl+I");

   // Load saved settings from settings.xml
   load_settings_from_xml(ui);

   // Root layout box (vertical)
   Conteneur c_root;
   conteneur_initialiser(&c_root);
   c_root.orientation = CONTENEUR_VERTICAL;
   c_root.espacement = 0;
   ui->root = conteneur_creer(&c_root);

   // ------------------ HEADER PANEL ------------------
   Conteneur c_header;
   conteneur_initialiser(&c_header);
   c_header.orientation = CONTENEUR_HORIZONTAL;    
   c_header.espacement = 12;
   c_header.marges.gauche = 15;
   c_header.marges.droite = 15;
   c_header.marges.haut = 8;
   c_header.marges.bas = 8;
   ui->header_widget = conteneur_creer(&c_header);
   Widget header = ui->header_widget;

   // Use the modular menu/action setup (Menu and Bouton custom widgets)
   bassin_menu_init(ui, header);
   conteneur_ajouter(&c_root, header);

   // Separator
   ui->sep_top_widget = widget_creer_separateur(1); // 1 = HORIZONTAL
   Widget sep_top = ui->sep_top_widget;
   conteneur_ajouter(&c_root, sep_top);

   // ------------------ CONTENT BOX (Aquarium + Sidebar) ------------------
   Conteneur c_content_box;
   conteneur_initialiser(&c_content_box);
   c_content_box.orientation = CONTENEUR_HORIZONTAL;
   c_content_box.espacement = 0;
   c_content_box.vexpand = true;
   Widget content_box = conteneur_creer(&c_content_box);

   // 1. Center Canvas aquarium (GtkFixed overlayed on background GtkPicture)
   Widget overlay = widget_creer_overlay();
   widget_set_hexpand(overlay, TRUE);
   widget_set_vexpand(overlay, TRUE);

   // Canvas Background Image (using GtkPicture with no aspect ratio constraint for dynamic stretching)
   ui->bg_widget = widget_creer_picture();
   apply_background(ui);
   widget_picture_set_keep_aspect_ratio(ui->bg_widget, false);
   widget_set_size(ui->bg_widget, ui->config_canvas_width, ui->config_canvas_height);
   widget_overlay_set_child(overlay, ui->bg_widget);

   ui->canvas = widget_creer_fixed();
   widget_fixed_set_overflow_hidden(ui->canvas);
   widget_set_halign_fill(ui->canvas);
   widget_set_valign_fill(ui->canvas);
   widget_set_size(ui->canvas, ui->config_canvas_width, ui->config_canvas_height);
   widget_overlay_add_overlay(overlay, ui->canvas);

   ui->debug_overlay = widget_creer_drawing_area();
   widget_set_can_target(ui->debug_overlay, false);
   widget_set_halign_fill(ui->debug_overlay);
   widget_set_valign_fill(ui->debug_overlay);
   widget_set_size(ui->debug_overlay, ui->config_canvas_width, ui->config_canvas_height);
   widget_drawing_area_set_draw_func(ui->debug_overlay, on_debug_draw, ui);
   widget_overlay_add_overlay(overlay, ui->debug_overlay);

   Conteneur c_sw;
   conteneur_initialiser(&c_sw);
   c_sw.scroll_mode = SCROLL_BOTH;
   c_sw.hexpand = TRUE;
   c_sw.vexpand = TRUE;
   c_sw.scroll_overlay = TRUE;
   Widget sw = conteneur_creer(&c_sw);
   conteneur_ajouter(&c_sw, overlay);

   conteneur_ajouter(&c_content_box, sw);

   // 2. Right Sidebar Panel
   Conteneur c_sidebar;
   conteneur_initialiser(&c_sidebar);
   c_sidebar.orientation = CONTENEUR_VERTICAL;
   c_sidebar.espacement = 10;
   c_sidebar.classe_css = strdup("sidebar-container");
   c_sidebar.hexpand = FALSE;
   c_sidebar.taille.largeur = 250;
   c_sidebar.taille.hauteur = ui->config_canvas_height;
   ui->sidebar = conteneur_creer(&c_sidebar);
   Widget sidebar = ui->sidebar;

   // Tab Headers
   Conteneur c_box_tabs;
   conteneur_initialiser(&c_box_tabs);
   c_box_tabs.orientation = CONTENEUR_HORIZONTAL;
   c_box_tabs.espacement = 4;
   Widget box_tabs = conteneur_creer(&c_box_tabs);

   Bouton *b_ent = calloc(1, sizeof(Bouton));
   bouton_initialiser(b_ent);
   free(b_ent->texte);
   b_ent->texte = strdup("Entités");
   free(b_ent->id_css);
   b_ent->id_css = strdup("btn_tab_entites");
   bouton_appliquer_preset(b_ent, BOUTON_STYLE_TAB_ACTIVE);
   b_ent->on_clic = (BoutonAction)on_tab_entites_clicked;
   b_ent->user_data = ui;
   b_ent->hexpand = true;
   ui->btn_tab_entites = bouton_creer(b_ent);
   widget_connect_destroy_signal(ui->btn_tab_entites, on_custom_bouton_destroy, b_ent);
   conteneur_ajouter(&c_box_tabs, ui->btn_tab_entites);

   Bouton *b_banc = calloc(1, sizeof(Bouton));
   bouton_initialiser(b_banc);
   free(b_banc->texte);
   b_banc->texte = strdup("Bancs");
   free(b_banc->id_css);
   b_banc->id_css = strdup("btn_tab_bancs");
   bouton_appliquer_preset(b_banc, BOUTON_STYLE_TAB_INACTIVE);
   b_banc->on_clic = (BoutonAction)on_tab_bancs_clicked;
   b_banc->user_data = ui;
   b_banc->hexpand = true;
   ui->btn_tab_bancs = bouton_creer(b_banc);
   widget_connect_destroy_signal(ui->btn_tab_bancs, on_custom_bouton_destroy, b_banc);
   conteneur_ajouter(&c_box_tabs, ui->btn_tab_bancs);

   conteneur_ajouter(&c_sidebar, box_tabs);

   // Scrolled Window for Entity lists
   Conteneur c_sidebar_list;
   conteneur_initialiser(&c_sidebar_list);
   c_sidebar_list.orientation = CONTENEUR_VERTICAL;
   c_sidebar_list.espacement = 0;
   c_sidebar_list.scroll_mode = SCROLL_VERTICAL;
   c_sidebar_list.vexpand = TRUE;
   ui->scrolled_sidebar_list = conteneur_creer(&c_sidebar_list);
   ui->box_sidebar_content = c_sidebar_list.widget;
   conteneur_ajouter(&c_sidebar, ui->scrolled_sidebar_list);

   conteneur_ajouter(&c_content_box, sidebar);
   conteneur_ajouter(&c_root, content_box);

   // Separator
   ui->sep_bottom_widget = widget_creer_separateur(1); // 1 = HORIZONTAL
   Widget sep_bottom = ui->sep_bottom_widget;
   conteneur_ajouter(&c_root, sep_bottom);

   // ------------------ BOTTOM STATUS BAR ------------------
   Conteneur c_status_bar;
   conteneur_initialiser(&c_status_bar);
   c_status_bar.orientation = CONTENEUR_HORIZONTAL;
   c_status_bar.espacement = 12;
   c_status_bar.marges.gauche = 15;
   c_status_bar.marges.droite = 15;
   c_status_bar.marges.haut = 6;
   c_status_bar.marges.bas = 6;
   ui->status_bar_widget = conteneur_creer(&c_status_bar);
   Widget status_bar = ui->status_bar_widget;

   Texte t_indicator;
   texte_initialiser(&t_indicator);
   t_indicator.texte = strdup("● En cours");
   t_indicator.classe_css = strdup("badge-banc");
   ui->lbl_status_indicator = texte_creer(&t_indicator);
   conteneur_ajouter(&c_status_bar, ui->lbl_status_indicator);

   Texte t_stats;
   texte_initialiser(&t_stats);
   t_stats.texte = strdup("0 entités actives   |   0 bancs   |   0 individuel   |   0 prédateur");
   ui->lbl_stats_text = texte_creer(&t_stats);
   conteneur_ajouter(&c_status_bar, ui->lbl_stats_text);

   Texte t_elapsed;
   texte_initialiser(&t_elapsed);
   t_elapsed.texte = strdup("t = 00:00:00");
   t_elapsed.alignement = TEXTE_ALIGN_RIGHT;
   t_elapsed.hexpand = TRUE;
   ui->lbl_elapsed_time = texte_creer(&t_elapsed);
   conteneur_ajouter(&c_status_bar, ui->lbl_elapsed_time);

   conteneur_ajouter(&c_root, status_bar);

   // Setup key controller
   widget_add_key_controller(ui->root, on_key_pressed, on_key_released, ui);

   // Connect destroy handler to stop timer on exit
   widget_connect_destroy_signal(ui->root, on_root_destroy, ui);

   // Start timer loop (30 FPS -> 33ms)
   ui->timer_id = widget_timer_add(33, update_simulation, ui);

   // Start with empty aquarium (0 fish) until loaded or added
   ui->next_id = 1;
   ui->num_bancs = 0;
   ui->elapsed_time = 0.0;
   update_sidebar_list(ui);
   update_status_bar(ui);

   // Apply loaded settings visibility configuration
   apply_fish_visibility_configs(ui);
   if (!ui->zen_mode)
   {
      widget_set_visible(ui->status_bar_widget, !ui->config_hide_status_bar);
      widget_set_visible(ui->sep_bottom_widget, !ui->config_hide_status_bar);
   }

   return ui->root;
}
