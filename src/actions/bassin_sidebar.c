#include "../simulation/bassin_private.h"
#include "../../widgets/headers/bouton.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

static GtkWidget *creer_bouton_custom(const char *texte, const char *id_css, BoutonPresetStyle preset, BoutonAction on_clic, gpointer user_data)
{
   Bouton *b = g_new0(Bouton, 1);
   bouton_initialiser(b);
   g_free(b->texte);
   b->texte = strdup(texte);
   g_free(b->id_css);
   b->id_css = strdup(id_css);
   bouton_appliquer_preset(b, preset);
   b->on_clic = on_clic;
   b->user_data = user_data;
   GtkWidget *w = bouton_creer(b);
   g_signal_connect(w, "destroy", G_CALLBACK(on_custom_bouton_destroy), b);
   return w;
}

void on_toggle_sidebar_clicked(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (!ui || !ui->sidebar)
      return;

   gboolean is_visible = gtk_widget_get_visible(ui->sidebar);
   gtk_widget_set_visible(ui->sidebar, !is_visible);
}

static void on_sidebar_item_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
   (void)n_press;
   (void)x;
   (void)y;
   (void)user_data;
   Poisson *p = g_object_get_data(G_OBJECT(gesture), "poisson");
   BassinUI *ui = g_object_get_data(G_OBJECT(gesture), "ui");
   if (p && ui)
   {
      GList *all_poissons = bassin_get_all_poissons(ui);
      for (GList *l = all_poissons; l; l = l->next)
      {
         Poisson *other = l->data;
         GtkWidget *img_widget = get_fish_picture_widget(other);
         if (img_widget)
         {
            gtk_widget_remove_css_class(img_widget, "fish-selected");
         }
      }
      g_list_free(all_poissons);

      GtkWidget *img_widget = get_fish_picture_widget(p);
      if (img_widget)
      {
         gtk_widget_add_css_class(img_widget, "fish-selected");
      }
      show_fish_details_dialog(ui, p);
   }
}

void on_tab_entites_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   BassinUI *ui = user_data;
   ui->active_tab = 0;
   gtk_widget_add_css_class(ui->btn_tab_entites, "tab-active");
   gtk_widget_remove_css_class(ui->btn_tab_entites, "tab-inactive");
   gtk_widget_add_css_class(ui->btn_tab_bancs, "tab-inactive");
   gtk_widget_remove_css_class(ui->btn_tab_bancs, "tab-active");

   update_sidebar_list(ui);
}

void on_tab_bancs_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   BassinUI *ui = user_data;
   ui->active_tab = 1;
   gtk_widget_add_css_class(ui->btn_tab_bancs, "tab-active");
   gtk_widget_remove_css_class(ui->btn_tab_bancs, "tab-inactive");
   gtk_widget_add_css_class(ui->btn_tab_entites, "tab-inactive");
   gtk_widget_remove_css_class(ui->btn_tab_entites, "tab-active");

   update_sidebar_list(ui);
}

