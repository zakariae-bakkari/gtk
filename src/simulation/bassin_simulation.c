#include "bassin_private.h"
#include "../core/sound.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Helper functions for shoals (Bancs)
GList *bassin_get_all_poissons(BassinUI *ui)
{
   GList *all = NULL;
   for (GList *l = ui->poissons; l; l = l->next)
   {
      all = g_list_append(all, l->data);
   }
   for (GList *l = ui->bancs; l; l = l->next)
   {
      Banc *b = l->data;
      for (GList *lp = b->poissons; lp; lp = lp->next)
      {
         all = g_list_append(all, lp->data);
      }
   }
   return all;
}

Banc *bassin_find_banc(BassinUI *ui, int banc_id)
{
   for (GList *l = ui->bancs; l; l = l->next)
   {
      Banc *b = l->data;
      if (b->id == banc_id)
         return b;
   }
   return NULL;
}

Banc *bassin_get_or_create_banc(BassinUI *ui, int banc_id, const char *species_name)
{
   Banc *b = bassin_find_banc(ui, banc_id);
   if (!b)
   {
      b = calloc(1, sizeof(Banc));
      b->id = banc_id;
      b->nom_espece = strdup(species_name ? species_name : "Poisson");
      b->poissons = NULL;
      b->leader = NULL;
      ui->bancs = g_list_append(ui->bancs, b);
      if (ui->num_bancs < banc_id)
         ui->num_bancs = banc_id;
   }
   return b;
}

void bassin_remove_poisson_from_banc(BassinUI *ui, Poisson *p)
{
   if (p->id_banc < 0) return;
   Banc *b = bassin_find_banc(ui, p->id_banc);
   if (b)
   {
      b->poissons = g_list_remove(b->poissons, p);
      if (b->leader == p)
      {
         if (b->poissons)
         {
            b->leader = b->poissons->data;
            b->leader->est_leader = TRUE;
            if (b->leader->widget_image)
            {
               GtkWidget *lbl_lead = gtk_widget_get_first_child(b->leader->widget_image);
               if (lbl_lead)
                  gtk_widget_set_visible(lbl_lead, TRUE);
            }
         }
         else
         {
            b->leader = NULL;
         }
      }
      if (!b->poissons)
      {
         free(b->nom_espece);
         ui->bancs = g_list_remove(ui->bancs, b);
         free(b);
      }
   }
   p->id_banc = -1;
   p->est_leader = FALSE;
}

void bassin_add_poisson_to_banc(BassinUI *ui, Poisson *p, int banc_id)
{
   if (p->id_banc >= 0)
   {
      bassin_remove_poisson_from_banc(ui, p);
   }
   else
   {
      ui->poissons = g_list_remove(ui->poissons, p);
   }
   p->id_banc = banc_id;
   Banc *b = bassin_get_or_create_banc(ui, banc_id, p->nom);
   b->poissons = g_list_append(b->poissons, p);
   if (!b->leader)
   {
      b->leader = p;
      p->est_leader = TRUE;
   }
   else
   {
      p->est_leader = FALSE;
   }
}

void bassin_add_poisson(BassinUI *ui, Poisson *p)
{
   if (p->id_banc >= 0)
   {
      Banc *b = bassin_get_or_create_banc(ui, p->id_banc, p->nom);
      b->poissons = g_list_append(b->poissons, p);
      if (p->est_leader)
         b->leader = p;
      else if (!b->leader)
      {
         b->leader = p;
         p->est_leader = TRUE;
      }
   }
   else
   {
      ui->poissons = g_list_append(ui->poissons, p);
   }
}

gboolean can_eat(BassinUI *ui, Poisson *predator, Poisson *prey)
{
   if (!predator || !prey) return FALSE;
   SpeciesConfig *pred_cfg = find_species_config(ui, predator->nom);
   if (!pred_cfg) return FALSE;

   if (pred_cfg->nb_diet > 0)
   {
      for (int i = 0; i < pred_cfg->nb_diet; i++)
      {
         if (strcmp(pred_cfg->diet[i], prey->nom) == 0)
         {
            return TRUE;
         }
      }
      return FALSE;
   }

   int pred_level = pred_cfg->level;
   int prey_level = get_fish_level(ui, prey);
   return pred_level > prey_level;
}

