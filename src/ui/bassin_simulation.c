#include "bassin_private.h"
#include <math.h>
#include <string.h>

// Helpers for type checking (declared in bassin_private.h)
// But defined in screen_bassin.c or bassin_private.h? Let's verify.
// They are defined in screen_bassin.c, so we can just use them here.

// Find closest prey
Poisson *find_nearest_prey(BassinUI *ui, Poisson *predator)
{
   Poisson *closest = NULL;
   double min_dist = 999999.0;
   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (p != predator && is_prey(ui, p) && p->sante > 0.0)
      {
         double dx = (p->x + p->taille / 2.0) - (predator->x + predator->taille / 2.0);
         double dy = (p->y + p->taille / 2.0) - (predator->y + predator->taille / 2.0);
         double dist = sqrt(dx * dx + dy * dy);
         if (dist < min_dist)
         {
            min_dist = dist;
            closest = p;
         }
      }
   }
   return closest;
}

// Find closest predator
Poisson *find_nearest_predator(BassinUI *ui, Poisson *fish)
{
   Poisson *closest = NULL;
   double min_dist = 999999.0;
   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (is_predator(ui, p))
      {
         double dx = (p->x + p->taille / 2.0) - (fish->x + fish->taille / 2.0);
         double dy = (p->y + p->taille / 2.0) - (fish->y + fish->taille / 2.0);
         double dist = sqrt(dx * dx + dy * dy);
         if (dist < min_dist)
         {
            min_dist = dist;
            closest = p;
         }
      }
   }
   return closest;
}

static void crediter_mort(Poisson *predator, const char *espece_nom)
{
   if (!predator || !espece_nom)
      return;

   // Check if we already have this species recorded
   for (int i = 0; i < predator->nb_kills_types; i++)
   {
      if (predator->kills_espece[i] && strcmp(predator->kills_espece[i], espece_nom) == 0)
      {
         predator->kills_count[i]++;
         return;
      }
   }

   // If not, and we have space (up to 8)
   if (predator->nb_kills_types < 8)
   {
      int idx = predator->nb_kills_types;
      predator->kills_espece[idx] = strdup(espece_nom);
      predator->kills_count[idx] = 1;
      predator->nb_kills_types++;
   }
}

void eat_fish(BassinUI *ui, Poisson *prey)
{
   if (!prey)
      return;

   if (prey->dernier_attaquant)
   {
      gboolean predator_alive = FALSE;
      for (GList *l = ui->poissons; l; l = l->next)
      {
         if (l->data == prey->dernier_attaquant)
         {
            predator_alive = TRUE;
            break;
         }
      }
      if (predator_alive)
      {
         crediter_mort(prey->dernier_attaquant, prey->nom);
      }
   }

   if (prey->widget_image)
   {
      gtk_widget_unparent(prey->widget_image);
      prey->widget_image = NULL;
   }
   ui->poissons = g_list_remove(ui->poissons, prey);
   poisson_free(prey);
}

