#include <gtk/gtk.h>
#include "bassin_private.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../widgets/headers/xml_parser.h"

int attr_int(const XmlNode *n, const char *name, int def)
{
   const char *v = xml_attr_get(n, name);
   return v ? atoi(v) : def;
}

static bool attr_bool(const XmlNode *n, const char *name, bool def)
{
   const char *v = xml_attr_get(n, name);
   if (!v)
      return def;
   return (strcmp(v, "true") == 0 || strcmp(v, "1") == 0 || strcmp(v, "oui") == 0);
}

void load_species_configs(BassinUI *ui)
{
   if (ui->species_configs)
   {
      for (GList *l = (GList *)ui->species_configs; l; l = l->next)
      {
         SpeciesConfig *cfg = l->data;
         free(cfg->nom);
         free(cfg->type);
         for (int i = 0; i < 3; i++)
         {
            if (cfg->chemin_frames[i])
               free(cfg->chemin_frames[i]);
         }
         for (int i = 0; i < cfg->nb_diet; i++)
         {
            if (cfg->diet[i])
               free(cfg->diet[i]);
         }
         free(cfg);
      }
      g_list_free((GList *)ui->species_configs);
      ui->species_configs = NULL;
   }

   XmlNode *root = xml_parser_parse_file("data/poissons_types.xml");
   if (!root)
   {
      root = xml_parser_parse_file("../data/poissons_types.xml");
   }
   if (!root)
   {
      fprintf(stderr, "[Error] Failed to load data/poissons_types.xml\n");
      return;
   }

   for (XmlNode *child = root->children; child; child = child->next)
   {
      if (strcmp(child->tag, "species") == 0)
      {
         SpeciesConfig *cfg = calloc(1, sizeof(SpeciesConfig));
         const char *name = xml_attr_get(child, "name");
         const char *type = xml_attr_get(child, "type");

         cfg->nom = name ? strdup(name) : strdup("Poisson");
         cfg->type = type ? strdup(type) : strdup("prey");
         cfg->level = attr_int(child, "level", 1);

         cfg->vitesse_normale = xml_attr_get(child, "speed_norm") ? atof(xml_attr_get(child, "speed_norm")) : 50.0;
         cfg->vitesse_fuite = xml_attr_get(child, "speed_escape") ? atof(xml_attr_get(child, "speed_escape")) : 80.0;
         cfg->vitesse_ralentie = xml_attr_get(child, "speed_slow") ? atof(xml_attr_get(child, "speed_slow")) : 25.0;
         cfg->taille = attr_int(child, "size", 64);
         cfg->perimetre_detection = attr_int(child, "detection", 100);
         cfg->health = xml_attr_get(child, "health") ? atof(xml_attr_get(child, "health")) : 100.0;

         int frame_idx = 0;
         int diet_idx = 0;
         for (XmlNode *sub = child->children; sub; sub = sub->next)
         {
            if (strcmp(sub->tag, "frame") == 0 && frame_idx < 3)
            {
               const char *frame_path = xml_attr_get(sub, "texte");
               if (frame_path)
               {
                  cfg->chemin_frames[frame_idx++] = strdup(frame_path);
               }
            }
            else if (strcmp(sub->tag, "eats") == 0 && diet_idx < 16)
            {
               const char *diet_name = xml_attr_get(sub, "texte");
               if (diet_name)
               {
                  cfg->diet[diet_idx++] = strdup(diet_name);
               }
            }
         }
         cfg->nb_frames = frame_idx;
         cfg->nb_diet = diet_idx;
         g_print("Loaded Species: %s, type: %s, level: %d, frames: %d, diet: %d\n", cfg->nom, cfg->type, cfg->level, cfg->nb_frames, cfg->nb_diet);
         for(int i = 0; i < cfg->nb_frames; i++) {
             g_print("  Frame %d: %s\n", i, cfg->chemin_frames[i]);
         }
         for(int i = 0; i < cfg->nb_diet; i++) {
             g_print("  Eats: %s\n", cfg->diet[i]);
         }
         ui->species_configs = g_list_append((GList *)ui->species_configs, cfg);
      }
   }
   xml_node_free(root);
}

SpeciesConfig *find_species_config(BassinUI *ui, const char *species_name)
{
   for (GList *l = (GList *)ui->species_configs; l; l = l->next)
   {
      SpeciesConfig *cfg = l->data;
      if (strcmp(cfg->nom, species_name) == 0)
      {
         return cfg;
      }
   }
   return NULL;
}

