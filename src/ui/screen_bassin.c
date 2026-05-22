#include "screen_bassin.h"
#include "../modele/poisson.h"
#include "../sound.h"

// Framework Widgets
#include "../widgets/headers/dialog.h"
#include "../widgets/headers/bouton_radio.h"
#include "../widgets/headers/champ_select.h"
#include "../widgets/headers/slider.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Custom CSS Data
static const char *css_data = 
   ".entity-header { font-weight: bold; font-size: 13px; color: #7F8C8D; margin-top: 15px; margin-bottom: 8px; text-transform: uppercase; letter-spacing: 0.8px; }\n"
   ".entity-item { padding: 8px 12px; background-color: #Fdfdfd; border: 1px solid #EAEAEA; border-radius: 8px; margin-bottom: 6px; box-shadow: 0 1px 3px rgba(0,0,0,0.02); }\n"
   ".badge-banc { background-color: #EBF5FB; color: #2980B9; font-size: 10px; font-weight: bold; padding: 2px 6px; border-radius: 4px; }\n"
   ".badge-solo { background-color: #F2F4F4; color: #7F8C8D; font-size: 10px; font-weight: bold; padding: 2px 6px; border-radius: 4px; }\n"
   ".badge-pred { background-color: #FDEDEC; color: #C0392B; font-size: 10px; font-weight: bold; padding: 2px 6px; border-radius: 4px; }\n"
   ".fish-tag { background-color: rgba(0, 0, 0, 0.6); color: white; font-size: 10px; padding: 2px 6px; border-radius: 4px; font-weight: bold; }\n"
   ".fish-tag-pred { background-color: rgba(192, 57, 43, 0.85); color: white; font-weight: bold; }\n"
   ".leader-tag { color: #F1C40F; font-size: 9px; font-weight: bold; text-shadow: 1px 1px 1px black; }\n"
   ".sidebar-container { border-left: 1px solid #E0E0E0; background-color: #FAFAFA; padding: 15px; }\n"
   ".tab-active { background-color: #3498DB; color: white; font-weight: bold; }\n"
   ".tab-inactive { background-color: #ECEFF1; color: #37474F; }\n"
   ".dialog-frame { background-color: #F9F9FB; border: 1px solid #E0E0E6; border-radius: 8px; padding: 12px; margin-top: 10px; }\n";

// Species Sprites Mapping
static const char *species_sprites[] = {
   "resources/kenney_fish-pack_2.0/PNG/Default/fish_blue.png",      // Hareng
   "resources/kenney_fish-pack_2.0/PNG/Default/fish_orange.png",    // Poisson-globe
   "resources/kenney_fish-pack_2.0/PNG/Default/fish_green.png",     // Poisson clown
   "resources/kenney_fish-pack_2.0/PNG/Default/fish_grey_long_a.png", // Requin
   "resources/kenney_fish-pack_2.0/PNG/Default/fish_grey_long_b.png"  // Dauphin
};

typedef struct
{
   GtkWidget *root;
   GtkWidget *canvas;
   GList *poissons; /* list of Poisson* */
   int next_id;

   // Simulation control
   gboolean simulation_running;
   int num_bancs;
   guint timer_id;

   // Sidebar elements
   GtkWidget *btn_tab_entites;
   GtkWidget *btn_tab_bancs;
   GtkWidget *scrolled_sidebar_list;
   GtkWidget *box_sidebar_content;
   int active_tab; // 0 = Entités, 1 = Bancs

   // Status bar labels
   GtkWidget *lbl_status_indicator;
   GtkWidget *lbl_stats_text;
   GtkWidget *lbl_elapsed_time;
   double elapsed_time;

   // Dialog reference (if open)
   Dialog active_dialog;
   char *dialog_selected_species;
   GtkWidget *dialog_school_frame;
   BoutonRadio dialog_radio_solo;
   BoutonRadio dialog_radio_banc;
   ChampSelect dialog_sel_banc;
   Slider dialog_sld_taille;

   // Configuration fields
   int config_fish_size;
   char *config_bg_path;
   int config_canvas_width;
   int config_canvas_height;
   GtkWidget *bg_widget;

   Dialog settings_dialog;
   Slider settings_sld_fish_size;
   ChampSelect settings_sel_bg;
   ChampSelect settings_sel_canvas;

} BassinUI;

// Prototypes
static void on_add_poisson_btn_clicked(GtkButton *btn, gpointer user_data);
static void on_remove_poisson_btn_clicked(GtkButton *btn, gpointer user_data);
static void on_play_pause_clicked(GtkButton *btn, gpointer user_data);
static void on_restart_clicked(GtkButton *btn, gpointer user_data);
static void on_settings_clicked(GtkButton *btn, gpointer user_data);
static void on_settings_reponse(int reponse, gpointer user_data);
static void on_tab_entites_clicked(GtkButton *btn, gpointer user_data);
static void on_tab_bancs_clicked(GtkButton *btn, gpointer user_data);
static void update_sidebar_list(BassinUI *ui);
static void update_status_bar(BassinUI *ui);
static void open_add_dialog(BassinUI *ui, const char *species);
static void add_fish_programmatic(BassinUI *ui, const char *species, gboolean in_banc, int target_banc_id);
static gboolean update_simulation(gpointer user_data);

// Safe cleanup for entities
static void eat_fish(BassinUI *ui, Poisson *prey)
{
   if (!prey) return;
   if (prey->widget_image)
   {
      gtk_widget_unparent(prey->widget_image);
      prey->widget_image = NULL;
   }
   ui->poissons = g_list_remove(ui->poissons, prey);
   poisson_free(prey);
}

