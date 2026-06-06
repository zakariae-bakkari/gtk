#include "bassin_private.h"
#include <stdlib.h>
#include <string.h>
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

static void on_create_species_dialog_destroy(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   SpeciesCreationCtx *ctx = (SpeciesCreationCtx *)user_data;
   if (!ctx)
      return;

   champ_texte_free(&ctx->champ_nom);
   champ_select_free(&ctx->champ_type);
   champ_nombre_free(&ctx->champ_level);
   champ_nombre_free(&ctx->champ_size);
   champ_nombre_free(&ctx->champ_detection);
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
      g_free(chk->id_css);
      g_free(chk);
   }
   g_list_free(ctx->diet_checklist_items);

   g_free(ctx);
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

      // Ensure name is unique to avoid collisions
      char unique_name[128];
      strcpy(unique_name, name);
      int suffix = 2;
      while (find_species_config(ui, unique_name))
      {
         snprintf(unique_name, sizeof(unique_name), "%s-%d", name, suffix++);
      }

      SpeciesConfig *cfg = calloc(1, sizeof(SpeciesConfig));
      cfg->nom = strdup(unique_name);

      const char *type_str = champ_select_get_string(&ctx->champ_type);
      cfg->type = strdup(type_str ? type_str : "prey");

      cfg->level = (int)champ_nombre_get_valeur(&ctx->champ_level);
      cfg->taille = (int)champ_nombre_get_valeur(&ctx->champ_size);
      cfg->perimetre_detection = (int)champ_nombre_get_valeur(&ctx->champ_detection);

      cfg->vitesse_normale = champ_nombre_get_valeur(&ctx->champ_speed_norm);
      cfg->vitesse_fuite = champ_nombre_get_valeur(&ctx->champ_speed_escape);
      cfg->vitesse_ralentie = champ_nombre_get_valeur(&ctx->champ_speed_slow);

      const char *f0 = champ_texte_get_texte(&ctx->champ_frame0);
      const char *f1 = champ_texte_get_texte(&ctx->champ_frame1);
      const char *f2 = champ_texte_get_texte(&ctx->champ_frame2);

      cfg->chemin_frames[0] = strdup(f0 && strlen(f0) > 0 ? f0 : "resources/images/fishes/sardine/frame1.png");
      cfg->chemin_frames[1] = strdup(f1 && strlen(f1) > 0 ? f1 : "resources/images/fishes/sardine/frame2.png");
      cfg->chemin_frames[2] = strdup(f2 && strlen(f2) > 0 ? f2 : "resources/images/fishes/sardine/frame1.png");
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

      // Add to list
      ui->species_configs = g_list_append(ui->species_configs, cfg);

      // Persist to XML
      save_species_configs_to_xml(ui);

      // If "Create and put in bassin", spawn it
      if (reponse == 11)
      {
         add_fish_programmatic(ui, cfg->nom, FALSE, -1);
      }

      update_sidebar_list(ui);
      update_status_bar(ui);
   }
}