void update_sidebar_list(BassinUI *ui)
{
   GtkWidget *child = gtk_widget_get_first_child(ui->box_sidebar_content);
   while (child)
   {
      GtkWidget *next = gtk_widget_get_next_sibling(child);
      gtk_box_remove(GTK_BOX(ui->box_sidebar_content), child);
      child = next;
   }

   if (ui->active_tab == 0) // Tab: Entités
   {
      // 1. Group by schools (bancs)
      for (GList *lb = ui->bancs; lb; lb = lb->next)
      {
         Banc *b = lb->data;
         int count = g_list_length(b->poissons);
         if (count > 0)
         {
            char header_buf[128];
            sprintf(header_buf, "BANC %sS", b->nom_espece);

            GtkWidget *lbl_header = gtk_label_new(header_buf);
            gtk_widget_set_halign(lbl_header, GTK_ALIGN_START);
            gtk_widget_add_css_class(lbl_header, "entity-header");
            gtk_box_append(GTK_BOX(ui->box_sidebar_content), lbl_header);

            for (GList *m = b->poissons; m; m = m->next)
            {
               Poisson *p = m->data;
               GtkWidget *item = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
               gtk_widget_add_css_class(item, "entity-item");

               const char *frame_path = p->chemin_frames[0] ? p->chemin_frames[0] : "resources/kenney_fish-pack_2.0/PNG/Default/fish_blue.png";
               GtkWidget *img = gtk_picture_new_for_filename(frame_path);
               gtk_widget_set_size_request(img, 24, 24);
               gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(img), TRUE);
               gtk_box_append(GTK_BOX(item), img);

               char name_buf[128];
               const char *short_nom = strcmp(p->nom, "Poisson clown") == 0 ? "Clown" : (strcmp(p->nom, "Poisson-globe") == 0 ? "Globe" : "Hareng");
               sprintf(name_buf, "%s #%d", short_nom, p->id);

               GtkWidget *lbl_name = gtk_label_new(name_buf);
               gtk_box_append(GTK_BOX(item), lbl_name);

               GtkWidget *lbl_role = gtk_label_new(p->est_leader ? "Leader" : "Membre");
               gtk_widget_set_halign(lbl_role, GTK_ALIGN_END);
               gtk_widget_set_hexpand(lbl_role, TRUE);
               gtk_box_append(GTK_BOX(item), lbl_role);

               GtkWidget *lbl_badge = gtk_label_new("banc");
               gtk_widget_add_css_class(lbl_badge, "badge-banc");
               gtk_box_append(GTK_BOX(item), lbl_badge);

               GtkGesture *sidebar_click = gtk_gesture_click_new();
               g_object_set_data(G_OBJECT(sidebar_click), "poisson", p);
               g_object_set_data(G_OBJECT(sidebar_click), "ui", ui);
               g_signal_connect(sidebar_click, "released", G_CALLBACK(on_sidebar_item_clicked), NULL);
               gtk_widget_add_controller(item, GTK_EVENT_CONTROLLER(sidebar_click));

               gtk_box_append(GTK_BOX(ui->box_sidebar_content), item);
            }
         }
      }

      // 2. Individuals (autonomous only)
      gboolean has_indiv_header = FALSE;
      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         if (!is_predator(ui, p) && !is_ally(ui, p))
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

            const char *frame_path = p->chemin_frames[0] ? p->chemin_frames[0] : "resources/kenney_fish-pack_2.0/PNG/Default/fish_blue.png";
            GtkWidget *img = gtk_picture_new_for_filename(frame_path);
            gtk_widget_set_size_request(img, 24, 24);
            gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(img), TRUE);
            gtk_box_append(GTK_BOX(item), img);

            char name_buf[128];
            const char *short_nom = strcmp(p->nom, "Poisson clown") == 0 ? "Clown" : (strcmp(p->nom, "Poisson-globe") == 0 ? "Globe" : "Hareng");
            sprintf(name_buf, "%s #%d", short_nom, p->id);

            GtkWidget *lbl_name = gtk_label_new(name_buf);
            gtk_box_append(GTK_BOX(item), lbl_name);

            GtkWidget *lbl_role = gtk_label_new("Solo");
            gtk_widget_set_halign(lbl_role, GTK_ALIGN_END);
            gtk_widget_set_hexpand(lbl_role, TRUE);
            gtk_box_append(GTK_BOX(item), lbl_role);

            GtkWidget *lbl_badge = gtk_label_new("solo");
            gtk_widget_add_css_class(lbl_badge, "badge-solo");
            gtk_box_append(GTK_BOX(item), lbl_badge);

            GtkGesture *sidebar_click = gtk_gesture_click_new();
            g_object_set_data(G_OBJECT(sidebar_click), "poisson", p);
            g_object_set_data(G_OBJECT(sidebar_click), "ui", ui);
            g_signal_connect(sidebar_click, "released", G_CALLBACK(on_sidebar_item_clicked), NULL);
            gtk_widget_add_controller(item, GTK_EVENT_CONTROLLER(sidebar_click));

            gtk_box_append(GTK_BOX(ui->box_sidebar_content), item);
         }
      }

      // 3. Predators & Allies
      gboolean has_pred_header = FALSE;
      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         if (is_predator(ui, p) || is_ally(ui, p))
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

            const char *frame_path = p->chemin_frames[0] ? p->chemin_frames[0] : "resources/kenney_fish-pack_2.0/PNG/Default/fish_blue.png";
            GtkWidget *img = gtk_picture_new_for_filename(frame_path);
            gtk_widget_set_size_request(img, 24, 24);
            gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(img), TRUE);
            gtk_box_append(GTK_BOX(item), img);

            char name_buf[128];
            sprintf(name_buf, "%s", p->nom);

            GtkWidget *lbl_name = gtk_label_new(name_buf);
            gtk_box_append(GTK_BOX(item), lbl_name);

            GtkWidget *lbl_role = gtk_label_new(is_predator(ui, p) ? "Chasseur" : "Défenseur");
            gtk_widget_set_halign(lbl_role, GTK_ALIGN_END);
            gtk_widget_set_hexpand(lbl_role, TRUE);
            gtk_box_append(GTK_BOX(item), lbl_role);

            GtkWidget *lbl_badge = gtk_label_new(is_predator(ui, p) ? "pred" : "allie");
            gtk_widget_add_css_class(lbl_badge, "badge-pred");
            gtk_box_append(GTK_BOX(item), lbl_badge);

            GtkGesture *sidebar_click = gtk_gesture_click_new();
            g_object_set_data(G_OBJECT(sidebar_click), "poisson", p);
            g_object_set_data(G_OBJECT(sidebar_click), "ui", ui);
            g_signal_connect(sidebar_click, "released", G_CALLBACK(on_sidebar_item_clicked), NULL);
            gtk_widget_add_controller(item, GTK_EVENT_CONTROLLER(sidebar_click));

            gtk_box_append(GTK_BOX(ui->box_sidebar_content), item);
         }
      }
   }
   else // Tab: Bancs
   {
      GtkWidget *btn_merge = creer_bouton_custom("🔗 Fusionner deux bancs", "btn_merge", BOUTON_STYLE_SUGGESTED, (BoutonAction)on_merge_bancs_clicked, ui);
      gtk_box_append(GTK_BOX(ui->box_sidebar_content), btn_merge);

      GtkWidget *lbl_header = gtk_label_new("LISTE DES BANCS");
      gtk_widget_set_halign(lbl_header, GTK_ALIGN_START);
      gtk_widget_add_css_class(lbl_header, "entity-header");
      gtk_box_append(GTK_BOX(ui->box_sidebar_content), lbl_header);

      for (GList *lb = ui->bancs; lb; lb = lb->next)
      {
         Banc *b = lb->data;
         int count = g_list_length(b->poissons);
         if (count > 0)
         {
            char leader_name[128] = "Aucun";
            if (b->leader)
            {
               const char *short_nom = strcmp(b->leader->nom, "Poisson clown") == 0 ? "Clown" : (strcmp(b->leader->nom, "Poisson-globe") == 0 ? "Globe" : "Hareng");
               sprintf(leader_name, "%s #%d", short_nom, b->leader->id);
            }

            GtkWidget *item = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
            gtk_widget_add_css_class(item, "entity-item");

            const char *emoji = "🐟";
            if (strcmp(b->nom_espece, "Poisson-globe") == 0)
               emoji = "🐡";
            else if (strcmp(b->nom_espece, "Poisson clown") == 0)
               emoji = "🐠";
            else if (strcmp(b->nom_espece, "Dauphin") == 0)
               emoji = "🐬";

            char name_buf[128];
            sprintf(name_buf, "%s Banc de %ss #%d", emoji, b->nom_espece, b->id);
            GtkWidget *lbl_name = gtk_label_new(name_buf);
            gtk_widget_set_halign(lbl_name, GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(item), lbl_name);

            char lead_buf[128];
            sprintf(lead_buf, "★ Leader: %s | Membres: %d", leader_name, count);
            GtkWidget *lbl_lead = gtk_label_new(lead_buf);
            gtk_widget_set_halign(lbl_lead, GTK_ALIGN_START);
            gtk_box_append(GTK_BOX(item), lbl_lead);

            GtkWidget *actions_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
            gtk_widget_set_margin_top(actions_box, 6);

            GtkWidget *btn_dissolve = creer_bouton_custom("🔓 Dissoudre", "btn_dissolve", BOUTON_STYLE_DESTRUCTIVE, (BoutonAction)on_dissolve_banc_clicked, NULL);
            g_object_set_data(G_OBJECT(btn_dissolve), "ui", ui);
            g_object_set_data(G_OBJECT(btn_dissolve), "banc_id", GINT_TO_POINTER(b->id));
            gtk_box_append(GTK_BOX(actions_box), btn_dissolve);

            GtkWidget *btn_split = creer_bouton_custom("✂️ Diviser", "btn_split", BOUTON_STYLE_NEUTRAL, (BoutonAction)on_split_banc_clicked, NULL);
            g_object_set_data(G_OBJECT(btn_split), "ui", ui);
            g_object_set_data(G_OBJECT(btn_split), "banc_id", GINT_TO_POINTER(b->id));
            gtk_box_append(GTK_BOX(actions_box), btn_split);

            gtk_box_append(GTK_BOX(item), actions_box);
            gtk_box_append(GTK_BOX(ui->box_sidebar_content), item);
         }
      }
   }
}

