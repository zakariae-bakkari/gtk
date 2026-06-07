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

static void on_prey_details_clicked(GtkButton *btn, gpointer user_data)
{
   BassinUI *ui = user_data;
   const char *species_name = g_object_get_data(G_OBJECT(btn), "species_name");
   if (ui && species_name)
   {
      open_species_details_dialog(ui, species_name);
   }
}

typedef struct {
   BassinUI *ui;
   char *species_name;
} SpeciesDetailsCtx;

static void on_species_details_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   SpeciesDetailsCtx *ctx = (SpeciesDetailsCtx *)user_data;
   if (ctx)
   {
      g_free(ctx->species_name);
      g_free(ctx);
   }
}

static void on_species_details_reponse(int reponse, gpointer user_data)
{
   SpeciesDetailsCtx *ctx = (SpeciesDetailsCtx *)user_data;
   if (!ctx)
      return;

   if (reponse == 150) // Modifier
   {
      open_create_species_dialog(ctx->ui, ctx->species_name);
   }
   else if (reponse == 151) // Supprimer
   {
      open_delete_species_confirmation(ctx->ui, ctx->species_name);
   }
}

void open_species_details_dialog(BassinUI *ui, const char *species_name)
{
   SpeciesConfig *cfg = find_species_config(ui, species_name);
   if (!cfg)
      return;

   Dialog *sd = g_new0(Dialog, 1);
   dialog_initialiser(sd);
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
   {
      sd->parent = GTK_WINDOW(toplevel);
   }

   char title_buf[128];
   sprintf(title_buf, "📚 Fiche Espèce : %s", cfg->nom);
   dialog_set_titre(sd, title_buf);

   SpeciesDetailsCtx *ctx = g_new0(SpeciesDetailsCtx, 1);
   ctx->ui = ui;
   ctx->species_name = g_strdup(species_name);

   sd->on_reponse = on_species_details_reponse;
   sd->user_data = ctx;

   dialog_ajouter_bouton(sd, "Modifier", "document-edit-symbolic", 150, TRUE);
   dialog_ajouter_bouton(sd, "Supprimer", "user-trash-symbolic", 151, FALSE);
   dialog_ajouter_bouton(sd, "Fermer", NULL, DIALOG_REPONSE_OK, FALSE);

   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   // Custom Image of the species
   Image *sp_img = g_new0(Image, 1);
   image_initialiser(sp_img);
   image_set_from_file(sp_img, cfg->chemin_frames[0] ? cfg->chemin_frames[0] : "resources/images/fish_blue.png");
   image_set_size(sp_img, 112, 112);
   image_set_fit_mode(sp_img, IMAGE_FIT_CONTAIN);
   image_set_halign(sp_img, WIDGET_ALIGN_CENTER);
   GtkWidget *w_sp_img = image_creer(sp_img);
   g_signal_connect(w_sp_img, "destroy", G_CALLBACK(on_image_widget_destroy), sp_img);
   gtk_box_append(GTK_BOX(box), w_sp_img);

   // Details info
   char info_buf[1024];
   const char *type_str = strcmp(cfg->type, "predator") == 0 ? "🦈 Prédateur" : (strcmp(cfg->type, "ally") == 0 ? "🐬 Allié" : "🐟 Proie");
   sprintf(info_buf,
           "<b>Nom :</b> %s\n"
           "<b>Type :</b> %s\n"
           "<b>Niveau :</b> %d\n"
           "<b>Taille par défaut :</b> %d px\n"
           "<b>Vitesse Normale :</b> %.1f px/s\n"
           "<b>Vitesse Fuite :</b> %.1f px/s\n"
           "<b>Rayon Détection :</b> %d px",
           cfg->nom,
           type_str,
           cfg->level,
           cfg->taille,
           cfg->vitesse_normale,
           cfg->vitesse_fuite,
           cfg->perimetre_detection);

   GtkWidget *lbl_info = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl_info), info_buf);
   gtk_label_set_justify(GTK_LABEL(lbl_info), GTK_JUSTIFY_LEFT);
   gtk_widget_set_halign(lbl_info, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_info);

   // Show diet for this species too!
   if (cfg->nb_diet > 0)
   {
      GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
      gtk_widget_set_margin_top(sep, 6);
      gtk_widget_set_margin_bottom(sep, 6);
      gtk_box_append(GTK_BOX(box), sep);

      GtkWidget *lbl_diet_title = gtk_label_new("<b>🍽️ Régime Alimentaire :</b>");
      gtk_label_set_use_markup(GTK_LABEL(lbl_diet_title), TRUE);
      gtk_widget_set_halign(lbl_diet_title, GTK_ALIGN_START);
      gtk_box_append(GTK_BOX(box), lbl_diet_title);

      for (int i = 0; i < cfg->nb_diet; i++)
      {
         char diet_buf[128];
         sprintf(diet_buf, "  • %s", cfg->diet[i]);
         GtkWidget *lbl_diet_item = gtk_label_new(diet_buf);
         gtk_widget_set_halign(lbl_diet_item, GTK_ALIGN_START);
         gtk_widget_set_margin_start(lbl_diet_item, 10);
         gtk_box_append(GTK_BOX(box), lbl_diet_item);
      }
   }

   GtkWidget *scroll = gtk_scrolled_window_new();
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
   gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroll), TRUE);
   gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroll), 480);
   gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), box);

   dialog_set_contenu(sd, scroll);
   dialog_creer(sd);
   dialog_afficher(sd);

   g_signal_connect_swapped(sd->window, "destroy", G_CALLBACK(dialog_free), sd);
   g_signal_connect(sd->window, "destroy", G_CALLBACK(on_species_details_destroy), ctx);
}

