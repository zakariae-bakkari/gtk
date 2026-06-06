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
   sd->boutons_preset = DIALOG_BOUTONS_OK;

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

      GtkWidget *lbl_table_title = gtk_label_new("<b>🍽️ Régime Alimentaire & Proies Dévorées :</b>");
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
   ui->config_fish_size = 0;
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
