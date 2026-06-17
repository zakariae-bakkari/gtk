#include "../simulation/bassin_private.h"
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>
#include "../../widgets/headers/champ_texte.h"
#include "../../widgets/headers/champ_nombre.h"
#include "../../widgets/headers/bouton_checklist.h"
#include "../../widgets/headers/bouton.h"

void show_shortcuts_help_dialog(BassinUI *ui);

static void on_custom_bouton_destroy(GtkWidget *widget, gpointer data)
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

static GtkWidget *creer_bouton_custom(const char *texte, const char *nom_icone, BoutonIconePos pos_icone, const char *id_css, BoutonPresetStyle preset, BoutonAction on_clic, gpointer user_data)
{
   Bouton *b = g_new0(Bouton, 1);
   bouton_initialiser(b);
   g_free(b->texte);
   b->texte = texte ? strdup(texte) : strdup("");
   g_free(b->id_css);
   b->id_css = strdup(id_css);
   if (nom_icone)
   {
      b->nom_icone = strdup(nom_icone);
      b->pos_icone = pos_icone;
   }
   bouton_appliquer_preset(b, preset);
   b->on_clic = on_clic;
   b->user_data = user_data;
   GtkWidget *w = bouton_creer(b);
   g_signal_connect(w, "destroy", G_CALLBACK(on_custom_bouton_destroy), b);
   return w;
}

static void on_btn_shortcuts_clicked(GtkWidget *widget, gpointer data)
{
   (void)widget;
   show_shortcuts_help_dialog((BassinUI *)data);
}

void on_insertion_mode_changed(GtkCheckButton *widget, gpointer user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   gboolean is_banc = bouton_radio_est_actif(&ui->dialog_radio_banc);
   gtk_widget_set_sensitive(ui->dialog_school_frame, is_banc);
}

void add_fish_programmatic(BassinUI *ui, const char *species, gboolean in_banc, int target_banc_id)
{
   Poisson *p = poisson_new(species);
   p->id = ui->next_id++;
   p->taille = 64 * ((ui->config_fish_size > 0 ? ui->config_fish_size : 100) / 100.0);

   SpeciesConfig *cfg = find_species_config(ui, species);
   if (cfg)
   {
      p->vitesse_normale = cfg->vitesse_normale;
      p->vitesse_fuite = cfg->vitesse_fuite;
      p->vitesse_ralentie = cfg->vitesse_ralentie;
      p->sante_max = cfg->health > 0.0 ? cfg->health : 100.0;
      p->sante = p->sante_max;
      double scale = (ui->config_fish_size > 0 ? ui->config_fish_size : 100) / 100.0;
      if (strcmp(cfg->type, "predator") == 0)
         scale *= 1.8;
      else if (strcmp(cfg->type, "ally") == 0)
         scale *= 1.5;
      p->taille = cfg->taille * scale;
      p->perimetre_detection = cfg->perimetre_detection;

      poisson_set_default_frames(p, cfg->chemin_frames[0], cfg->chemin_frames[1], cfg->chemin_frames[2]);

      if (strcmp(cfg->type, "predator") == 0 || strcmp(cfg->type, "ally") == 0)
      {
         p->id_banc = -1;
         p->est_leader = FALSE;
      }
      else
      {
         p->id_banc = in_banc ? target_banc_id : -1;
         if (in_banc)
         {
            gboolean has_leader = FALSE;
            Banc *b = bassin_find_banc(ui, target_banc_id);
            if (b && b->leader)
            {
               has_leader = TRUE;
            }
            p->est_leader = !has_leader;
         }
         else
         {
            p->est_leader = FALSE;
         }
      }
   }
   else
   {
      // Fallback defaults
      p->vitesse_normale = 50.0;
      p->vitesse_fuite = 80.0;
      p->id_banc = in_banc ? target_banc_id : -1;
      p->est_leader = FALSE;
   }

   // Random position relative to configurable canvas size
   double rx = -1.0;
   double ry = -1.0;

   if (p->id_banc >= 0)
   {
      Banc *b = bassin_find_banc(ui, p->id_banc);
      if (b && b->poissons)
      {
         for (GList *l = b->poissons; l; l = l->next)
         {
            Poisson *other = l->data;
            if (other != p)
            {
               rx = other->x + (rand() % 60 - 30);
               ry = other->y + (rand() % 60 - 30);
               if (rx < 50) rx = 50;
               if (ry < 50) ry = 50;
               if (rx > ui->config_canvas_width - 50) rx = ui->config_canvas_width - 50;
               if (ry > ui->config_canvas_height - 50) ry = ui->config_canvas_height - 50;
               break;
            }
         }
      }
   }

   if (rx < 0.0 || ry < 0.0)
   {
      rx = 100 + (rand() % (ui->config_canvas_width > 200 ? ui->config_canvas_width - 200 : 100));
      ry = 100 + (rand() % (ui->config_canvas_height > 200 ? ui->config_canvas_height - 200 : 100));
   }
   
   poisson_set_position(p, rx, ry);

   // Random velocities
   double angle_rad = (rand() % 360) * M_PI / 180.0;
   p->vx = cos(angle_rad) * p->vitesse_normale;
   p->vy = sin(angle_rad) * p->vitesse_normale;

   // Create the widget representation using the coordinator helper
   create_poisson_widget(ui, p);

   bassin_add_poisson(ui, p);
}

void on_dialog_reponse(int reponse, gpointer user_data)
{
   BassinUI *ui = user_data;
   if (reponse == DIALOG_REPONSE_OK)
   {
      gboolean is_banc = bouton_radio_est_actif(&ui->dialog_radio_banc);
      int target_banc_id = -1;

      if (is_banc)
      {
         int idx = champ_select_get_index(&ui->dialog_sel_banc);
         if (idx == 0) // Create new school
         {
            ui->num_bancs++;
            target_banc_id = ui->num_bancs;
         }
         else // Map to an existing active school
         {
            int current_idx = 1;
            for (GList *l = ui->bancs; l; l = l->next)
            {
               Banc *b = l->data;
               int count = g_list_length(b->poissons);
               if (count > 0)
               {
                  if (current_idx == idx)
                  {
                     target_banc_id = b->id;
                     break;
                  }
                  current_idx++;
               }
            }
         }
      }

      int count = 1;
      if (is_banc)
      {
         count = (int)slider_get_valeur(&ui->dialog_sld_taille);
      }

      for (int i = 0; i < count; i++)
      {
         add_fish_programmatic(ui, ui->dialog_selected_species, is_banc, target_banc_id);
      }

      update_sidebar_list(ui);
      update_status_bar(ui);
   }

   dialog_fermer(&ui->active_dialog);
}

static void open_add_dialog(BassinUI *ui, const char *species)
{
   if (ui->dialog_selected_species)
      free(ui->dialog_selected_species);
   ui->dialog_selected_species = strdup(species);

   dialog_initialiser(&ui->active_dialog);
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
   {
      ui->active_dialog.parent = GTK_WINDOW(toplevel);
   }

   char title_buf[128];
   const char *emoji = "🐟";
   SpeciesConfig *cfg = find_species_config(ui, species);
   if (cfg)
   {
      if (strcmp(cfg->type, "predator") == 0)
         emoji = "🦈";
      else if (strcmp(cfg->type, "ally") == 0)
         emoji = "🐬";
      else if (strcmp(species, "Poisson-globe") == 0)
         emoji = "🐡";
      else if (strcmp(species, "Poisson clown") == 0)
         emoji = "🐠";
   }

   sprintf(title_buf, "Ajouter %s %s", emoji, species);
   dialog_set_titre(&ui->active_dialog, title_buf);

   ui->active_dialog.boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
   ui->active_dialog.on_reponse = on_dialog_reponse;
   ui->active_dialog.user_data = ui;

   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   GtkWidget *lbl_mode = gtk_label_new("Choisir le mode d'insertion");
   gtk_widget_set_halign(lbl_mode, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_mode);

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

   ui->dialog_school_frame = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
   gtk_widget_add_css_class(ui->dialog_school_frame, "dialog-frame");

   GtkWidget *lbl_target = gtk_label_new("Banc cible");
   gtk_widget_set_halign(lbl_target, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(ui->dialog_school_frame), lbl_target);

   champ_select_initialiser(&ui->dialog_sel_banc);
   champ_select_add_item(&ui->dialog_sel_banc, "Créer un nouveau banc...");

   for (GList *l = ui->bancs; l; l = l->next)
   {
      Banc *b = l->data;
      int count = g_list_length(b->poissons);
      if (count > 0)
      {
         char name_buf[128];
         sprintf(name_buf, "Banc %ss (%d membres)", b->nom_espece ? b->nom_espece : "poissons", count);
         champ_select_add_item(&ui->dialog_sel_banc, name_buf);
      }
   }

   GtkWidget *w_sel_banc = champ_select_creer(&ui->dialog_sel_banc);
   gtk_box_append(GTK_BOX(ui->dialog_school_frame), w_sel_banc);

   GtkWidget *lbl_size = gtk_label_new("Taille initiale du banc");
   gtk_widget_set_halign(lbl_size, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(ui->dialog_school_frame), lbl_size);

   slider_initialiser(&ui->dialog_sld_taille);
   slider_set_bornes(&ui->dialog_sld_taille, 2, 20);
   slider_set_valeur(&ui->dialog_sld_taille, 4);
   slider_set_digits(&ui->dialog_sld_taille, 0);
   slider_set_afficher_valeur(&ui->dialog_sld_taille, TRUE);
   GtkWidget *w_sld_taille = slider_creer(&ui->dialog_sld_taille);
   gtk_box_append(GTK_BOX(ui->dialog_school_frame), w_sld_taille);

   gtk_box_append(GTK_BOX(box), ui->dialog_school_frame);
   gtk_widget_set_sensitive(ui->dialog_school_frame, FALSE);

   dialog_set_contenu(&ui->active_dialog, box);
   dialog_creer(&ui->active_dialog);
   dialog_afficher(&ui->active_dialog);
}