// Find closest prey
static Poisson *find_nearest_prey(BassinUI *ui, Poisson *predator)
{
   Poisson *closest = NULL;
   double min_dist = 999999.0;
   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (p != predator && strcmp(p->nom, "Requin") != 0 && strcmp(p->nom, "Dauphin") != 0)
      {
         double dx = p->x - predator->x;
         double dy = p->y - predator->y;
         double dist = sqrt(dx*dx + dy*dy);
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
static Poisson *find_nearest_requin(BassinUI *ui, Poisson *fish)
{
   Poisson *closest = NULL;
   double min_dist = 999999.0;
   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (strcmp(p->nom, "Requin") == 0)
      {
         double dx = p->x - fish->x;
         double dy = p->y - fish->y;
         double dist = sqrt(dx*dx + dy*dy);
         if (dist < min_dist)
         {
            min_dist = dist;
            closest = p;
         }
      }
   }
   return closest;
}

// Dialog Mode Insertion toggle
static void on_insertion_mode_changed(GtkCheckButton *widget, gpointer user_data)
{
   BassinUI *ui = user_data;
   gboolean is_banc = bouton_radio_est_actif(&ui->dialog_radio_banc);
   gtk_widget_set_sensitive(ui->dialog_school_frame, is_banc);
}

// Add Dialog Confirm Response Callback
static void on_dialog_reponse(int reponse, gpointer user_data)
{
   BassinUI *ui = user_data;
   if (reponse == DIALOG_REPONSE_OK)
   {
      gboolean in_banc = bouton_radio_est_actif(&ui->dialog_radio_banc);
      int target_banc_id = -1; // New school

      if (in_banc)
      {
         int idx = champ_select_get_index(&ui->dialog_sel_banc);
         // Dropdown values: 0 = "Creer un nouveau banc...", >=1 = existing schools
         if (idx > 0)
         {
            // Find existing school ID by scanning active fish schools
            int current_idx = 1;
            for (int b_id = 1; b_id <= ui->num_bancs; b_id++)
            {
               gboolean found = FALSE;
               for (GList *l = ui->poissons; l; l = l->next)
               {
                  Poisson *p = l->data;
                  if (p->id_banc == b_id)
                  {
                     found = TRUE;
                     break;
                  }
               }
               if (found)
               {
                  if (current_idx == idx)
                  {
                     target_banc_id = b_id;
                     break;
                  }
                  current_idx++;
               }
            }
         }
      }

      int count = in_banc ? (int)slider_get_valeur(&ui->dialog_sld_taille) : 1;
      if (in_banc && target_banc_id == -1)
      {
         // Create a brand new school
         ui->num_bancs++;
         target_banc_id = ui->num_bancs;
         
         // The first added fish is marked as the leader!
         add_fish_programmatic(ui, ui->dialog_selected_species, TRUE, target_banc_id);
         count--;
      }

      // Add remaining members
      for (int i = 0; i < count; i++)
      {
         add_fish_programmatic(ui, ui->dialog_selected_species, in_banc, target_banc_id);
      }

      update_sidebar_list(ui);
      update_status_bar(ui);
   }

   dialog_fermer(&ui->active_dialog);
}

// Species Selected from popup list
static void on_species_selected(GtkButton *btn, gpointer user_data)
{
   BassinUI *ui = g_object_get_data(G_OBJECT(btn), "ui");
   const char *species = g_object_get_data(G_OBJECT(btn), "species");
   
   // Dismiss the popover menu
   GtkWidget *popover = gtk_widget_get_ancestor(GTK_WIDGET(btn), GTK_TYPE_POPOVER);
   if (popover)
   {
      gtk_popover_popdown(GTK_POPOVER(popover));
   }

   // Requin and Dauphin can ONLY be added individually
   if (strcmp(species, "Requin") == 0 || strcmp(species, "Dauphin") == 0)
   {
      add_fish_programmatic(ui, species, FALSE, -1);
      update_sidebar_list(ui);
      update_status_bar(ui);
   }
   else
   {
      open_add_dialog(ui, species);
   }
}

// Custom Dialog Setup
static void open_add_dialog(BassinUI *ui, const char *species)
{
   if (ui->dialog_selected_species) free(ui->dialog_selected_species);
   ui->dialog_selected_species = strdup(species);

   dialog_initialiser(&ui->active_dialog);
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
   {
      ui->active_dialog.parent = GTK_WINDOW(toplevel);
   }
   
   char title_buf[128];
   const char *emoji = strcmp(species, "Hareng") == 0 ? "🐟" : (strcmp(species, "Poisson-globe") == 0 ? "🐡" : "🐠");
   sprintf(title_buf, "Ajouter %s %s", emoji, species);
   dialog_set_titre(&ui->active_dialog, title_buf);

   ui->active_dialog.boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
   ui->active_dialog.on_reponse = on_dialog_reponse;
   ui->active_dialog.user_data = ui;

   // Main layout box for dialog content
   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   GtkWidget *lbl_mode = gtk_label_new("Choisir le mode d'insertion");
   gtk_widget_set_halign(lbl_mode, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_mode);

   // Radio Buttons for Solo or School
   bouton_radio_initialiser(&ui->dialog_radio_solo);
   ui->dialog_radio_solo.label = "Individuel\nUn seul poisson, autonome";
   ui->dialog_radio_solo.est_actif = TRUE;
   ui->dialog_radio_solo.on_toggled = on_insertion_mode_changed;
   ui->dialog_radio_solo.user_data = ui;
   GtkWidget *w_solo = bouton_radio_creer(&ui->dialog_radio_solo);
   gtk_box_append(GTK_BOX(box), w_solo);

   bouton_radio_initialiser(&ui->dialog_radio_banc);
   ui->dialog_radio_banc.label = "Banc (groupe)\nAjouter à un banc existant ou créer";
   ui->dialog_radio_banc.est_actif = FALSE;
   ui->dialog_radio_banc.on_toggled = on_insertion_mode_changed;
   ui->dialog_radio_banc.user_data = ui;
   GtkWidget *w_banc = bouton_radio_creer(&ui->dialog_radio_banc);
   bouton_radio_set_groupe(&ui->dialog_radio_banc, GTK_CHECK_BUTTON(w_solo));
   gtk_box_append(GTK_BOX(box), w_banc);

   // School Config Frame
   ui->dialog_school_frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
   gtk_widget_add_css_class(ui->dialog_school_frame, "dialog-frame");
   
   GtkWidget *lbl_target = gtk_label_new("Banc cible");
   gtk_widget_set_halign(lbl_target, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(ui->dialog_school_frame), lbl_target);

   // Populate dropdown with active schools
   champ_select_initialiser(&ui->dialog_sel_banc);
   champ_select_add_item(&ui->dialog_sel_banc, "Créer un nouveau banc...");
   
   // Find existing school names
   for (int b_id = 1; b_id <= ui->num_bancs; b_id++)
   {
      int count = 0;
      char sp_name[64] = "poissons";
      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         if (p->id_banc == b_id)
         {
            count++;
            strcpy(sp_name, p->nom);
         }
      }
      if (count > 0)
      {
         char name_buf[128];
         sprintf(name_buf, "Banc %ss (%d membres)", sp_name, count);
         champ_select_add_item(&ui->dialog_sel_banc, name_buf);
      }
   }

   GtkWidget *w_sel_banc = champ_select_creer(&ui->dialog_sel_banc);
   gtk_box_append(GTK_BOX(ui->dialog_school_frame), w_sel_banc);

   GtkWidget *lbl_size = gtk_label_new("Taille initiale du banc");
   gtk_widget_set_halign(lbl_size, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(ui->dialog_school_frame), lbl_size);

   // Slider for School Size
   slider_initialiser(&ui->dialog_sld_taille);
   slider_set_bornes(&ui->dialog_sld_taille, 2, 20);
   slider_set_valeur(&ui->dialog_sld_taille, 4);
   slider_set_digits(&ui->dialog_sld_taille, 0);
   slider_set_afficher_valeur(&ui->dialog_sld_taille, TRUE);
   GtkWidget *w_sld_taille = slider_creer(&ui->dialog_sld_taille);
   gtk_box_append(GTK_BOX(ui->dialog_school_frame), w_sld_taille);

   gtk_box_append(GTK_BOX(box), ui->dialog_school_frame);

   // Make school config initially insensitive (since "Individuel" is default selected)
   gtk_widget_set_sensitive(ui->dialog_school_frame, FALSE);

   dialog_set_contenu(&ui->active_dialog, box);
   dialog_creer(&ui->active_dialog);
   dialog_afficher(&ui->active_dialog);
}

