#include "screen_bassin.h"
#include "bassin_private.h"
#include "../sound.h"
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Custom CSS loaded from resources/bassin.css

// Helper functions for species type checking (non-static)
int get_fish_level(BassinUI *ui, Poisson *p)
{
   if (!p) return 0;
   SpeciesConfig *cfg = find_species_config(ui, p->nom);
   return cfg ? cfg->level : 1;
}

gboolean is_predator(BassinUI *ui, Poisson *p)
{
   if (!p) return FALSE;
   SpeciesConfig *cfg = find_species_config(ui, p->nom);
   return cfg && (strcmp(cfg->type, "predator") == 0);
}

gboolean is_ally(BassinUI *ui, Poisson *p)
{
   if (!p) return FALSE;
   SpeciesConfig *cfg = find_species_config(ui, p->nom);
   return cfg && (strcmp(cfg->type, "ally") == 0);
}

gboolean is_prey(BassinUI *ui, Poisson *p)
{
   if (!p) return FALSE;
   SpeciesConfig *cfg = find_species_config(ui, p->nom);
   return cfg && (strcmp(cfg->type, "prey") == 0);
}

// Floating Damage numbers tick animation
typedef struct {
   GtkWidget *label;
   double x, y;
   double vy;
   int ticks_remaining;
   BassinUI *ui;
} FloatingDamage;

static gboolean animate_floating_damage_tick(gpointer user_data)
{
   FloatingDamage *fd = (FloatingDamage *)user_data;
   if (!fd->ui->simulation_running)
   {
      return TRUE;
   }

   fd->ticks_remaining--;
   if (fd->ticks_remaining <= 0)
   {
      gtk_widget_unparent(fd->label);
      g_free(fd);
      return FALSE;
   }

   fd->y += fd->vy;
   double opacity = (double)fd->ticks_remaining / 20.0;
   gtk_widget_set_opacity(fd->label, opacity);

   gtk_fixed_move(GTK_FIXED(fd->ui->canvas), fd->label, (int)fd->x, (int)fd->y);
   return TRUE;
}

void spawn_floating_damage(BassinUI *ui, double x, double y, double damage)
{
   char buf[32];
   sprintf(buf, "-%.0f", damage);

   FloatingDamage *fd = g_new0(FloatingDamage, 1);
   fd->label = gtk_label_new(buf);
   fd->x = x;
   fd->y = y;
   fd->vy = -1.5;
   fd->ticks_remaining = 20;
   fd->ui = ui;

   gtk_widget_add_css_class(fd->label, "floating-damage");
   gtk_fixed_put(GTK_FIXED(ui->canvas), fd->label, (int)fd->x, (int)fd->y);

   g_timeout_add(33, animate_floating_damage_tick, fd);
}

// Drag structures and handlers
typedef struct
{
   double start_x;
   double start_y;
} DragStart;

static void on_fish_drag_begin(GtkGestureDrag *gesture, double start_x, double start_y, gpointer user_data)
{
   (void)start_x;
   (void)start_y;
   (void)user_data;
   Poisson *p = g_object_get_data(G_OBJECT(gesture), "poisson");
   BassinUI *ui = g_object_get_data(G_OBJECT(gesture), "ui");
   if (!p || !ui)
      return;

   // Only allow repositioning when simulation is paused
   if (ui->simulation_running)
      return;

   DragStart *ds = g_new0(DragStart, 1);
   ds->start_x = p->x;
   ds->start_y = p->y;
   g_object_set_data_full(G_OBJECT(gesture), "drag_start", ds, g_free);
   GtkWidget *w = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
   if (w)
      g_object_set_data(G_OBJECT(w), "dragging", GINT_TO_POINTER(1));
}