gboolean update_simulation(gpointer user_data)
{
   BassinUI *ui = user_data;
   if (!ui->simulation_running)
      return TRUE;

   double dt = 0.033 * ui->simulation_speed; // 33ms step multiplied by speed factor
   ui->elapsed_time += dt;

   GList *dead_poissons = NULL;

   // Dynamically update canvas and background bounds if window resizes
   GtkWidget *parent = gtk_widget_get_parent(ui->canvas);
   int canvas_w = parent ? gtk_widget_get_width(parent) : gtk_widget_get_width(ui->canvas);
   int canvas_h = parent ? gtk_widget_get_height(parent) : gtk_widget_get_height(ui->canvas);
   if (parent)
   {
      GtkWidget *curr = parent;
      while (curr && !GTK_IS_SCROLLED_WINDOW(curr))
      {
         curr = gtk_widget_get_parent(curr);
      }
      if (curr && GTK_IS_SCROLLED_WINDOW(curr))
      {
         GtkAdjustment *hadj = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(curr));
         GtkAdjustment *vadj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(curr));
         if (gtk_adjustment_get_value(hadj) != 0.0)
            gtk_adjustment_set_value(hadj, 0.0);
         if (gtk_adjustment_get_value(vadj) != 0.0)
            gtk_adjustment_set_value(vadj, 0.0);
      }
   }
   if (canvas_w > 50 && canvas_h > 50)
   {
      if (canvas_w != ui->config_canvas_width || canvas_h != ui->config_canvas_height)
      {
         ui->config_canvas_width = canvas_w;
         ui->config_canvas_height = canvas_h;
         gtk_widget_set_size_request(ui->canvas, canvas_w, canvas_h);
         if (ui->debug_overlay)
         {
            gtk_widget_set_size_request(ui->debug_overlay, canvas_w, canvas_h);
         }
         if (ui->bg_widget)
         {
            gtk_widget_set_size_request(ui->bg_widget, canvas_w, canvas_h);
         }
      }
   }

   // 1. Gather all species movements & boids calculations
   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      double force_x = 0;
      double force_y = 0;

      if (p->etat == ETAT_HORS_CADRE)
      {
         p->temps_hors_cadre += dt;
         if (p->temps_hors_cadre >= 2.5) // Wait 2.5 seconds, then turn back
         {
            double center_x = ui->config_canvas_width / 2.0;
            double center_y = ui->config_canvas_height / 2.0;
            double dx = center_x - p->x;
            double dy = center_y - p->y;
            double dist = sqrt(dx * dx + dy * dy);
            if (dist > 0)
            {
               force_x = (dx / dist) * p->vitesse_normale;
               force_y = (dy / dist) * p->vitesse_normale;
            }
         }
         else
         {
            force_x = p->vx;
            force_y = p->vy;
         }
      }
      else if (is_predator(ui, p))
      {
         // Predator chases nearest prey
         Poisson *target = find_nearest_prey(ui, p);
         if (target)
         {
            double dx = (target->x + target->taille / 2.0) - (p->x + p->taille / 2.0);
            double dy = (target->y + target->taille / 2.0) - (p->y + p->taille / 2.0);
            double dist = sqrt(dx * dx + dy * dy);
            if (dist > 0)
            {
               force_x = (dx / dist) * p->vitesse_fuite;
               force_y = (dy / dist) * p->vitesse_fuite;

               // Check if attacking
               if (dist < (p->taille + target->taille) * 0.5)
               {
                  double damage_rate = 180.0;
                  target->sante -= damage_rate * dt;
                  target->temps_effet_attaque = 0.5;
                  target->dernier_attaquant = p;
                  target->degats_accumules += damage_rate * dt;
               }
            }
         }
         else
         {
            force_x = p->vx;
            force_y = p->vy;
         }
      }
      else if (is_ally(ui, p))
      {
         // Ally (e.g. Dauphin) chases nearest predator
         Poisson *target = find_nearest_predator(ui, p);
         if (target)
         {
            double dx = (target->x + target->taille / 2.0) - (p->x + p->taille / 2.0);
            double dy = (target->y + target->taille / 2.0) - (p->y + p->taille / 2.0);
            double dist = sqrt(dx * dx + dy * dy);
            if (dist > 0)
            {
               force_x = (dx / dist) * p->vitesse_fuite;
               force_y = (dy / dist) * p->vitesse_fuite;
            }
         }
         else
         {
            force_x = p->vx;
            force_y = p->vy;
         }
      }
      else
      {
         // Prey: Hareng, Poisson-globe, Poisson clown
         Poisson *pred = find_nearest_predator(ui, p);
         if (pred)
         {
            double dx = (pred->x + pred->taille / 2.0) - (p->x + p->taille / 2.0);
            double dy = (pred->y + pred->taille / 2.0) - (p->y + p->taille / 2.0);
            double dist = sqrt(dx * dx + dy * dy);
            if (dist < p->perimetre_detection)
            {
               p->etat = ETAT_FUITE;
               if (dist > 0)
               {
                  force_x = -(dx / dist) * p->vitesse_fuite;
                  force_y = -(dy / dist) * p->vitesse_fuite;
               }
            }
            else
            {
               p->etat = ETAT_NORMAL;
            }
         }

         if (p->etat == ETAT_NORMAL)
         {
            if (p->id_banc >= 0)
            {
               // Flocking behaviors
               double coh_x = 0, coh_y = 0;
               double align_vx = 0, align_vy = 0;
               double sep_x = 0, sep_y = 0;
               int num_banc_members = 0;
               Poisson *leader = NULL;

               for (GList *o = ui->poissons; o; o = o->next)
               {
                  Poisson *other = o->data;
                  if (other != p && other->id_banc == p->id_banc)
                  {
                     coh_x += other->x;
                     coh_y += other->y;
                     align_vx += other->vx;
                     align_vy += other->vy;

                     double dx = p->x - other->x;
                     double dy = p->y - other->y;
                     double d = sqrt(dx * dx + dy * dy);
                     if (d < 45.0 && d > 0)
                     {
                        sep_x += dx / d;
                        sep_y += dy / d;
                      }
                      if (other->est_leader)
                      {
                         leader = other;
                      }
                      num_banc_members++;
                   }
                }

                if (num_banc_members > 0)
                {
                   coh_x /= num_banc_members;
                   coh_y /= num_banc_members;
                   align_vx /= num_banc_members;
                   align_vy /= num_banc_members;

                   // Cohesion force
                   double coh_fx = coh_x - p->x;
                   double coh_fy = coh_y - p->y;
                   double coh_d = sqrt(coh_fx * coh_fx + coh_fy * coh_fy);
                   if (coh_d > 0)
                   {
                      coh_fx = (coh_fx / coh_d) * p->vitesse_normale;
                      coh_fy = (coh_fy / coh_d) * p->vitesse_normale;
                   }

                   // Alignment force
                   double align_d = sqrt(align_vx * align_vx + align_vy * align_vy);
                   if (align_d > 0)
                   {
                      align_vx = (align_vx / align_d) * p->vitesse_normale;
                      align_vy = (align_vy / align_d) * p->vitesse_normale;
                   }

                   force_x = coh_fx * 0.4 + align_vx * 0.3 + sep_x * 0.4 * p->vitesse_normale;
                   force_y = coh_fy * 0.4 + align_vy * 0.3 + sep_y * 0.4 * p->vitesse_normale;

                   if (leader && !p->est_leader)
                   {
                      double lead_dx = leader->x - p->x;
                      double lead_dy = leader->y - p->y;
                      double lead_d = sqrt(lead_dx * lead_dx + lead_dy * lead_dy);
                      if (lead_d > 0)
                      {
                         force_x += (lead_dx / lead_d) * p->vitesse_normale * 0.3;
                         force_y += (lead_dy / lead_d) * p->vitesse_normale * 0.3;
                      }
                   }
                }
                else
                {
                   force_x = p->vx;
                   force_y = p->vy;
                }
             }
             else
             {
                // Solo random swim
                force_x = p->vx;
                force_y = p->vy;
             }
          }
       }

       // Smooth velocity transitions
       p->vx = p->vx * 0.85 + force_x * 0.15;
       p->vy = p->vy * 0.85 + force_y * 0.15;

       // Restrict speed to target bounds
       double cur_speed = sqrt(p->vx * p->vx + p->vy * p->vy);
       double tar_speed = (p->etat == ETAT_FUITE) ? p->vitesse_fuite : p->vitesse_normale;
       if (cur_speed > 0)
       {
          p->vx = (p->vx / cur_speed) * tar_speed;
          p->vy = (p->vy / cur_speed) * tar_speed;
       }
    }

    // 2. Apply movement coordinates and check boundary collisions
    for (GList *l = ui->poissons; l; l = l->next)
    {
       Poisson *p = l->data;

       // Decrement attack effect timer
       if (p->temps_effet_attaque > 0.0)
       {
          p->temps_effet_attaque -= dt;
          if (p->temps_effet_attaque < 0.0)
             p->temps_effet_attaque = 0.0;
       }

       // Decrement floating damage timer
       if (p->temps_dernier_floating_damage > 0.0)
       {
          p->temps_dernier_floating_damage -= dt;
          if (p->temps_dernier_floating_damage < 0.0)
             p->temps_dernier_floating_damage = 0.0;
       }

       // Check if we need to spawn floating damage
       if (p->temps_dernier_floating_damage <= 0.0 && p->degats_accumules >= 1.0)
       {
          spawn_floating_damage(ui, p->x + p->taille * 0.8, p->y + p->taille * 0.1, p->degats_accumules);
          p->degats_accumules = 0.0;
          p->temps_dernier_floating_damage = 0.35;
       }

       // Check if dead
       if (p->sante <= 0.0)
       {
          if (p->degats_accumules >= 1.0)
          {
             spawn_floating_damage(ui, p->x + p->taille * 0.8, p->y + p->taille * 0.1, p->degats_accumules);
             p->degats_accumules = 0.0;
          }
          dead_poissons = g_list_append(dead_poissons, p);
          continue;
       }

       p->x += p->vx * dt;
       p->y += p->vy * dt;

       // Frame constraints relative to configurable canvas size
       double margin = 10;
       double right_bound = ui->config_canvas_width - p->taille - margin;
       double bottom_bound = ui->config_canvas_height - p->taille - margin;
       gboolean is_outside = (p->x < margin || p->x > right_bound || p->y < margin || p->y > bottom_bound);

       if (is_outside)
       {
          if (p->etat != ETAT_HORS_CADRE)
          {
             p->etat = ETAT_HORS_CADRE;
             p->temps_hors_cadre = 0.0;
          }

          // Safety boundary: prevent swimming too far off-screen (hard limit of 150px)
          double left_limit = margin - 150.0;
          double right_limit = right_bound + 150.0;
          double top_limit = margin - 150.0;
          double bottom_limit = bottom_bound + 150.0;

          if (p->x < left_limit)
          {
             p->x = left_limit;
             p->vx = -p->vx;
          }
          else if (p->x > right_limit)
          {
             p->x = right_limit;
             p->vx = -p->vx;
          }

          if (p->y < top_limit)
          {
             p->y = top_limit;
             p->vy = -p->vy;
          }
          else if (p->y > bottom_limit)
          {
             p->y = bottom_limit;
             p->vy = -p->vy;
          }
       }
       else
       {
          // Inside bounds: reset out-of-bounds state
          if (p->etat == ETAT_HORS_CADRE)
          {
             p->etat = ETAT_NORMAL;
             p->temps_hors_cadre = 0.0;
          }
       }

       // Animate/move widget
       if (p->widget_image)
       {
          // Update fish animation frame if multiple frames exist
          if (p->chemin_frames[1] != NULL)
          {
             p->temps_depuis_frame += dt;
             if (p->temps_depuis_frame >= 0.25) // Change frame every 250ms
             {
                p->temps_depuis_frame = 0.0;
                int next_frame = p->frame_courante + 1;
                if (next_frame >= 3 || p->chemin_frames[next_frame] == NULL)
                {
                   next_frame = 0;
                }
                p->frame_courante = next_frame;
             }
          }

          GtkWidget *lead_widget = gtk_widget_get_first_child(p->widget_image);
          GtkWidget *health_bar = lead_widget ? gtk_widget_get_next_sibling(lead_widget) : NULL;
          GtkWidget *img_widget = health_bar ? gtk_widget_get_next_sibling(health_bar) : NULL;

          if (health_bar && GTK_IS_PROGRESS_BAR(health_bar))
          {
             gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(health_bar), p->sante / p->sante_max);
             if (p->sante < 40.0)
                gtk_widget_add_css_class(health_bar, "low-health");
             else
                gtk_widget_remove_css_class(health_bar, "low-health");
          }

          double scale = 1.0;
          if (p->temps_effet_attaque > 0.0)
          {
             scale = 1.0 + 0.3 * sin(p->temps_effet_attaque * M_PI * 4.0);
             gtk_widget_add_css_class(p->widget_image, "under-attack");
          }
          else
          {
             gtk_widget_remove_css_class(p->widget_image, "under-attack");
          }

          if (img_widget && GTK_IS_PICTURE(img_widget))
          {
             int display_size = (int)(p->taille * scale);
             gtk_widget_set_size_request(img_widget, display_size, display_size);
             gtk_picture_set_filename(GTK_PICTURE(img_widget), p->chemin_frames[p->frame_courante] ? p->chemin_frames[p->frame_courante] : p->chemin_frames[0]);
             gboolean flipped = (p->vx < 0.0);
             if (strcmp(p->nom, "Puffer") == 0)
             {
                flipped = !flipped;
             }
             double angle_deg = atan2(!flipped ? p->vy : -p->vy, !flipped ? p->vx : -p->vx) * 180.0 / M_PI;
             int rounded_angle = (int)round(angle_deg);

             int last_angle = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(img_widget), "last_angle"));
             gboolean last_flipped = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(img_widget), "last_flipped"));
             gboolean has_last = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(img_widget), "has_last"));

             if (!has_last || last_angle != rounded_angle || last_flipped != flipped)
             {
                GtkCssProvider *provider = g_object_get_data(G_OBJECT(img_widget), "fish_css_provider");
                if (!provider)
                {
                   provider = gtk_css_provider_new();
                   GtkStyleContext *context = gtk_widget_get_style_context(img_widget);
                   gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
                   g_object_set_data_full(G_OBJECT(img_widget), "fish_css_provider", provider, g_object_unref);
                }
                char css_buf[128];
                if (flipped)
                {
                   sprintf(css_buf, "* { transform: scaleX(-1) rotate(%ddeg); }", rounded_angle);
                }
                else
                {
                   sprintf(css_buf, "* { transform: rotate(%ddeg); }", rounded_angle);
                }
                gtk_css_provider_load_from_data(provider, css_buf, -1);
                g_object_set_data(G_OBJECT(img_widget), "last_angle", GINT_TO_POINTER(rounded_angle));
                g_object_set_data(G_OBJECT(img_widget), "last_flipped", GINT_TO_POINTER(flipped));
                g_object_set_data(G_OBJECT(img_widget), "has_last", GINT_TO_POINTER(1));
             }
          }

          gtk_fixed_move(GTK_FIXED(ui->canvas), p->widget_image, (int)p->x, (int)p->y);
       }
    }

    // 3. Clean up dead poissons
    if (dead_poissons)
    {
       for (GList *d = dead_poissons; d; d = d->next)
       {
          Poisson *p = d->data;
          eat_fish(ui, p);
       }
       g_list_free(dead_poissons);
       update_sidebar_list(ui);
    }

    if (ui->debug_mode && ui->debug_overlay)
    {
       gtk_widget_queue_draw(ui->debug_overlay);
    }

    update_status_bar(ui);
    return TRUE;
}