// Add fish with species and school info
static void add_fish_programmatic(BassinUI *ui, const char *species, gboolean in_banc, int target_banc_id)
{
   Poisson *p = poisson_new(species);
   p->id = ui->next_id++;
   p->taille = ui->config_fish_size;

   // Select Sprite Path
   const char *sprite = species_sprites[0]; // default
   if (strcmp(species, "Poisson-globe") == 0) sprite = species_sprites[1];
   else if (strcmp(species, "Poisson clown") == 0) sprite = species_sprites[2];
   else if (strcmp(species, "Requin") == 0) sprite = species_sprites[3];
   else if (strcmp(species, "Dauphin") == 0) sprite = species_sprites[4];
   
   poisson_set_default_frames(p, sprite, NULL, NULL);

   // Set Flocking vs Solo vs Predator roles
   if (strcmp(species, "Requin") == 0)
   {
      p->vitesse_normale = 60.0;
      p->vitesse_fuite = 90.0;
      p->id_banc = -1;
      p->est_leader = FALSE;
   }
   else if (strcmp(species, "Dauphin") == 0)
   {
      p->vitesse_normale = 70.0;
      p->vitesse_fuite = 100.0;
      p->id_banc = -1;
      p->est_leader = FALSE;
   }
   else
   {
      p->vitesse_normale = 50.0;
      p->vitesse_fuite = 80.0;
      p->id_banc = in_banc ? target_banc_id : -1;
      
      // Determine leader
      if (in_banc)
      {
         // Check if this school already has a leader
         gboolean has_leader = FALSE;
         for (GList *l = ui->poissons; l; l = l->next)
         {
            Poisson *other = l->data;
            if (other->id_banc == target_banc_id && other->est_leader)
            {
               has_leader = TRUE;
               break;
            }
         }
         p->est_leader = !has_leader;
      }
      else
      {
         p->est_leader = FALSE;
      }
   }

   // Random position relative to configurable canvas size
   double rx = 100 + (rand() % (ui->config_canvas_width - 200));
   double ry = 100 + (rand() % (ui->config_canvas_height - 200));
   poisson_set_position(p, rx, ry);

   // Random velocities
   double angle_rad = (rand() % 360) * M_PI / 180.0;
   p->vx = cos(angle_rad) * p->vitesse_normale;
   p->vy = sin(angle_rad) * p->vitesse_normale;

   // Visual Representation Box: [Leader tag] -> [Image] -> [Name tag]
   GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
   
   // Leader Tag
   GtkWidget *lbl_lead = gtk_label_new("★ Leader");
   gtk_widget_add_css_class(lbl_lead, "leader-tag");
   gtk_widget_set_visible(lbl_lead, p->est_leader);
   gtk_box_append(GTK_BOX(container), lbl_lead);

   // Sprite (using GtkPicture for premium and accurate auto-scaling to p->taille)
   GtkWidget *img = gtk_picture_new_for_filename(p->chemin_frames[0]);
   gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(img), TRUE);
   gtk_widget_set_size_request(img, p->taille, p->taille);
   gtk_box_append(GTK_BOX(container), img);

   // Tag Label
   char tag_buf[64];
   if (strcmp(p->nom, "Requin") == 0)
   {
      sprintf(tag_buf, "Requin Alpha");
   }
   else if (strcmp(p->nom, "Dauphin") == 0)
   {
      sprintf(tag_buf, "Dauphin");
   }
   else
   {
      const char *short_nom = strcmp(p->nom, "Poisson clown") == 0 ? "Clown" : (strcmp(p->nom, "Poisson-globe") == 0 ? "Globe" : "Hareng");
      sprintf(tag_buf, "%s #%d", short_nom, p->id);
   }

   GtkWidget *lbl_tag = gtk_label_new(tag_buf);
   gtk_widget_add_css_class(lbl_tag, "fish-tag");
   if (strcmp(p->nom, "Requin") == 0)
   {
      gtk_widget_add_css_class(lbl_tag, "fish-tag-pred");
   }
   gtk_box_append(GTK_BOX(container), lbl_tag);

   poisson_set_widget(p, container);
   gtk_fixed_put(GTK_FIXED(ui->canvas), container, (int)p->x, (int)p->y);

   ui->poissons = g_list_prepend(ui->poissons, p);
}