static void on_popover_species_selected(GtkButton *btn, gpointer user_data)
{
   const char *selected = (const char *)user_data;
   BassinUI *ui = g_object_get_data(G_OBJECT(btn), "ui");
   GtkWidget *popover = g_object_get_data(G_OBJECT(btn), "popover");

   if (popover)
      gtk_popover_popdown(GTK_POPOVER(popover));
   if (ui && selected)
      open_add_dialog(ui, selected);
}

typedef struct
{
   BassinUI *ui;
   Dialog *dialog;
   char *edit_species_name; // Optional: name of species to edit
   ChampTexte champ_nom;
   ChampSelect champ_type;
   ChampNombre champ_level;
   ChampNombre champ_size;
   ChampNombre champ_detection;
   ChampNombre champ_speed_norm;
   ChampNombre champ_speed_escape;
   ChampNombre champ_speed_slow;
   ChampTexte champ_frame0;
   ChampTexte champ_frame1;
   ChampTexte champ_frame2;
   GList *diet_checklist_items; // list of BoutonChecklist*
   ChampNombre champ_health;
} SpeciesCreationCtx;

static void add_form_row(GtkWidget *box, const char *label_text, GtkWidget *field_widget)
{
   GtkWidget *row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   GtkWidget *lbl = gtk_label_new(NULL);
   char *markup = g_strdup_printf("<b>%s</b>", label_text);
   gtk_label_set_markup(GTK_LABEL(lbl), markup);
   g_free(markup);
   gtk_widget_set_halign(lbl, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(row), lbl);
   gtk_box_append(GTK_BOX(row), field_widget);
   gtk_box_append(GTK_BOX(box), row);
}
static void on_create_species_dialog_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   SpeciesCreationCtx *ctx = (SpeciesCreationCtx *)user_data;
   if (!ctx)
      return;

   g_free(ctx->edit_species_name);

   champ_texte_free(&ctx->champ_nom);
   champ_select_free(&ctx->champ_type);
   champ_nombre_free(&ctx->champ_level);
   champ_nombre_free(&ctx->champ_size);
   champ_nombre_free(&ctx->champ_detection);
   champ_nombre_free(&ctx->champ_health);
   champ_nombre_free(&ctx->champ_speed_norm);
   champ_nombre_free(&ctx->champ_speed_escape);
   champ_nombre_free(&ctx->champ_speed_slow);
   champ_texte_free(&ctx->champ_frame0);
   champ_texte_free(&ctx->champ_frame1);
   champ_texte_free(&ctx->champ_frame2);

   for (GList *l = ctx->diet_checklist_items; l; l = l->next)
   {
      BoutonChecklist *chk = l->data;
      g_free(chk->label);
      g_free(chk->tooltip);
      free(chk->id_css);
      free(chk->style.couleur_texte);
      free(chk->style.couleur_texte_hover);
      g_free(chk);
   }
   g_list_free(ctx->diet_checklist_items);

   g_free(ctx);
}

static const char *get_file_extension(const char *filename)
{
   const char *dot = strrchr(filename, '.');
   if(!dot || dot == filename) return "";
   return dot;
}