void open_create_species_dialog(BassinUI *ui)
{
   SpeciesCreationCtx *ctx = g_new0(SpeciesCreationCtx, 1);
   ctx->ui = ui;

   Dialog *d = g_new0(Dialog, 1);
   dialog_initialiser(d);
   ctx->dialog = d;

   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
   {
      d->parent = GTK_WINDOW(toplevel);
   }

   dialog_set_titre(d, "✨ Créer une Nouvelle Espèce");

   // Custom buttons
   dialog_ajouter_bouton(d, "Annuler", NULL, DIALOG_REPONSE_ANNULER, FALSE);
   dialog_ajouter_bouton(d, "Créer", "document-save-symbolic", 10, FALSE);
   dialog_ajouter_bouton(d, "Créer et mettre dans le bassin", "list-add-symbolic", 11, TRUE);

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
   champ_texte_set_texte(&ctx->champ_nom, "Poisson-perso");
   GtkWidget *w_nom = champ_texte_creer(&ctx->champ_nom);
   add_form_row(box, "Nom de l'espèce :", w_nom);

   // 2. Type
   champ_select_initialiser(&ctx->champ_type);
   champ_select_add_item(&ctx->champ_type, "prey");
   champ_select_add_item(&ctx->champ_type, "predator");
   champ_select_add_item(&ctx->champ_type, "ally");
   champ_select_set_index(&ctx->champ_type, 0); // prey by default
   GtkWidget *w_type = champ_select_creer(&ctx->champ_type);
   add_form_row(box, "Type (comportement) :", w_type);

   // 3. Niveau
   champ_nombre_initialiser(&ctx->champ_level);
   champ_nombre_set_bornes(&ctx->champ_level, 1, 5);
   champ_nombre_set_valeur(&ctx->champ_level, 1);
   champ_nombre_set_digits(&ctx->champ_level, 0);
   GtkWidget *w_level = champ_nombre_creer(&ctx->champ_level);
   add_form_row(box, "Niveau de force (1-5) :", w_level);

   // 4. Taille
   champ_nombre_initialiser(&ctx->champ_size);
   champ_nombre_set_bornes(&ctx->champ_size, 10, 300);
   champ_nombre_set_valeur(&ctx->champ_size, 64);
   champ_nombre_set_digits(&ctx->champ_size, 0);
   GtkWidget *w_size = champ_nombre_creer(&ctx->champ_size);
   add_form_row(box, "Taille par défaut (pixels) :", w_size);

   // 5. Périmètre détection
   champ_nombre_initialiser(&ctx->champ_detection);
   champ_nombre_set_bornes(&ctx->champ_detection, 10, 1000);
   champ_nombre_set_valeur(&ctx->champ_detection, 100);
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
   champ_nombre_set_valeur(&ctx->champ_speed_norm, 50);
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
   champ_nombre_set_valeur(&ctx->champ_speed_escape, 80);
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
   champ_nombre_set_valeur(&ctx->champ_speed_slow, 25);
   champ_nombre_set_digits(&ctx->champ_speed_slow, 0);
   gtk_box_append(GTK_BOX(col_slow), lbl_slow);
   gtk_box_append(GTK_BOX(col_slow), champ_nombre_creer(&ctx->champ_speed_slow));
   gtk_box_append(GTK_BOX(speeds_box), col_slow);
   gtk_widget_set_hexpand(col_slow, TRUE);

   add_form_row(box, "Vitesses de déplacement (px/s) :", speeds_box);

   // 7. Animation Frames
   GtkWidget *frames_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

   champ_texte_initialiser(&ctx->champ_frame0);
   ctx->champ_frame0.placeholder = "Frame 1 path";
   champ_texte_set_texte(&ctx->champ_frame0, "resources/images/fishes/sardine/frame1.png");
   gtk_box_append(GTK_BOX(frames_box), champ_texte_creer(&ctx->champ_frame0));

   champ_texte_initialiser(&ctx->champ_frame1);
   ctx->champ_frame1.placeholder = "Frame 2 path";
   champ_texte_set_texte(&ctx->champ_frame1, "resources/images/fishes/sardine/frame2.png");
   gtk_box_append(GTK_BOX(frames_box), champ_texte_creer(&ctx->champ_frame1));

   champ_texte_initialiser(&ctx->champ_frame2);
   ctx->champ_frame2.placeholder = "Frame 3 path";
   champ_texte_set_texte(&ctx->champ_frame2, "resources/images/fishes/sardine/frame1.png");
   gtk_box_append(GTK_BOX(frames_box), champ_texte_creer(&ctx->champ_frame2));

   add_form_row(box, "Chemins des images d'animation (Frames 1, 2, 3) :", frames_box);

   // 8. Régime alimentaire
   GtkWidget *diet_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
   ctx->diet_checklist_items = NULL;

   for (GList *l = ui->species_configs; l; l = l->next)
   {
      SpeciesConfig *other_cfg = l->data;
      BoutonChecklist *chk = g_new0(BoutonChecklist, 1);
      bouton_checklist_initialiser(chk);
      chk->label = g_strdup(other_cfg->nom);
      chk->etat = CHECKLIST_UNCHECKED;

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
   open_create_species_dialog(ui);
}

void on_add_poisson_btn_clicked(GtkWidget *widget, gpointer user_data)
{
   BassinUI *ui = user_data;

   GtkWidget *popover = gtk_popover_new();
   gtk_widget_set_parent(GTK_WIDGET(popover), GTK_WIDGET(widget));
   gtk_popover_set_autohide(GTK_POPOVER(popover), TRUE);

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
         GtkWidget *btn_sp = gtk_button_new_with_label(label_buf);
         g_object_set_data(G_OBJECT(btn_sp), "ui", ui);
         g_object_set_data(G_OBJECT(btn_sp), "popover", popover);
         g_signal_connect(btn_sp, "clicked", G_CALLBACK(on_popover_species_selected), (gpointer)cfg->nom);
         gtk_box_append(GTK_BOX(box), btn_sp);
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
         GtkWidget *btn_sp = gtk_button_new_with_label(label_buf);
         g_object_set_data(G_OBJECT(btn_sp), "ui", ui);
         g_object_set_data(G_OBJECT(btn_sp), "popover", popover);
         g_signal_connect(btn_sp, "clicked", G_CALLBACK(on_popover_species_selected), (gpointer)cfg->nom);
         gtk_box_append(GTK_BOX(box), btn_sp);
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

   dialog_set_contenu(&ui->settings_dialog, box);
   dialog_creer(&ui->settings_dialog);
   dialog_afficher(&ui->settings_dialog);
}