// Tab Switching
static void on_tab_entites_clicked(GtkButton *btn, gpointer user_data)
{
   BassinUI *ui = user_data;
   ui->active_tab = 0;
   gtk_widget_add_css_class(ui->btn_tab_entites, "tab-active");
   gtk_widget_remove_css_class(ui->btn_tab_entites, "tab-inactive");
   gtk_widget_add_css_class(ui->btn_tab_bancs, "tab-inactive");
   gtk_widget_remove_css_class(ui->btn_tab_bancs, "tab-active");

   update_sidebar_list(ui);
}

static void on_tab_bancs_clicked(GtkButton *btn, gpointer user_data)
{
   BassinUI *ui = user_data;
   ui->active_tab = 1;
   gtk_widget_add_css_class(ui->btn_tab_bancs, "tab-active");
   gtk_widget_remove_css_class(ui->btn_tab_bancs, "tab-inactive");
   gtk_widget_add_css_class(ui->btn_tab_entites, "tab-inactive");
   gtk_widget_remove_css_class(ui->btn_tab_entites, "tab-active");

   update_sidebar_list(ui);
}

// Update the right lateral sidebar contents
static void update_sidebar_list(BassinUI *ui)
{
   // Clear old sidebar items
   GtkWidget *child = gtk_widget_get_first_child(ui->box_sidebar_content);
   while (child)
   {
      GtkWidget *next = gtk_widget_get_next_sibling(child);
      gtk_box_remove(GTK_BOX(ui->box_sidebar_content), child);
      child = next;
   }

   if (ui->active_tab == 0) // Tab: Entités
   {
      // 1. Group by schools
      for (int b_id = 1; b_id <= ui->num_bancs; b_id++)
      {
         GList *members = NULL;
         char species_nom[64] = "";
         for (GList *l = ui->poissons; l; l = l->next)
         {
            Poisson *p = l->data;
            if (p->id_banc == b_id)
            {
               members = g_list_append(members, p);
               strcpy(species_nom, p->nom);
            }
         }

         if (members)
         {
            char header_buf[128];
            const char *emoji = strcmp(species_nom, "Hareng") == 0 ? "🐟" : (strcmp(species_nom, "Poisson-globe") == 0 ? "🐡" : "🐠");
            sprintf(header_buf, "BANC %sS", species_nom);
            
            GtkWidget *lbl_header = gtk_label_new(header_buf);
            gtk_widget_set_halign(lbl_header, GTK_ALIGN_START);
            gtk_widget_add_css_class(lbl_header, "entity-header");
            gtk_box_append(GTK_BOX(ui->box_sidebar_content), lbl_header);

            for (GList *m = members; m; m = m->next)
            {
               Poisson *p = m->data;
               GtkWidget *item = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
               gtk_widget_add_css_class(item, "entity-item");

               char name_buf[128];
               const char *short_nom = strcmp(p->nom, "Poisson clown") == 0 ? "Clown" : (strcmp(p->nom, "Poisson-globe") == 0 ? "Globe" : "Hareng");
               sprintf(name_buf, "%s %s #%d", emoji, short_nom, p->id);
               
               GtkWidget *lbl_name = gtk_label_new(name_buf);
               gtk_box_append(GTK_BOX(item), lbl_name);

               GtkWidget *lbl_role = gtk_label_new(p->est_leader ? "Leader du banc" : "Membre");
               gtk_widget_set_halign(lbl_role, GTK_ALIGN_END);
               gtk_widget_set_hexpand(lbl_role, TRUE);
               gtk_box_append(GTK_BOX(item), lbl_role);

               GtkWidget *lbl_badge = gtk_label_new("banc");
               gtk_widget_add_css_class(lbl_badge, "badge-banc");
               gtk_box_append(GTK_BOX(item), lbl_badge);

               gtk_box_append(GTK_BOX(ui->box_sidebar_content), item);
            }
            g_list_free(members);
         }
      }

      // 2. Individuals
      gboolean has_indiv_header = FALSE;
      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         if (p->id_banc == -1 && strcmp(p->nom, "Requin") != 0 && strcmp(p->nom, "Dauphin") != 0)
         {
            if (!has_indiv_header)
            {
               GtkWidget *lbl_header = gtk_label_new("INDIVIDUEL");
               gtk_widget_set_halign(lbl_header, GTK_ALIGN_START);
               gtk_widget_add_css_class(lbl_header, "entity-header");
               gtk_box_append(GTK_BOX(ui->box_sidebar_content), lbl_header);
               has_indiv_header = TRUE;
            }

            GtkWidget *item = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            gtk_widget_add_css_class(item, "entity-item");

            const char *emoji = strcmp(p->nom, "Hareng") == 0 ? "🐟" : (strcmp(p->nom, "Poisson-globe") == 0 ? "🐡" : "🐠");
            char name_buf[128];
            const char *short_nom = strcmp(p->nom, "Poisson clown") == 0 ? "Clown" : (strcmp(p->nom, "Poisson-globe") == 0 ? "Globe" : "Hareng");
            sprintf(name_buf, "%s %s #%d", emoji, short_nom, p->id);

            GtkWidget *lbl_name = gtk_label_new(name_buf);
            gtk_box_append(GTK_BOX(item), lbl_name);

            GtkWidget *lbl_role = gtk_label_new("Solo");
            gtk_widget_set_halign(lbl_role, GTK_ALIGN_END);
            gtk_widget_set_hexpand(lbl_role, TRUE);
            gtk_box_append(GTK_BOX(item), lbl_role);

            GtkWidget *lbl_badge = gtk_label_new("solo");
            gtk_widget_add_css_class(lbl_badge, "badge-solo");
            gtk_box_append(GTK_BOX(item), lbl_badge);

            gtk_box_append(GTK_BOX(ui->box_sidebar_content), item);
         }
      }

      // 3. Predators & Allies
      gboolean has_pred_header = FALSE;
      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         if (strcmp(p->nom, "Requin") == 0 || strcmp(p->nom, "Dauphin") == 0)
         {
            if (!has_pred_header)
            {
               GtkWidget *lbl_header = gtk_label_new("PRÉDATEURS & ALLIÉS");
               gtk_widget_set_halign(lbl_header, GTK_ALIGN_START);
               gtk_widget_add_css_class(lbl_header, "entity-header");
               gtk_box_append(GTK_BOX(ui->box_sidebar_content), lbl_header);
               has_pred_header = TRUE;
            }

            GtkWidget *item = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            gtk_widget_add_css_class(item, "entity-item");

            const char *emoji = strcmp(p->nom, "Requin") == 0 ? "🦈" : "🐬";
            char name_buf[128];
            sprintf(name_buf, "%s %s", emoji, p->nom);

            GtkWidget *lbl_name = gtk_label_new(name_buf);
            gtk_box_append(GTK_BOX(item), lbl_name);

            GtkWidget *lbl_role = gtk_label_new(strcmp(p->nom, "Requin") == 0 ? "Chasseur" : "Défenseur");
            gtk_widget_set_halign(lbl_role, GTK_ALIGN_END);
            gtk_widget_set_hexpand(lbl_role, TRUE);
            gtk_box_append(GTK_BOX(item), lbl_role);

            GtkWidget *lbl_badge = gtk_label_new(strcmp(p->nom, "Requin") == 0 ? "pred" : "allie");
            gtk_widget_add_css_class(lbl_badge, "badge-pred");
            gtk_box_append(GTK_BOX(item), lbl_badge);

            gtk_box_append(GTK_BOX(ui->box_sidebar_content), item);
         }
      }
   }
   else // Tab: Bancs
   {
      GtkWidget *lbl_header = gtk_label_new("LISTE DES BANCS");
      gtk_widget_set_halign(lbl_header, GTK_ALIGN_START);
      gtk_widget_add_css_class(lbl_header, "entity-header");
      gtk_box_append(GTK_BOX(ui->box_sidebar_content), lbl_header);

      for (int b_id = 1; b_id <= ui->num_bancs; b_id++)
      {
         int count = 0;
         char leader_name[128] = "Aucun";
         char species_nom[64] = "";
         
         for (GList *l = ui->poissons; l; l = l->next)
         {
            Poisson *p = l->data;
            if (p->id_banc == b_id)
            {
               count++;
               strcpy(species_nom, p->nom);
               if (p->est_leader)
               {
                  const char *short_nom = strcmp(p->nom, "Poisson clown") == 0 ? "Clown" : (strcmp(p->nom, "Poisson-globe") == 0 ? "Globe" : "Hareng");
                  sprintf(leader_name, "%s #%d", short_nom, p->id);
               }
            }
         }

         if (count > 0)
         {
            GtkWidget *item = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
            gtk_widget_add_css_class(item, "entity-item");

            const char *emoji = strcmp(species_nom, "Hareng") == 0 ? "🐟" : (strcmp(species_nom, "Poisson-globe") == 0 ? "🐡" : "🐠");
            char name_buf[128];
            sprintf(name_buf, "%s Banc de %ss #%d", emoji, species_nom, b_id);
            GtkWidget *lbl_name = gtk_label_new(name_buf);
            gtk_widget_set_halign(lbl_name, GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(item), lbl_name);

            char lead_buf[128];
            sprintf(lead_buf, "★ Leader: %s | Membres: %d", leader_name, count);
            GtkWidget *lbl_lead = gtk_label_new(lead_buf);
            gtk_widget_set_halign(lbl_lead, GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(item), lbl_lead);

            gtk_box_append(GTK_BOX(ui->box_sidebar_content), item);
         }
      }
   }
}