static void on_create_species_response(int reponse, gpointer user_data)
{
   SpeciesCreationCtx *ctx = (SpeciesCreationCtx *)user_data;
   BassinUI *ui = ctx->ui;

   if (reponse == 10 || reponse == 11)
   {
      const char *entered_name = champ_texte_get_texte(&ctx->champ_nom);
      char name[128];
      if (entered_name && strlen(entered_name) > 0)
      {
         strncpy(name, entered_name, sizeof(name) - 1);
         name[sizeof(name) - 1] = '\0';
      }
      else
      {
         strcpy(name, "Poisson-perso");
      }

      gboolean is_edit = (ctx->edit_species_name != NULL);
      SpeciesConfig *cfg = NULL;

      if (is_edit)
      {
         cfg = find_species_config(ui, ctx->edit_species_name);
         if (!cfg)
         {
            g_print("[ERROR] Species to edit not found: %s\n", ctx->edit_species_name);
            return;
         }
         
         // Free old sub-allocated buffers
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
         memset(cfg->diet, 0, sizeof(cfg->diet));
      }
      else
      {
         // Ensure name is unique to avoid collisions
         char unique_name[128];
         strcpy(unique_name, name);
         int suffix = 2;
         while (find_species_config(ui, unique_name))
         {
            snprintf(unique_name, sizeof(unique_name), "%s-%d", name, suffix++);
         }

         cfg = calloc(1, sizeof(SpeciesConfig));
         cfg->nom = strdup(unique_name);
      }

      const char *type_str = champ_select_get_string(&ctx->champ_type);
      cfg->type = strdup(type_str ? type_str : "prey");

      cfg->level = (int)champ_nombre_get_valeur(&ctx->champ_level);
      cfg->taille = (int)champ_nombre_get_valeur(&ctx->champ_size);
      cfg->perimetre_detection = (int)champ_nombre_get_valeur(&ctx->champ_detection);
      cfg->health = champ_nombre_get_valeur(&ctx->champ_health);

      cfg->vitesse_normale = champ_nombre_get_valeur(&ctx->champ_speed_norm);
      cfg->vitesse_fuite = champ_nombre_get_valeur(&ctx->champ_speed_escape);
      cfg->vitesse_ralentie = champ_nombre_get_valeur(&ctx->champ_speed_slow);
      
      g_print("[DEBUG] Updated Species '%s': Norm=%.1f, Esc=%.1f, Slow=%.1f\n", 
              cfg->nom, cfg->vitesse_normale, cfg->vitesse_fuite, cfg->vitesse_ralentie);

      // Safe folder name in resources/images/fishes/<safe_name>/
      char safe_dir_name[128];
      const char *folder_name = cfg->nom;
      int len = strlen(folder_name);
      for (int i = 0; i < len && i < 127; i++)
      {
         char c = folder_name[i];
         if (g_ascii_isalnum(c) || c == '_' || c == '-')
         {
            safe_dir_name[i] = g_ascii_tolower(c);
         }
         else
         {
            safe_dir_name[i] = '_';
         }
      }
      safe_dir_name[len] = '\0';

      char dest_dir[256];
      snprintf(dest_dir, sizeof(dest_dir), "resources/images/fishes/%s", safe_dir_name);
      g_mkdir_with_parents(dest_dir, 0755);

      const char *src_paths[3];
      src_paths[0] = champ_texte_get_texte(&ctx->champ_frame0);
      src_paths[1] = champ_texte_get_texte(&ctx->champ_frame1);
      src_paths[2] = champ_texte_get_texte(&ctx->champ_frame2);

      // If frame 0 is custom, and frame 1 or 2 are default/empty, fallback to frame 0
      gboolean frame0_is_custom = (src_paths[0] && strlen(src_paths[0]) > 0 &&
                                   strcmp(src_paths[0], "resources/images/fishes/sardine/frame1.png") != 0);
      if (frame0_is_custom)
      {
         if (!src_paths[1] || strlen(src_paths[1]) == 0 ||
             strcmp(src_paths[1], "resources/images/fishes/sardine/frame2.png") == 0)
         {
            src_paths[1] = src_paths[0];
         }
         if (!src_paths[2] || strlen(src_paths[2]) == 0 ||
             strcmp(src_paths[2], "resources/images/fishes/sardine/frame1.png") == 0)
         {
            src_paths[2] = src_paths[0];
         }
      }

      char dest_paths[3][256];
      for (int i = 0; i < 3; i++)
      {
         const char *src = src_paths[i];
         if (src && strlen(src) > 0)
         {
            const char *ext = get_file_extension(src);
            if (strlen(ext) == 0) ext = ".png";

            snprintf(dest_paths[i], sizeof(dest_paths[i]), "%s/frame%d%s", dest_dir, i + 1, ext);

            GFile *src_file = g_file_new_for_path(src);
            GFile *dst_file = g_file_new_for_path(dest_paths[i]);
            GError *error = NULL;
            gboolean copy_ok = g_file_copy(src_file, dst_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &error);
            if (!copy_ok)
            {
               g_print("[WARN] Failed to copy frame %d: %s\n", i + 1, error ? error->message : "unknown error");
               if (error) g_clear_error(&error);

               if (g_file_test(src, G_FILE_TEST_EXISTS))
               {
                  strncpy(dest_paths[i], src, sizeof(dest_paths[i]) - 1);
                  dest_paths[i][sizeof(dest_paths[i]) - 1] = '\0';
               }
               else
               {
                  snprintf(dest_paths[i], sizeof(dest_paths[i]), "resources/images/fishes/sardine/frame%d.png", (i == 1 ? 2 : 1));
               }
            }
            g_object_unref(src_file);
            g_object_unref(dst_file);
         }
         else
         {
            snprintf(dest_paths[i], sizeof(dest_paths[i]), "resources/images/fishes/sardine/frame%d.png", (i == 1 ? 2 : 1));
         }
      }

      cfg->chemin_frames[0] = strdup(dest_paths[0]);
      cfg->chemin_frames[1] = strdup(dest_paths[1]);
      cfg->chemin_frames[2] = strdup(dest_paths[2]);
      cfg->nb_frames = 3;

      int diet_idx = 0;
      for (GList *l = ctx->diet_checklist_items; l; l = l->next)
      {
         BoutonChecklist *chk = l->data;
         if (bouton_checklist_get_etat(chk) == CHECKLIST_CHECKED && diet_idx < 16)
         {
            cfg->diet[diet_idx++] = strdup(chk->label);
         }
      }
      cfg->nb_diet = diet_idx;

      if (!is_edit)
      {
         // Add to list
         ui->species_configs = g_list_append(ui->species_configs, cfg);
      }
      else
      {
         // Update all swimming fish of this species
         for (GList *l = ui->poissons; l; l = l->next)
         {
            Poisson *p = l->data;
            if (strcmp(p->nom, cfg->nom) == 0)
            {
               p->vitesse_normale = cfg->vitesse_normale;
               p->vitesse_fuite = cfg->vitesse_fuite;
               p->vitesse_ralentie = cfg->vitesse_ralentie;
               double scale = (ui->config_fish_size > 0 ? ui->config_fish_size : 100) / 100.0;
               if (strcmp(cfg->type, "predator") == 0)
                  scale *= 1.8;
               else if (strcmp(cfg->type, "ally") == 0)
                  scale *= 1.5;
               p->taille = cfg->taille * scale;
               p->perimetre_detection = cfg->perimetre_detection;
               p->sante_max = cfg->health > 0.0 ? cfg->health : 100.0;
               if (p->sante > p->sante_max)
                  p->sante = p->sante_max;

               for (int i = 0; i < 3; i++)
               {
                  g_free(p->chemin_frames[i]);
                  p->chemin_frames[i] = cfg->chemin_frames[i] ? g_strdup(cfg->chemin_frames[i]) : NULL;
               }

               if (p->widget_image)
               {
                  GtkWidget *lead_widget = gtk_widget_get_first_child(p->widget_image);
                  GtkWidget *health_bar = lead_widget ? gtk_widget_get_next_sibling(lead_widget) : NULL;
                  GtkWidget *img_widget = health_bar ? gtk_widget_get_next_sibling(health_bar) : NULL;
                  
                  if (health_bar)
                  {
                     gtk_widget_set_size_request(health_bar, (int)(p->taille * 0.5), 4);
                  }
                  if (img_widget && GTK_IS_PICTURE(img_widget))
                  {
                     gtk_widget_set_size_request(img_widget, p->taille, p->taille);
                     gtk_picture_set_filename(GTK_PICTURE(img_widget), p->chemin_frames[0]);
                  }
               }
            }
         }
      }

      // Persist to XML
      save_species_configs_to_xml(ui);

      // If "Create and put in bassin", spawn it
      if (reponse == 11 && !is_edit)
      {
         add_fish_programmatic(ui, cfg->nom, FALSE, -1);
      }

      update_sidebar_list(ui);
      update_status_bar(ui);
   }
}

static void on_frame_file_selected(GtkNativeDialog *dialog, int response_id, gpointer user_data)
{
   ChampTexte *champ = (ChampTexte *)user_data;
   if (response_id == GTK_RESPONSE_ACCEPT)
   {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
      GFile *file = gtk_file_chooser_get_file(chooser);
      if (file)
      {
         char *path = g_file_get_path(file);
         if (path)
         {
            champ_texte_set_texte(champ, path);
            g_free(path);
         }
         g_object_unref(file);
      }
   }
   g_object_unref(dialog);
}

static void on_browse_frame_clicked(GtkButton *btn, gpointer user_data)
{
   ChampTexte *champ = (ChampTexte *)user_data;
   GtkWindow *parent_win = g_object_get_data(G_OBJECT(btn), "parent_win");

   GtkFileChooserNative *native = gtk_file_chooser_native_new(
       "Sélectionner une image pour la frame",
       parent_win,
       GTK_FILE_CHOOSER_ACTION_OPEN,
       "Ouvrir",
       "Annuler"
   );

   GtkFileFilter *filter = gtk_file_filter_new();
   gtk_file_filter_add_mime_type(filter, "image/*");
   gtk_file_filter_add_pattern(filter, "*.png");
   gtk_file_filter_add_pattern(filter, "*.jpg");
   gtk_file_filter_add_pattern(filter, "*.jpeg");
   gtk_file_filter_set_name(filter, "Images (*.png, *.jpg, *.jpeg)");
   gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);

   g_signal_connect(native, "response", G_CALLBACK(on_frame_file_selected), champ);
   gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

static GtkWidget *create_frame_selector(ChampTexte *champ, const char *default_val, GtkWindow *parent_win)
{
   champ_texte_initialiser(champ);
   champ_texte_set_texte(champ, default_val);
   GtkWidget *w_entry_container = champ_texte_creer(champ);

   GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
   gtk_box_append(GTK_BOX(hbox), w_entry_container);
   gtk_widget_set_hexpand(w_entry_container, TRUE);

   GtkWidget *btn_browse = creer_bouton_custom(NULL, "folder-open-symbolic", ICONE_SEULE, "btn_browse_frame", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_browse_frame_clicked, champ);
   gtk_widget_set_tooltip_text(btn_browse, "Parcourir...");
   g_object_set_data(G_OBJECT(btn_browse), "parent_win", parent_win);
   gtk_box_append(GTK_BOX(hbox), btn_browse);

   return hbox;
}