static void on_fish_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data)
{
   (void)user_data;
   Poisson *p = g_object_get_data(G_OBJECT(gesture), "poisson");
   BassinUI *ui = g_object_get_data(G_OBJECT(gesture), "ui");
   if (!p || !ui)
      return;

   // Only allow repositioning when simulation is paused
   if (ui->simulation_running)
      return;

   DragStart *ds = g_object_get_data(G_OBJECT(gesture), "drag_start");
   if (!ds)
      return;

   double new_x = ds->start_x + offset_x;
   double new_y = ds->start_y + offset_y;

   // Clamp to canvas bounds
   double margin = 10;
   double right_bound = ui->config_canvas_width - p->taille - margin;
   double bottom_bound = ui->config_canvas_height - p->taille - margin;
   if (new_x < margin)
      new_x = margin;
   if (new_y < margin)
      new_y = margin;
   if (new_x > right_bound)
      new_x = right_bound;
   if (new_y > bottom_bound)
      new_y = bottom_bound;

   p->x = new_x;
   p->y = new_y;
   if (p->widget_image)
   {
      gtk_fixed_move(GTK_FIXED(ui->canvas), p->widget_image, (int)p->x, (int)p->y);
   }
}

static void on_fish_drag_end(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data)
{
   (void)user_data;
   Poisson *p = g_object_get_data(G_OBJECT(gesture), "poisson");
   BassinUI *ui = g_object_get_data(G_OBJECT(gesture), "ui");
   if (!p || !ui)
      return;

   // Only allow repositioning when simulation is paused
   if (ui->simulation_running)
      return;

   DragStart *ds = g_object_get_data(G_OBJECT(gesture), "drag_start");
   if (!ds)
      return;

   double final_x = ds->start_x + offset_x;
   double final_y = ds->start_y + offset_y;

   double dx = final_x - ds->start_x;
   double dy = final_y - ds->start_y;
   double dist = sqrt(dx * dx + dy * dy);
   if (dist > 1.0)
   {
      // Set new heading and normalize velocity to species normal speed
      double nx = dx / dist;
      double ny = dy / dist;
      p->vx = nx * p->vitesse_normale;
      p->vy = ny * p->vitesse_normale;
   }

   // Remove drag_start
   g_object_set_data(G_OBJECT(gesture), "drag_start", NULL);
   GtkWidget *w = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
   if (w)
      g_object_set_data(G_OBJECT(w), "dragging", NULL);
}

// Click on fish handler
static void on_fish_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
   Poisson *p = g_object_get_data(G_OBJECT(gesture), "poisson");
   BassinUI *ui = g_object_get_data(G_OBJECT(gesture), "ui");
   GtkWidget *w = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
   if (w && GPOINTER_TO_INT(g_object_get_data(G_OBJECT(w), "dragging")))
   {
      // A drag is in progress on this widget — ignore click
      return;
   }
   (void)n_press;
   (void)x;
   (void)y;
   (void)user_data;

   if (!p || !ui)
      return;

   // If delete mode is active, delete this fish instead of opening details
   if (ui->delete_mode)
   {
      if (p->widget_image)
      {
         gtk_widget_unparent(p->widget_image);
      }
      ui->poissons = g_list_remove(ui->poissons, p);
      poisson_free(p);
      ui->delete_mode = FALSE;
      /* Reset cursor to default */
      gtk_widget_set_cursor(GTK_WIDGET(ui->canvas), NULL);
      if (ui->btn_remove)
      {
         gtk_button_set_label(GTK_BUTTON(ui->btn_remove), "🗑️ Supprimer");
      }
      update_sidebar_list(ui);
      update_status_bar(ui);
      return;
   }

   // 1. Clear previous selected fish CSS highlight
   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *other = l->data;
      if (other->widget_image)
      {
         gtk_widget_remove_css_class(other->widget_image, "fish-selected");
      }
   }

   // 2. Add highlight CSS class to clicked fish
   if (p->widget_image)
   {
      gtk_widget_add_css_class(p->widget_image, "fish-selected");
   }

   // 3. Open a beautiful GtkDialog showing characteristics
   Dialog *details = g_new0(Dialog, 1);
   dialog_initialiser(details);
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
   {
      details->parent = GTK_WINDOW(toplevel);
   }

   char title_buf[128];
   sprintf(title_buf, "🔍 Inspecteur : %s #%d", p->nom, p->id);
   dialog_set_titre(details, title_buf);
   details->boutons_preset = DIALOG_BOUTONS_OK;

   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   // Add details text fields
   char info_buf[2048];
   char *banc_str = p->id_banc >= 0 ? g_strdup_printf("Banc #%d", p->id_banc) : g_strdup("Aucun (Solo)");
   char *role_str = p->est_leader ? g_strdup("★ Leader du Banc") : (p->id_banc >= 0 ? g_strdup("Membre du Banc") : g_strdup("Autonome"));

   sprintf(info_buf,
           "<b>Espèce :</b> %s\n"
           "<b>ID Unique :</b> %d\n"
           "<b>Groupe :</b> %s\n"
           "<b>Rôle :</b> %s\n"
           "<b>Position :</b> X=%.1f, Y=%.1f\n"
           "<b>Vitesse :</b> Vx=%.1f, Vy=%.1f\n"
           "<b>Santé :</b> %.1f / %.1f\n\n"
           "<i>Caractéristiques de l'Espèce :</i>\n"
           "• Vitesse Normale : %.1f px/s\n"
           "• Vitesse Fuite : %.1f px/s\n"
           "• Taille : %d px\n"
           "• Rayon de Détection : %d px",
           p->nom,
           p->id,
           banc_str,
           role_str,
           p->x, p->y,
           p->vx, p->vy,
           p->sante, p->sante_max,
           p->vitesse_normale,
           p->vitesse_fuite,
           p->taille,
           p->perimetre_detection);

   g_free(banc_str);
   g_free(role_str);

   // Add prey statistics for predators or species with kills
   if (p->nb_kills_types > 0 || is_predator(ui, p))
   {
      strcat(info_buf, "\n\n<b>Proies dévorées :</b>");
      if (p->nb_kills_types == 0)
      {
         strcat(info_buf, "\n• Aucune proie pour le moment");
      }
      else
      {
         for (int i = 0; i < p->nb_kills_types; i++)
         {
            char kill_line[128];
            sprintf(kill_line, "\n• %s : %d", p->kills_espece[i], p->kills_count[i]);
            strcat(info_buf, kill_line);
         }
      }
   }

   GtkWidget *lbl_info = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl_info), info_buf);
   gtk_label_set_justify(GTK_LABEL(lbl_info), GTK_JUSTIFY_LEFT);
   gtk_widget_set_halign(lbl_info, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_info);

   dialog_set_contenu(details, box);
   dialog_creer(details);
   dialog_afficher(details);

   g_signal_connect_swapped(details->window, "destroy", G_CALLBACK(dialog_free), details);
}

