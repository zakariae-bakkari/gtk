#include "../simulation/bassin_private.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Helper functions for species type checking
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

void spawn_floating_kill(BassinUI *ui, double x, double y)
{
   FloatingDamage *fd = g_new0(FloatingDamage, 1);
   fd->label = gtk_label_new("+1 Kill");
   fd->x = x;
   fd->y = y;
   fd->vy = -1.5;
   fd->ticks_remaining = 20;
   fd->ui = ui;

   gtk_widget_add_css_class(fd->label, "floating-kill");
   gtk_fixed_put(GTK_FIXED(ui->canvas), fd->label, (int)fd->x, (int)fd->y);

   g_timeout_add(33, animate_floating_damage_tick, fd);
}

void spawn_floating_heal(BassinUI *ui, double x, double y, double amount)
{
   char buf[32];
   sprintf(buf, "+%.0f", amount);

   FloatingDamage *fd = g_new0(FloatingDamage, 1);
   fd->label = gtk_label_new(buf);
   fd->x = x;
   fd->y = y;
   fd->vy = -1.5;
   fd->ticks_remaining = 20;
   fd->ui = ui;

   gtk_widget_add_css_class(fd->label, "floating-kill"); // Use green color for healing
   gtk_fixed_put(GTK_FIXED(ui->canvas), fd->label, (int)fd->x, (int)fd->y);

   g_timeout_add(33, animate_floating_damage_tick, fd);
}

void spawn_food(BassinUI *ui, double x, double y)
{
   Food *f = g_new0(Food, 1);
   f->id = ++ui->next_food_id;
   f->x = x;
   f->y = y;
   f->vy = 40.0; // Sinking speed
   f->image_index = (rand() % 3) + 1;
   
   // Set health restore based on food type
   if (f->image_index == 1) f->health_restore = 15.0;
   else if (f->image_index == 2) f->health_restore = 25.0;
   else f->health_restore = 40.0;

   char path[128];
   sprintf(path, "resources/images/fishFood/food%d.png", f->image_index);
   f->widget = gtk_picture_new_for_filename(path);
   gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(f->widget), TRUE);
   gtk_widget_set_size_request(f->widget, 24, 24);

   ui->foods = g_list_append(ui->foods, f);
   gtk_fixed_put(GTK_FIXED(ui->canvas), f->widget, (int)f->x, (int)f->y);
   
   sound_play(SOUND_BUBBLES);
}

void on_image_widget_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   Image *img = (Image *)user_data;
   if (img)
   {
      image_free(img);
      g_free(img);
   }
}

GtkWidget *get_fish_picture_widget(Poisson *p)
{
   if (!p || !p->widget_image)
      return NULL;
   GtkWidget *lead_widget = gtk_widget_get_first_child(p->widget_image);
   GtkWidget *health_bar = lead_widget ? gtk_widget_get_next_sibling(lead_widget) : NULL;
   GtkWidget *img_widget = health_bar ? gtk_widget_get_next_sibling(health_bar) : NULL;
   return img_widget;
}

void update_fish_widget_tags(BassinUI *ui, Poisson *p)
{
   if (!p || !p->widget_image)
      return;

   GtkWidget *img_widget = get_fish_picture_widget(p);
   GtkWidget *lbl_tag = img_widget ? gtk_widget_get_next_sibling(img_widget) : NULL;

   if (lbl_tag && GTK_IS_LABEL(lbl_tag))
   {
      char tag_buf[128];
      if (ui->controlled_fish == p)
      {
         sprintf(tag_buf, "🕹️ [JOUEUR] %s", p->nom);
         if (img_widget)
            gtk_widget_add_css_class(img_widget, "fish-controlled");
      }
      else
      {
         if (img_widget)
            gtk_widget_remove_css_class(img_widget, "fish-controlled");
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
      }
      gtk_label_set_text(GTK_LABEL(lbl_tag), tag_buf);
   }
}