void open_create_species_dialog(BassinUI *ui, const char *edit_species_name)
{
   SpeciesCreationCtx *ctx = g_new0(SpeciesCreationCtx, 1);
   ctx->ui = ui;
   ctx->edit_species_name = edit_species_name ? g_strdup(edit_species_name) : NULL;

   Dialog *d = g_new0(Dialog, 1);
   dialog_initialiser(d);
   ctx->dialog = d;

   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
   {
      d->parent = GTK_WINDOW(toplevel);
   }

   SpeciesConfig *cfg = NULL;
   if (edit_species_name)
   {
      cfg = find_species_config(ui, edit_species_name);
   }

   if (cfg)
   {
      char title_buf[128];
      snprintf(title_buf, sizeof(title_buf), "⚙️ Modifier l'espèce : %s", cfg->nom);
      dialog_set_titre(d, title_buf);

      // Custom buttons for edit mode
      dialog_ajouter_bouton(d, "Annuler", NULL, DIALOG_REPONSE_ANNULER, FALSE);
      dialog_ajouter_bouton(d, "Enregistrer", "document-save-symbolic", 10, TRUE);
   }
   else
   {
      dialog_set_titre(d, "✨ Créer une Nouvelle Espèce");

      // Custom buttons for creation mode
      dialog_ajouter_bouton(d, "Annuler", NULL, DIALOG_REPONSE_ANNULER, FALSE);
      dialog_ajouter_bouton(d, "Créer", "document-save-symbolic", 10, FALSE);
      dialog_ajouter_bouton(d, "Créer et mettre dans le bassin", "list-add-symbolic", 11, TRUE);
   }

   d->on_reponse = on_create_species_response;
   d->user_data = ctx;

   // Main content box
   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   // 1. Nom
   champ_texte_initialiser(&ctx->champ_nom);
   champ_texte_set_placeholder(&ctx->champ_nom, "Ex: Thon, Dorade...");
   ctx->champ_nom.required = TRUE;
   champ_texte_set_texte(&ctx->champ_nom, cfg ? cfg->nom : "Poisson-perso");
   if (cfg)
   {
      champ_texte_set_editable(&ctx->champ_nom, FALSE);
   }
   GtkWidget *w_nom = champ_texte_creer(&ctx->champ_nom);
   add_form_row(box, "Nom de l'espèce :", w_nom);

   // 2. Type
   champ_select_initialiser(&ctx->champ_type);
   champ_select_add_item(&ctx->champ_type, "prey");
   champ_select_add_item(&ctx->champ_type, "predator");
   champ_select_add_item(&ctx->champ_type, "ally");
   
   int type_idx = 0;
   if (cfg)
   {
      if (strcmp(cfg->type, "predator") == 0) type_idx = 1;
      else if (strcmp(cfg->type, "ally") == 0) type_idx = 2;
   }
   champ_select_set_index(&ctx->champ_type, type_idx);
   GtkWidget *w_type = champ_select_creer(&ctx->champ_type);
   add_form_row(box, "Type (comportement) :", w_type);

   // 3. Niveau
   champ_nombre_initialiser(&ctx->champ_level);
   champ_nombre_set_bornes(&ctx->champ_level, 1, 5);
   champ_nombre_set_valeur(&ctx->champ_level, cfg ? cfg->level : 1);
   champ_nombre_set_digits(&ctx->champ_level, 0);
   GtkWidget *w_level = champ_nombre_creer(&ctx->champ_level);
   add_form_row(box, "Niveau de force (1-5) :", w_level);

   // 4. Taille
   champ_nombre_initialiser(&ctx->champ_size);
   champ_nombre_set_bornes(&ctx->champ_size, 10, 500);
   champ_nombre_set_valeur(&ctx->champ_size, cfg ? cfg->taille : 64);
   champ_nombre_set_digits(&ctx->champ_size, 0);
   GtkWidget *w_size = champ_nombre_creer(&ctx->champ_size);
   add_form_row(box, "Taille par défaut (pixels) :", w_size);

   // 5. Périmètre détection
   champ_nombre_initialiser(&ctx->champ_detection);
   champ_nombre_set_bornes(&ctx->champ_detection, 10, 1000);
   champ_nombre_set_valeur(&ctx->champ_detection, cfg ? cfg->perimetre_detection : 100);
   champ_nombre_set_digits(&ctx->champ_detection, 0);
   GtkWidget *w_detection = champ_nombre_creer(&ctx->champ_detection);
   add_form_row(box, "Périmètre de détection (pixels) :", w_detection);
 
   // 5b. Santé maximale
   champ_nombre_initialiser(&ctx->champ_health);
   champ_nombre_set_bornes(&ctx->champ_health, 10, 1000);
   champ_nombre_set_valeur(&ctx->champ_health, cfg ? cfg->health : 100);
   champ_nombre_set_digits(&ctx->champ_health, 0);
   GtkWidget *w_health = champ_nombre_creer(&ctx->champ_health);
   add_form_row(box, "Santé maximale :", w_health);

   // 6. Vitesses
   GtkWidget *speeds_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
   
   GtkWidget *col_norm = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   GtkWidget *lbl_norm = gtk_label_new("Normale");
   gtk_widget_set_halign(lbl_norm, GTK_ALIGN_START);
   champ_nombre_initialiser(&ctx->champ_speed_norm);
   champ_nombre_set_bornes(&ctx->champ_speed_norm, 5, 500);
   champ_nombre_set_valeur(&ctx->champ_speed_norm, cfg ? cfg->vitesse_normale : 50);
   champ_nombre_set_digits(&ctx->champ_speed_norm, 0);
   gtk_box_append(GTK_BOX(col_norm), lbl_norm);
   gtk_box_append(GTK_BOX(col_norm), champ_nombre_creer(&ctx->champ_speed_norm));
   gtk_box_append(GTK_BOX(speeds_box), col_norm);
   gtk_widget_set_hexpand(col_norm, TRUE);

   GtkWidget *col_esc = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   GtkWidget *lbl_esc = gtk_label_new("Fuite");
   gtk_widget_set_halign(lbl_esc, GTK_ALIGN_START);
   champ_nombre_initialiser(&ctx->champ_speed_escape);
   champ_nombre_set_bornes(&ctx->champ_speed_escape, 5, 500);
   champ_nombre_set_valeur(&ctx->champ_speed_escape, cfg ? cfg->vitesse_fuite : 80);
   champ_nombre_set_digits(&ctx->champ_speed_escape, 0);
   gtk_box_append(GTK_BOX(col_esc), lbl_esc);
   gtk_box_append(GTK_BOX(col_esc), champ_nombre_creer(&ctx->champ_speed_escape));
   gtk_box_append(GTK_BOX(speeds_box), col_esc);
   gtk_widget_set_hexpand(col_esc, TRUE);

   GtkWidget *col_slow = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   GtkWidget *lbl_slow = gtk_label_new("Ralentie");
   gtk_widget_set_halign(lbl_slow, GTK_ALIGN_START);
   champ_nombre_initialiser(&ctx->champ_speed_slow);
   champ_nombre_set_bornes(&ctx->champ_speed_slow, 5, 500);
   champ_nombre_set_valeur(&ctx->champ_speed_slow, cfg ? cfg->vitesse_ralentie : 25);
   champ_nombre_set_digits(&ctx->champ_speed_slow, 0);
   gtk_box_append(GTK_BOX(col_slow), lbl_slow);
   gtk_box_append(GTK_BOX(col_slow), champ_nombre_creer(&ctx->champ_speed_slow));
   gtk_box_append(GTK_BOX(speeds_box), col_slow);
   gtk_widget_set_hexpand(col_slow, TRUE);

   add_form_row(box, "Vitesses de déplacement (px/s) :", speeds_box);

   // 7. Animation Frames
   GtkWidget *frames_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   GtkWindow *p_win = toplevel ? GTK_WINDOW(toplevel) : NULL;

   const char *f0 = (cfg && cfg->nb_frames > 0 && cfg->chemin_frames[0]) ? cfg->chemin_frames[0] : "resources/images/fishes/sardine/frame1.png";
   const char *f1 = (cfg && cfg->nb_frames > 1 && cfg->chemin_frames[1]) ? cfg->chemin_frames[1] : "resources/images/fishes/sardine/frame2.png";
   const char *f2 = (cfg && cfg->nb_frames > 2 && cfg->chemin_frames[2]) ? cfg->chemin_frames[2] : "resources/images/fishes/sardine/frame1.png";

   gtk_box_append(GTK_BOX(frames_box), create_frame_selector(&ctx->champ_frame0, f0, p_win));
   gtk_box_append(GTK_BOX(frames_box), create_frame_selector(&ctx->champ_frame1, f1, p_win));
   gtk_box_append(GTK_BOX(frames_box), create_frame_selector(&ctx->champ_frame2, f2, p_win));

   add_form_row(box, "Images d'animation (Frames 1, 2, 3) :", frames_box);

   // 8. Régime alimentaire
   GtkWidget *diet_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   ctx->diet_checklist_items = NULL;

   for (GList *l = ui->species_configs; l; l = l->next)
   {
      SpeciesConfig *other_cfg = l->data;
      if (cfg && strcmp(other_cfg->nom, cfg->nom) == 0)
         continue; // skip self

      BoutonChecklist *chk = g_new0(BoutonChecklist, 1);
      bouton_checklist_initialiser(chk);
      chk->label = g_strdup(other_cfg->nom);
      
      gboolean eats_other = FALSE;
      if (cfg)
      {
         for (int d_i = 0; d_i < cfg->nb_diet; d_i++)
         {
            if (strcmp(cfg->diet[d_i], other_cfg->nom) == 0)
            {
               eats_other = TRUE;
               break;
            }
         }
      }
      chk->etat = eats_other ? CHECKLIST_CHECKED : CHECKLIST_UNCHECKED;

      GtkWidget *w_chk = bouton_checklist_creer(chk);
      gtk_box_append(GTK_BOX(diet_box), w_chk);
      ctx->diet_checklist_items = g_list_append(ctx->diet_checklist_items, chk);
   }
   add_form_row(box, "Régime alimentaire (mange d'autres espèces) :", diet_box);

   // Wrap everything in a scrolled window
   GtkWidget *scroll = gtk_scrolled_window_new();
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
   gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroll), TRUE);
   gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroll), 480);
   gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), box);

   dialog_set_contenu(d, scroll);
   dialog_creer(d);

   // Connect destroy signals
   g_signal_connect_swapped(d->window, "destroy", G_CALLBACK(dialog_free), d);
   g_signal_connect(d->window, "destroy", G_CALLBACK(on_create_species_dialog_destroy), ctx);

   dialog_afficher(d);
}