static void on_delete_species_confirmed(int reponse, gpointer user_data)
{
   SpeciesDetailsCtx *ctx = (SpeciesDetailsCtx *)user_data;
   if (!ctx)
      return;

   if (reponse == DIALOG_REPONSE_OUI)
   {
      BassinUI *ui = ctx->ui;
      const char *species_name = ctx->species_name;

      // 1. Find species config in ui->species_configs and remove it
      GList *cfg_link = NULL;
      for (GList *l = ui->species_configs; l; l = l->next)
      {
         SpeciesConfig *cfg = l->data;
         if (strcmp(cfg->nom, species_name) == 0)
         {
            cfg_link = l;
            break;
         }
      }

      if (cfg_link)
      {
         SpeciesConfig *cfg = cfg_link->data;
         
         // Free the sub-allocations of cfg
         g_free(cfg->nom);
         g_free(cfg->type);
         for (int i = 0; i < 3; i++)
         {
            if (cfg->chemin_frames[i])
               g_free(cfg->chemin_frames[i]);
         }
         for (int i = 0; i < cfg->nb_diet; i++)
         {
            if (cfg->diet[i])
               g_free(cfg->diet[i]);
         }
         g_free(cfg);

         ui->species_configs = g_list_delete_link(ui->species_configs, cfg_link);
      }

      // 2. Clean up from other species' diets
      for (GList *l = ui->species_configs; l; l = l->next)
      {
         SpeciesConfig *cfg = l->data;
         int new_nb_diet = 0;
         for (int i = 0; i < cfg->nb_diet; i++)
         {
            if (strcmp(cfg->diet[i], species_name) == 0)
            {
               g_free(cfg->diet[i]);
            }
            else
            {
               cfg->diet[new_nb_diet++] = cfg->diet[i];
            }
         }
         cfg->nb_diet = new_nb_diet;
      }

      // 3. Remove all active fish of this species
      GList *node = ui->poissons;
      while (node)
      {
         GList *next = node->next;
         Poisson *p = node->data;
         if (strcmp(p->nom, species_name) == 0)
         {
            // Remove image widget if it exists
            if (p->widget_image)
            {
               gtk_widget_unparent(p->widget_image);
            }
            poisson_free(p);
            ui->poissons = g_list_delete_link(ui->poissons, node);
         }
         node = next;
      }

      // 4. Save configs to XML
      save_species_configs_to_xml(ui);

      // 5. Update UI
      update_sidebar_list(ui);
      update_status_bar(ui);
   }

   // Free the context
   g_free(ctx->species_name);
   g_free(ctx);
}

