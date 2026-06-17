#include <gtk/gtk.h>
#include "../simulation/bassin_private.h"
#include "../../widgets/headers/bouton.h"
#include <string.h>
#include <stdlib.h>

static void on_custom_bouton_destroy(Widget widget, void *data)
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

static Widget creer_bouton_custom(const char *texte, const char *id_css, BoutonPresetStyle preset, BoutonAction on_clic, void *user_data)
{
   Bouton *b = calloc(1, sizeof(Bouton));
   bouton_initialiser(b);
   free(b->texte);
   b->texte = strdup(texte);
   free(b->id_css);
   b->id_css = strdup(id_css);
   bouton_appliquer_preset(b, preset);
   b->on_clic = on_clic;
   b->user_data = user_data;
   Widget w = bouton_creer(b);
   widget_connect_destroy_signal(w, on_custom_bouton_destroy, b);
   return w;
}

static void bassin_menu_set_button_label(Widget btn, const char *label)
{
   GtkWidget *child = gtk_button_get_child(GTK_BUTTON(btn));
   if (child && GTK_IS_BOX(child))
   {
      GtkWidget *l = gtk_widget_get_first_child(child);
      while (l)
      {
         if (GTK_IS_LABEL(l))
         {
            gtk_label_set_text(GTK_LABEL(l), label);
            break;
         }
         l = gtk_widget_get_next_sibling(l);
      }
   }
}

void on_remove_poisson_btn_clicked(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (!ui)
      return;

   ui->delete_mode = !ui->delete_mode;
   if (ui->delete_mode)
   {
      if (ui->btn_remove)
      {
         bassin_menu_set_button_label(ui->btn_remove, "✖ Annuler");
      }
      GdkCursor *c = gdk_cursor_new_from_name("not-allowed", NULL);
      if (!c)
         c = gdk_cursor_new_from_name("crosshair", NULL);
      if (c)
         {
            gtk_widget_set_cursor(GTK_WIDGET(ui->canvas), c);
            g_object_unref(c);
         }
   }
   else
   {
      if (ui->btn_remove)
      {
         bassin_menu_set_button_label(ui->btn_remove, "🗑️ Supprimer");
      }
      gtk_widget_set_cursor(GTK_WIDGET(ui->canvas), NULL);
   }
}

void on_play_pause_clicked(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (!ui)
      return;

   ui->simulation_running = !ui->simulation_running;
   if (ui->btn_play)
   {
      if (ui->simulation_running)
         bassin_menu_set_button_label(ui->btn_play, "⏸ Pause");
      else
         bassin_menu_set_button_label(ui->btn_play, "▶ Play");
   }

   update_status_bar(ui);
}

static void on_speed_changed(GObject *object, GParamSpec *pspec, gpointer user_data)
{
   (void)pspec;
   BassinUI *ui = user_data;
   guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(object));
   if (selected == 0)
      ui->simulation_speed = 1.0;
   else if (selected == 1)
      ui->simulation_speed = 1.5;
   else if (selected == 2)
      ui->simulation_speed = 2.0;
   else if (selected == 3)
      ui->simulation_speed = 3.0;
}

void on_restart_clicked(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;

   // Free all fish in ui->poissons
   while (ui->poissons)
   {
      GList *node = (GList *)ui->poissons;
      Poisson *p = (Poisson *)node->data;
      if (p->widget_image)
      {
         gtk_widget_unparent(GTK_WIDGET(p->widget_image));
      }
      poisson_free(p);
      ui->poissons = g_list_delete_link((GList *)ui->poissons, node);
   }

   // Free all fish in ui->bancs
   while (ui->bancs)
   {
      GList *node = (GList *)ui->bancs;
      Banc *b = node->data;
      while (b->poissons)
      {
         GList *p_node = b->poissons;
         Poisson *p = p_node->data;
         if (p->widget_image)
         {
            gtk_widget_unparent(GTK_WIDGET(p->widget_image));
         }
         poisson_free(p);
         b->poissons = g_list_delete_link(b->poissons, p_node);
      }
      free(b->nom_espece);
      free(b);
      ui->bancs = g_list_delete_link((GList *)ui->bancs, node);
   }

   // Free all food
   while (ui->foods)
   {
      GList *node = (GList *)ui->foods;
      Food *f = (Food *)node->data;
      if (f->widget)
      {
         gtk_widget_unparent(GTK_WIDGET(f->widget));
      }
      free(f);
      ui->foods = g_list_delete_link((GList *)ui->foods, node);
   }

   ui->next_id = 1;
   ui->num_bancs = 0;
   ui->elapsed_time = 0.0;
   ui->controlled_fish = NULL;

   update_sidebar_list(ui);
   update_status_bar(ui);
}

void on_vider_clicked(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;

   while (ui->poissons)
   {
      GList *node = (GList *)ui->poissons;
      Poisson *p = (Poisson *)node->data;
      if (p->widget_image)
      {
         gtk_widget_unparent(GTK_WIDGET(p->widget_image));
      }
      poisson_free(p);
      ui->poissons = g_list_delete_link((GList *)ui->poissons, node);
   }

   while (ui->bancs)
   {
      GList *node = (GList *)ui->bancs;
      Banc *b = node->data;
      while (b->poissons)
      {
         GList *p_node = b->poissons;
         Poisson *p = p_node->data;
         if (p->widget_image)
         {
            gtk_widget_unparent(GTK_WIDGET(p->widget_image));
         }
         poisson_free(p);
         b->poissons = g_list_delete_link(b->poissons, p_node);
      }
      free(b->nom_espece);
      free(b);
      ui->bancs = g_list_delete_link((GList *)ui->bancs, node);
   }

   while (ui->foods)
   {
      GList *node = (GList *)ui->foods;
      Food *f = (Food *)node->data;
      if (f->widget)
      {
         gtk_widget_unparent(GTK_WIDGET(f->widget));
      }
      free(f);
      ui->foods = g_list_delete_link((GList *)ui->foods, node);
   }

   ui->next_id = 1;
   ui->num_bancs = 0;
   ui->elapsed_time = 0.0;
   ui->controlled_fish = NULL;

   update_sidebar_list(ui);
   update_status_bar(ui);
}