void update_status_bar(BassinUI *ui)
{
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

   GList *all_poissons = bassin_get_all_poissons(ui);
   int active_entities = g_list_length(all_poissons);
   int active_bancs = g_list_length(ui->bancs);
   int active_indiv = 0;
   int active_preds = 0;

   for (GList *l = all_poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (is_predator(ui, p) || is_ally(ui, p))
      {
         active_preds++;
      }
      else if (p->id_banc == -1)
      {
         active_indiv++;
      }
   }
   g_list_free(all_poissons);

   char stats_buf[256];
   sprintf(stats_buf, "%d entités actives   |   %d bancs   |   %d individuel   |   %d prédateur",
           active_entities, active_bancs, active_indiv, active_preds);
   gtk_label_set_text(GTK_LABEL(ui->lbl_stats_text), stats_buf);

   int total_sec = (int)ui->elapsed_time;
   int hours = total_sec / 3600;
   int mins = (total_sec % 3600) / 60;
   int secs = total_sec % 60;
   char time_buf[64];
   sprintf(time_buf, "t = %02d:%02d:%02d", hours, mins, secs);
   gtk_label_set_text(GTK_LABEL(ui->lbl_elapsed_time), time_buf);
}

void on_dissolve_banc_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   (void)user_data;
   BassinUI *ui = g_object_get_data(G_OBJECT(btn), "ui");
   int b_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "banc_id"));

   if (!ui)
      return;

   Banc *b = bassin_find_banc(ui, b_id);
   if (b)
   {
      GList *lp = b->poissons;
      while (lp)
      {
         Poisson *p = lp->data;
         p->id_banc = -1;
         p->est_leader = FALSE;

         if (p->widget_image)
         {
            GtkWidget *lbl_lead = gtk_widget_get_first_child(p->widget_image);
            if (lbl_lead)
               gtk_widget_set_visible(lbl_lead, FALSE);
         }
         ui->poissons = g_list_append(ui->poissons, p);
         lp = lp->next;
      }
      g_list_free(b->poissons);
      free(b->nom_espece);
      ui->bancs = g_list_remove(ui->bancs, b);
      free(b);
   }

   update_sidebar_list(ui);
   update_status_bar(ui);
}