extern void on_fish_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);
extern void on_fish_drag_begin(GtkGestureDrag *gesture, double start_x, double start_y, gpointer user_data);
extern void on_fish_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data);
extern void on_fish_drag_end(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data);

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
   gtk_widget_set_visible(health_bar, !ui->config_hide_health_bar);
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
   gtk_widget_set_visible(lbl_tag, !ui->config_hide_fish_name);
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

void apply_fish_visibility_configs(BassinUI *ui)
{
   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (p->widget_image)
      {
         GtkWidget *lead_widget = gtk_widget_get_first_child(p->widget_image);
         GtkWidget *health_bar = lead_widget ? gtk_widget_get_next_sibling(lead_widget) : NULL;
         GtkWidget *img_widget = health_bar ? gtk_widget_get_next_sibling(health_bar) : NULL;
         GtkWidget *lbl_tag = img_widget ? gtk_widget_get_next_sibling(img_widget) : NULL;

         if (health_bar)
         {
            gtk_widget_set_visible(health_bar, !ui->config_hide_health_bar);
         }
         if (lbl_tag)
         {
            gtk_widget_set_visible(lbl_tag, !ui->config_hide_fish_name);
         }
      }
   }
}

void apply_zen_mode(BassinUI *ui)
{
   if (ui->zen_mode)
   {
      ui->sidebar_was_visible = gtk_widget_get_visible(ui->sidebar);

      if (ui->header_widget)
         gtk_widget_set_visible(ui->header_widget, FALSE);
      if (ui->sep_top_widget)
         gtk_widget_set_visible(ui->sep_top_widget, FALSE);
      if (ui->sidebar)
         gtk_widget_set_visible(ui->sidebar, FALSE);
      if (ui->sep_bottom_widget)
         gtk_widget_set_visible(ui->sep_bottom_widget, FALSE);
      if (ui->status_bar_widget)
         gtk_widget_set_visible(ui->status_bar_widget, FALSE);
   }
   else
   {
      if (ui->header_widget)
         gtk_widget_set_visible(ui->header_widget, TRUE);
      if (ui->sep_top_widget)
         gtk_widget_set_visible(ui->sep_top_widget, TRUE);
      if (ui->sidebar)
         gtk_widget_set_visible(ui->sidebar, ui->sidebar_was_visible);

      gboolean show_status = !ui->config_hide_status_bar;
      if (ui->sep_bottom_widget)
         gtk_widget_set_visible(ui->sep_bottom_widget, show_status);
      if (ui->status_bar_widget)
         gtk_widget_set_visible(ui->status_bar_widget, show_status);
   }
}

static gboolean is_video_file(const char *filename)
{
    if (!filename) return FALSE;
    const char *ext = strrchr(filename, '.');
    if (!ext) return FALSE;
    if (g_ascii_strcasecmp(ext, ".mp4") == 0 ||
        g_ascii_strcasecmp(ext, ".webm") == 0 ||
        g_ascii_strcasecmp(ext, ".mkv") == 0 ||
        g_ascii_strcasecmp(ext, ".avi") == 0)
    {
        return TRUE;
    }
    return FALSE;
}

void apply_background(BassinUI *ui)
{
    if (!ui || !ui->bg_widget || !ui->config_bg_path) return;

    if (ui->bg_stream)
    {
        gtk_media_stream_pause(ui->bg_stream);
        g_object_unref(ui->bg_stream);
        ui->bg_stream = NULL;
    }

    if (is_video_file(ui->config_bg_path))
    {
        ui->bg_stream = gtk_media_file_new_for_filename(ui->config_bg_path);
        if (ui->bg_stream)
        {
            gtk_media_stream_set_loop(ui->bg_stream, TRUE);
            gtk_picture_set_paintable(GTK_PICTURE(ui->bg_widget), GDK_PAINTABLE(ui->bg_stream));
            gtk_media_stream_play(ui->bg_stream);
        }
    }
    else
    {
        gtk_picture_set_filename(GTK_PICTURE(ui->bg_widget), ui->config_bg_path);
    }
}