static void on_create_new_species_clicked(GtkButton *btn, gpointer user_data)
{
   BassinUI *ui = user_data;
   GtkWidget *popover = g_object_get_data(G_OBJECT(btn), "popover");
   if (popover)
   {
      gtk_popover_popdown(GTK_POPOVER(popover));
   }
   open_create_species_dialog(ui, NULL);
}

static void on_popover_closed(GtkWidget *popover, gpointer user_data)
{
   (void)user_data;
   gtk_widget_unparent(popover);
}

static void on_popover_details_clicked(GtkButton *btn, gpointer user_data)
{
   const char *species_name = user_data;
   BassinUI *ui = g_object_get_data(G_OBJECT(btn), "ui");
   GtkWidget *popover = g_object_get_data(G_OBJECT(btn), "popover");
   if (popover)
   {
      gtk_popover_popdown(GTK_POPOVER(popover));
   }
   if (ui && species_name)
   {
      open_species_details_dialog(ui, species_name);
   }
}

void on_add_poisson_btn_clicked(GtkWidget *widget, gpointer user_data)
{
   BassinUI *ui = user_data;

   GtkWidget *popover = gtk_popover_new();
   gtk_widget_set_parent(GTK_WIDGET(popover), GTK_WIDGET(widget));
   gtk_popover_set_autohide(GTK_POPOVER(popover), TRUE);
   g_signal_connect(popover, "closed", G_CALLBACK(on_popover_closed), NULL);

   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   gtk_widget_set_margin_start(box, 10);
   gtk_widget_set_margin_end(box, 10);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   GtkWidget *lbl_p = gtk_label_new("POISSONS");
   gtk_widget_set_halign(lbl_p, GTK_ALIGN_START);
   gtk_widget_add_css_class(lbl_p, "entity-header");
   gtk_box_append(GTK_BOX(box), lbl_p);

   for (GList *l = ui->species_configs; l; l = l->next)
   {
      SpeciesConfig *cfg = l->data;
      if (strcmp(cfg->type, "prey") == 0)
      {
         char label_buf[128];
         const char *emoji = "🐟";
         if (strcmp(cfg->nom, "Poisson-globe") == 0)
            emoji = "🐡";
         else if (strcmp(cfg->nom, "Poisson clown") == 0)
            emoji = "🐠";

         sprintf(label_buf, "%s  %s (Banc possible)", emoji, cfg->nom);
         
         GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
         
         GtkWidget *btn_sp = creer_bouton_custom(label_buf, NULL, ICONE_GAUCHE, "btn_popover_sp", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_popover_species_selected, (gpointer)cfg->nom);
         gtk_widget_set_hexpand(btn_sp, TRUE);
         gtk_widget_set_halign(btn_sp, GTK_ALIGN_FILL);
         g_object_set_data(G_OBJECT(btn_sp), "ui", ui);
         g_object_set_data(G_OBJECT(btn_sp), "popover", popover);
         gtk_box_append(GTK_BOX(row), btn_sp);

         GtkWidget *btn_info = creer_bouton_custom(NULL, "help-about-symbolic", ICONE_SEULE, "btn_popover_info", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_popover_details_clicked, (gpointer)cfg->nom);
         gtk_widget_set_tooltip_text(btn_info, "Détails de l'espèce");
         g_object_set_data(G_OBJECT(btn_info), "ui", ui);
         g_object_set_data(G_OBJECT(btn_info), "popover", popover);
         gtk_box_append(GTK_BOX(row), btn_info);

         gtk_box_append(GTK_BOX(box), row);
      }
   }

   GtkWidget *lbl_pr = gtk_label_new("PRÉDATEURS & ALLIÉS");
   gtk_widget_set_halign(lbl_pr, GTK_ALIGN_START);
   gtk_widget_add_css_class(lbl_pr, "entity-header");
   gtk_box_append(GTK_BOX(box), lbl_pr);

   for (GList *l = ui->species_configs; l; l = l->next)
   {
      SpeciesConfig *cfg = l->data;
      if (strcmp(cfg->type, "prey") != 0)
      {
         char label_buf[128];
         const char *emoji = strcmp(cfg->type, "predator") == 0 ? "🦈" : "🐬";
         sprintf(label_buf, "%s  %s", emoji, cfg->nom);
         
         GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
         
         GtkWidget *btn_sp = creer_bouton_custom(label_buf, NULL, ICONE_GAUCHE, "btn_popover_sp", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_popover_species_selected, (gpointer)cfg->nom);
         gtk_widget_set_hexpand(btn_sp, TRUE);
         gtk_widget_set_halign(btn_sp, GTK_ALIGN_FILL);
         g_object_set_data(G_OBJECT(btn_sp), "ui", ui);
         g_object_set_data(G_OBJECT(btn_sp), "popover", popover);
         gtk_box_append(GTK_BOX(row), btn_sp);

         GtkWidget *btn_info = creer_bouton_custom(NULL, "help-about-symbolic", ICONE_SEULE, "btn_popover_info", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_popover_details_clicked, (gpointer)cfg->nom);
         gtk_widget_set_tooltip_text(btn_info, "Détails de l'espèce");
         g_object_set_data(G_OBJECT(btn_info), "ui", ui);
         g_object_set_data(G_OBJECT(btn_info), "popover", popover);
         gtk_box_append(GTK_BOX(row), btn_info);

         gtk_box_append(GTK_BOX(box), row);
      }
   }

   // Separator
   GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_widget_set_margin_top(sep, 6);
   gtk_widget_set_margin_bottom(sep, 6);
   gtk_box_append(GTK_BOX(box), sep);

   // Create new one button
   GtkWidget *btn_create_new = creer_bouton_custom("✨ Create new one", NULL, ICONE_GAUCHE, "btn_popover_new", BOUTON_STYLE_SUGGESTED, (BoutonAction)on_create_new_species_clicked, ui);
   g_object_set_data(G_OBJECT(btn_create_new), "popover", popover);
   gtk_box_append(GTK_BOX(box), btn_create_new);

   gtk_popover_set_child(GTK_POPOVER(popover), box);
   gtk_popover_popup(GTK_POPOVER(popover));
}

static void on_settings_dialog_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   BassinUI *ui = (BassinUI *)user_data;
   if (!ui)
      return;

   slider_free(&ui->settings_sld_fish_size);
   champ_texte_free(&ui->settings_txt_bg);
   champ_select_free(&ui->settings_sel_canvas);

   champ_texte_free(&ui->settings_txt_shortcut_play);
   champ_texte_free(&ui->settings_txt_shortcut_zen);
   champ_texte_free(&ui->settings_txt_shortcut_debug);
   champ_texte_free(&ui->settings_txt_shortcut_settings);
   champ_texte_free(&ui->settings_txt_shortcut_add);
   champ_texte_free(&ui->settings_txt_shortcut_sidebar);
   champ_texte_free(&ui->settings_txt_shortcut_restart);
}