// Coordinate & build the single unified Poisson representation widget (non-static)
void create_poisson_widget(BassinUI *ui, Poisson *p)
{
   // Visual Representation Box: [Leader tag] -> [Health bar] -> [Image] -> [Name tag]
   GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

   // Leader Tag
   GtkWidget *lbl_lead = gtk_label_new("★ Leader");
   gtk_widget_add_css_class(lbl_lead, "leader-tag");
   gtk_widget_set_visible(lbl_lead, p->est_leader);
   gtk_box_append(GTK_BOX(container), lbl_lead);

   GtkWidget *health_bar = gtk_progress_bar_new();
   gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(health_bar), p->sante / p->sante_max);
   gtk_widget_add_css_class(health_bar, "fish-health-bar");
   if (p->sante < 40.0)
      gtk_widget_add_css_class(health_bar, "low-health");
   gtk_widget_set_halign(health_bar, GTK_ALIGN_CENTER);
   gtk_widget_set_size_request(health_bar, (int)(p->taille * 0.5), 4);
   gtk_box_append(GTK_BOX(container), health_bar);

   // Sprite (using GtkPicture for premium and accurate auto-scaling to p->taille)
   GtkWidget *img = gtk_picture_new_for_filename(p->chemin_frames[0] ? p->chemin_frames[0] : "resources/kenney_fish-pack_2.0/PNG/Default/fish_blue.png");
   gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(img), TRUE);
   gtk_widget_set_size_request(img, p->taille, p->taille);
   gtk_box_append(GTK_BOX(container), img);

   // Tag Label
   char tag_buf[64];
   if (is_predator(ui, p))
   {
      sprintf(tag_buf, "%s Alpha", p->nom);
   }
   else if (is_ally(ui, p))
   {
      sprintf(tag_buf, "%s", p->nom);
   }
   else
   {
      const char *short_nom = p->nom;
      if (strcmp(p->nom, "Poisson clown") == 0) short_nom = "Clown";
      else if (strcmp(p->nom, "Poisson-globe") == 0) short_nom = "Globe";
      sprintf(tag_buf, "%s #%d", short_nom, p->id);
   }

   GtkWidget *lbl_tag = gtk_label_new(tag_buf);
   gtk_widget_add_css_class(lbl_tag, "fish-tag");
   if (is_predator(ui, p))
   {
      gtk_widget_add_css_class(lbl_tag, "fish-tag-pred");
   }
   gtk_box_append(GTK_BOX(container), lbl_tag);

   // Enable click gesture
   GtkGesture *gesture = gtk_gesture_click_new();
   g_object_set_data(G_OBJECT(gesture), "poisson", p);
   g_object_set_data(G_OBJECT(gesture), "ui", ui);
   g_signal_connect(gesture, "released", G_CALLBACK(on_fish_clicked), NULL);
   gtk_widget_add_controller(container, GTK_EVENT_CONTROLLER(gesture));

   /* Drag-to-reposition while simulation is paused */
   GtkGesture *drag = gtk_gesture_drag_new();
   g_object_set_data(G_OBJECT(drag), "poisson", p);
   g_object_set_data(G_OBJECT(drag), "ui", ui);
   g_signal_connect(drag, "drag-begin", G_CALLBACK(on_fish_drag_begin), NULL);
   g_signal_connect(drag, "drag-update", G_CALLBACK(on_fish_drag_update), NULL);
   g_signal_connect(drag, "drag-end", G_CALLBACK(on_fish_drag_end), NULL);
   gtk_widget_add_controller(container, GTK_EVENT_CONTROLLER(drag));

   poisson_set_widget(p, container);
   gtk_fixed_put(GTK_FIXED(ui->canvas), container, (int)p->x, (int)p->y);
}

