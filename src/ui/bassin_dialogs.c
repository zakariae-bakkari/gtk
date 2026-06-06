#include "bassin_private.h"
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>
#include "../../widgets/headers/champ_texte.h"
#include "../../widgets/headers/champ_nombre.h"
#include "../../widgets/headers/bouton_checklist.h"

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
   p->taille = ui->config_fish_size > 0 ? ui->config_fish_size : 64;

   SpeciesConfig *cfg = find_species_config(ui, species);
   if (cfg)
   {
      p->vitesse_normale = cfg->vitesse_normale;
      p->vitesse_fuite = cfg->vitesse_fuite;
      p->vitesse_ralentie = cfg->vitesse_ralentie;
      double scale = 1.0;
      if (strcmp(cfg->type, "predator") == 0)
         scale = 1.8;
      else if (strcmp(cfg->type, "ally") == 0)
         scale = 1.5;
      p->taille = (ui->config_fish_size > 0 ? ui->config_fish_size : cfg->taille) * scale;
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
   double rx = 100 + (rand() % (ui->config_canvas_width - 200));
   double ry = 100 + (rand() % (ui->config_canvas_height - 200));
   poisson_set_position(p, rx, ry);

   // Random velocities
   double angle_rad = (rand() % 360) * M_PI / 180.0;
   p->vx = cos(angle_rad) * p->vitesse_normale;
   p->vy = sin(angle_rad) * p->vitesse_normale;

   // Create the widget representation using the coordinator helper
   create_poisson_widget(ui, p);

   ui->poissons = g_list_prepend(ui->poissons, p);
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
            for (int b_id = 1; b_id <= ui->num_bancs; b_id++)
            {
               int count = 0;
               for (GList *l = ui->poissons; l; l = l->next)
               {
                  Poisson *p = l->data;
                  if (p->id_banc == b_id)
                     count++;
               }
               if (count > 0)
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

static void safe_free_widget_style(WidgetStyle *style)
{
   if (!style)
      return;
   free(style->bg_normal);
   free(style->fg_normal);
   free(style->couleur_bordure);
   free(style->couleur_bordure_error);
   free(style->bg_error);
}

static void safe_free_champ_texte(ChampTexte *cfg)
{
   free(cfg->id_css);
   free(cfg->placeholder);
   free(cfg->erreur_couleur);
   safe_free_widget_style(&cfg->style);
}

static void safe_free_champ_select(ChampSelect *cfg)
{
   free(cfg->id_css);
   if (cfg->model)
   {
      g_object_unref(cfg->model);
   }
   safe_free_widget_style(&cfg->style);
}

static void safe_free_champ_nombre(ChampNombre *cfg)
{
   free(cfg->id_css);
   safe_free_widget_style(&cfg->style);
}

static void on_create_species_dialog_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   SpeciesCreationCtx *ctx = (SpeciesCreationCtx *)user_data;
   if (!ctx)
      return;

   g_free(ctx->edit_species_name);

   safe_free_champ_texte(&ctx->champ_nom);
   safe_free_champ_select(&ctx->champ_type);
   safe_free_champ_nombre(&ctx->champ_level);
   safe_free_champ_nombre(&ctx->champ_size);
   safe_free_champ_nombre(&ctx->champ_detection);
   safe_free_champ_nombre(&ctx->champ_speed_norm);
   safe_free_champ_nombre(&ctx->champ_speed_escape);
   safe_free_champ_nombre(&ctx->champ_speed_slow);
   safe_free_champ_texte(&ctx->champ_frame0);
   safe_free_champ_texte(&ctx->champ_frame1);
   safe_free_champ_texte(&ctx->champ_frame2);

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

      cfg->vitesse_normale = champ_nombre_get_valeur(&ctx->champ_speed_norm);
      cfg->vitesse_fuite = champ_nombre_get_valeur(&ctx->champ_speed_escape);
      cfg->vitesse_ralentie = champ_nombre_get_valeur(&ctx->champ_speed_slow);

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
               p->taille = cfg->taille;
               p->perimetre_detection = cfg->perimetre_detection;

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

   GtkWidget *btn_browse = gtk_button_new_from_icon_name("folder-open-symbolic");
   gtk_widget_set_tooltip_text(btn_browse, "Parcourir...");
   g_object_set_data(G_OBJECT(btn_browse), "parent_win", parent_win);
   g_signal_connect(btn_browse, "clicked", G_CALLBACK(on_browse_frame_clicked), champ);
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
   ctx->champ_nom.placeholder = "Ex: Thon, Dorade...";
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
   champ_nombre_set_bornes(&ctx->champ_size, 10, 300);
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
         
         GtkWidget *btn_sp = gtk_button_new_with_label(label_buf);
         gtk_widget_set_hexpand(btn_sp, TRUE);
         g_object_set_data(G_OBJECT(btn_sp), "ui", ui);
         g_object_set_data(G_OBJECT(btn_sp), "popover", popover);
         g_signal_connect(btn_sp, "clicked", G_CALLBACK(on_popover_species_selected), (gpointer)cfg->nom);
         gtk_box_append(GTK_BOX(row), btn_sp);

         GtkWidget *btn_info = gtk_button_new_from_icon_name("help-about-symbolic");
         gtk_widget_add_css_class(btn_info, "flat");
         gtk_widget_set_tooltip_text(btn_info, "Détails de l'espèce");
         g_object_set_data(G_OBJECT(btn_info), "ui", ui);
         g_object_set_data(G_OBJECT(btn_info), "popover", popover);
         g_signal_connect(btn_info, "clicked", G_CALLBACK(on_popover_details_clicked), (gpointer)cfg->nom);
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
         
         GtkWidget *btn_sp = gtk_button_new_with_label(label_buf);
         gtk_widget_set_hexpand(btn_sp, TRUE);
         g_object_set_data(G_OBJECT(btn_sp), "ui", ui);
         g_object_set_data(G_OBJECT(btn_sp), "popover", popover);
         g_signal_connect(btn_sp, "clicked", G_CALLBACK(on_popover_species_selected), (gpointer)cfg->nom);
         gtk_box_append(GTK_BOX(row), btn_sp);

         GtkWidget *btn_info = gtk_button_new_from_icon_name("help-about-symbolic");
         gtk_widget_add_css_class(btn_info, "flat");
         gtk_widget_set_tooltip_text(btn_info, "Détails de l'espèce");
         g_object_set_data(G_OBJECT(btn_info), "ui", ui);
         g_object_set_data(G_OBJECT(btn_info), "popover", popover);
         g_signal_connect(btn_info, "clicked", G_CALLBACK(on_popover_details_clicked), (gpointer)cfg->nom);
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
   GtkWidget *btn_create_new = gtk_button_new_with_label("✨ Create new one");
   gtk_widget_add_css_class(btn_create_new, "principal");
   g_object_set_data(G_OBJECT(btn_create_new), "popover", popover);
   g_signal_connect(btn_create_new, "clicked", G_CALLBACK(on_create_new_species_clicked), ui);
   gtk_box_append(GTK_BOX(box), btn_create_new);

   gtk_popover_set_child(GTK_POPOVER(popover), box);
   gtk_popover_popup(GTK_POPOVER(popover));
}

static void on_settings_reponse(int reponse, gpointer user_data)
{
   BassinUI *ui = user_data;
   if (reponse == DIALOG_REPONSE_OK)
   {
      int new_fish_size = (int)slider_get_valeur(&ui->settings_sld_fish_size);
      ui->config_fish_size = new_fish_size;

      int bg_idx = champ_select_get_index(&ui->settings_sel_bg);
      if (bg_idx == 0)
      {
         ui->config_bg_path = "resources/images/background_banc.png";
      }
      else
      {
         ui->config_bg_path = "resources/images/background2.png";
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
         gtk_picture_set_filename(GTK_PICTURE(ui->bg_widget), ui->config_bg_path);
         gtk_widget_set_size_request(ui->bg_widget, ui->config_canvas_width, ui->config_canvas_height);
      }

      ui->config_hide_health_bar = gtk_switch_get_active(GTK_SWITCH(ui->settings_sw_hide_health));
      ui->config_hide_fish_name = gtk_switch_get_active(GTK_SWITCH(ui->settings_sw_hide_name));
      ui->config_hide_status_bar = gtk_switch_get_active(GTK_SWITCH(ui->settings_sw_hide_status));

      apply_fish_visibility_configs(ui);
      if (!ui->zen_mode)
      {
         gtk_widget_set_visible(ui->status_bar_widget, !ui->config_hide_status_bar);
         gtk_widget_set_visible(ui->sep_bottom_widget, !ui->config_hide_status_bar);
      }

      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         double scale = 1.0;
         SpeciesConfig *cfg = find_species_config(ui, p->nom);
         if (cfg)
         {
            if (strcmp(cfg->type, "predator") == 0)
               scale = 1.8;
            else if (strcmp(cfg->type, "ally") == 0)
               scale = 1.5;
         }
         p->taille = ui->config_fish_size * scale;
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
   }

   dialog_fermer(&ui->settings_dialog);
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

   GtkWidget *lbl_size = gtk_label_new("Taille des poissons (pixels)");
   gtk_widget_set_halign(lbl_size, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_size);

   slider_initialiser(&ui->settings_sld_fish_size);
   slider_set_bornes(&ui->settings_sld_fish_size, 32, 128);
   slider_set_valeur(&ui->settings_sld_fish_size, ui->config_fish_size > 0 ? ui->config_fish_size : 64);
   slider_set_digits(&ui->settings_sld_fish_size, 0);
   slider_set_afficher_valeur(&ui->settings_sld_fish_size, TRUE);
   GtkWidget *w_sld_size = slider_creer(&ui->settings_sld_fish_size);
   gtk_box_append(GTK_BOX(box), w_sld_size);

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

   dialog_set_contenu(&ui->settings_dialog, box);
   dialog_creer(&ui->settings_dialog);
   dialog_afficher(&ui->settings_dialog);
}