static void on_settings_reponse(int reponse, gpointer user_data)
{
   BassinUI *ui = user_data;
   if (reponse == DIALOG_REPONSE_OK)
   {
      int new_fish_size = (int)slider_get_valeur(&ui->settings_sld_fish_size);
      ui->config_fish_size = new_fish_size;

      const char *new_bg = champ_texte_get_texte(&ui->settings_txt_bg);
      if (new_bg && strlen(new_bg) > 0)
      {
         if (ui->config_bg_path)
            free(ui->config_bg_path);
         ui->config_bg_path = strdup(new_bg);
      }

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

      gtk_widget_set_size_request(ui->canvas, ui->config_canvas_width, ui->config_canvas_height);

      if (ui->bg_widget)
      {
         apply_background(ui);
         gtk_widget_set_size_request(ui->bg_widget, ui->config_canvas_width, ui->config_canvas_height);
      }

      ui->config_hide_health_bar = gtk_switch_get_active(GTK_SWITCH(ui->settings_sw_hide_health));
      ui->config_hide_fish_name = gtk_switch_get_active(GTK_SWITCH(ui->settings_sw_hide_name));
      ui->config_hide_status_bar = gtk_switch_get_active(GTK_SWITCH(ui->settings_sw_hide_status));
      ui->config_always_eat = gtk_switch_get_active(GTK_SWITCH(ui->settings_sw_always_eat));

      apply_fish_visibility_configs(ui);
      if (!ui->zen_mode)
      {
         gtk_widget_set_visible(ui->status_bar_widget, !ui->config_hide_status_bar);
         gtk_widget_set_visible(ui->sep_bottom_widget, !ui->config_hide_status_bar);
      }

      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         double scale = (ui->config_fish_size > 0 ? ui->config_fish_size : 100) / 100.0;
         SpeciesConfig *cfg = find_species_config(ui, p->nom);
         if (cfg)
         {
            p->taille = cfg->taille * scale;
         }
         else
         {
             p->taille = 64 * scale;
         }
         
         if (p->widget_image)
         {
            GtkWidget *lead = gtk_widget_get_first_child(p->widget_image);
            GtkWidget *bar = lead ? gtk_widget_get_next_sibling(lead) : NULL;
            GtkWidget *img_widget = bar ? gtk_widget_get_next_sibling(bar) : NULL;
            if (img_widget)
            {
               gtk_widget_set_size_request(img_widget, p->taille, p->taille);
            }
         }
      }

      // Save shortcuts settings
      const char *sh_play = champ_texte_get_texte(&ui->settings_txt_shortcut_play);
      if (sh_play)
      {
         g_free(ui->shortcut_play);
         ui->shortcut_play = g_strdup(sh_play);
      }
      const char *sh_zen = champ_texte_get_texte(&ui->settings_txt_shortcut_zen);
      if (sh_zen)
      {
         g_free(ui->shortcut_zen);
         ui->shortcut_zen = g_strdup(sh_zen);
      }
      const char *sh_debug = champ_texte_get_texte(&ui->settings_txt_shortcut_debug);
      if (sh_debug)
      {
         g_free(ui->shortcut_debug);
         ui->shortcut_debug = g_strdup(sh_debug);
      }
      const char *sh_settings = champ_texte_get_texte(&ui->settings_txt_shortcut_settings);
      if (sh_settings)
      {
         g_free(ui->shortcut_settings);
         ui->shortcut_settings = g_strdup(sh_settings);
      }
      const char *sh_add = champ_texte_get_texte(&ui->settings_txt_shortcut_add);
      if (sh_add)
      {
         g_free(ui->shortcut_add);
         ui->shortcut_add = g_strdup(sh_add);
      }
      const char *sh_sidebar = champ_texte_get_texte(&ui->settings_txt_shortcut_sidebar);
      if (sh_sidebar)
      {
         g_free(ui->shortcut_sidebar);
         ui->shortcut_sidebar = g_strdup(sh_sidebar);
      }
      const char *sh_restart = champ_texte_get_texte(&ui->settings_txt_shortcut_restart);
      if (sh_restart)
      {
         g_free(ui->shortcut_restart);
         ui->shortcut_restart = g_strdup(sh_restart);
      }

      save_settings_to_xml(ui);
   }

   dialog_fermer(&ui->settings_dialog);
}

static void on_bg_file_selected(GtkNativeDialog *dialog, int response_id, gpointer user_data)
{
   ChampTexte *champ = (ChampTexte *)user_data;
   if (response_id == GTK_RESPONSE_ACCEPT)
   {
      GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
      GFile *file = gtk_file_chooser_get_file(chooser);
      if (file)
      {
         char *path = g_file_get_path(file);
         if (path)
         {
            champ_texte_set_texte(champ, path);
            g_free(path);
         }
         g_object_unref(file);
      }
   }
   g_object_unref(dialog);
}

static void on_browse_bg_clicked(GtkButton *btn, gpointer user_data)
{
   ChampTexte *champ = (ChampTexte *)user_data;
   GtkWindow *parent_win = g_object_get_data(G_OBJECT(btn), "parent_win");

   GtkFileChooserNative *native = gtk_file_chooser_native_new(
       "Sélectionner une image ou une vidéo d'arrière-plan",
       parent_win,
       GTK_FILE_CHOOSER_ACTION_OPEN,
       "Ouvrir",
       "Annuler"
   );

   GtkFileFilter *filter = gtk_file_filter_new();
   gtk_file_filter_add_mime_type(filter, "image/*");
   gtk_file_filter_add_mime_type(filter, "video/*");
   gtk_file_filter_add_pattern(filter, "*.png");
   gtk_file_filter_add_pattern(filter, "*.jpg");
   gtk_file_filter_add_pattern(filter, "*.jpeg");
   gtk_file_filter_add_pattern(filter, "*.mp4");
   gtk_file_filter_add_pattern(filter, "*.webm");
   gtk_file_filter_add_pattern(filter, "*.mkv");
   gtk_file_filter_set_name(filter, "Images & Vidéos");
   gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filter);

   g_signal_connect(native, "response", G_CALLBACK(on_bg_file_selected), champ);
   gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