void bassin_save_to_xml(BassinUI *ui, const char *filename)
{
   FILE *f = fopen(filename, "w");
   if (!f)
   {
      char parent_path[256];
      sprintf(parent_path, "../%s", filename);
      f = fopen(parent_path, "w");
   }
   if (!f)
   {
      fprintf(stderr, "[Error] Failed to open %s for saving\n", filename);
      return;
   }

   fprintf(f, "<bassin>\n");
   fprintf(f, "  <canvas width=\"%d\" height=\"%d\" fish_size=\"%d\" bg=\"%s\" elapsed_time=\"%f\" num_bancs=\"%d\"/>\n",
           ui->config_canvas_width, ui->config_canvas_height, ui->config_fish_size, ui->config_bg_path, ui->elapsed_time, ui->num_bancs);
   fprintf(f, "  <poissons>\n");

   GList *all_poissons = (GList *)bassin_get_all_poissons(ui);
   for (GList *l = all_poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      fprintf(f, "    <poisson id=\"%d\" nom=\"%s\" x=\"%f\" y=\"%f\" vx=\"%f\" vy=\"%f\" id_banc=\"%d\" est_leader=\"%s\" sante=\"%f\"/>\n",
              p->id, p->nom, p->x, p->y, p->vx, p->vy, p->id_banc, p->est_leader ? "true" : "false", p->sante);
   }
   g_list_free(all_poissons);

   fprintf(f, "  </poissons>\n");
   fprintf(f, "</bassin>\n");
   fclose(f);
}

void bassin_load_from_xml(BassinUI *ui, const char *filename)
{
   XmlNode *root = xml_parser_parse_file(filename);
   if (!root)
   {
      char parent_path[256];
      sprintf(parent_path, "../%s", filename);
      root = xml_parser_parse_file(parent_path);
   }
   if (!root)
   {
      fprintf(stderr, "[Error] Failed to parse save file %s\n", filename);
      return;
   }

   // 1. Clear current poissons & bancs
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

   ui->next_id = 1;

   // 2. Traverse nodes
   for (XmlNode *child = root->children; child; child = child->next)
   {
      if (strcmp(child->tag, "canvas") == 0)
      {
         ui->config_canvas_width = attr_int(child, "width", 900);
         ui->config_canvas_height = attr_int(child, "height", 600);
         ui->config_fish_size = attr_int(child, "fish_size", 0);
         ui->elapsed_time = xml_attr_get(child, "elapsed_time") ? atof(xml_attr_get(child, "elapsed_time")) : 0.0;
         ui->num_bancs = attr_int(child, "num_bancs", 0);

         const char *bg = xml_attr_get(child, "bg");
         if (bg)
         {
            if (ui->config_bg_path)
               free(ui->config_bg_path);
            ui->config_bg_path = strdup(bg);
         }

         widget_set_size(ui->canvas, ui->config_canvas_width, ui->config_canvas_height);
         if (ui->debug_overlay)
         {
            widget_set_size(ui->debug_overlay, ui->config_canvas_width, ui->config_canvas_height);
         }
         if (ui->bg_widget)
         {
            apply_background(ui);
            widget_set_size(ui->bg_widget, ui->config_canvas_width, ui->config_canvas_height);
         }
      }
      else if (strcmp(child->tag, "poissons") == 0)
      {
         for (XmlNode *p_node = child->children; p_node; p_node = p_node->next)
         {
            if (strcmp(p_node->tag, "poisson") == 0)
            {
               const char *nom = xml_attr_get(p_node, "nom");
               int p_id = attr_int(p_node, "id", 0);
               double x = xml_attr_get(p_node, "x") ? atof(xml_attr_get(p_node, "x")) : 0.0;
               double y = xml_attr_get(p_node, "y") ? atof(xml_attr_get(p_node, "y")) : 0.0;
               double vx = xml_attr_get(p_node, "vx") ? atof(xml_attr_get(p_node, "vx")) : 0.0;
               double vy = xml_attr_get(p_node, "vy") ? atof(xml_attr_get(p_node, "vy")) : 0.0;
               int id_banc = attr_int(p_node, "id_banc", -1);
               bool est_leader = attr_bool(p_node, "est_leader", false);
               double sante = xml_attr_get(p_node, "sante") ? atof(xml_attr_get(p_node, "sante")) : 100.0;

               Poisson *p = poisson_new(nom);
               p->id = p_id;
               p->x = x;
               p->y = y;
               p->vx = vx;
               p->vy = vy;
               p->id_banc = id_banc;
               p->est_leader = est_leader;
               p->sante = sante;

               // Apply species configurations
               SpeciesConfig *cfg = find_species_config(ui, nom);
               if (cfg)
               {
                  p->vitesse_normale = cfg->vitesse_normale;
                  p->vitesse_fuite = cfg->vitesse_fuite;
                  p->vitesse_ralentie = cfg->vitesse_ralentie;
                  double scale = (ui->config_fish_size > 0 ? ui->config_fish_size : 100) / 100.0;
                  p->taille = cfg->taille * scale;
                  p->perimetre_detection = cfg->perimetre_detection;
                  p->sante_max = cfg->health > 0.0 ? cfg->health : 100.0;
                  poisson_set_default_frames(p, cfg->chemin_frames[0], cfg->chemin_frames[1], cfg->chemin_frames[2]);
               }

               // If next_id is <= p->id, update it
               if (ui->next_id <= p->id)
               {
                  ui->next_id = p->id + 1;
               }

               // Build widget using the helper
               create_poisson_widget(ui, p);

               bassin_add_poisson(ui, p);
            }
         }
      }
   }

   xml_node_free(root);
   update_sidebar_list(ui);
   update_status_bar(ui);
}