// Update the bottom status bar stats
static void update_status_bar(BassinUI *ui)
{
   // Status indicator
   if (ui->simulation_running)
   {
      gtk_label_set_text(GTK_LABEL(ui->lbl_status_indicator), "● En cours");
      gtk_widget_set_sensitive(ui->lbl_status_indicator, TRUE);
   }
   else
   {
      gtk_label_set_text(GTK_LABEL(ui->lbl_status_indicator), "● En pause");
      gtk_widget_set_sensitive(ui->lbl_status_indicator, FALSE);
   }

   // Count entities
   int active_entities = g_list_length(ui->poissons);
   int active_bancs = 0;
   int active_indiv = 0;
   int active_preds = 0;

   for (int b_id = 1; b_id <= ui->num_bancs; b_id++)
   {
      gboolean found = FALSE;
      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         if (p->id_banc == b_id)
         {
            found = TRUE;
            break;
         }
      }
      if (found) active_bancs++;
   }

   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (strcmp(p->nom, "Requin") == 0 || strcmp(p->nom, "Dauphin") == 0)
      {
         active_preds++;
      }
      else if (p->id_banc == -1)
      {
         active_indiv++;
      }
   }

   char stats_buf[256];
   sprintf(stats_buf, "%d entités actives   |   %d bancs   |   %d individuel   |   %d prédateur", 
           active_entities, active_bancs, active_indiv, active_preds);
   gtk_label_set_text(GTK_LABEL(ui->lbl_stats_text), stats_buf);

   // Time formatter
   int total_sec = (int)ui->elapsed_time;
   int hours = total_sec / 3600;
   int mins = (total_sec % 3600) / 60;
   int secs = total_sec % 60;
   char time_buf[64];
   sprintf(time_buf, "t = %02d:%02d:%02d", hours, mins, secs);
   gtk_label_set_text(GTK_LABEL(ui->lbl_elapsed_time), time_buf);
}