Poisson *find_nearest_prey(BassinUI *ui, Poisson *predator)
{
   Poisson *closest = NULL;
   double min_dist = 999999.0;
   GList *all_poissons = bassin_get_all_poissons(ui);
   for (GList *l = all_poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (p != predator && p->sante > 0.0 && p->etat != ETAT_HORS_CADRE)
      {
         if (can_eat(ui, predator, p))
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
   }
   g_list_free(all_poissons);
   return closest;
}

Poisson *find_nearest_predator(BassinUI *ui, Poisson *fish)
{
   Poisson *closest = NULL;
   double min_dist = 999999.0;
   GList *all_poissons = bassin_get_all_poissons(ui);
   for (GList *l = all_poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (p != fish && p->sante > 0.0 && p->visible && p->etat != ETAT_HORS_CADRE)
      {
         if (can_eat(ui, p, fish))
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
   }
   g_list_free(all_poissons);
   return closest;
}

static void crediter_mort(Poisson *predator, const char *espece_nom)
{
   if (!predator || !espece_nom)
      return;

   for (int i = 0; i < predator->nb_kills_types; i++)
   {
      if (predator->kills_espece[i] && strcmp(predator->kills_espece[i], espece_nom) == 0)
      {
         predator->kills_count[i]++;
         return;
      }
   }

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
      GList *all_poissons = bassin_get_all_poissons(ui);
      for (GList *l = all_poissons; l; l = l->next)
      {
         if (l->data == prey->dernier_attaquant)
         {
            predator_alive = TRUE;
            break;
         }
      }
      g_list_free(all_poissons);
      
      if (predator_alive)
      {
         crediter_mort(prey->dernier_attaquant, prey->nom);
      }

      if (ui->controlled_fish && prey->dernier_attaquant == ui->controlled_fish)
      {
         spawn_floating_kill(ui, prey->x + prey->taille / 2.0, prey->y + prey->taille / 2.0);
         sound_play(SOUND_BITE);
         sound_play(SOUND_VICTORY);
      }
   }

   if (prey == ui->controlled_fish)
   {
      ui->controlled_fish = NULL;
      sound_play(SOUND_GAME_OVER);
      GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
      dialog_afficher_erreur(toplevel ? GTK_WINDOW(toplevel) : NULL,
                             "Défaite",
                             "Votre poisson a été mangé ! Vous avez perdu.",
                             NULL, NULL);
   }

   if (prey->widget_image)
   {
      gtk_widget_unparent(prey->widget_image);
      prey->widget_image = NULL;
   }
   
   if (prey->id_banc >= 0)
   {
      bassin_remove_poisson_from_banc(ui, prey);
   }
   else
   {
      ui->poissons = g_list_remove(ui->poissons, prey);
   }
   poisson_free(prey);

   update_sidebar_list(ui);
   update_status_bar(ui);
}

void bassin_tick_bancs_behavior(BassinUI *ui, double dt)
{
   (void)dt;
   // 1. Check if predator is near any member of a shoal to trigger split
   GList *bancs_copy = g_list_copy(ui->bancs);
   GList *all_poissons = bassin_get_all_poissons(ui);

   for (GList *lb = bancs_copy; lb; lb = lb->next)
   {
      Banc *b = lb->data;
      int count = g_list_length(b->poissons);
      if (count < 4) // Only split if school has >= 4 fish
         continue;

      gboolean predator_near = FALSE;
      for (GList *lp = b->poissons; lp; lp = lp->next)
      {
         Poisson *p = lp->data;
         for (GList *lo = all_poissons; lo; lo = lo->next)
         {
            Poisson *other = lo->data;
            if (is_predator(ui, other) && other->sante > 0.0 && other->etat != ETAT_HORS_CADRE && p->etat != ETAT_HORS_CADRE)
            {
               double dx = other->x - p->x;
               double dy = other->y - p->y;
               double dist = sqrt(dx * dx + dy * dy);
               if (dist < p->perimetre_detection)
               {
                  predator_near = TRUE;
                  break;
               }
            }
         }
         if (predator_near)
            break;
      }

      if (predator_near)
      {
         // Split school in two
         ui->num_bancs++;
         int new_banc_id = ui->num_bancs;

         Banc *new_b = calloc(1, sizeof(Banc));
         new_b->id = new_banc_id;
         new_b->nom_espece = strdup(b->nom_espece);

         int half = count / 2;
         int idx = 0;

         GList *old_poissons = b->poissons;
         b->poissons = NULL;
         b->leader = NULL;

         // Calculate average velocity of the school to get its general direction of motion
         double avg_vx = 0.0;
         double avg_vy = 0.0;
         int b_count = 0;
         for (GList *l = old_poissons; l; l = l->next)
         {
            Poisson *p = l->data;
            avg_vx += p->vx;
            avg_vy += p->vy;
            b_count++;
         }
         if (b_count > 0)
         {
            avg_vx /= b_count;
            avg_vy /= b_count;
         }
         else
         {
            avg_vx = 50.0;
            avg_vy = 0.0;
         }

         double speed = sqrt(avg_vx * avg_vx + avg_vy * avg_vy);
         double theta = atan2(avg_vy, avg_vx);
         if (speed < 1.0)
         {
            theta = 0.0;
         }

         for (GList *l = old_poissons; l; l = l->next)
         {
            Poisson *p = l->data;
            double p_speed = p->vitesse_fuite > 0.0 ? p->vitesse_fuite : 80.0;
            if (idx < half)
            {
               p->id_banc = new_banc_id;
               p->est_leader = (idx == 0);
               new_b->poissons = g_list_append(new_b->poissons, p);
               if (p->est_leader)
                  new_b->leader = p;

               // Flee direction: angle shifted by -45 degrees (-M_PI / 4.0) to go left-diagonal
               double angle = theta - M_PI / 4.0;
               p->vx = cos(angle) * p_speed;
               p->vy = sin(angle) * p_speed;
               p->etat = ETAT_FUITE;
            }
            else
            {
               p->est_leader = (idx == half);
               b->poissons = g_list_append(b->poissons, p);
               if (p->est_leader)
                  b->leader = p;

               // Flee direction: angle shifted by +45 degrees (+M_PI / 4.0) to go right-diagonal
               double angle = theta + M_PI / 4.0;
               p->vx = cos(angle) * p_speed;
               p->vy = sin(angle) * p_speed;
               p->etat = ETAT_FUITE;
            }

            if (p->widget_image)
            {
               GtkWidget *lbl_lead = gtk_widget_get_first_child(p->widget_image);
               if (lbl_lead)
                  gtk_widget_set_visible(lbl_lead, p->est_leader);
            }
            idx++;
         }

         g_list_free(old_poissons);
         ui->bancs = g_list_append(ui->bancs, new_b);

         if (b->leader)
         {
            spawn_floating_damage(ui, b->leader->x, b->leader->y, 0.0);
         }
         update_sidebar_list(ui);
      }
   }

   // 2. Automatic merge if schools of same species are close and no predator is nearby
   for (GList *la = ui->bancs; la; la = la->next)
   {
      Banc *ba = la->data;
      for (GList *lb = la->next; lb; lb = lb->next)
      {
         Banc *bb = lb->data;
         if (strcmp(ba->nom_espece, bb->nom_espece) == 0 && ba->leader && bb->leader && ba->leader->etat != ETAT_HORS_CADRE && bb->leader->etat != ETAT_HORS_CADRE)
         {
            double dx = ba->leader->x - bb->leader->x;
            double dy = ba->leader->y - bb->leader->y;
            double dist = sqrt(dx * dx + dy * dy);
            if (dist < 150.0)
            {
               gboolean predator_near = FALSE;
               for (GList *lo = all_poissons; lo; lo = lo->next)
               {
                  Poisson *other = lo->data;
                  if (is_predator(ui, other) && other->sante > 0.0 && other->etat != ETAT_HORS_CADRE)
                  {
                     double d_a = sqrt((other->x - ba->leader->x)*(other->x - ba->leader->x) + (other->y - ba->leader->y)*(other->y - ba->leader->y));
                     double d_b = sqrt((other->x - bb->leader->x)*(other->x - bb->leader->x) + (other->y - bb->leader->y)*(other->y - bb->leader->y));
                     if (d_a < ba->leader->perimetre_detection || d_b < bb->leader->perimetre_detection)
                     {
                        predator_near = TRUE;
                        break;
                     }
                  }
               }

               if (!predator_near)
               {
                  // Merge bb into ba
                  GList *lp = bb->poissons;
                  while (lp)
                  {
                     Poisson *p = lp->data;
                     p->id_banc = ba->id;
                     p->est_leader = FALSE;

                     if (p->widget_image)
                     {
                        GtkWidget *lbl_lead = gtk_widget_get_first_child(p->widget_image);
                        if (lbl_lead)
                           gtk_widget_set_visible(lbl_lead, FALSE);
                     }
                     ba->poissons = g_list_append(ba->poissons, p);
                     lp = lp->next;
                  }
                  g_list_free(bb->poissons);
                  free(bb->nom_espece);
                  ui->bancs = g_list_remove(ui->bancs, bb);
                  free(bb);

                  update_sidebar_list(ui);
                  break;
               }
            }
         }
      }
   }

   g_list_free(bancs_copy);
   g_list_free(all_poissons);
}

gboolean update_simulation(gpointer user_data)
{
   BassinUI *ui = user_data;
   if (!ui->simulation_running)
      return TRUE;

   double dt = 0.033 * ui->simulation_speed;
   ui->elapsed_time += dt;
   ui->time_since_last_alert += dt;

   tick_floating_labels(ui);

   if (ui->controlled_fish)
   {
      Poisson *p = ui->controlled_fish;
      Poisson *pred = find_nearest_predator(ui, p);
      gboolean in_danger = FALSE;

      if (pred)
      {
         double dx = (pred->x + pred->taille / 2.0) - (p->x + p->taille / 2.0);
         double dy = (pred->y + pred->taille / 2.0) - (p->y + p->taille / 2.0);
         double dist = sqrt(dx * dx + dy * dy);
         if (dist < p->perimetre_detection)
         {
            in_danger = TRUE;

            if (p->etat != ETAT_FUITE)
            {
               p->etat = ETAT_FUITE;
               double cur_speed = sqrt(p->vx * p->vx + p->vy * p->vy);
               if (cur_speed > 0)
               {
                  p->vx = (p->vx / cur_speed) * p->vitesse_fuite;
                  p->vy = (p->vy / cur_speed) * p->vitesse_fuite;
               }
               if (p->widget_image)
                  gtk_widget_add_css_class(p->widget_image, "fish-danger");
            }

            if (ui->time_since_last_alert >= 4.0)
            {
               sound_play(SOUND_SHARK_ALERT);
               ui->time_since_last_alert = 0.0;
            }
         }
      }

      if (!in_danger && p->etat == ETAT_FUITE)
      {
         p->etat = ETAT_NORMAL;
         if (p->widget_image)
            gtk_widget_remove_css_class(p->widget_image, "fish-danger");
      }
   }

   GList *dead_poissons = NULL;

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
      }
   }

   // 1. Gather all species movements & boids calculations
   GList *all_poissons = bassin_get_all_poissons(ui);
   for (GList *l = all_poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (p->etat == ETAT_HORS_CADRE)
      {
         continue;
      }
      if (p == ui->controlled_fish)
      {
         if (is_prey(ui, p))
         {
            gboolean hungry = (p->sante < p->sante_max) || ui->config_always_eat;
            if (hungry)
            {
               double p_cx = p->x + p->taille / 2.0;
               double p_cy = p->y + p->taille / 2.0;
               Food *eaten = NULL;

               for (GList *f_node = ui->foods; f_node; f_node = f_node->next)
               {
                  Food *f = f_node->data;
                  double f_cx = f->x + 12.0;
                  double f_cy = f->y + 12.0;
                  double dx = f_cx - p_cx;
                  double dy = f_cy - p_cy;
                  double dist = sqrt(dx * dx + dy * dy);

                  if (dist < (p->taille / 2.0 + 12.0))
                  {
                     eaten = f;
                     break;
                  }
               }

               if (eaten)
               {
                  double heal_amount = eaten->health_restore;
                  p->sante += heal_amount;
                  if (p->sante > p->sante_max) p->sante = p->sante_max;
                  spawn_floating_heal(ui, p->x + p->taille, p->y, heal_amount);
                  sound_play(SOUND_COIN);

                  if (eaten->widget) gtk_widget_unparent(eaten->widget);
                  ui->foods = g_list_remove(ui->foods, eaten);
                  g_free(eaten);
               }
            }
         }

         int lvl = get_fish_level(ui, p);
         if (lvl > 1)
         {
            Poisson *target = find_nearest_prey(ui, p);
            if (target)
            {
               double dx = (target->x + target->taille / 2.0) - (p->x + p->taille / 2.0);
               double dy = (target->y + target->taille / 2.0) - (p->y + p->taille / 2.0);
               double dist = sqrt(dx * dx + dy * dy);
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
         continue;
      }

      double force_x = 0;
      double force_y = 0;
      gboolean is_chasing = FALSE;
      gboolean has_evasion = FALSE;
      double evade_fx = 0.0;
      double evade_fy = 0.0;

      Poisson *pred = find_nearest_predator(ui, p);
      if (pred && (p->perimetre_detection > 0))
      {
         double dx = (pred->x + pred->taille / 2.0) - (p->x + p->taille / 2.0);
         double dy = (pred->y + pred->taille / 2.0) - (p->y + p->taille / 2.0);
         double dist = sqrt(dx * dx + dy * dy);
         if (dist < p->perimetre_detection)
         {
            p->etat = ETAT_FUITE;
            if (dist > 0)
            {
               if (p->id_banc >= 0)
               {
                  double ux = dx / dist;
                  double uy = dy / dist;
                  double ex = -ux;
                  double ey = -uy;

                  // Rotate by +135 degrees and -135 degrees
                  double cos135 = -0.70710678;
                  double sin135 = 0.70710678;

                  double rx1 = ex * cos135 - ey * sin135;
                  double ry1 = ex * sin135 + ey * cos135;

                  double rx2 = ex * cos135 - ey * (-sin135);
                  double ry2 = ex * (-sin135) + ey * cos135;

                  // Choose the one that aligns better with current velocity
                  double dot1 = rx1 * p->vx + ry1 * p->vy;
                  double dot2 = rx2 * p->vx + ry2 * p->vy;

                  double bypass_x = (dot1 >= dot2) ? rx1 : rx2;
                  double bypass_y = (dot1 >= dot2) ? ry1 : ry2;

                  evade_fx = bypass_x * p->vitesse_fuite;
                  evade_fy = bypass_y * p->vitesse_fuite;
               }
               else
               {
                  evade_fx = -(dx / dist) * p->vitesse_fuite;
                  evade_fy = -(dy / dist) * p->vitesse_fuite;
               }
               has_evasion = TRUE;
            }
         }
         else
         {
            if (p->etat == ETAT_FUITE)
               p->etat = ETAT_NORMAL;
         }
      }
      else
      {
         if (p->etat == ETAT_FUITE)
            p->etat = ETAT_NORMAL;
      }

      if (p->id_banc >= 0)
      {
         double coh_x = 0, coh_y = 0;
         double align_vx = 0, align_vy = 0;
         double sep_x = 0, sep_y = 0;
         int num_banc_members = 0;
         Poisson *leader = NULL;

         for (GList *o = all_poissons; o; o = o->next)
         {
            Poisson *other = o->data;
            if (other != p && other->id_banc == p->id_banc && other->etat != ETAT_HORS_CADRE)
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

         double speed_ref = (p->etat == ETAT_FUITE) ? p->vitesse_fuite : p->vitesse_normale;

         if (num_banc_members > 0)
         {
            coh_x /= num_banc_members;
            coh_y /= num_banc_members;
            align_vx /= num_banc_members;
            align_vy /= num_banc_members;

            double coh_fx = coh_x - p->x;
            double coh_fy = coh_y - p->y;
            double coh_d = sqrt(coh_fx * coh_fx + coh_fy * coh_fy);
            if (coh_d > 0)
            {
               coh_fx = (coh_fx / coh_d) * speed_ref;
               coh_fy = (coh_fy / coh_d) * speed_ref;
            }

            double align_d = sqrt(align_vx * align_vx + align_vy * align_vy);
            if (align_d > 0)
            {
               align_vx = (align_vx / align_d) * speed_ref;
               align_vy = (align_vy / align_d) * speed_ref;
            }

            double boids_x = coh_fx * 0.4 + align_vx * 0.3 + sep_x * 0.4 * speed_ref;
            double boids_y = coh_fy * 0.4 + align_vy * 0.3 + sep_y * 0.4 * speed_ref;

            if (p->est_leader)
            {
               // Add wandering steer force for the leader to guide the school dynamically
               double wander_angle = atan2(p->vy, p->vx);
               if ((rand() % 100) < 6) // 6% chance per frame to turn significantly
               {
                  double angle_change = ((double)(rand() % 100) - 50.0) * M_PI / 180.0; // +/- 50 degrees
                  wander_angle += angle_change;
               }
               else
               {
                  // Small continuous wiggle
                  double wiggle = ((double)(rand() % 20) - 10.0) * 0.1 * M_PI / 180.0;
                  wander_angle += wiggle;
               }
               // Add a significant steer weight to overcome alignment stabilization
               boids_x += cos(wander_angle) * speed_ref * 0.55;
               boids_y += sin(wander_angle) * speed_ref * 0.55;
            }

            if (leader && !p->est_leader)
            {
               double lead_dx = leader->x - p->x;
               double lead_dy = leader->y - p->y;
               double lead_d = sqrt(lead_dx * lead_dx + lead_dy * lead_dy);
               if (lead_d > 0)
               {
                  boids_x += (lead_dx / lead_d) * speed_ref * 0.3;
                  boids_y += (lead_dy / lead_d) * speed_ref * 0.3;
               }
            }

            if (has_evasion)
            {
               // Blend boids with evasion
               force_x = boids_x * 0.5 + evade_fx * 0.5;
               force_y = boids_y * 0.5 + evade_fy * 0.5;
            }
            else
            {
               force_x = boids_x;
               force_y = boids_y;
            }
         }
         else
         {
            if (has_evasion)
            {
               force_x = evade_fx;
               force_y = evade_fy;
            }
            else
            {
               force_x = p->vx;
               force_y = p->vy;
            }
         }
      }
      else
      {
         if (has_evasion)
         {
            force_x = evade_fx;
            force_y = evade_fy;
         }
         else
         {
            int lvl = get_fish_level(ui, p);
            Poisson *target = NULL;
            if (lvl > 1)
            {
               target = find_nearest_prey(ui, p);
            }

            if (target)
            {
               double dx = (target->x + target->taille / 2.0) - (p->x + p->taille / 2.0);
               double dy = (target->y + target->taille / 2.0) - (p->y + p->taille / 2.0);
               double dist = sqrt(dx * dx + dy * dy);
               if (dist > 0 && dist < p->perimetre_detection)
               {
                  is_chasing = TRUE;
                  force_x = (dx / dist) * p->vitesse_fuite;
                  force_y = (dy / dist) * p->vitesse_fuite;

                  if (dist < (p->taille + target->taille) * 0.5)
                  {
                     double damage_rate = 180.0;
                     target->sante -= damage_rate * dt;
                     target->temps_effet_attaque = 0.5;
                     target->dernier_attaquant = p;
                     target->degats_accumules += damage_rate * dt;
                  }
               }
               else if (dist > 0)
               {
                  force_x = (dx / dist) * p->vitesse_normale;
                  force_y = (dy / dist) * p->vitesse_normale;
               }
            }
            else
            {
               if (fabs(p->vx) < 5.0 && fabs(p->vy) < 5.0)
               {
                  double angle = (double)(rand() % 360) * M_PI / 180.0;
                  p->vx = cos(angle) * p->vitesse_normale;
                  p->vy = sin(angle) * p->vitesse_normale;
               }

               double cur_angle = atan2(p->vy, p->vx);
               if ((rand() % 100) < 5)
               {
                  double angle_change = ((double)(rand() % 120) - 60.0) * M_PI / 180.0;
                  double new_angle = cur_angle + angle_change;
                  force_x = cos(new_angle) * p->vitesse_normale;
                  force_y = sin(new_angle) * p->vitesse_normale;
               }
               else
               {
                  double wiggle = ((double)(rand() % 20) - 10.0) * 0.1 * M_PI / 180.0;
                  double new_angle = cur_angle + wiggle;
                  force_x = cos(new_angle) * p->vitesse_normale;
                  force_y = sin(new_angle) * p->vitesse_normale;
               }
            }
         }
      }

      if (p->etat == ETAT_FUITE || is_chasing)
      {
         p->vx = p->vx * 0.1 + force_x * 0.9;
         p->vy = p->vy * 0.1 + force_y * 0.9;
      }
      else
      {
         p->vx = p->vx * 0.85 + force_x * 0.15;
         p->vy = p->vy * 0.85 + force_y * 0.15;
      }

      double cur_speed = sqrt(p->vx * p->vx + p->vy * p->vy);
      double tar_speed = (p->etat == ETAT_FUITE || is_chasing) ? p->vitesse_fuite : p->vitesse_normale;
      if (cur_speed > 0)
      {
         p->vx = (p->vx / cur_speed) * tar_speed;
         p->vy = (p->vy / cur_speed) * tar_speed;
      }

      if (p->etat == ETAT_NORMAL && is_prey(ui, p))
      {
         gboolean hungry = (p->sante < p->sante_max) || ui->config_always_eat;
         if (hungry)
         {
            Food *nearest_food = NULL;
            double min_food_dist = 999999.0;
            double p_cx = p->x + p->taille / 2.0;
            double p_cy = p->y + p->taille / 2.0;

            for (GList *f_node = ui->foods; f_node; f_node = f_node->next)
            {
               Food *f = f_node->data;
               double f_cx = f->x + 12.0;
               double f_cy = f->y + 12.0;
               double dx = f_cx - p_cx;
               double dy = f_cy - p_cy;
               double dist = sqrt(dx * dx + dy * dy);
               if (dist < min_food_dist && dist < p->perimetre_detection)
               {
                  min_food_dist = dist;
                  nearest_food = f;
               }
            }

            if (nearest_food)
            {
               double f_cx = nearest_food->x + 12.0;
               double f_cy = nearest_food->y + 12.0;
               double dx = f_cx - p_cx;
               double dy = f_cy - p_cy;
               double dist = sqrt(dx * dx + dy * dy);

               if (dist > 0)
               {
                  p->vx = p->vx * 0.7 + (dx / dist) * p->vitesse_normale * 0.3;
                  p->vy = p->vy * 0.7 + (dy / dist) * p->vitesse_normale * 0.3;
               }

               if (dist < (p->taille / 2.0 + 12.0))
               {
                  double heal_amount = nearest_food->health_restore;
                  p->sante += heal_amount;
                  if (p->sante > p->sante_max) p->sante = p->sante_max;
                  spawn_floating_heal(ui, p->x + p->taille, p->y, heal_amount);
                  sound_play(SOUND_COIN);

                  if (nearest_food->widget) gtk_widget_unparent(nearest_food->widget);
                  ui->foods = g_list_remove(ui->foods, nearest_food);
                  g_free(nearest_food);
               }
            }
         }
      }
   }

   GList *food_node = ui->foods;
   while (food_node)
   {
      GList *next = food_node->next;
      Food *f = food_node->data;
      
      double floor_y = ui->config_canvas_height - 30;
      
      if (f->y < floor_y)
      {
         f->y += f->vy * dt;
         if (f->y > floor_y) f->y = floor_y;
         gtk_fixed_move(GTK_FIXED(ui->canvas), f->widget, (int)f->x, (int)f->y);
      }
      else
      {
         f->ticks_at_bottom++;
         if (f->ticks_at_bottom > 300) 
         {
            if (f->widget) gtk_widget_unparent(f->widget);
            ui->foods = g_list_remove(ui->foods, f);
            g_free(f);
         }
      }
      food_node = next;
   }

   // 2. Apply movement coordinates and check wrapping boundaries
   for (GList *l = all_poissons; l; l = l->next)
   {
      Poisson *p = l->data;

      if (p->temps_effet_attaque > 0.0)
      {
         p->temps_effet_attaque -= dt;
         if (p->temps_effet_attaque < 0.0)
            p->temps_effet_attaque = 0.0;
      }

      if (p->temps_dernier_floating_damage > 0.0)
      {
         p->temps_dernier_floating_damage -= dt;
         if (p->temps_dernier_floating_damage < 0.0)
            p->temps_dernier_floating_damage = 0.0;
      }

      if (p->temps_dernier_floating_damage <= 0.0 && p->degats_accumules >= 1.0)
      {
         spawn_floating_damage(ui, p->x + p->taille * 0.8, p->y + p->taille * 0.1, p->degats_accumules);
         p->degats_accumules = 0.0;
         p->temps_dernier_floating_damage = 0.35;
      }

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

      if (p->etat != ETAT_HORS_CADRE)
      {
         p->x += p->vx * dt;
         p->y += p->vy * dt;
      }

      // Coordinate relaxation pass for fish in the same shoal to prevent overlap
      if (p->id_banc >= 0 && p->etat != ETAT_HORS_CADRE)
      {
         for (GList *o = all_poissons; o; o = o->next)
         {
            Poisson *other = o->data;
            if (other != p && other->sante > 0.0 && other->id_banc == p->id_banc && other->etat != ETAT_HORS_CADRE)
            {
               double dx = p->x - other->x;
               double dy = p->y - other->y;

               // Toroidal distance correction
               if (ui->config_canvas_width > 50)
               {
                  if (dx > ui->config_canvas_width / 2.0) dx -= ui->config_canvas_width;
                  else if (dx < -ui->config_canvas_width / 2.0) dx += ui->config_canvas_width;
               }
               if (ui->config_canvas_height > 50)
               {
                  if (dy > ui->config_canvas_height / 2.0) dy -= ui->config_canvas_height;
                  else if (dy < -ui->config_canvas_height / 2.0) dy += ui->config_canvas_height;
               }

               double dist = sqrt(dx * dx + dy * dy);
               if (dist < 0.1)
               {
                  // Nudge randomly if exactly at the same coordinates
                  double angle = (double)(rand() % 360) * M_PI / 180.0;
                  p->x += cos(angle) * 3.0;
                  p->y += sin(angle) * 3.0;
                  dx = p->x - other->x;
                  dy = p->y - other->y;
                  if (ui->config_canvas_width > 50)
                  {
                     if (dx > ui->config_canvas_width / 2.0) dx -= ui->config_canvas_width;
                     else if (dx < -ui->config_canvas_width / 2.0) dx += ui->config_canvas_width;
                  }
                  if (ui->config_canvas_height > 50)
                  {
                     if (dy > ui->config_canvas_height / 2.0) dy -= ui->config_canvas_height;
                     else if (dy < -ui->config_canvas_height / 2.0) dy += ui->config_canvas_height;
                  }
                  dist = sqrt(dx * dx + dy * dy);
               }

               double min_sep = (p->taille + other->taille) * 0.25;
               if (min_sep < 20.0) min_sep = 20.0; // Enforce a solid floor so they don't overlap too much

               if (dist < min_sep)
               {
                  double overlap = min_sep - dist;
                  // Push p away from other
                  p->x += (dx / dist) * overlap * 0.5;
                  p->y += (dy / dist) * overlap * 0.5;

                  // Push other away from p
                  other->x -= (dx / dist) * overlap * 0.5;
                  other->y -= (dy / dist) * overlap * 0.5;
               }
            }
         }
      }

      // Off-screen boundary logic with 3 seconds delay
      if (p->etat == ETAT_HORS_CADRE)
      {
         p->temps_hors_cadre += dt;
         if (p->temps_hors_cadre >= 3.0)
         {
            // Teleport to the opposite side of the screen, just off-screen
            if (p->cote_sortie == 1) // exited Right -> re-appear just off-screen on the Left
            {
               p->x = -p->taille;
            }
            else if (p->cote_sortie == 2) // exited Left -> re-appear just off-screen on the Right
            {
               p->x = ui->config_canvas_width;
            }
            else if (p->cote_sortie == 3) // exited Bottom -> re-appear just off-screen on the Top
            {
               p->y = -p->taille;
            }
            else if (p->cote_sortie == 4) // exited Top -> re-appear just off-screen on the Bottom
            {
               p->y = ui->config_canvas_height;
            }

            p->etat = ETAT_NORMAL;
            p->temps_hors_cadre = 0.0;
            if (p->widget_image)
            {
               gtk_widget_set_visible(p->widget_image, TRUE);
            }
         }
      }
      else
      {
         // Check if the fish has exited the outer boundaries (fully off-screen)
         if (p->x > ui->config_canvas_width)
         {
            p->etat = ETAT_HORS_CADRE;
            p->temps_hors_cadre = 0.0;
            p->cote_sortie = 1; // Right
            if (p->widget_image)
            {
               gtk_widget_set_visible(p->widget_image, FALSE);
            }
         }
         else if (p->x < -p->taille)
         {
            p->etat = ETAT_HORS_CADRE;
            p->temps_hors_cadre = 0.0;
            p->cote_sortie = 2; // Left
            if (p->widget_image)
            {
               gtk_widget_set_visible(p->widget_image, FALSE);
            }
         }

         if (p->y > ui->config_canvas_height)
         {
            p->etat = ETAT_HORS_CADRE;
            p->temps_hors_cadre = 0.0;
            p->cote_sortie = 3; // Bottom
            if (p->widget_image)
            {
               gtk_widget_set_visible(p->widget_image, FALSE);
            }
         }
         else if (p->y < -p->taille)
         {
            p->etat = ETAT_HORS_CADRE;
            p->temps_hors_cadre = 0.0;
            p->cote_sortie = 4; // Top
            if (p->widget_image)
            {
               gtk_widget_set_visible(p->widget_image, FALSE);
            }
         }
      }

      if (p->widget_image)
      {
         if (p->chemin_frames[1] != NULL)
         {
            p->temps_depuis_frame += dt;
            if (p->temps_depuis_frame >= 0.25)
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

         int cached_frame = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(p->widget_image), "last_frame"));

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
            int last_size = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(img_widget), "last_size"));
            if (display_size != last_size)
            {
               gtk_widget_set_size_request(img_widget, display_size, display_size);
               g_object_set_data(G_OBJECT(img_widget), "last_size", GINT_TO_POINTER(display_size));
            }

            if (p->frame_courante != cached_frame)
            {
               const char *frame_path = p->chemin_frames[p->frame_courante]
                                        ? p->chemin_frames[p->frame_courante]
                                        : p->chemin_frames[0];
               gtk_picture_set_filename(GTK_PICTURE(img_widget), frame_path);
               g_object_set_data(G_OBJECT(p->widget_image), "last_frame", GINT_TO_POINTER(p->frame_courante));
            }

            gboolean flipped = (p->vx < 0.0);
            if (strcmp(p->nom, "Puffer") == 0)
            {
               flipped = !flipped;
            }

            double angle_deg = 0.0;
            if (flipped)
            {
               angle_deg = -atan2(-p->vy, -p->vx) * 180.0 / M_PI;
            }
            else
            {
               angle_deg = atan2(p->vy, p->vx) * 180.0 / M_PI;
            }
            int rounded_angle = (int)(round(angle_deg / 5.0) * 5);

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

   g_list_free(all_poissons);

   // 2.5. Update shoal split / merge behaviors dynamically
   bassin_tick_bancs_behavior(ui, dt);

   // 3. Clean up dead poissons
   if (dead_poissons)
   {
      for (GList *d = dead_poissons; d; d = d->next)
      {
         Poisson *p = d->data;
         eat_fish(ui, p);
      }
      g_list_free(dead_poissons);
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

   cairo_set_source_rgba(cr, 1.0, 0.65, 0.0, 0.8);
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

   GList *all_poissons = bassin_get_all_poissons(ui);
   for (GList *l = all_poissons; l; l = l->next)
   {
      Poisson *p = l->data;

      double cx = p->x + p->taille / 2.0;
      double cy = p->y + p->taille / 2.0;

      double radius = p->perimetre_detection;
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

      sprintf(info_text, "ID:%d [%s] HP:%.0f/%.0f (%.0f, %.0f) Banc:%d",
              p->id, state_str, p->sante, p->sante_max, p->x, p->y, p->id_banc);

      cairo_text_extents_t extents;
      cairo_text_extents(cr, info_text, &extents);

      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.55);
      cairo_rectangle(cr, cx - extents.width / 2.0 - 4, cy - p->taille / 2.0 - 18, extents.width + 8, extents.height + 6);
      cairo_fill(cr);

      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
      cairo_move_to(cr, cx - extents.width / 2.0, cy - p->taille / 2.0 - 8);
      cairo_show_text(cr, info_text);
   }
   g_list_free(all_poissons);
}