void on_save_clicked(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   bassin_save_to_xml(ui, "data/bassin.xml");

   GtkWidget *toplevel = gtk_widget_get_ancestor(GTK_WIDGET(ui->root), GTK_TYPE_WINDOW);
   dialog_afficher_info(toplevel ? GTK_WINDOW(toplevel) : NULL,
                        "Succès",
                        "L'état du bassin a été sauvegardé avec succès dans data/bassin.xml.",
                        NULL,
                        NULL);
}

void on_load_clicked(Widget widget, void *user_data)
{
   (void)widget;
   BassinUI *ui = user_data;
   bassin_load_from_xml(ui, "data/bassin.xml");

   GtkWidget *toplevel = gtk_widget_get_ancestor(GTK_WIDGET(ui->root), GTK_TYPE_WINDOW);
   dialog_afficher_info(toplevel ? GTK_WINDOW(toplevel) : NULL,
                        "Succès",
                        "L'état du bassin a été rechargé depuis data/bassin.xml.",
                        NULL,
                        NULL);
}

void save_species_configs_to_xml(BassinUI *ui)
{
   FILE *f = fopen("data/poissons_types.xml", "w");
   if (!f)
   {
      f = fopen("../data/poissons_types.xml", "w");
   }
   if (!f)
   {
      fprintf(stderr, "[Error] Failed to open poissons_types.xml for saving\n");
      return;
   }

   fprintf(f, "<species_list>\n");
   for (GList *l = (GList *)ui->species_configs; l; l = l->next)
   {
      SpeciesConfig *cfg = l->data;
      fprintf(f, "    <species name=\"%s\" type=\"%s\" level=\"%d\" speed_norm=\"%.1f\" speed_escape=\"%.1f\" speed_slow=\"%.1f\" size=\"%d\" detection=\"%d\" health=\"%.1f\">\n",
              cfg->nom, cfg->type, cfg->level, cfg->vitesse_normale, cfg->vitesse_fuite, cfg->vitesse_ralentie, cfg->taille, cfg->perimetre_detection, cfg->health);
      for (int i = 0; i < cfg->nb_frames; i++)
      {
         fprintf(f, "        <frame>%s</frame>\n", cfg->chemin_frames[i]);
      }
      for (int i = 0; i < cfg->nb_diet; i++)
      {
         fprintf(f, "        <eats>%s</eats>\n", cfg->diet[i]);
      }
      fprintf(f, "    </species>\n");
   }
   fprintf(f, "</species_list>\n");
   fclose(f);
}