void on_debug_draw(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data)
{
   (void)drawing_area;
   (void)width;
   (void)height;
   BassinUI *ui = user_data;
   if (!ui->debug_mode)
      return;

   double margin = 10.0;

   // Base Fish Zone reference boundaries (calculated around base fish size)
   cairo_set_source_rgba(cr, 1.0, 0.65, 0.0, 0.8); // Orange
   cairo_set_line_width(cr, 2.0);
   double dashes[] = {6.0, 4.0};
   cairo_set_dash(cr, dashes, 2, 0.0);

   cairo_rectangle(cr, margin, margin, ui->config_canvas_width - 2 * margin, ui->config_canvas_height - 2 * margin);
   cairo_stroke(cr);

   cairo_select_font_face(cr, "Inter", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(cr, 10.0);
   cairo_move_to(cr, margin + 5, margin + 15);
   cairo_show_text(cr, "LIMIT: ACTIVE ZONE BOUNDS");

   cairo_set_dash(cr, NULL, 0, 0.0);

   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;

      double cx = p->x + p->taille / 2.0;
      double cy = p->y + p->taille / 2.0;

      // Draw detection radius
      double radius = p->perimetre_detection; // use actual configured detection radius
      if (is_predator(ui, p))
      {
         cairo_set_source_rgba(cr, 0.9, 0.29, 0.23, 0.12);
         cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
         cairo_fill_preserve(cr);
         cairo_set_source_rgba(cr, 0.9, 0.29, 0.23, 0.5);
         cairo_set_line_width(cr, 1.5);
         cairo_stroke(cr);
      }
      else if (is_ally(ui, p))
      {
         cairo_set_source_rgba(cr, 0.18, 0.8, 0.44, 0.12);
         cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
         cairo_fill_preserve(cr);
         cairo_set_source_rgba(cr, 0.18, 0.8, 0.44, 0.5);
         cairo_set_line_width(cr, 1.5);
         cairo_stroke(cr);
      }
      else
      {
         cairo_set_source_rgba(cr, 0.2, 0.6, 0.86, 0.08);
         cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
         cairo_fill_preserve(cr);
         cairo_set_source_rgba(cr, 0.2, 0.6, 0.86, 0.3);
         cairo_set_line_width(cr, 1.0);
         cairo_stroke(cr);
      }

      // Draw direction vector arrow
      double speed = sqrt(p->vx * p->vx + p->vy * p->vy);
      if (speed > 0)
      {
         double arrow_len = 35.0;
         double dx = (p->vx / speed) * arrow_len;
         double dy = (p->vy / speed) * arrow_len;

         cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.85);
         cairo_set_line_width(cr, 2.0);
         cairo_move_to(cr, cx, cy);
         cairo_line_to(cr, cx + dx, cy + dy);
         cairo_stroke(cr);

         double angle = atan2(dy, dx);
         double arrow_angle = M_PI / 6.0;
         double head_len = 7.0;

         cairo_move_to(cr, cx + dx, cy + dy);
         cairo_line_to(cr,
                       cx + dx - head_len * cos(angle - arrow_angle),
                       cy + dy - head_len * sin(angle - arrow_angle));
         cairo_line_to(cr,
                       cx + dx - head_len * cos(angle + arrow_angle),
                       cy + dy - head_len * sin(angle + arrow_angle));
         cairo_close_path(cr);
         cairo_fill(cr);
      }

      // Draw label showing state, ID, health and coords
      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
      cairo_select_font_face(cr, "Consolas", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
      cairo_set_font_size(cr, 8.5);

      char info_text[128];
      const char *state_str = "NORMAL";
      if (p->etat == ETAT_FUITE)
         state_str = "FUITE";
      else if (p->etat == ETAT_RALENTI)
         state_str = "RALENTI";
      else if (p->etat == ETAT_HORS_CADRE)
         state_str = "HORS_CADRE";

      sprintf(info_text, "ID:%d [%s] HP:%.0f/%.0f (%.0f, %.0f)",
              p->id, state_str, p->sante, p->sante_max, p->x, p->y);

      cairo_text_extents_t extents;
      cairo_text_extents(cr, info_text, &extents);

      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.55);
      cairo_rectangle(cr, cx - extents.width / 2.0 - 4, cy - p->taille / 2.0 - 18, extents.width + 8, extents.height + 6);
      cairo_fill(cr);

      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
      cairo_move_to(cr, cx - extents.width / 2.0, cy - p->taille / 2.0 - 8);
      cairo_show_text(cr, info_text);
   }
}