// Boids Flocking Simulation Movement Step
static gboolean update_simulation(gpointer user_data)
{
   BassinUI *ui = user_data;
   if (!ui->simulation_running) return TRUE;

   double dt = 0.033; // 33ms step
   ui->elapsed_time += dt;

   // 1. Gather all species movements & boids calculations
   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      double force_x = 0;
      double force_y = 0;

      if (strcmp(p->nom, "Requin") == 0)
      {
         // Requin chases nearest prey
         Poisson *target = find_nearest_prey(ui, p);
         if (target)
         {
            double dx = target->x - p->x;
            double dy = target->y - p->y;
            double dist = sqrt(dx*dx + dy*dy);
            if (dist > 0)
            {
               force_x = (dx / dist) * p->vitesse_fuite;
               force_y = (dy / dist) * p->vitesse_fuite;
               
               // Check if eating
               if (dist < 30.0)
               {
                  eat_fish(ui, target);
                  update_sidebar_list(ui);
                  update_status_bar(ui);
               }
            }
         }
         else
         {
            force_x = p->vx;
            force_y = p->vy;
         }
      }
      else if (strcmp(p->nom, "Dauphin") == 0)
      {
         // Dauphin chases Requin
         Poisson *target = find_nearest_requin(ui, p);
         if (target)
         {
            double dx = target->x - p->x;
            double dy = target->y - p->y;
            double dist = sqrt(dx*dx + dy*dy);
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
         Poisson *pred = find_nearest_requin(ui, p);
         if (pred)
         {
            double dx = pred->x - p->x;
            double dy = pred->y - p->y;
            double dist = sqrt(dx*dx + dy*dy);
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
                     double d = sqrt(dx*dx + dy*dy);
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
                  double coh_d = sqrt(coh_fx*coh_fx + coh_fy*coh_fy);
                  if (coh_d > 0)
                  {
                     coh_fx = (coh_fx / coh_d) * p->vitesse_normale;
                     coh_fy = (coh_fy / coh_d) * p->vitesse_normale;
                  }

                  // Alignment force
                  double align_d = sqrt(align_vx*align_vx + align_vy*align_vy);
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
                     double lead_d = sqrt(lead_dx*lead_dx + lead_dy*lead_dy);
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
      double cur_speed = sqrt(p->vx*p->vx + p->vy*p->vy);
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
      p->x += p->vx * dt;
      p->y += p->vy * dt;

      // Frame constraints relative to configurable canvas size
      double margin = 10;
      double right_bound = ui->config_canvas_width - p->taille - margin;
      double bottom_bound = ui->config_canvas_height - p->taille - margin;

      if (p->x < margin)
      {
         p->x = margin;
         p->vx = -p->vx;
      }
      else if (p->x > right_bound)
      {
         p->x = right_bound;
         p->vx = -p->vx;
      }

      if (p->y < margin)
      {
         p->y = margin;
         p->vy = -p->vy;
      }
      else if (p->y > bottom_bound)
      {
         p->y = bottom_bound;
         p->vy = -p->vy;
      }

      // Animate/move widget
      if (p->widget_image)
      {
         // Flip fish horizontally based on movement direction
         GtkWidget *img_widget = gtk_widget_get_first_child(p->widget_image); // skips the leader label
         if (img_widget && GTK_IS_LABEL(img_widget)) 
         {
            img_widget = gtk_widget_get_next_sibling(img_widget); // this is the image
         }
         
         if (img_widget)
         {
            if (p->vx > 0)
            {
               // Facing right: transform flip
               GtkStyleContext *style_ctx = gtk_widget_get_style_context(img_widget);
               // Add native layout flip or scale if needed
            }
         }

         gtk_fixed_move(GTK_FIXED(ui->canvas), p->widget_image, (int)p->x, (int)p->y);
      }
   }

   update_status_bar(ui);
   return TRUE;
}

// Add Button Click Action: Show popover menu with species list
static void on_add_poisson_btn_clicked(GtkButton *btn, gpointer user_data)
{
   BassinUI *ui = user_data;

   GtkWidget *popover = gtk_popover_new();
   gtk_widget_set_parent(GTK_WIDGET(popover), GTK_WIDGET(btn));
   gtk_popover_set_autohide(GTK_POPOVER(popover), TRUE);

   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   gtk_widget_set_margin_start(box, 10);
   gtk_widget_set_margin_end(box, 10);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   // POISSONS header
   GtkWidget *lbl_p = gtk_label_new("POISSONS");
   gtk_widget_set_halign(lbl_p, GTK_ALIGN_START);
   gtk_widget_add_css_class(lbl_p, "entity-header");
   gtk_box_append(GTK_BOX(box), lbl_p);

   // Hareng
   GtkWidget *btn_har = gtk_button_new_with_label("🐟  Hareng (Banc possible)");
   g_object_set_data(G_OBJECT(btn_har), "ui", ui);
   g_object_set_data(G_OBJECT(btn_har), "species", "Hareng");
   g_signal_connect(btn_har, "clicked", G_CALLBACK(on_species_selected), NULL);
   gtk_box_append(GTK_BOX(box), btn_har);

   // Poisson-globe
   GtkWidget *btn_gl = gtk_button_new_with_label("🐡  Poisson-globe (Banc possible)");
   g_object_set_data(G_OBJECT(btn_gl), "ui", ui);
   g_object_set_data(G_OBJECT(btn_gl), "species", "Poisson-globe");
   g_signal_connect(btn_gl, "clicked", G_CALLBACK(on_species_selected), NULL);
   gtk_box_append(GTK_BOX(box), btn_gl);

   // Poisson clown
   GtkWidget *btn_cl = gtk_button_new_with_label("🐠  Poisson clown (Banc possible)");
   g_object_set_data(G_OBJECT(btn_cl), "ui", ui);
   g_object_set_data(G_OBJECT(btn_cl), "species", "Poisson clown");
   g_signal_connect(btn_cl, "clicked", G_CALLBACK(on_species_selected), NULL);
   gtk_box_append(GTK_BOX(box), btn_cl);

   // PRÉDATEURS header
   GtkWidget *lbl_pr = gtk_label_new("PRÉDATEURS & ALLIÉS");
   gtk_widget_set_halign(lbl_pr, GTK_ALIGN_START);
   gtk_widget_add_css_class(lbl_pr, "entity-header");
   gtk_box_append(GTK_BOX(box), lbl_pr);

   // Requin
   GtkWidget *btn_req = gtk_button_new_with_label("🦈  Requin (Individuel)");
   g_object_set_data(G_OBJECT(btn_req), "ui", ui);
   g_object_set_data(G_OBJECT(btn_req), "species", "Requin");
   g_signal_connect(btn_req, "clicked", G_CALLBACK(on_species_selected), NULL);
   gtk_box_append(GTK_BOX(box), btn_req);

   // Dauphin
   GtkWidget *btn_dau = gtk_button_new_with_label("🐬  Dauphin (Individuel)");
   g_object_set_data(G_OBJECT(btn_dau), "ui", ui);
   g_object_set_data(G_OBJECT(btn_dau), "species", "Dauphin");
   g_signal_connect(btn_dau, "clicked", G_CALLBACK(on_species_selected), NULL);
   gtk_box_append(GTK_BOX(box), btn_dau);

   gtk_popover_set_child(GTK_POPOVER(popover), box);
   gtk_popover_popup(GTK_POPOVER(popover));
}

// Remove last entity clicked action
static void on_remove_poisson_btn_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   BassinUI *ui = user_data;
   if (!ui->poissons) return;

   GList *node = ui->poissons;
   Poisson *p = (Poisson *)node->data;
   if (p->widget_image)
   {
      gtk_widget_unparent(p->widget_image);
   }
   poisson_free(p);
   ui->poissons = g_list_delete_link(ui->poissons, node);

   update_sidebar_list(ui);
   update_status_bar(ui);
}

// Play/Pause Action
static void on_play_pause_clicked(GtkButton *btn, gpointer user_data)
{
   BassinUI *ui = user_data;
   ui->simulation_running = !ui->simulation_running;

   if (ui->simulation_running)
   {
      gtk_button_set_label(btn, "⏸"); // Pause icon
   }
   else
   {
      gtk_button_set_label(btn, "▶"); // Play icon
   }
   update_status_bar(ui);
}

// Settings Menu clicked: Open Settings GtkDialog
static void on_settings_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   BassinUI *ui = user_data;

   dialog_initialiser(&ui->settings_dialog);
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
   {
      ui->settings_dialog.parent = GTK_WINDOW(toplevel);
   }
   dialog_set_titre(&ui->settings_dialog, "⚙️ Paramètres du Bassin");
   
   ui->settings_dialog.boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
   ui->settings_dialog.on_reponse = on_settings_reponse;
   ui->settings_dialog.user_data = ui;

   // Main layout box for dialog content
   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   // Slider for fish size
   GtkWidget *lbl_size = gtk_label_new("Taille des poissons (pixels)");
   gtk_widget_set_halign(lbl_size, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_size);

   slider_initialiser(&ui->settings_sld_fish_size);
   slider_set_bornes(&ui->settings_sld_fish_size, 32, 128);
   slider_set_valeur(&ui->settings_sld_fish_size, ui->config_fish_size);
   slider_set_digits(&ui->settings_sld_fish_size, 0);
   slider_set_afficher_valeur(&ui->settings_sld_fish_size, TRUE);
   GtkWidget *w_sld_size = slider_creer(&ui->settings_sld_fish_size);
   gtk_box_append(GTK_BOX(box), w_sld_size);

   // Dropdown for background selection
   GtkWidget *lbl_bg = gtk_label_new("Image d'arrière-plan");
   gtk_widget_set_halign(lbl_bg, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_bg);

   champ_select_initialiser(&ui->settings_sel_bg);
   champ_select_add_item(&ui->settings_sel_bg, "Bleu Profond (Défaut)");
   champ_select_add_item(&ui->settings_sel_bg, "Récif Corallien (Alternative)");
   int bg_idx = 0;
   if (strcmp(ui->config_bg_path, "resources/images/background2.png") == 0)
   {
      bg_idx = 1;
   }
   champ_select_set_index(&ui->settings_sel_bg, bg_idx);
   GtkWidget *w_sel_bg = champ_select_creer(&ui->settings_sel_bg);
   gtk_box_append(GTK_BOX(box), w_sel_bg);

   // Dropdown for canvas size selection
   GtkWidget *lbl_canvas = gtk_label_new("Dimensions de l'aquarium");
   gtk_widget_set_halign(lbl_canvas, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_canvas);

   champ_select_initialiser(&ui->settings_sel_canvas);
   champ_select_add_item(&ui->settings_sel_canvas, "Standard (900 x 600)");
   champ_select_add_item(&ui->settings_sel_canvas, "Grand (1100 x 700)");
   champ_select_add_item(&ui->settings_sel_canvas, "Géant (1300 x 800)");
   int canvas_idx = 0;
   if (ui->config_canvas_width == 1100)
   {
      canvas_idx = 1;
   }
   else if (ui->config_canvas_width == 1300)
   {
      canvas_idx = 2;
   }
   champ_select_set_index(&ui->settings_sel_canvas, canvas_idx);
   GtkWidget *w_sel_canvas = champ_select_creer(&ui->settings_sel_canvas);
   gtk_box_append(GTK_BOX(box), w_sel_canvas);

   dialog_set_contenu(&ui->settings_dialog, box);
   dialog_creer(&ui->settings_dialog);
   dialog_afficher(&ui->settings_dialog);
}

