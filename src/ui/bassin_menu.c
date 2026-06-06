#include "bassin_private.h"
#include <string.h>

void on_remove_poisson_btn_clicked(GtkWidget *widget, gpointer user_data)
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
         gtk_button_set_label(GTK_BUTTON(ui->btn_remove), "✖ Annuler");
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
         gtk_button_set_label(GTK_BUTTON(ui->btn_remove), "🗑️ Supprimer");
      }
      gtk_widget_set_cursor(GTK_WIDGET(ui->canvas), NULL);
   }
}

void on_play_pause_clicked(GtkWidget *widget, gpointer user_data)
{
   BassinUI *ui = user_data;
   if (!ui || !widget)
      return;

   ui->simulation_running = !ui->simulation_running;
   if (ui->simulation_running)
      gtk_button_set_label(GTK_BUTTON(widget), "⏸ Pause");
   else
      gtk_button_set_label(GTK_BUTTON(widget), "▶ Play");

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

void on_restart_clicked(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
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

   // Pre-populate default entities dynamically
   int preys_found = 0;
   int preds_found = 0;
   for (GList *l = ui->species_configs; l; l = l->next)
   {
      SpeciesConfig *cfg = l->data;
      if (strcmp(cfg->type, "prey") == 0)
      {
         preys_found++;
         if (preys_found == 1)
         {
            ui->num_bancs++;
            for (int i = 0; i < 4; i++)
               add_fish_programmatic(ui, cfg->nom, TRUE, ui->num_bancs);
         }
         else if (preys_found == 2)
         {
            ui->num_bancs++;
            for (int i = 0; i < 3; i++)
               add_fish_programmatic(ui, cfg->nom, TRUE, ui->num_bancs);
         }
         else if (preys_found == 3)
         {
            add_fish_programmatic(ui, cfg->nom, FALSE, -1);
         }
      }
      else if (strcmp(cfg->type, "predator") == 0)
      {
         preds_found++;
         if (preds_found == 1)
         {
            add_fish_programmatic(ui, cfg->nom, FALSE, -1);
         }
      }
   }

   update_sidebar_list(ui);
   update_status_bar(ui);
}

void on_vider_clicked(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   BassinUI *ui = user_data;

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

   update_sidebar_list(ui);
   update_status_bar(ui);
}

void on_toggle_zen_mode_clicked(GtkWidget *widget, gpointer user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   if (ui)
   {
      ui->zen_mode = !ui->zen_mode;
      apply_zen_mode(ui);
   }
}

void bassin_menu_init(BassinUI *ui, GtkWidget *header_box)
{
   // Title label styled
   GtkWidget *lbl_title = gtk_label_new("Simulateur de bancs de poissons");
   gtk_widget_add_css_class(lbl_title, "header-title");
   gtk_box_append(GTK_BOX(header_box), lbl_title);

   // Standard GtkButtons for options
   GtkWidget *btn_add = gtk_button_new_with_label("+ Ajouter");
   gtk_widget_add_css_class(btn_add, "suggested-action");
   g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_poisson_btn_clicked), ui);
   gtk_box_append(GTK_BOX(header_box), btn_add);

   GtkWidget *btn_remove = gtk_button_new_with_label("🗑️ Supprimer");
   gtk_widget_add_css_class(btn_remove, "destructive-action");
   g_signal_connect(btn_remove, "clicked", G_CALLBACK(on_remove_poisson_btn_clicked), ui);
   gtk_box_append(GTK_BOX(header_box), btn_remove);
   ui->btn_remove = btn_remove;

   GtkWidget *btn_vider = gtk_button_new_with_label("🗑️ Vider");
   gtk_widget_add_css_class(btn_vider, "destructive-action");
   g_signal_connect(btn_vider, "clicked", G_CALLBACK(on_vider_clicked), ui);
   gtk_box_append(GTK_BOX(header_box), btn_vider);

   GtkWidget *btn_save = gtk_button_new_with_label("💾 Sauvegarder");
   g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), ui);
   gtk_box_append(GTK_BOX(header_box), btn_save);

   GtkWidget *btn_load = gtk_button_new_with_label("📂 Charger");
   g_signal_connect(btn_load, "clicked", G_CALLBACK(on_load_clicked), ui);
   gtk_box_append(GTK_BOX(header_box), btn_load);

   // Right-aligned simulation controls
   GtkWidget *sim_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
   gtk_widget_set_halign(sim_controls, GTK_ALIGN_END);
   gtk_widget_set_hexpand(sim_controls, TRUE);

   GtkWidget *lbl_sim = gtk_label_new("Simulation");
   gtk_box_append(GTK_BOX(sim_controls), lbl_sim);

   // Standard GtkButton for Play/Pause
   GtkWidget *btn_play = gtk_button_new_with_label(ui->simulation_running ? "⏸ Pause" : "▶ Play");
   g_signal_connect(btn_play, "clicked", G_CALLBACK(on_play_pause_clicked), ui);
   gtk_box_append(GTK_BOX(sim_controls), btn_play);

   // Speed Dropdown
   const char *speed_options[] = {"x1", "x1.5", "x2", "x3", NULL};
   GtkWidget *dropdown_speed = gtk_drop_down_new_from_strings((const char *const *)speed_options);
   g_signal_connect(dropdown_speed, "notify::selected", G_CALLBACK(on_speed_changed), ui);
   gtk_box_append(GTK_BOX(sim_controls), dropdown_speed);

   // Reset standard button
   GtkWidget *btn_reset = gtk_button_new_with_label("🔄 Reset");
   g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_restart_clicked), ui);
   gtk_box_append(GTK_BOX(sim_controls), btn_reset);

   // Settings standard button
   GtkWidget *btn_settings = gtk_button_new_with_label("⚙️ Settings");
   g_signal_connect(btn_settings, "clicked", G_CALLBACK(on_settings_clicked), ui);
   gtk_box_append(GTK_BOX(sim_controls), btn_settings);

   // Toggle Sidebar button
   GtkWidget *btn_toggle_sidebar = gtk_button_new_with_label("📋 Sidebar");
   g_signal_connect(btn_toggle_sidebar, "clicked", G_CALLBACK(on_toggle_sidebar_clicked), ui);
   gtk_box_append(GTK_BOX(sim_controls), btn_toggle_sidebar);

   // Toggle Zen Mode button
   GtkWidget *btn_zen = gtk_button_new_with_label("🧘 Mode Zen");
   g_signal_connect(btn_zen, "clicked", G_CALLBACK(on_toggle_zen_mode_clicked), ui);
   gtk_box_append(GTK_BOX(sim_controls), btn_zen);

   gtk_box_append(GTK_BOX(header_box), sim_controls);
}