void on_split_banc_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   (void)user_data;
   BassinUI *ui = g_object_get_data(G_OBJECT(btn), "ui");
   int b_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "banc_id"));

   if (!ui)
      return;

   Banc *b = bassin_find_banc(ui, b_id);
   if (!b)
      return;

   int total = g_list_length(b->poissons);
   if (total < 2)
      return;

   ui->num_bancs++;
   int new_banc_id = ui->num_bancs;

   Banc *new_b = calloc(1, sizeof(Banc));
   new_b->id = new_banc_id;
   new_b->nom_espece = strdup(b->nom_espece);

   int half = total / 2;
   int count = 0;

   GList *old_poissons = b->poissons;
   b->poissons = NULL;
   b->leader = NULL;

   for (GList *l = old_poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (count < half)
      {
         p->id_banc = new_banc_id;
         p->est_leader = (count == 0);
         new_b->poissons = g_list_append(new_b->poissons, p);
         if (p->est_leader)
            new_b->leader = p;
      }
      else
      {
         p->est_leader = (count == half);
         b->poissons = g_list_append(b->poissons, p);
         if (p->est_leader)
            b->leader = p;
      }

      if (p->widget_image)
      {
         GtkWidget *lbl_lead = gtk_widget_get_first_child(p->widget_image);
         if (lbl_lead)
            gtk_widget_set_visible(lbl_lead, p->est_leader);
      }
      count++;
   }

   g_list_free(old_poissons);
   ui->bancs = g_list_append(ui->bancs, new_b);

   update_sidebar_list(ui);
   update_status_bar(ui);
}

typedef struct
{
   BassinUI *ui;
   Dialog dialog;
   ChampSelect sel1;
   ChampSelect sel2;
   int *banc_ids;
   int nb_bancs;
} MergeContext;