// Apply settings dialog choices
static void on_settings_reponse(int reponse, gpointer user_data)
{
   BassinUI *ui = user_data;
   if (reponse == DIALOG_REPONSE_OK)
   {
      // 1. Update fish size configuration
      int new_fish_size = (int)slider_get_valeur(&ui->settings_sld_fish_size);
      ui->config_fish_size = new_fish_size;

      // 2. Update background image path
      int bg_idx = champ_select_get_index(&ui->settings_sel_bg);
      if (bg_idx == 0)
      {
         ui->config_bg_path = "resources/images/background.png";
      }
      else
      {
         ui->config_bg_path = "resources/images/background2.png";
      }

      // 3. Update canvas dimensions
      int canvas_idx = champ_select_get_index(&ui->settings_sel_canvas);
      if (canvas_idx == 0)
      {
         ui->config_canvas_width = 900;
         ui->config_canvas_height = 600;
      }
      else if (canvas_idx == 1)
      {
         ui->config_canvas_width = 1100;
         ui->config_canvas_height = 700;
      }
      else if (canvas_idx == 2)
      {
         ui->config_canvas_width = 1300;
         ui->config_canvas_height = 800;
      }

      // 4. Set size requests to widgets
      gtk_widget_set_size_request(ui->canvas, ui->config_canvas_width, ui->config_canvas_height);
      
      if (ui->bg_widget)
      {
         gtk_picture_set_filename(GTK_PICTURE(ui->bg_widget), ui->config_bg_path);
         gtk_widget_set_size_request(ui->bg_widget, ui->config_canvas_width, ui->config_canvas_height);
      }

      // 5. Update existing fish sizes dynamically
      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         p->taille = ui->config_fish_size;
         if (p->widget_image)
         {
            // The widget container structure: Box [Label, Picture, Label]
            GtkWidget *img_widget = gtk_widget_get_first_child(p->widget_image);
            if (img_widget && GTK_IS_LABEL(img_widget))
            {
               img_widget = gtk_widget_get_next_sibling(img_widget);
            }
            if (img_widget)
            {
               gtk_widget_set_size_request(img_widget, p->taille, p->taille);
            }
         }
      }
   }

   dialog_fermer(&ui->settings_dialog);
}

