#include "screen_bassin.h"
#include "../simulation/bassin_private.h"
#include <stdlib.h>
#include <string.h>

static void on_root_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (ui->timer_id > 0)
   {
      g_source_remove(ui->timer_id);
      ui->timer_id = 0;
   }
   if (ui->bg_stream)
   {
      gtk_media_stream_pause(ui->bg_stream);
      g_object_unref(ui->bg_stream);
      ui->bg_stream = NULL;
   }
}

extern void on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data);
extern void on_key_released(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data);

// Create Main Aquarium UI Interface
GtkWidget *screen_bassin_create(void)
{
   // Load premium CSS styles
   GtkCssProvider *css_provider = gtk_css_provider_new();
   gtk_css_provider_load_from_path(css_provider, "resources/bassin.css");
   gtk_style_context_add_provider_for_display(
       gdk_display_get_default(),
       GTK_STYLE_PROVIDER(css_provider),
       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   g_object_unref(css_provider);

   BassinUI *ui = calloc(1, sizeof(BassinUI));
   ui->simulation_running = TRUE;
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
   ui->config_hide_health_bar = FALSE;
   ui->config_hide_fish_name = FALSE;
   ui->config_hide_status_bar = FALSE;
   ui->zen_mode = FALSE;
   ui->sidebar_was_visible = TRUE;

   // Default customizable shortcuts
   ui->shortcut_play = g_strdup("Ctrl+P");
   ui->shortcut_zen = g_strdup("Ctrl+H");
   ui->shortcut_debug = g_strdup("Ctrl+D");
   ui->shortcut_settings = g_strdup("Ctrl+S");
   ui->shortcut_add = g_strdup("Ctrl+N");
   ui->shortcut_sidebar = g_strdup("Ctrl+B");
   ui->shortcut_restart = g_strdup("Ctrl+R");
   ui->shortcut_food = g_strdup("Ctrl+I");

   // Load saved settings from settings.xml
   load_settings_from_xml(ui);

   // Root layout box (vertical)
   ui->root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

   // ------------------ HEADER PANEL ------------------
   ui->header_widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
   GtkWidget *header = ui->header_widget;
   gtk_widget_set_margin_start(header, 15);
   gtk_widget_set_margin_end(header, 15);
   gtk_widget_set_margin_top(header, 8);
   gtk_widget_set_margin_bottom(header, 8);

   // Use the modular menu/action setup (Menu and Bouton custom widgets)
   bassin_menu_init(ui, header);
   gtk_box_append(GTK_BOX(ui->root), header);

   // Separator
   ui->sep_top_widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
   GtkWidget *sep_top = ui->sep_top_widget;
   gtk_box_append(GTK_BOX(ui->root), sep_top);

   // ------------------ CONTENT BOX (Aquarium + Sidebar) ------------------
   GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
   gtk_widget_set_vexpand(content_box, TRUE);

   // 1. Center Canvas aquarium (GtkFixed overlayed on background GtkPicture)
   GtkWidget *overlay = gtk_overlay_new();
   gtk_widget_set_hexpand(overlay, TRUE);
   gtk_widget_set_vexpand(overlay, TRUE);

   // Canvas Background Image (using GtkPicture with no aspect ratio constraint for dynamic stretching)
   ui->bg_widget = gtk_picture_new();
   apply_background(ui);
   gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(ui->bg_widget), FALSE);
   gtk_widget_set_size_request(ui->bg_widget, ui->config_canvas_width, ui->config_canvas_height);
   gtk_overlay_set_child(GTK_OVERLAY(overlay), ui->bg_widget);

   ui->canvas = gtk_fixed_new();
   gtk_widget_set_overflow(ui->canvas, GTK_OVERFLOW_HIDDEN);
   gtk_widget_set_halign(ui->canvas, GTK_ALIGN_FILL);
   gtk_widget_set_valign(ui->canvas, GTK_ALIGN_FILL);
   gtk_widget_set_size_request(ui->canvas, ui->config_canvas_width, ui->config_canvas_height);
   gtk_overlay_add_overlay(GTK_OVERLAY(overlay), ui->canvas);

   ui->debug_overlay = gtk_drawing_area_new();
   gtk_widget_set_can_target(ui->debug_overlay, FALSE);
   gtk_widget_set_halign(ui->debug_overlay, GTK_ALIGN_FILL);
   gtk_widget_set_valign(ui->debug_overlay, GTK_ALIGN_FILL);
   gtk_widget_set_size_request(ui->debug_overlay, ui->config_canvas_width, ui->config_canvas_height);
   gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(ui->debug_overlay), on_debug_draw, ui, NULL);
   gtk_overlay_add_overlay(GTK_OVERLAY(overlay), ui->debug_overlay);

   GtkWidget *sw = gtk_scrolled_window_new();
   gtk_widget_set_hexpand(sw, TRUE);
   gtk_widget_set_vexpand(sw, TRUE);
   gtk_scrolled_window_set_propagate_natural_width(GTK_SCROLLED_WINDOW(sw), FALSE);
   gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(sw), FALSE);
   gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(sw), FALSE);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
   gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), overlay);

   gtk_box_append(GTK_BOX(content_box), sw);

   // 2. Right Sidebar Panel
   GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
   ui->sidebar = sidebar;
   gtk_widget_add_css_class(sidebar, "sidebar-container");
   gtk_widget_set_hexpand(sidebar, FALSE);
   gtk_widget_set_size_request(sidebar, 250, ui->config_canvas_height);

   // Tab Headers
   GtkWidget *box_tabs = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

   ui->btn_tab_entites = gtk_button_new_with_label("Entités");
   gtk_widget_set_hexpand(ui->btn_tab_entites, TRUE);
   gtk_widget_add_css_class(ui->btn_tab_entites, "tab-active");
   g_signal_connect(ui->btn_tab_entites, "clicked", G_CALLBACK(on_tab_entites_clicked), ui);
   gtk_box_append(GTK_BOX(box_tabs), ui->btn_tab_entites);

   ui->btn_tab_bancs = gtk_button_new_with_label("Bancs");
   gtk_widget_set_hexpand(ui->btn_tab_bancs, TRUE);
   gtk_widget_add_css_class(ui->btn_tab_bancs, "tab-inactive");
   g_signal_connect(ui->btn_tab_bancs, "clicked", G_CALLBACK(on_tab_bancs_clicked), ui);
   gtk_box_append(GTK_BOX(box_tabs), ui->btn_tab_bancs);

   gtk_box_append(GTK_BOX(sidebar), box_tabs);

   // Scrolled Window for Entity lists
   ui->scrolled_sidebar_list = gtk_scrolled_window_new();
   gtk_widget_set_vexpand(ui->scrolled_sidebar_list, TRUE);
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ui->scrolled_sidebar_list),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

   ui->box_sidebar_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
   gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(ui->scrolled_sidebar_list), ui->box_sidebar_content);
   gtk_box_append(GTK_BOX(sidebar), ui->scrolled_sidebar_list);

   gtk_box_append(GTK_BOX(content_box), sidebar);
   gtk_box_append(GTK_BOX(ui->root), content_box);

   // Separator
   ui->sep_bottom_widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
   GtkWidget *sep_bottom = ui->sep_bottom_widget;
   gtk_box_append(GTK_BOX(ui->root), sep_bottom);

   // ------------------ BOTTOM STATUS BAR ------------------
   ui->status_bar_widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
   GtkWidget *status_bar = ui->status_bar_widget;
   gtk_widget_set_margin_start(status_bar, 15);
   gtk_widget_set_margin_end(status_bar, 15);
   gtk_widget_set_margin_top(status_bar, 6);
   gtk_widget_set_margin_bottom(status_bar, 6);

   ui->lbl_status_indicator = gtk_label_new("● En cours");
   gtk_widget_add_css_class(ui->lbl_status_indicator, "badge-banc");
   gtk_box_append(GTK_BOX(status_bar), ui->lbl_status_indicator);

   ui->lbl_stats_text = gtk_label_new("0 entités actives   |   0 bancs   |   0 individuel   |   0 prédateur");
   gtk_box_append(GTK_BOX(status_bar), ui->lbl_stats_text);

   ui->lbl_elapsed_time = gtk_label_new("t = 00:00:00");
   gtk_widget_set_halign(ui->lbl_elapsed_time, GTK_ALIGN_END);
   gtk_widget_set_hexpand(ui->lbl_elapsed_time, TRUE);
   gtk_box_append(GTK_BOX(status_bar), ui->lbl_elapsed_time);

   gtk_box_append(GTK_BOX(ui->root), status_bar);

   // Setup key controller
   GtkEventController *key_controller = gtk_event_controller_key_new();
   g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), ui);
   g_signal_connect(key_controller, "key-released", G_CALLBACK(on_key_released), ui);
   gtk_widget_add_controller(ui->root, key_controller);

   // Connect destroy handler to stop timer on exit
   g_signal_connect(ui->root, "destroy", G_CALLBACK(on_root_destroy), ui);

   // Start timer loop (30 FPS -> 33ms)
   ui->timer_id = g_timeout_add(33, update_simulation, ui);

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
      gtk_widget_set_visible(ui->status_bar_widget, !ui->config_hide_status_bar);
      gtk_widget_set_visible(ui->sep_bottom_widget, !ui->config_hide_status_bar);
   }

   return ui->root;
}