void open_delete_species_confirmation(BassinUI *ui, const char *species_name)
{
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   char msg[256];
   snprintf(msg, sizeof(msg), "Voulez-vous vraiment supprimer l'espèce '%s' ?\nTous les poissons de cette espèce seront retirés du bassin.", species_name);
   
   SpeciesDetailsCtx *ctx = g_new0(SpeciesDetailsCtx, 1);
   ctx->ui = ui;
   ctx->species_name = g_strdup(species_name);

   dialog_afficher_confirmation(toplevel ? GTK_WINDOW(toplevel) : NULL,
                                "Confirmation de suppression",
                                msg,
                                on_delete_species_confirmed,
                                ctx);
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

static void on_control_fish_clicked(GtkButton *btn, gpointer user_data)
{
   (void)user_data;
   BassinUI *ui = g_object_get_data(G_OBJECT(btn), "ui");
   Poisson *p = g_object_get_data(G_OBJECT(btn), "poisson");
   Dialog *dialog = g_object_get_data(G_OBJECT(btn), "dialog");

   if (ui && p)
   {
      Poisson *old = ui->controlled_fish;
      ui->controlled_fish = p;

      // Reset velocities
      p->vx = 0.0;
      p->vy = 0.0;

      // Update tags
      if (old)
         update_fish_widget_tags(ui, old);
      update_fish_widget_tags(ui, p);

      // Play sound effect
      sound_play(SOUND_SPLASH);

      if (dialog)
      {
         dialog_fermer(dialog);
      }
   }
}

void show_fish_details_dialog(BassinUI *ui, Poisson *p)
{
   if (!ui || !p)
      return;

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

   // Main content box (vertical)
   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   // 1. Header Row (Horizontal Box): Image on left, details on right
   GtkWidget *header_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
   gtk_widget_set_halign(header_row, GTK_ALIGN_FILL);

   // Custom Image of the fish (frame1)
   Image *fish_img = g_new0(Image, 1);
   image_initialiser(fish_img);
   image_set_from_file(fish_img, p->chemin_frames[0] ? p->chemin_frames[0] : "resources/kenney_fish-pack_2.0/PNG/Default/fish_blue.png");
   image_set_size(fish_img, 96, 96);
   image_set_fit_mode(fish_img, IMAGE_FIT_CONTAIN);
   image_set_halign(fish_img, WIDGET_ALIGN_CENTER);
   GtkWidget *w_fish_img = image_creer(fish_img);
   g_signal_connect(w_fish_img, "destroy", G_CALLBACK(on_image_widget_destroy), fish_img);
   gtk_box_append(GTK_BOX(header_row), w_fish_img);

   // Right side info (vertical box)
   GtkWidget *header_text_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   gtk_widget_set_valign(header_text_box, GTK_ALIGN_CENTER);

   char *banc_str = p->id_banc >= 0 ? g_strdup_printf("Banc #%d", p->id_banc) : g_strdup("Aucun (Solo)");
   char *role_str = p->est_leader ? g_strdup("★ Leader du Banc") : (p->id_banc >= 0 ? g_strdup("Membre du Banc") : g_strdup("Autonome"));

   char header_lbl_buf[512];
   sprintf(header_lbl_buf,
           "<span size='large' weight='bold'>%s</span>\n"
           "<b>ID Unique :</b> #%d\n"
           "<b>Groupe :</b> %s\n"
           "<b>Rôle :</b> %s",
           p->nom, p->id, banc_str, role_str);
   g_free(banc_str);
   g_free(role_str);

   GtkWidget *lbl_header_text = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl_header_text), header_lbl_buf);
   gtk_widget_set_halign(lbl_header_text, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(header_text_box), lbl_header_text);

   gtk_box_append(GTK_BOX(header_row), header_text_box);
   gtk_box_append(GTK_BOX(box), header_row);

   // Separator
   GtkWidget *sep1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_box_append(GTK_BOX(box), sep1);

   // 2. Core Characteristics
   char char_lbl_buf[1024];
   sprintf(char_lbl_buf,
           "<b>Position :</b> X=%.1f, Y=%.1f\n"
           "<b>Vitesse courante :</b> Vx=%.1f, Vy=%.1f\n"
           "<b>Santé :</b> %.1f / %.1f\n\n"
           "<i>Paramètres de l'espèce :</i>\n"
           "• Vitesse Normale : %.1f px/s\n"
           "• Vitesse Fuite : %.1f px/s\n"
           "• Taille actuelle : %d px\n"
           "• Rayon de Détection : %d px",
           p->x, p->y,
           p->vx, p->vy,
           p->sante, p->sante_max,
           p->vitesse_normale,
           p->vitesse_fuite,
           p->taille,
           p->perimetre_detection);

   GtkWidget *lbl_char = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl_char), char_lbl_buf);
   gtk_widget_set_halign(lbl_char, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_char);

   // 3. Diet table
   SpeciesConfig *cfg = find_species_config(ui, p->nom);
   if (cfg && cfg->nb_diet > 0)
   {
      GtkWidget *sep2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
      gtk_widget_set_margin_top(sep2, 6);
      gtk_widget_set_margin_bottom(sep2, 6);
      gtk_box_append(GTK_BOX(box), sep2);

      GtkWidget *lbl_table_title = gtk_label_new("<b>🍽️ Régime Alimentaire &amp; Proies Dévorées :</b>");
      gtk_label_set_use_markup(GTK_LABEL(lbl_table_title), TRUE);
      gtk_widget_set_halign(lbl_table_title, GTK_ALIGN_START);
      gtk_box_append(GTK_BOX(box), lbl_table_title);

      GtkWidget *grid = gtk_grid_new();
      gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
      gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
      gtk_widget_set_margin_top(grid, 6);

      // Add table headers
      GtkWidget *h_img = gtk_label_new("<b>Proie</b>");
      gtk_label_set_use_markup(GTK_LABEL(h_img), TRUE);
      gtk_widget_set_halign(h_img, GTK_ALIGN_START);
      gtk_grid_attach(GTK_GRID(grid), h_img, 0, 0, 1, 1);

      GtkWidget *h_name = gtk_label_new("<b>Espèce</b>");
      gtk_label_set_use_markup(GTK_LABEL(h_name), TRUE);
      gtk_widget_set_halign(h_name, GTK_ALIGN_START);
      gtk_grid_attach(GTK_GRID(grid), h_name, 1, 0, 1, 1);

      GtkWidget *h_kills = gtk_label_new("<b>Captures</b>");
      gtk_label_set_use_markup(GTK_LABEL(h_kills), TRUE);
      gtk_widget_set_halign(h_kills, GTK_ALIGN_START);
      gtk_grid_attach(GTK_GRID(grid), h_kills, 2, 0, 1, 1);

      GtkWidget *h_actions = gtk_label_new("<b>Action</b>");
      gtk_label_set_use_markup(GTK_LABEL(h_actions), TRUE);
      gtk_widget_set_halign(h_actions, GTK_ALIGN_START);
      gtk_grid_attach(GTK_GRID(grid), h_actions, 3, 0, 1, 1);

      for (int i = 0; i < cfg->nb_diet; i++)
      {
         int r = i + 1; // row index
         const char *prey_name = cfg->diet[i];
         SpeciesConfig *prey_cfg = find_species_config(ui, prey_name);
         const char *prey_frame = (prey_cfg && prey_cfg->chemin_frames[0]) ? prey_cfg->chemin_frames[0] : "resources/kenney_fish-pack_2.0/PNG/Default/fish_blue.png";

         // Prey image (using the custom Image widget)
         Image *prey_img = g_new0(Image, 1);
         image_initialiser(prey_img);
         image_set_from_file(prey_img, prey_frame);
         image_set_size(prey_img, 32, 32);
         image_set_fit_mode(prey_img, IMAGE_FIT_CONTAIN);
         image_set_halign(prey_img, WIDGET_ALIGN_CENTER);
         GtkWidget *w_prey_img = image_creer(prey_img);
         g_signal_connect(w_prey_img, "destroy", G_CALLBACK(on_image_widget_destroy), prey_img);
         gtk_grid_attach(GTK_GRID(grid), w_prey_img, 0, r, 1, 1);

         // Prey name label
         GtkWidget *lbl_prey_name = gtk_label_new(prey_name);
         gtk_widget_set_halign(lbl_prey_name, GTK_ALIGN_START);
         gtk_grid_attach(GTK_GRID(grid), lbl_prey_name, 1, r, 1, 1);

         // Kills: small image + number of kills
         int kills = 0;
         for (int j = 0; j < p->nb_kills_types; j++)
         {
            if (strcmp(p->kills_espece[j], prey_name) == 0)
            {
               kills = p->kills_count[j];
               break;
            }
         }

         GtkWidget *kills_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
         gtk_widget_set_halign(kills_box, GTK_ALIGN_START);
         gtk_widget_set_valign(kills_box, GTK_ALIGN_CENTER);

         // Small image
         Image *small_prey_img = g_new0(Image, 1);
         image_initialiser(small_prey_img);
         image_set_from_file(small_prey_img, prey_frame);
         image_set_size(small_prey_img, 20, 20);
         small_prey_img->rayon_arrondi = 2;
         small_prey_img->fit_mode = IMAGE_FIT_CONTAIN;
         GtkWidget *w_small_img = image_creer(small_prey_img);
         g_signal_connect(w_small_img, "destroy", G_CALLBACK(on_image_widget_destroy), small_prey_img);
         gtk_box_append(GTK_BOX(kills_box), w_small_img);

         // Number of kills label
         char kill_lbl_buf[32];
         sprintf(kill_lbl_buf, "x %d", kills);
         GtkWidget *lbl_kills_count = gtk_label_new(kill_lbl_buf);
         gtk_box_append(GTK_BOX(kills_box), lbl_kills_count);

         gtk_grid_attach(GTK_GRID(grid), kills_box, 2, r, 1, 1);

         // Details button
         GtkWidget *btn_prey_detail = gtk_button_new_with_label("🔍 Détails");
         gtk_widget_add_css_class(btn_prey_detail, "secondaire");
         g_object_set_data(G_OBJECT(btn_prey_detail), "species_name", (gpointer)prey_name);
         g_signal_connect(btn_prey_detail, "clicked", G_CALLBACK(on_prey_details_clicked), ui);
         gtk_grid_attach(GTK_GRID(grid), btn_prey_detail, 3, r, 1, 1);
      }

      gtk_box_append(GTK_BOX(box), grid);
   }

   // Control button
   GtkWidget *btn_control = gtk_button_new_with_label("🕹️ Prendre le contrôle");
   gtk_widget_add_css_class(btn_control, "suggested-action");
   gtk_widget_set_margin_top(btn_control, 10);
   g_object_set_data(G_OBJECT(btn_control), "ui", ui);
   g_object_set_data(G_OBJECT(btn_control), "poisson", p);
   g_object_set_data(G_OBJECT(btn_control), "dialog", details);
   g_signal_connect(btn_control, "clicked", G_CALLBACK(on_control_fish_clicked), NULL);
   gtk_box_append(GTK_BOX(box), btn_control);

   GtkWidget *scroll = gtk_scrolled_window_new();
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
   gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroll), TRUE);
   gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroll), 480);
   gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), box);

   dialog_set_contenu(details, scroll);
   dialog_creer(details);
   dialog_afficher(details);

   g_signal_connect_swapped(details->window, "destroy", G_CALLBACK(dialog_free), details);
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