void on_settings_clicked(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
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

   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   GtkWidget *lbl_size = gtk_label_new("Échelle des poissons (%)");
   gtk_widget_set_halign(lbl_size, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_size);

   slider_initialiser(&ui->settings_sld_fish_size);
   slider_set_bornes(&ui->settings_sld_fish_size, 10, 300);
   slider_set_valeur(&ui->settings_sld_fish_size, ui->config_fish_size > 0 ? ui->config_fish_size : 100);
   slider_set_digits(&ui->settings_sld_fish_size, 0);
   slider_set_afficher_valeur(&ui->settings_sld_fish_size, TRUE);
   GtkWidget *w_sld_size = slider_creer(&ui->settings_sld_fish_size);
   gtk_box_append(GTK_BOX(box), w_sld_size);

   GtkWidget *lbl_bg = gtk_label_new("Image/Vidéo d'arrière-plan");
   gtk_widget_set_halign(lbl_bg, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_bg);

   champ_texte_initialiser(&ui->settings_txt_bg);
   champ_texte_set_texte(&ui->settings_txt_bg, ui->config_bg_path ? ui->config_bg_path : "");
   GtkWidget *w_txt_bg = champ_texte_creer(&ui->settings_txt_bg);
   
   GtkWidget *hbox_bg = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
   gtk_box_append(GTK_BOX(hbox_bg), w_txt_bg);
   gtk_widget_set_hexpand(w_txt_bg, TRUE);
   
   GtkWidget *btn_browse_bg = creer_bouton_custom(NULL, "folder-open-symbolic", ICONE_SEULE, "btn_browse_bg", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_browse_bg_clicked, &ui->settings_txt_bg);
   gtk_widget_set_tooltip_text(btn_browse_bg, "Parcourir...");
   g_object_set_data(G_OBJECT(btn_browse_bg), "parent_win", (gpointer)(toplevel ? GTK_WINDOW(toplevel) : NULL));
   gtk_box_append(GTK_BOX(hbox_bg), btn_browse_bg);

   gtk_box_append(GTK_BOX(box), hbox_bg);

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

   // Hiding switches box section
   GtkWidget *lbl_display_options = gtk_label_new("Options d'affichage");
   gtk_widget_set_halign(lbl_display_options, GTK_ALIGN_START);
   gtk_widget_add_css_class(lbl_display_options, "entity-header");
   gtk_widget_set_margin_top(lbl_display_options, 8);
   gtk_box_append(GTK_BOX(box), lbl_display_options);

   // Health bar switch row
   GtkWidget *row_health = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
   GtkWidget *lbl_health = gtk_label_new("Masquer la barre de santé");
   gtk_widget_set_hexpand(lbl_health, TRUE);
   gtk_widget_set_halign(lbl_health, GTK_ALIGN_START);
   ui->settings_sw_hide_health = gtk_switch_new();
   gtk_switch_set_active(GTK_SWITCH(ui->settings_sw_hide_health), ui->config_hide_health_bar);
   gtk_widget_set_halign(ui->settings_sw_hide_health, GTK_ALIGN_END);
   gtk_box_append(GTK_BOX(row_health), lbl_health);
   gtk_box_append(GTK_BOX(row_health), ui->settings_sw_hide_health);
   gtk_box_append(GTK_BOX(box), row_health);

   // Fish name switch row
   GtkWidget *row_name = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
   GtkWidget *lbl_name = gtk_label_new("Masquer le nom des poissons");
   gtk_widget_set_hexpand(lbl_name, TRUE);
   gtk_widget_set_halign(lbl_name, GTK_ALIGN_START);
   ui->settings_sw_hide_name = gtk_switch_new();
   gtk_switch_set_active(GTK_SWITCH(ui->settings_sw_hide_name), ui->config_hide_fish_name);
   gtk_widget_set_halign(ui->settings_sw_hide_name, GTK_ALIGN_END);
   gtk_box_append(GTK_BOX(row_name), lbl_name);
   gtk_box_append(GTK_BOX(row_name), ui->settings_sw_hide_name);
   gtk_box_append(GTK_BOX(box), row_name);

   // Status bar switch row
   GtkWidget *row_status = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
   GtkWidget *lbl_status = gtk_label_new("Masquer la barre d'état");
   gtk_widget_set_hexpand(lbl_status, TRUE);
   gtk_widget_set_halign(lbl_status, GTK_ALIGN_START);
   ui->settings_sw_hide_status = gtk_switch_new();
   gtk_switch_set_active(GTK_SWITCH(ui->settings_sw_hide_status), ui->config_hide_status_bar);
   gtk_widget_set_halign(ui->settings_sw_hide_status, GTK_ALIGN_END);
   gtk_box_append(GTK_BOX(row_status), lbl_status);
   gtk_box_append(GTK_BOX(row_status), ui->settings_sw_hide_status);
   gtk_box_append(GTK_BOX(box), row_status);

   // Always eat switch row
   GtkWidget *row_always_eat = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
   GtkWidget *lbl_always_eat = gtk_label_new("Manger même avec santé pleine");
   gtk_widget_set_hexpand(lbl_always_eat, TRUE);
   gtk_widget_set_halign(lbl_always_eat, GTK_ALIGN_START);
   ui->settings_sw_always_eat = gtk_switch_new();
   gtk_switch_set_active(GTK_SWITCH(ui->settings_sw_always_eat), ui->config_always_eat);
   gtk_widget_set_halign(ui->settings_sw_always_eat, GTK_ALIGN_END);
   gtk_box_append(GTK_BOX(row_always_eat), lbl_always_eat);
   gtk_box_append(GTK_BOX(row_always_eat), ui->settings_sw_always_eat);
   gtk_box_append(GTK_BOX(box), row_always_eat);

   // Separator
   GtkWidget *sep_help = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_widget_set_margin_top(sep_help, 8);
   gtk_widget_set_margin_bottom(sep_help, 8);
   gtk_box_append(GTK_BOX(box), sep_help);

   GtkWidget *btn_shortcuts = creer_bouton_custom("⌨️ Raccourcis Clavier", NULL, ICONE_GAUCHE, "btn_shortcuts_help", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_btn_shortcuts_clicked, ui);
   gtk_box_append(GTK_BOX(box), btn_shortcuts);

   // Raccourcis Clavier Section
   GtkWidget *lbl_shortcuts_sec = gtk_label_new("Configuration des Raccourcis");
   gtk_widget_set_halign(lbl_shortcuts_sec, GTK_ALIGN_START);
   gtk_widget_add_css_class(lbl_shortcuts_sec, "entity-header");
   gtk_widget_set_margin_top(lbl_shortcuts_sec, 12);
   gtk_box_append(GTK_BOX(box), lbl_shortcuts_sec);

   champ_texte_initialiser(&ui->settings_txt_shortcut_play);
   champ_texte_set_texte(&ui->settings_txt_shortcut_play, ui->shortcut_play);
   add_form_row(box, "Lecture / Pause :", champ_texte_creer(&ui->settings_txt_shortcut_play));

   champ_texte_initialiser(&ui->settings_txt_shortcut_zen);
   champ_texte_set_texte(&ui->settings_txt_shortcut_zen, ui->shortcut_zen);
   add_form_row(box, "Mode Zen :", champ_texte_creer(&ui->settings_txt_shortcut_zen));

   champ_texte_initialiser(&ui->settings_txt_shortcut_debug);
   champ_texte_set_texte(&ui->settings_txt_shortcut_debug, ui->shortcut_debug);
   add_form_row(box, "Mode Débogage :", champ_texte_creer(&ui->settings_txt_shortcut_debug));

   champ_texte_initialiser(&ui->settings_txt_shortcut_settings);
   champ_texte_set_texte(&ui->settings_txt_shortcut_settings, ui->shortcut_settings);
   add_form_row(box, "Paramètres :", champ_texte_creer(&ui->settings_txt_shortcut_settings));

   champ_texte_initialiser(&ui->settings_txt_shortcut_add);
   champ_texte_set_texte(&ui->settings_txt_shortcut_add, ui->shortcut_add);
   add_form_row(box, "Ajouter un poisson :", champ_texte_creer(&ui->settings_txt_shortcut_add));

   champ_texte_initialiser(&ui->settings_txt_shortcut_sidebar);
   champ_texte_set_texte(&ui->settings_txt_shortcut_sidebar, ui->shortcut_sidebar);
   add_form_row(box, "Afficher/Masquer barre latérale :", champ_texte_creer(&ui->settings_txt_shortcut_sidebar));

   champ_texte_initialiser(&ui->settings_txt_shortcut_restart);
   champ_texte_set_texte(&ui->settings_txt_shortcut_restart, ui->shortcut_restart);
   add_form_row(box, "Réinitialiser la simulation :", champ_texte_creer(&ui->settings_txt_shortcut_restart));

   // Wrap settings layout inside a scrolled window for clean vertical sizing
   GtkWidget *scroll = gtk_scrolled_window_new();
   gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
   gtk_scrolled_window_set_propagate_natural_height(GTK_SCROLLED_WINDOW(scroll), TRUE);
   gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(scroll), 480);
   gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), box);

    dialog_set_contenu(&ui->settings_dialog, scroll);
    dialog_creer(&ui->settings_dialog);

    g_signal_connect(ui->settings_dialog.window, "destroy", G_CALLBACK(on_settings_dialog_destroy), ui);

    dialog_afficher(&ui->settings_dialog);
}

typedef struct
{
   BassinUI *ui;
   Dialog *dialog;
   Slider slider_count;
   BoutonChecklist chk_predators;
   BoutonChecklist chk_allies;
   BoutonChecklist chk_bancs;
} RandomLoadCtx;

static void on_random_load_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   RandomLoadCtx *ctx = user_data;
   if (ctx)
   {
      slider_free(&ctx->slider_count);

      g_free(ctx->chk_predators.label);
      g_free(ctx->chk_predators.tooltip);
      free(ctx->chk_predators.id_css);
      free(ctx->chk_predators.style.couleur_texte);
      free(ctx->chk_predators.style.couleur_texte_hover);

      g_free(ctx->chk_allies.label);
      g_free(ctx->chk_allies.tooltip);
      free(ctx->chk_allies.id_css);
      free(ctx->chk_allies.style.couleur_texte);
      free(ctx->chk_allies.style.couleur_texte_hover);

      g_free(ctx->chk_bancs.label);
      g_free(ctx->chk_bancs.tooltip);
      free(ctx->chk_bancs.id_css);
      free(ctx->chk_bancs.style.couleur_texte);
      free(ctx->chk_bancs.style.couleur_texte_hover);

      g_free(ctx);
   }
}