void load_settings_from_xml(BassinUI *ui)
{
   XmlNode *root = xml_parser_parse_file("data/settings.xml");
   if (!root)
   {
      root = xml_parser_parse_file("../data/settings.xml");
   }
   if (!root)
   {
      return;
   }

   ui->config_fish_size = attr_int(root, "fish_size", ui->config_fish_size);

   const char *bg = xml_attr_get(root, "bg_path");
   if (bg && bg[0] && g_file_test(bg, G_FILE_TEST_EXISTS))
   {
      if (ui->config_bg_path)
         free(ui->config_bg_path);
      ui->config_bg_path = strdup(bg);
   }

   ui->config_canvas_width = attr_int(root, "canvas_width", ui->config_canvas_width);
   ui->config_canvas_height = attr_int(root, "canvas_height", ui->config_canvas_height);
   ui->config_hide_health_bar = attr_bool(root, "hide_health_bar", ui->config_hide_health_bar);
   ui->config_hide_fish_name = attr_bool(root, "hide_fish_name", ui->config_hide_fish_name);
   ui->config_hide_status_bar = attr_bool(root, "hide_status_bar", ui->config_hide_status_bar);
   ui->config_always_eat = attr_bool(root, "always_eat", ui->config_always_eat);

   const char *sh_play = xml_attr_get(root, "shortcut_play");
   if (sh_play)
   {
      free(ui->shortcut_play);
      ui->shortcut_play = strdup(sh_play);
   }
   const char *sh_zen = xml_attr_get(root, "shortcut_zen");
   if (sh_zen)
   {
      free(ui->shortcut_zen);
      ui->shortcut_zen = strdup(sh_zen);
   }
   const char *sh_debug = xml_attr_get(root, "shortcut_debug");
   if (sh_debug)
   {
      free(ui->shortcut_debug);
      ui->shortcut_debug = strdup(sh_debug);
   }
   const char *sh_settings = xml_attr_get(root, "shortcut_settings");
   if (sh_settings)
   {
      free(ui->shortcut_settings);
      ui->shortcut_settings = strdup(sh_settings);
   }
   const char *sh_add = xml_attr_get(root, "shortcut_add");
   if (sh_add)
   {
      free(ui->shortcut_add);
      ui->shortcut_add = strdup(sh_add);
   }
   const char *sh_sidebar = xml_attr_get(root, "shortcut_sidebar");
   if (sh_sidebar)
   {
      free(ui->shortcut_sidebar);
      ui->shortcut_sidebar = strdup(sh_sidebar);
   }
   const char *sh_restart = xml_attr_get(root, "shortcut_restart");
   if (sh_restart)
   {
      free(ui->shortcut_restart);
      ui->shortcut_restart = strdup(sh_restart);
   }
   const char *sh_food = xml_attr_get(root, "shortcut_food");
   if (sh_food)
   {
      free(ui->shortcut_food);
      ui->shortcut_food = strdup(sh_food);
   }

   xml_node_free(root);
}

void save_settings_to_xml(BassinUI *ui)
{
   FILE *f = fopen("data/settings.xml", "w");
   if (!f)
   {
      f = fopen("../data/settings.xml", "w");
   }
   if (!f)
   {
      fprintf(stderr, "[Error] Failed to open settings.xml for saving settings\n");
      return;
   }

   fprintf(f, "<settings fish_size=\"%d\" bg_path=\"%s\" canvas_width=\"%d\" canvas_height=\"%d\" hide_health_bar=\"%s\" hide_fish_name=\"%s\" hide_status_bar=\"%s\" always_eat=\"%s\" ",
           ui->config_fish_size,
           ui->config_bg_path ? ui->config_bg_path : "",
           ui->config_canvas_width,
           ui->config_canvas_height,
           ui->config_hide_health_bar ? "true" : "false",
           ui->config_hide_fish_name ? "true" : "false",
           ui->config_hide_status_bar ? "true" : "false",
           ui->config_always_eat ? "true" : "false");

   fprintf(f, "shortcut_play=\"%s\" shortcut_zen=\"%s\" shortcut_debug=\"%s\" shortcut_settings=\"%s\" shortcut_add=\"%s\" shortcut_sidebar=\"%s\" shortcut_restart=\"%s\" shortcut_food=\"%s\"/>\n",
           ui->shortcut_play ? ui->shortcut_play : "",
           ui->shortcut_zen ? ui->shortcut_zen : "",
           ui->shortcut_debug ? ui->shortcut_debug : "",
           ui->shortcut_settings ? ui->shortcut_settings : "",
           ui->shortcut_add ? ui->shortcut_add : "",
           ui->shortcut_sidebar ? ui->shortcut_sidebar : "",
           ui->shortcut_restart ? ui->shortcut_restart : "",
           ui->shortcut_food ? ui->shortcut_food : "");
   fclose(f);
}