void on_merge_reponse(int reponse, gpointer user_data)
{
   MergeContext *ctx = user_data;
   BassinUI *ui = ctx->ui;

   if (reponse == DIALOG_REPONSE_OK)
   {
      int idx1 = champ_select_get_index(&ctx->sel1);
      int idx2 = champ_select_get_index(&ctx->sel2);

      if (idx1 >= 0 && idx2 >= 0 && idx1 < ctx->nb_bancs && idx2 < ctx->nb_bancs && idx1 != idx2)
      {
         int b1_id = ctx->banc_ids[idx1];
         int b2_id = ctx->banc_ids[idx2];

         Banc *b1 = bassin_find_banc(ui, b1_id);
         Banc *b2 = bassin_find_banc(ui, b2_id);

         if (b1 && b2)
         {
            GList *lp = b2->poissons;
            while (lp)
            {
               Poisson *p = lp->data;
               p->id_banc = b1->id;
               p->est_leader = FALSE;

               if (p->widget_image)
               {
                  GtkWidget *lbl_lead = gtk_widget_get_first_child(p->widget_image);
                  if (lbl_lead)
                     gtk_widget_set_visible(lbl_lead, FALSE);
               }
               b1->poissons = g_list_append(b1->poissons, p);
               lp = lp->next;
            }
            g_list_free(b2->poissons);
            free(b2->nom_espece);
            ui->bancs = g_list_remove(ui->bancs, b2);
            free(b2);

            update_sidebar_list(ui);
            update_status_bar(ui);
         }
      }
   }

   dialog_fermer(&ctx->dialog);
   free(ctx->banc_ids);
   free(ctx);
}

void on_merge_bancs_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   BassinUI *ui = user_data;

   int active_schools[100];
   int school_counts[100];
   char *school_species[100];
   int count_schools = 0;

   for (GList *l = ui->bancs; l; l = l->next)
   {
      Banc *b = l->data;
      int m_count = g_list_length(b->poissons);
      if (m_count > 0 && count_schools < 100)
      {
         active_schools[count_schools] = b->id;
         school_counts[count_schools] = m_count;
         school_species[count_schools] = b->nom_espece;
         count_schools++;
      }
   }

   if (count_schools < 2)
   {
      GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
      dialog_afficher_erreur(toplevel ? GTK_WINDOW(toplevel) : NULL,
                             "Fusion impossible",
                             "Il doit y avoir au moins 2 bancs actifs dans le bassin pour pouvoir les fusionner.",
                             NULL,
                             NULL);
      return;
   }

   MergeContext *ctx = calloc(1, sizeof(MergeContext));
   ctx->ui = ui;
   ctx->nb_bancs = count_schools;
   ctx->banc_ids = calloc(count_schools, sizeof(int));
   memcpy(ctx->banc_ids, active_schools, count_schools * sizeof(int));

   dialog_initialiser(&ctx->dialog);
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   if (toplevel)
      ctx->dialog.parent = GTK_WINDOW(toplevel);
   dialog_set_titre(&ctx->dialog, "🔗 Fusionner deux bancs");
   ctx->dialog.boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
   ctx->dialog.on_reponse = on_merge_reponse;
   ctx->dialog.user_data = ctx;

   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);

   GtkWidget *lbl_1 = gtk_label_new("Sélectionner le premier banc :");
   gtk_widget_set_halign(lbl_1, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_1);

   champ_select_initialiser(&ctx->sel1);
   for (int i = 0; i < count_schools; i++)
   {
      char name_buf[128];
      sprintf(name_buf, "Banc #%d (%s, %d membres)", active_schools[i], school_species[i], school_counts[i]);
      champ_select_add_item(&ctx->sel1, name_buf);
   }
   GtkWidget *w_sel1 = champ_select_creer(&ctx->sel1);
   gtk_box_append(GTK_BOX(box), w_sel1);

   GtkWidget *lbl_2 = gtk_label_new("Sélectionner le deuxième banc à fusionner dedans :");
   gtk_widget_set_halign(lbl_2, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_2);

   champ_select_initialiser(&ctx->sel2);
   for (int i = 0; i < count_schools; i++)
   {
      char name_buf[128];
      sprintf(name_buf, "Banc #%d (%s, %d membres)", active_schools[i], school_species[i], school_counts[i]);
      champ_select_add_item(&ctx->sel2, name_buf);
   }
   champ_select_set_index(&ctx->sel2, 1);
   GtkWidget *w_sel2 = champ_select_creer(&ctx->sel2);
   gtk_box_append(GTK_BOX(box), w_sel2);

   dialog_set_contenu(&ctx->dialog, box);
   dialog_creer(&ctx->dialog);
   dialog_afficher(&ctx->dialog);
}