static void on_random_load_reponse(int reponse, gpointer user_data)
{
   RandomLoadCtx *ctx = user_data;
   if (!ctx)
      return;

   if (reponse == DIALOG_REPONSE_OK)
   {
      BassinUI *ui = ctx->ui;

      // 1. Get options
      int total_count = (int)slider_get_valeur(&ctx->slider_count);
      gboolean include_preds = (bouton_checklist_get_etat(&ctx->chk_predators) == CHECKLIST_CHECKED);
      gboolean include_allies = (bouton_checklist_get_etat(&ctx->chk_allies) == CHECKLIST_CHECKED);
      gboolean group_in_bancs = (bouton_checklist_get_etat(&ctx->chk_bancs) == CHECKLIST_CHECKED);

      // 2. Clear current basin (Vider)
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
      ui->elapsed_time = 0.0;

      // 3. Build list of allowed species configs
      GList *allowed_species = NULL;
      for (GList *l = ui->species_configs; l; l = l->next)
      {
         SpeciesConfig *cfg = l->data;
         gboolean ok = FALSE;
         if (strcmp(cfg->type, "prey") == 0)
         {
            ok = TRUE;
         }
         else if (strcmp(cfg->type, "predator") == 0 && include_preds)
         {
            ok = TRUE;
         }
         else if (strcmp(cfg->type, "ally") == 0 && include_allies)
         {
            ok = TRUE;
         }

         if (ok)
         {
            allowed_species = g_list_append(allowed_species, cfg);
         }
      }

      int allowed_count = g_list_length(allowed_species);
      if (allowed_count > 0)
      {
         int spawned = 0;
         while (spawned < total_count)
         {
            // Pick a random species from allowed list
            int rand_idx = rand() % allowed_count;
            SpeciesConfig *cfg = g_list_nth_data(allowed_species, rand_idx);

            if (strcmp(cfg->type, "prey") == 0 && group_in_bancs)
            {
               // Spawn as a school of size 3 to 5 (or remaining target count)
               int banc_size = 3 + (rand() % 3); // 3, 4, or 5
               if (spawned + banc_size > total_count)
               {
                  banc_size = total_count - spawned;
               }

               if (banc_size > 0)
               {
                  ui->num_bancs++;
                  for (int j = 0; j < banc_size; j++)
                  {
                     add_fish_programmatic(ui, cfg->nom, TRUE, ui->num_bancs);
                  }
                  spawned += banc_size;
               }
            }
            else
            {
               // Spawn individual fish (solo)
               add_fish_programmatic(ui, cfg->nom, FALSE, -1);
               spawned++;
            }
         }
      }

      if (allowed_species)
      {
         g_list_free(allowed_species);
      }

      // Update sidebar and status bar
      update_sidebar_list(ui);
      update_status_bar(ui);

      // Play sound effect
      sound_play(SOUND_SPLASH);
   }

   dialog_fermer(ctx->dialog);
}

void open_random_load_dialog(BassinUI *ui)
{
   RandomLoadCtx *ctx = g_new0(RandomLoadCtx, 1);
   ctx->ui = ui;

   ctx->dialog = g_new0(Dialog, 1);
   dialog_initialiser(ctx->dialog);
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
   {
      ctx->dialog->parent = GTK_WINDOW(toplevel);
   }
   dialog_set_titre(ctx->dialog, "🎲 Générer aléatoirement");
   ctx->dialog->boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
   ctx->dialog->on_reponse = on_random_load_reponse;
   ctx->dialog->user_data = ctx;

   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   // Slider: Nombre de poissons
   GtkWidget *lbl_count = gtk_label_new("Nombre de poissons à générer :");
   gtk_widget_set_halign(lbl_count, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_count);

   slider_initialiser(&ctx->slider_count);
   slider_set_bornes(&ctx->slider_count, 5, 50);
   slider_set_valeur(&ctx->slider_count, 15);
   slider_set_digits(&ctx->slider_count, 0);
   slider_set_afficher_valeur(&ctx->slider_count, TRUE);
   GtkWidget *w_sld = slider_creer(&ctx->slider_count);
   gtk_box_append(GTK_BOX(box), w_sld);

   // Checklist checkboxes
   GtkWidget *lbl_opts = gtk_label_new("Options de peuplement :");
   gtk_widget_set_halign(lbl_opts, GTK_ALIGN_START);
   gtk_widget_add_css_class(lbl_opts, "entity-header");
   gtk_box_append(GTK_BOX(box), lbl_opts);

   bouton_checklist_initialiser(&ctx->chk_predators);
   g_free(ctx->chk_predators.label);
   ctx->chk_predators.label = g_strdup("Inclure des prédateurs (Requins, etc.)");
   ctx->chk_predators.etat = CHECKLIST_CHECKED;
   GtkWidget *w_chk_pred = bouton_checklist_creer(&ctx->chk_predators);
   gtk_box_append(GTK_BOX(box), w_chk_pred);

   bouton_checklist_initialiser(&ctx->chk_allies);
   g_free(ctx->chk_allies.label);
   ctx->chk_allies.label = g_strdup("Inclure des alliés (Dauphins, etc.)");
   ctx->chk_allies.etat = CHECKLIST_CHECKED;
   GtkWidget *w_chk_allies = bouton_checklist_creer(&ctx->chk_allies);
   gtk_box_append(GTK_BOX(box), w_chk_allies);

   bouton_checklist_initialiser(&ctx->chk_bancs);
   g_free(ctx->chk_bancs.label);
   ctx->chk_bancs.label = g_strdup("Former des bancs pour les proies");
   ctx->chk_bancs.etat = CHECKLIST_CHECKED;
   GtkWidget *w_chk_bancs = bouton_checklist_creer(&ctx->chk_bancs);
   gtk_box_append(GTK_BOX(box), w_chk_bancs);

   dialog_set_contenu(ctx->dialog, box);
   dialog_creer(ctx->dialog);
   dialog_afficher(ctx->dialog);

   g_signal_connect_swapped(ctx->dialog->window, "destroy", G_CALLBACK(dialog_free), ctx->dialog);
   g_signal_connect(ctx->dialog->window, "destroy", G_CALLBACK(on_random_load_destroy), ctx);
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

static void on_modify_fish_species_clicked(GtkButton *btn, gpointer user_data)
{
   (void)user_data;
   BassinUI *ui = g_object_get_data(G_OBJECT(btn), "ui");
   Poisson *p = g_object_get_data(G_OBJECT(btn), "poisson");
   Dialog *dialog = g_object_get_data(G_OBJECT(btn), "dialog");

   if (ui && p)
   {
      const char *species_name = p->nom;
      if (dialog)
      {
         dialog_fermer(dialog);
      }
      open_create_species_dialog(ui, species_name);
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
         GtkWidget *btn_prey_detail = creer_bouton_custom("🔍 Détails", NULL, ICONE_GAUCHE, "btn_prey_details", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_prey_details_clicked, ui);
         g_object_set_data(G_OBJECT(btn_prey_detail), "species_name", (gpointer)prey_name);
         gtk_grid_attach(GTK_GRID(grid), btn_prey_detail, 3, r, 1, 1);
      }

      gtk_box_append(GTK_BOX(box), grid);
   }

   // Control button
   GtkWidget *btn_control = creer_bouton_custom("🕹️ Prendre le contrôle", NULL, ICONE_GAUCHE, "btn_control_fish", BOUTON_STYLE_SUGGESTED, (BoutonAction)on_control_fish_clicked, NULL);
   gtk_widget_set_margin_top(btn_control, 10);
   g_object_set_data(G_OBJECT(btn_control), "ui", ui);
   g_object_set_data(G_OBJECT(btn_control), "poisson", p);
   g_object_set_data(G_OBJECT(btn_control), "dialog", details);
   gtk_box_append(GTK_BOX(box), btn_control);

   // Modify species button
   GtkWidget *btn_modify = creer_bouton_custom("⚙️ Modifier l'espèce", NULL, ICONE_GAUCHE, "btn_modify_species", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_modify_fish_species_clicked, NULL);
   gtk_widget_set_margin_top(btn_modify, 5);
   g_object_set_data(G_OBJECT(btn_modify), "ui", ui);
   g_object_set_data(G_OBJECT(btn_modify), "poisson", p);
   g_object_set_data(G_OBJECT(btn_modify), "dialog", details);
   gtk_box_append(GTK_BOX(box), btn_modify);

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