// Restart Action
static void on_restart_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   BassinUI *ui = user_data;

   // Free all fish
   while (ui->poissons)
   {
      GList *node = ui->poissons;
      Poisson *p = (Poisson *)node->data;
      if (p->widget_image)
      {
         gtk_widget_unparent(p->widget_image);
      }
      poisson_free(p);
      ui->poissons = g_list_delete_link(ui->poissons, node);
   }

   ui->next_id = 1;
   ui->num_bancs = 0;
   ui->elapsed_time = 0;

   // Pre-populate default entities: 3 schools of different fish + 1 predator + 1 ally
   ui->num_bancs++;
   for (int i = 0; i < 4; i++) add_fish_programmatic(ui, "Hareng", TRUE, ui->num_bancs); // school of 4
   
   ui->num_bancs++;
   for (int i = 0; i < 3; i++) add_fish_programmatic(ui, "Poisson-globe", TRUE, ui->num_bancs); // school of 3

   add_fish_programmatic(ui, "Poisson clown", FALSE, -1); // solo
   add_fish_programmatic(ui, "Requin", FALSE, -1); // predator

   update_sidebar_list(ui);
   update_status_bar(ui);
}

// Create Main Aquarium UI Interface
GtkWidget *screen_bassin_create(void)
{
   // Load premium CSS styles
   GtkCssProvider *css_provider = gtk_css_provider_new();
   gtk_css_provider_load_from_data(css_provider, css_data, -1);
   gtk_style_context_add_provider_for_display(
       gdk_display_get_default(),
       GTK_STYLE_PROVIDER(css_provider),
       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   g_object_unref(css_provider);

    BassinUI *ui = calloc(1, sizeof(BassinUI));
    ui->simulation_running = TRUE;
    ui->active_tab = 0; // Entités
    ui->elapsed_time = 0;

    // Default configuration values
    ui->config_fish_size = 64;
    ui->config_bg_path = "resources/images/background.png";
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

   GtkWidget *lbl_title = gtk_label_new("Simulateur de bancs de poissons");
   // Set style (bold, larger)
   GtkStyleContext *title_ctx = gtk_widget_get_style_context(lbl_title);
   gtk_box_append(GTK_BOX(header), lbl_title);

   // Control actions
   GtkWidget *btn_add = gtk_button_new_with_label("+ Ajouter");
   gtk_widget_add_css_class(btn_add, "suggested-action");
   g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_poisson_btn_clicked), ui);
   gtk_box_append(GTK_BOX(header), btn_add);

   GtkWidget *btn_remove = gtk_button_new_with_label("🗑️ Supprimer");
   gtk_widget_add_css_class(btn_remove, "destructive-action");
   g_signal_connect(btn_remove, "clicked", G_CALLBACK(on_remove_poisson_btn_clicked), ui);
   gtk_box_append(GTK_BOX(header), btn_remove);

   // Simulation status controls on the right
   GtkWidget *sim_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
   gtk_widget_set_halign(sim_controls, GTK_ALIGN_END);
   gtk_widget_set_hexpand(sim_controls, TRUE);

   GtkWidget *lbl_sim = gtk_label_new("Simulation");
   gtk_box_append(GTK_BOX(sim_controls), lbl_sim);

   GtkWidget *btn_play = gtk_button_new_with_label("⏸");
   g_signal_connect(btn_play, "clicked", G_CALLBACK(on_play_pause_clicked), ui);
   gtk_box_append(GTK_BOX(sim_controls), btn_play);

   GtkWidget *btn_reset = gtk_button_new_with_label("🔄");
   g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_restart_clicked), ui);
   gtk_box_append(GTK_BOX(sim_controls), btn_reset);

   GtkWidget *btn_settings = gtk_button_new_with_label("⚙️");
   g_signal_connect(btn_settings, "clicked", G_CALLBACK(on_settings_clicked), ui);
   gtk_box_append(GTK_BOX(sim_controls), btn_settings);

   gtk_box_append(GTK_BOX(header), sim_controls);
   gtk_box_append(GTK_BOX(ui->root), header);

   // Separator
   GtkWidget *sep_top = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_box_append(GTK_BOX(ui->root), sep_top);

   // ------------------ CONTENT BOX (Aquarium + Sidebar) ------------------
   GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
   gtk_widget_set_vexpand(content_box, TRUE);

   // 1. Center Canvas aquarium (GtkFixed)
   ui->canvas = gtk_fixed_new();
   gtk_widget_set_hexpand(ui->canvas, TRUE);
   gtk_widget_set_size_request(ui->canvas, ui->config_canvas_width, ui->config_canvas_height);
    
   // Canvas Background Image (using GtkPicture with no aspect ratio constraint for dynamic stretching)
   ui->bg_widget = gtk_picture_new_for_filename(ui->config_bg_path);
   gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(ui->bg_widget), FALSE);
   gtk_widget_set_size_request(ui->bg_widget, ui->config_canvas_width, ui->config_canvas_height);
   gtk_fixed_put(GTK_FIXED(ui->canvas), ui->bg_widget, 0, 0);

   gtk_box_append(GTK_BOX(content_box), ui->canvas);

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

   // Start timer loop (30 FPS -> 33ms)
   ui->timer_id = g_timeout_add(33, update_simulation, ui);

   // Populate default simulator entities
   on_restart_clicked(NULL, ui);

   return ui->root;
}
