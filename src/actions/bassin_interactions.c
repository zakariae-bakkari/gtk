#include "../simulation/bassin_private.h"
#include <math.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>

// Drag structures and handlers
typedef struct
{
   double start_x;
   double start_y;
} DragStart;

void on_fish_drag_begin(GtkGestureDrag *gesture, double start_x, double start_y, gpointer user_data)
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

void on_fish_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data)
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

void on_fish_drag_end(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data)
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

void on_throw_food_clicked(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   // Throw food at a random top position
   double rx = (double)(rand() % (ui->config_canvas_width - 100)) + 50.0;
   spawn_food(ui, rx, 10.0);
}

// Click on fish handler
void on_fish_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
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
      GtkWidget *img_widget = get_fish_picture_widget(other);
      if (img_widget)
      {
         gtk_widget_remove_css_class(img_widget, "fish-selected");
      }
   }

   // 2. Add highlight CSS class to clicked fish
   GtkWidget *img_widget = get_fish_picture_widget(p);
   if (img_widget)
   {
      gtk_widget_add_css_class(img_widget, "fish-selected");
   }

   // 3. Open the fish details dialog using the custom show_fish_details_dialog function
   show_fish_details_dialog(ui, p);
}

// Shortcut parsing and matching helpers
static gboolean parse_shortcut(const char *str, guint *out_keyval, GdkModifierType *out_mods)
{
   if (!str || strlen(str) == 0)
      return FALSE;

   char *buf = g_strdup(str);
   GString *gstr = g_string_new("");
   
   char **parts = g_strsplit(buf, "+", -1);
   int i = 0;
   while (parts[i] != NULL)
   {
      char *part = parts[i];
      g_strstrip(part);
      
      if (parts[i+1] == NULL)
      {
         if (strlen(part) == 1)
         {
            g_string_append_c(gstr, g_ascii_tolower(part[0]));
         }
         else
         {
            g_string_append(gstr, part);
         }
      }
      else
      {
         if (g_ascii_strcasecmp(part, "Ctrl") == 0 || g_ascii_strcasecmp(part, "Control") == 0)
         {
            g_string_append(gstr, "<Control>");
         }
         else if (g_ascii_strcasecmp(part, "Alt") == 0)
         {
            g_string_append(gstr, "<Alt>");
         }
         else if (g_ascii_strcasecmp(part, "Shift") == 0)
         {
            g_string_append(gstr, "<Shift>");
         }
         else
         {
            g_string_append_printf(gstr, "<%s>", part);
         }
      }
      i++;
   }
   g_strfreev(parts);
   g_free(buf);

   guint key = 0;
   GdkModifierType mods = 0;
   gboolean ok = gtk_accelerator_parse(gstr->str, &key, &mods);
   g_string_free(gstr, TRUE);

   if (ok && key != 0)
   {
      *out_keyval = key;
      *out_mods = mods;
      return TRUE;
   }
   return FALSE;
}

static gboolean match_shortcut(const char *shortcut_str, guint keyval, GdkModifierType state)
{
   guint target_key = 0;
   GdkModifierType target_mods = 0;
   if (!parse_shortcut(shortcut_str, &target_key, &target_mods))
      return FALSE;

   GdkModifierType clean_state = state & (GDK_CONTROL_MASK | GDK_ALT_MASK | GDK_SHIFT_MASK | GDK_META_MASK);
   
   guint clean_keyval = gdk_keyval_to_lower(keyval);
   guint clean_target_key = gdk_keyval_to_lower(target_key);

   return (clean_keyval == clean_target_key && clean_state == target_mods);
}

void on_key_released(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data)
{
   (void)controller;
   (void)keycode;
   (void)state;
   BassinUI *ui = user_data;
   if (!ui || !ui->controlled_fish)
      return;

   if (keyval == GDK_KEY_Up || keyval == GDK_KEY_w || keyval == GDK_KEY_W || keyval == GDK_KEY_z || keyval == GDK_KEY_Z)
   {
      if (ui->controlled_fish->vy < 0)
         ui->controlled_fish->vy = 0;
   }
   else if (keyval == GDK_KEY_Down || keyval == GDK_KEY_s || keyval == GDK_KEY_S)
   {
      if (ui->controlled_fish->vy > 0)
         ui->controlled_fish->vy = 0;
   }
   else if (keyval == GDK_KEY_Left || keyval == GDK_KEY_a || keyval == GDK_KEY_A || keyval == GDK_KEY_q || keyval == GDK_KEY_Q)
   {
      if (ui->controlled_fish->vx < 0)
         ui->controlled_fish->vx = 0;
   }
   else if (keyval == GDK_KEY_Right || keyval == GDK_KEY_d || keyval == GDK_KEY_D)
   {
      if (ui->controlled_fish->vx > 0)
         ui->controlled_fish->vx = 0;
   }
}

gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data)
{
   (void)controller;
   (void)keycode;
   BassinUI *ui = user_data;
   GtkRoot *root = gtk_widget_get_root(ui->root);
   GtkWindow *window = GTK_IS_WINDOW(root) ? GTK_WINDOW(root) : NULL;

   gboolean ctrl = (state & GDK_CONTROL_MASK) != 0;

   // Playable controlled fish driving controls
   if (ui->controlled_fish && !ctrl)
   {
      if (keyval == GDK_KEY_Escape)
      {
         Poisson *old = ui->controlled_fish;
         ui->controlled_fish = NULL;
         update_fish_widget_tags(ui, old);
         return TRUE;
      }

      double speed = ui->controlled_fish->vitesse_normale * 1.5;
      gboolean handled = FALSE;

      if (keyval == GDK_KEY_Up || keyval == GDK_KEY_w || keyval == GDK_KEY_W || keyval == GDK_KEY_z || keyval == GDK_KEY_Z)
      {
         ui->controlled_fish->vy = -speed;
         handled = TRUE;
      }
      else if (keyval == GDK_KEY_Down || keyval == GDK_KEY_s || keyval == GDK_KEY_S)
      {
         ui->controlled_fish->vy = speed;
         handled = TRUE;
      }
      else if (keyval == GDK_KEY_Left || keyval == GDK_KEY_a || keyval == GDK_KEY_A || keyval == GDK_KEY_q || keyval == GDK_KEY_Q)
      {
         ui->controlled_fish->vx = -speed;
         handled = TRUE;
      }
      else if (keyval == GDK_KEY_Right || keyval == GDK_KEY_d || keyval == GDK_KEY_D)
      {
         ui->controlled_fish->vx = speed;
         handled = TRUE;
      }

      if (handled)
         return TRUE;
   }

   // F1 : Help
   if (keyval == GDK_KEY_F1)
   {
      show_shortcuts_help_dialog(ui);
      return TRUE;
   }

   // Debug Mode
   if (match_shortcut(ui->shortcut_debug, keyval, state))
   {
      ui->debug_mode = !ui->debug_mode;
      g_print("Debug mode toggled: %d\n", ui->debug_mode);
      if (ui->debug_overlay)
      {
         gtk_widget_queue_draw(ui->debug_overlay);
      }
      return TRUE;
   }

   // Zen Mode
   if (match_shortcut(ui->shortcut_zen, keyval, state))
   {
      ui->zen_mode = !ui->zen_mode;
      apply_zen_mode(ui);
      return TRUE;
   }

   // Settings
   if (match_shortcut(ui->shortcut_settings, keyval, state))
   {
      on_settings_clicked(NULL, ui);
      return TRUE;
   }

   // Add New Fish
   if (match_shortcut(ui->shortcut_add, keyval, state))
   {
      on_add_poisson_btn_clicked(NULL, ui);
      return TRUE;
   }

   // Play / Pause
   if (match_shortcut(ui->shortcut_play, keyval, state))
   {
      on_play_pause_clicked(NULL, ui);
      return TRUE;
   }

   // Toggle Sidebar
   if (match_shortcut(ui->shortcut_sidebar, keyval, state))
   {
      on_toggle_sidebar_clicked(NULL, ui);
      return TRUE;
   }

   // F11 or Ctrl+F : Fullscreen
   if (keyval == GDK_KEY_F11 || (match_shortcut("Ctrl+F", keyval, state)))
   {
      if (window)
      {
         static gboolean is_fullscreen = FALSE;
         if (!is_fullscreen)
            gtk_window_fullscreen(window);
         else
            gtk_window_unfullscreen(window);
         is_fullscreen = !is_fullscreen;
      }
      return TRUE;
   }

   // Ctrl+I : Throw Food
   if (match_shortcut(ui->shortcut_food, keyval, state))
   {
      on_throw_food_clicked(NULL, ui);
      return TRUE;
   }

   // Restart
   if (match_shortcut(ui->shortcut_restart, keyval, state))
   {
      on_restart_clicked(NULL, ui);
      return TRUE;
   }

   return FALSE;
}