// Controller key bindings and destroy callbacks
static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data)
{
   (void)controller;
   (void)keycode;
   BassinUI *ui = user_data;

   if ((keyval == GDK_KEY_d || keyval == GDK_KEY_D) && (state & GDK_CONTROL_MASK) != 0)
   {
      ui->debug_mode = !ui->debug_mode;
      g_print("Debug mode toggled: %d\n", ui->debug_mode);
      if (ui->debug_overlay)
      {
         gtk_widget_queue_draw(ui->debug_overlay);
      }
      return TRUE; // handled
   }
   return FALSE;
}

static void on_root_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (ui->timer_id > 0)
   {
      g_source_remove(ui->timer_id);
      ui->timer_id = 0;
   }
}

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
   ui->config_fish_size = 64;
   ui->config_bg_path = "resources/images/background_banc.png";
   ui->config_canvas_width = 900;
   ui->config_canvas_height = 600;

   // Root layout box (vertical)
   ui->root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

   // ------------------ HEADER PANEL ------------------
   GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
   gtk_widget_set_margin_start(header, 15);
   gtk_widget_set_margin_end(header, 15);
   gtk_widget_set_margin_top(header, 8);
   gtk_widget_set_margin_bottom(header, 8);

   // Use the modular menu/action setup (Menu and Bouton custom widgets)
   bassin_menu_init(ui, header);
   gtk_box_append(GTK_BOX(ui->root), header);

   // Separator
   GtkWidget *sep_top = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_box_append(GTK_BOX(ui->root), sep_top);

   // ------------------ CONTENT BOX (Aquarium + Sidebar) ------------------
   GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
   gtk_widget_set_vexpand(content_box, TRUE);

   // 1. Center Canvas aquarium (GtkFixed overlayed on background GtkPicture)
   GtkWidget *overlay = gtk_overlay_new();
   gtk_widget_set_hexpand(overlay, TRUE);
   gtk_widget_set_vexpand(overlay, TRUE);

   // Canvas Background Image (using GtkPicture with no aspect ratio constraint for dynamic stretching)
   ui->bg_widget = gtk_picture_new_for_filename(ui->config_bg_path);
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
   GtkWidget *sep_bottom = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_box_append(GTK_BOX(ui->root), sep_bottom);

   // ------------------ BOTTOM STATUS BAR ------------------
   GtkWidget *status_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
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

   // Setup key controller for debug mode shortcut (Ctrl+D)
   GtkEventController *key_controller = gtk_event_controller_key_new();
   g_signal_connect(key_controller, "key-pressed", G_CALLBACK(on_key_pressed), ui);
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

   return ui->root;
}