void on_toggle_zen_mode_clicked(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (ui)
   {
      ui->zen_mode = !ui->zen_mode;
      apply_zen_mode(ui);
   }
}

static void on_random_clicked(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (ui)
   {
      open_random_load_dialog(ui);
   }
}

void on_stop_control_clicked(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (ui && ui->controlled_fish)
   {
      Poisson *old = ui->controlled_fish;
      ui->controlled_fish = NULL;
      update_fish_widget_tags(ui, old);
   }
}

void bassin_menu_init(BassinUI *ui, Widget header_box)
{
   // Use custom Bouton widgets
   Widget btn_add = creer_bouton_custom("+ Ajouter", "btn_add", BOUTON_STYLE_SUGGESTED, on_add_poisson_btn_clicked, ui);
   gtk_box_append(GTK_BOX(header_box), GTK_WIDGET(btn_add));

   Widget btn_stop = creer_bouton_custom("🎮 Stop Contrôle", "btn_stop", BOUTON_STYLE_NEUTRAL, on_stop_control_clicked, ui);
   gtk_box_append(GTK_BOX(header_box), GTK_WIDGET(btn_stop));

   Widget btn_random = creer_bouton_custom("🎲 Aléatoire", "btn_random", BOUTON_STYLE_NEUTRAL, on_random_clicked, ui);
   gtk_box_append(GTK_BOX(header_box), GTK_WIDGET(btn_random));

   Widget btn_remove = creer_bouton_custom("🗑️ Supprimer", "btn_remove", BOUTON_STYLE_DESTRUCTIVE, on_remove_poisson_btn_clicked, ui);
   gtk_box_append(GTK_BOX(header_box), GTK_WIDGET(btn_remove));
   ui->btn_remove = btn_remove;

   Widget btn_vider = creer_bouton_custom("🗑️ Vider", "btn_vider", BOUTON_STYLE_DESTRUCTIVE, on_vider_clicked, ui);
   gtk_box_append(GTK_BOX(header_box), GTK_WIDGET(btn_vider));

   Widget btn_save = creer_bouton_custom("💾 Sauvegarder", "btn_save", BOUTON_STYLE_NEUTRAL, on_save_clicked, ui);
   gtk_box_append(GTK_BOX(header_box), GTK_WIDGET(btn_save));

   Widget btn_load = creer_bouton_custom("📂 Charger", "btn_load", BOUTON_STYLE_NEUTRAL, on_load_clicked, ui);
   gtk_box_append(GTK_BOX(header_box), GTK_WIDGET(btn_load));

   Widget btn_food = creer_bouton_custom("🍱 Nourrir", "btn_food", BOUTON_STYLE_SUGGESTED, on_throw_food_clicked, ui);
   gtk_box_append(GTK_BOX(header_box), GTK_WIDGET(btn_food));

   // Right-aligned simulation controls
   Widget sim_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
   gtk_widget_set_halign(GTK_WIDGET(sim_controls), GTK_ALIGN_END);
   widget_set_hexpand(sim_controls, true);

   Widget lbl_sim = gtk_label_new("Simulation");
   gtk_box_append(GTK_BOX(sim_controls), GTK_WIDGET(lbl_sim));

   Widget btn_play = creer_bouton_custom(ui->simulation_running ? "⏸ Pause" : "▶ Play", "btn_play", BOUTON_STYLE_NEUTRAL, on_play_pause_clicked, ui);
   ui->btn_play = btn_play;
   gtk_box_append(GTK_BOX(sim_controls), GTK_WIDGET(btn_play));

   // Speed Dropdown
   const char *speed_options[] = {"x1", "x1.5", "x2", "x3", NULL};
   Widget dropdown_speed = gtk_drop_down_new_from_strings((const char *const *)speed_options);
   g_signal_connect(dropdown_speed, "notify::selected", G_CALLBACK(on_speed_changed), ui);
   gtk_box_append(GTK_BOX(sim_controls), GTK_WIDGET(dropdown_speed));

   Widget btn_reset = creer_bouton_custom("🔄 Reset", "btn_reset", BOUTON_STYLE_NEUTRAL, on_restart_clicked, ui);
   gtk_box_append(GTK_BOX(sim_controls), GTK_WIDGET(btn_reset));

   Widget btn_settings = creer_bouton_custom("⚙️ Settings", "btn_settings", BOUTON_STYLE_NEUTRAL, on_settings_clicked, ui);
   gtk_box_append(GTK_BOX(sim_controls), GTK_WIDGET(btn_settings));

   Widget btn_toggle_sidebar = creer_bouton_custom("📋 Sidebar", "btn_toggle_sidebar", BOUTON_STYLE_NEUTRAL, on_toggle_sidebar_clicked, ui);
   gtk_box_append(GTK_BOX(sim_controls), GTK_WIDGET(btn_toggle_sidebar));

   Widget btn_zen = creer_bouton_custom("🧘 Mode Zen", "btn_zen", BOUTON_STYLE_NEUTRAL, on_toggle_zen_mode_clicked, ui);
   gtk_box_append(GTK_BOX(sim_controls), GTK_WIDGET(btn_zen));

   gtk_box_append(GTK_BOX(header_box), GTK_WIDGET(sim_controls));
}