void show_shortcuts_help_dialog(BassinUI *ui)
{
   if (!ui) return;

   Dialog *help = g_new0(Dialog, 1);
   dialog_initialiser(help);
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
   {
      help->parent = GTK_WINDOW(toplevel);
   }

   dialog_set_titre(help, "⌨️ Raccourcis Clavier");
   help->boutons_preset = DIALOG_BOUTONS_OK;

   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
   gtk_widget_set_margin_start(box, 20);
   gtk_widget_set_margin_end(box, 20);
   gtk_widget_set_margin_top(box, 20);
   gtk_widget_set_margin_bottom(box, 20);

   GtkWidget *lbl_intro = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl_intro), "<span size='large' weight='bold'>Liste des raccourcis disponibles</span>");
   gtk_box_append(GTK_BOX(box), lbl_intro);

   GtkWidget *grid = gtk_grid_new();
   gtk_grid_set_column_spacing(GTK_GRID(grid), 30);
   gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
   gtk_widget_set_halign(grid, GTK_ALIGN_CENTER);

   char play_str[128];
   char zen_str[128];
   char debug_str[128];
   char settings_str[128];
   char add_str[128];
   char sidebar_str[128];
   char restart_str[128];

   snprintf(play_str, sizeof(play_str), "<b>%s</b>", ui->shortcut_play ? ui->shortcut_play : "Non configuré");
   snprintf(zen_str, sizeof(zen_str), "<b>%s</b>", ui->shortcut_zen ? ui->shortcut_zen : "Non configuré");
   snprintf(debug_str, sizeof(debug_str), "<b>%s</b>", ui->shortcut_debug ? ui->shortcut_debug : "Non configuré");
   snprintf(settings_str, sizeof(settings_str), "<b>%s</b>", ui->shortcut_settings ? ui->shortcut_settings : "Non configuré");
   snprintf(add_str, sizeof(add_str), "<b>%s</b>", ui->shortcut_add ? ui->shortcut_add : "Non configuré");
   snprintf(sidebar_str, sizeof(sidebar_str), "<b>%s</b>", ui->shortcut_sidebar ? ui->shortcut_sidebar : "Non configuré");
   snprintf(restart_str, sizeof(restart_str), "<b>%s</b>", ui->shortcut_restart ? ui->shortcut_restart : "Non configuré");

   const char *shortcuts[][2] = {
      {"<b>F1</b>",             "Afficher cette aide"},
      {"<b>F11</b> / <b>Ctrl+F</b>", "Plein écran"},
      {zen_str,                 "Mode Zen (Masquer l'UI)"},
      {debug_str,               "Mode Débogage"},
      {settings_str,             "Paramètres"},
      {add_str,                 "Ajouter un poisson"},
      {play_str,                 "Lecture / Pause"},
      {sidebar_str,             "Afficher/Masquer la barre latérale"},
      {restart_str,             "Réinitialiser la simulation"},
      {NULL, NULL}
   };

   for (int i = 0; shortcuts[i][0] != NULL; i++)
   {
      GtkWidget *lbl_key = gtk_label_new(NULL);
      gtk_label_set_markup(GTK_LABEL(lbl_key), shortcuts[i][0]);
      gtk_widget_set_halign(lbl_key, GTK_ALIGN_END);

      GtkWidget *lbl_desc = gtk_label_new(shortcuts[i][1]);
      gtk_widget_set_halign(lbl_desc, GTK_ALIGN_START);

      gtk_grid_attach(GTK_GRID(grid), lbl_key, 0, i, 1, 1);
      gtk_grid_attach(GTK_GRID(grid), lbl_desc, 1, i, 1, 1);
   }

   gtk_box_append(GTK_BOX(box), grid);

   dialog_set_contenu(help, box);
   dialog_creer(help);
   dialog_afficher(help);

   g_signal_connect_swapped(help->window, "destroy", G_CALLBACK(dialog_free), help);
}

// Controller key bindings and destroy callbacks
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

static void on_key_released(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data)
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

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data)
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
   if (keyval == GDK_KEY_F11 || ((keyval == GDK_KEY_f || keyval == GDK_KEY_F) && ctrl))
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

   // Restart
   if (match_shortcut(ui->shortcut_restart, keyval, state))
   {
      on_restart_clicked(NULL, ui);
      return TRUE;
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
   ui->config_bg_path = strdup("resources/images/background_banc.png");
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

   // Setup key controller for debug mode shortcut (Ctrl+D)
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
