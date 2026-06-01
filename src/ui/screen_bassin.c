#include "screen_bassin.h"
#include "../modele/poisson.h"
#include "../sound.h"

// Framework Widgets
#include "dialog.h"
#include "bouton_radio.h"
#include "champ_select.h"
#include "slider.h"
#include "xml_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Custom CSS Data
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
   ".dialog-frame { background-color: #F9F9FB; border: 1px solid #E0E0E6; border-radius: 8px; padding: 12px; margin-top: 10px; }\n"
   ".fish-selected { border: 2px solid #3498DB; border-radius: 8px; box-shadow: 0 0 12px #3498DB; padding: 4px; background-color: rgba(52, 152, 219, 0.15); }\n"
   ".flipped { transform: scaleX(-1); }\n";

typedef struct
{
   char *nom;
   char *type; // "prey", "predator", "ally"
   double vitesse_normale;
   double vitesse_fuite;
   double vitesse_ralentie;
   int taille;
   int perimetre_detection;
   char *chemin_frames[3];
   int nb_frames;
} SpeciesConfig;

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

   // Dynamic data definitions
   GList *species_configs; /* list of SpeciesConfig* */

} BassinUI;

// Prototypes
static void load_species_configs(BassinUI *ui);
static SpeciesConfig *find_species_config(BassinUI *ui, const char *species_name);
static void on_fish_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data);
static void bassin_save_to_xml(BassinUI *ui, const char *filename);
static void bassin_load_from_xml(BassinUI *ui, const char *filename);
static void on_save_clicked(GtkButton *btn, gpointer user_data);
static void on_load_clicked(GtkButton *btn, gpointer user_data);
static void on_dissolve_banc_clicked(GtkButton *btn, gpointer user_data);
static void on_split_banc_clicked(GtkButton *btn, gpointer user_data);
static void on_merge_reponse(int reponse, gpointer user_data);
static void on_merge_bancs_clicked(GtkButton *btn, gpointer user_data);

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



static int attr_int(const XmlNode *n, const char *name, int def)
{
    const char *v = xml_attr_get(n, name);
    return v ? atoi(v) : def;
}

static gboolean attr_bool(const XmlNode *n, const char *name, gboolean def)
{
    const char *v = xml_attr_get(n, name);
    if (!v)
        return def;
    return (strcmp(v, "true") == 0 || strcmp(v, "1") == 0 || strcmp(v, "oui") == 0)
               ? TRUE
               : FALSE;
}

static void load_species_configs(BassinUI *ui)
{
   if (ui->species_configs)
   {
      for (GList *l = ui->species_configs; l; l = l->next)
      {
         SpeciesConfig *cfg = l->data;
         free(cfg->nom);
         free(cfg->type);
         for (int i = 0; i < 3; i++)
         {
            if (cfg->chemin_frames[i]) free(cfg->chemin_frames[i]);
         }
         free(cfg);
      }
      g_list_free(ui->species_configs);
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
         
         cfg->vitesse_normale = xml_attr_get(child, "speed_norm") ? atof(xml_attr_get(child, "speed_norm")) : 50.0;
         cfg->vitesse_fuite = xml_attr_get(child, "speed_escape") ? atof(xml_attr_get(child, "speed_escape")) : 80.0;
         cfg->vitesse_ralentie = xml_attr_get(child, "speed_slow") ? atof(xml_attr_get(child, "speed_slow")) : 25.0;
         cfg->taille = attr_int(child, "size", 64);
         cfg->perimetre_detection = attr_int(child, "detection", 100);

         int frame_idx = 0;
         for (XmlNode *frame_node = child->children; frame_node; frame_node = frame_node->next)
         {
            if (strcmp(frame_node->tag, "frame") == 0 && frame_idx < 3)
            {
               const char *frame_path = xml_attr_get(frame_node, "texte");
               if (frame_path)
               {
                  cfg->chemin_frames[frame_idx++] = strdup(frame_path);
               }
            }
         }
         cfg->nb_frames = frame_idx;
         ui->species_configs = g_list_append(ui->species_configs, cfg);
      }
   }
   xml_node_free(root);
}

static SpeciesConfig *find_species_config(BassinUI *ui, const char *species_name)
{
   for (GList *l = ui->species_configs; l; l = l->next)
   {
      SpeciesConfig *cfg = l->data;
      if (strcmp(cfg->nom, species_name) == 0)
      {
         return cfg;
      }
   }
   return NULL;
}

static void on_fish_clicked(GtkGestureClick *gesture, int n_press, double x, double y, gpointer user_data)
{
   Poisson *p = g_object_get_data(G_OBJECT(gesture), "poisson");
   BassinUI *ui = g_object_get_data(G_OBJECT(gesture), "ui");
   (void)n_press;
   (void)x;
   (void)y;
   (void)user_data;
   
   if (!p || !ui) return;

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

   // 3. Open a beautiful GtkDialog showing characteristics
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
   
   GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
   gtk_widget_set_margin_start(box, 15);
   gtk_widget_set_margin_end(box, 15);
   gtk_widget_set_margin_top(box, 10);
   gtk_widget_set_margin_bottom(box, 10);
   
   // Add details text fields
   char info_buf[1024];
   char *banc_str = p->id_banc >= 0 ? g_strdup_printf("Banc #%d", p->id_banc) : g_strdup("Aucun (Solo)");
   char *role_str = p->est_leader ? g_strdup("★ Leader du Banc") : (p->id_banc >= 0 ? g_strdup("Membre du Banc") : g_strdup("Autonome"));
   
   sprintf(info_buf, 
           "<b>Espèce :</b> %s\n"
           "<b>ID Unique :</b> %d\n"
           "<b>Groupe :</b> %s\n"
           "<b>Rôle :</b> %s\n"
           "<b>Position :</b> X=%.1f, Y=%.1f\n"
           "<b>Vitesse :</b> Vx=%.1f, Vy=%.1f\n\n"
           "<i>Caractéristiques de l'Espèce :</i>\n"
           "• Vitesse Normale : %.1f px/s\n"
           "• Vitesse Fuite : %.1f px/s\n"
           "• Taille : %d px\n"
           "• Rayon de Détection : %d px",
           p->nom,
           p->id,
           banc_str,
           role_str,
           p->x, p->y,
           p->vx, p->vy,
           p->vitesse_normale,
           p->vitesse_fuite,
           p->taille,
           p->perimetre_detection);

   g_free(banc_str);
   g_free(role_str);

   GtkWidget *lbl_info = gtk_label_new(NULL);
   gtk_label_set_markup(GTK_LABEL(lbl_info), info_buf);
   gtk_label_set_justify(GTK_LABEL(lbl_info), GTK_JUSTIFY_LEFT);
   gtk_widget_set_halign(lbl_info, GTK_ALIGN_START);
   gtk_box_append(GTK_BOX(box), lbl_info);
   
   dialog_set_contenu(details, box);
   dialog_creer(details);
   dialog_afficher(details);

   g_signal_connect_swapped(details->window, "destroy", G_CALLBACK(dialog_free), details);
}

static void bassin_save_to_xml(BassinUI *ui, const char *filename)
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

   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      fprintf(f, "    <poisson id=\"%d\" nom=\"%s\" x=\"%f\" y=\"%f\" vx=\"%f\" vy=\"%f\" id_banc=\"%d\" est_leader=\"%s\"/>\n",
              p->id, p->nom, p->x, p->y, p->vx, p->vy, p->id_banc, p->est_leader ? "true" : "false");
   }

   fprintf(f, "  </poissons>\n");
   fprintf(f, "</bassin>\n");
   fclose(f);
}

static void bassin_load_from_xml(BassinUI *ui, const char *filename)
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

   // 1. Clear current poissons
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

   // 2. Traverse nodes
   for (XmlNode *child = root->children; child; child = child->next)
   {
      if (strcmp(child->tag, "canvas") == 0)
      {
         ui->config_canvas_width = attr_int(child, "width", 900);
         ui->config_canvas_height = attr_int(child, "height", 600);
         ui->config_fish_size = attr_int(child, "fish_size", 64);
         ui->elapsed_time = xml_attr_get(child, "elapsed_time") ? atof(xml_attr_get(child, "elapsed_time")) : 0.0;
         ui->num_bancs = attr_int(child, "num_bancs", 0);
         
         const char *bg = xml_attr_get(child, "bg");
         if (bg) ui->config_bg_path = strdup(bg);

         gtk_widget_set_size_request(ui->canvas, ui->config_canvas_width, ui->config_canvas_height);
         if (ui->bg_widget)
         {
            gtk_picture_set_filename(GTK_PICTURE(ui->bg_widget), ui->config_bg_path);
            gtk_widget_set_size_request(ui->bg_widget, ui->config_canvas_width, ui->config_canvas_height);
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
               gboolean est_leader = attr_bool(p_node, "est_leader", FALSE);

               Poisson *p = poisson_new(nom);
               p->id = p_id;
               p->x = x;
               p->y = y;
               p->vx = vx;
               p->vy = vy;
               p->id_banc = id_banc;
               p->est_leader = est_leader;

               // Apply species configurations
               SpeciesConfig *cfg = find_species_config(ui, nom);
               if (cfg)
               {
                  p->vitesse_normale = cfg->vitesse_normale;
                  p->vitesse_fuite = cfg->vitesse_fuite;
                  p->vitesse_ralentie = cfg->vitesse_ralentie;
                  p->taille = ui->config_fish_size;
                  p->perimetre_detection = cfg->perimetre_detection;
                  poisson_set_default_frames(p, cfg->chemin_frames[0], cfg->chemin_frames[1], cfg->chemin_frames[2]);
               }

               // If next_id is <= p->id, update it
               if (ui->next_id <= p->id)
               {
                  ui->next_id = p->id + 1;
               }

               // Build widget
               GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
               
               GtkWidget *lbl_lead = gtk_label_new("★ Leader");
               gtk_widget_add_css_class(lbl_lead, "leader-tag");
               gtk_widget_set_visible(lbl_lead, p->est_leader);
               gtk_box_append(GTK_BOX(container), lbl_lead);

               GtkWidget *img = gtk_picture_new_for_filename(p->chemin_frames[0]);
               gtk_picture_set_keep_aspect_ratio(GTK_PICTURE(img), TRUE);
               gtk_widget_set_size_request(img, p->taille, p->taille);
               gtk_box_append(GTK_BOX(container), img);

               char tag_buf[64];
               if (strcmp(p->nom, "Requin") == 0) sprintf(tag_buf, "Requin Alpha");
               else if (strcmp(p->nom, "Dauphin") == 0) sprintf(tag_buf, "Dauphin");
               else
               {
                  const char *short_nom = strcmp(p->nom, "Poisson clown") == 0 ? "Clown" : (strcmp(p->nom, "Poisson-globe") == 0 ? "Globe" : "Hareng");
                  sprintf(tag_buf, "%s #%d", short_nom, p->id);
               }

               GtkWidget *lbl_tag = gtk_label_new(tag_buf);
               gtk_widget_add_css_class(lbl_tag, "fish-tag");
               if (strcmp(p->nom, "Requin") == 0) gtk_widget_add_css_class(lbl_tag, "fish-tag-pred");
               gtk_box_append(GTK_BOX(container), lbl_tag);

               // Enable click gesture
               GtkGesture *gesture = gtk_gesture_click_new();
               g_object_set_data(G_OBJECT(gesture), "poisson", p);
               g_object_set_data(G_OBJECT(gesture), "ui", ui);
               g_signal_connect(gesture, "released", G_CALLBACK(on_fish_clicked), NULL);
               gtk_widget_add_controller(container, GTK_EVENT_CONTROLLER(gesture));

               poisson_set_widget(p, container);
               gtk_fixed_put(GTK_FIXED(ui->canvas), container, (int)p->x, (int)p->y);

               ui->poissons = g_list_prepend(ui->poissons, p);
            }
         }
      }
   }

   xml_node_free(root);
   update_sidebar_list(ui);
   update_status_bar(ui);
}

static void on_save_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   BassinUI *ui = user_data;
   bassin_save_to_xml(ui, "data/bassin.xml");
   
   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   dialog_afficher_info(toplevel ? GTK_WINDOW(toplevel) : NULL, 
                        "Succès", 
                        "L'état du bassin a été sauvegardé avec succès dans data/bassin.xml.", 
                        NULL, 
                        NULL);
}

static void on_load_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   BassinUI *ui = user_data;
   bassin_load_from_xml(ui, "data/bassin.xml");

   GtkWidget *toplevel = gtk_widget_get_ancestor(ui->root, GTK_TYPE_WINDOW);
   dialog_afficher_info(toplevel ? GTK_WINDOW(toplevel) : NULL, 
                        "Succès", 
                        "L'état du bassin a été rechargé depuis data/bassin.xml.", 
                        NULL, 
                        NULL);
}

static void on_dissolve_banc_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   (void)user_data;
   BassinUI *ui = g_object_get_data(G_OBJECT(btn), "ui");
   int b_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "banc_id"));

   if (!ui) return;

   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (p->id_banc == b_id)
      {
         p->id_banc = -1;
         p->est_leader = FALSE;
         
         if (p->widget_image)
         {
            GtkWidget *lbl_lead = gtk_widget_get_first_child(p->widget_image);
            if (lbl_lead) gtk_widget_set_visible(lbl_lead, FALSE);
         }
      }
   }

   update_sidebar_list(ui);
   update_status_bar(ui);
}

static void on_split_banc_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   (void)user_data;
   BassinUI *ui = g_object_get_data(G_OBJECT(btn), "ui");
   int b_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "banc_id"));

   if (!ui) return;

   GList *members = NULL;
   for (GList *l = ui->poissons; l; l = l->next)
   {
      Poisson *p = l->data;
      if (p->id_banc == b_id)
      {
         members = g_list_append(members, p);
      }
   }

   int total = g_list_length(members);
   if (total < 2)
   {
      g_list_free(members);
      return; 
   }

   ui->num_bancs++;
   int new_banc_id = ui->num_bancs;

   int half = total / 2;
   int count = 0;
   
   for (GList *l = members; l; l = l->next)
   {
      Poisson *p = l->data;
      if (count < half)
      {
         p->id_banc = new_banc_id;
         p->est_leader = (count == 0);
      }
      else
      {
         p->est_leader = (count == half);
      }
      
      if (p->widget_image)
      {
         GtkWidget *lbl_lead = gtk_widget_get_first_child(p->widget_image);
         if (lbl_lead) gtk_widget_set_visible(lbl_lead, p->est_leader);
      }
      count++;
   }

   g_list_free(members);
   update_sidebar_list(ui);
   update_status_bar(ui);
}

typedef struct {
   BassinUI *ui;
   Dialog dialog;
   ChampSelect sel1;
   ChampSelect sel2;
   int *banc_ids;
   int nb_bancs;
} MergeContext;

static void on_merge_reponse(int reponse, gpointer user_data)
{
   MergeContext *ctx = user_data;
   BassinUI *ui = ctx->ui;

   if (reponse == DIALOG_REPONSE_OK)
   {
      int idx1 = champ_select_get_index(&ctx->sel1);
      int idx2 = champ_select_get_index(&ctx->sel2);

      if (idx1 >= 0 && idx2 >= 0 && idx1 < ctx->nb_bancs && idx2 < ctx->nb_bancs && idx1 != idx2)
      {
         int b1 = ctx->banc_ids[idx1];
         int b2 = ctx->banc_ids[idx2];

         for (GList *l = ui->poissons; l; l = l->next)
         {
            Poisson *p = l->data;
            if (p->id_banc == b2)
            {
               p->id_banc = b1;
               p->est_leader = FALSE;
               
               if (p->widget_image)
               {
                  GtkWidget *lbl_lead = gtk_widget_get_first_child(p->widget_image);
                  if (lbl_lead) gtk_widget_set_visible(lbl_lead, FALSE);
               }
            }
         }

         update_sidebar_list(ui);
         update_status_bar(ui);
      }
   }

   dialog_fermer(&ctx->dialog);
   free(ctx->banc_ids);
   free(ctx);
}

static void on_merge_bancs_clicked(GtkButton *btn, gpointer user_data)
{
   (void)btn;
   BassinUI *ui = user_data;

   int active_schools[100];
   int school_counts[100];
   char *school_species[100];
   int count_schools = 0;

   for (int b_id = 1; b_id <= ui->num_bancs; b_id++)
   {
      int m_count = 0;
      char *sp_name = NULL;
      for (GList *l = ui->poissons; l; l = l->next)
      {
         Poisson *p = l->data;
         if (p->id_banc == b_id)
         {
            m_count++;
            sp_name = p->nom;
         }
      }
      if (m_count > 0 && count_schools < 100)
      {
         active_schools[count_schools] = b_id;
         school_counts[count_schools] = m_count;
         school_species[count_schools] = sp_name;
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
   if (toplevel) ctx->dialog.parent = GTK_WINDOW(toplevel);
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
   (void)user_data;
   
   // Dismiss the popover menu
   GtkWidget *popover = gtk_widget_get_ancestor(GTK_WIDGET(btn), GTK_TYPE_POPOVER);
   if (popover)
   {
      gtk_popover_popdown(GTK_POPOVER(popover));
   }

   SpeciesConfig *cfg = find_species_config(ui, species);
   gboolean is_pred_or_ally = cfg ? (strcmp(cfg->type, "prey") != 0) : FALSE;

   if (is_pred_or_ally)
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
   const char *emoji = "🐟";
   SpeciesConfig *cfg = find_species_config(ui, species);
   if (cfg)
   {
      if (strcmp(cfg->type, "predator") == 0) emoji = "🦈";
      else if (strcmp(cfg->type, "ally") == 0) emoji = "🐬";
      else if (strcmp(species, "Poisson-globe") == 0) emoji = "🐡";
      else if (strcmp(species, "Poisson clown") == 0) emoji = "🐠";
   }

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

   SpeciesConfig *cfg = find_species_config(ui, species);
   if (cfg)
   {
      p->vitesse_normale = cfg->vitesse_normale;
      p->vitesse_fuite = cfg->vitesse_fuite;
      p->vitesse_ralentie = cfg->vitesse_ralentie;
      p->taille = ui->config_fish_size > 0 ? ui->config_fish_size : cfg->taille;
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

   // Visual Representation Box: [Leader tag] -> [Image] -> [Name tag]
   GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
   
   // Leader Tag
   GtkWidget *lbl_lead = gtk_label_new("★ Leader");
   gtk_widget_add_css_class(lbl_lead, "leader-tag");
   gtk_widget_set_visible(lbl_lead, p->est_leader);
   gtk_box_append(GTK_BOX(container), lbl_lead);

   // Sprite (using GtkPicture for premium and accurate auto-scaling to p->taille)
   GtkWidget *img = gtk_picture_new_for_filename(p->chemin_frames[0] ? p->chemin_frames[0] : "resources/kenney_fish-pack_2.0/PNG/Default/fish_blue.png");
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

   // Enable click gesture
   GtkGesture *gesture = gtk_gesture_click_new();
   g_object_set_data(G_OBJECT(gesture), "poisson", p);
   g_object_set_data(G_OBJECT(gesture), "ui", ui);
   g_signal_connect(gesture, "released", G_CALLBACK(on_fish_clicked), NULL);
   gtk_widget_add_controller(container, GTK_EVENT_CONTROLLER(gesture));

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
      GtkWidget *btn_merge = gtk_button_new_with_label("🔗 Fusionner deux bancs");
      gtk_widget_add_css_class(btn_merge, "suggested-action");
      g_signal_connect(btn_merge, "clicked", G_CALLBACK(on_merge_bancs_clicked), ui);
      gtk_box_append(GTK_BOX(ui->box_sidebar_content), btn_merge);

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

            const char *emoji = "🐟";
            if (strcmp(species_nom, "Poisson-globe") == 0) emoji = "🐡";
            else if (strcmp(species_nom, "Poisson clown") == 0) emoji = "🐠";
            else if (strcmp(species_nom, "Requin") == 0) emoji = "🦈";
            else if (strcmp(species_nom, "Dauphin") == 0) emoji = "🐬";

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

            // Add action buttons
            GtkWidget *actions_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
            gtk_widget_set_margin_top(actions_box, 6);
            
            GtkWidget *btn_dissolve = gtk_button_new_with_label("🔓 Dissoudre");
            gtk_widget_add_css_class(btn_dissolve, "destructive-action");
            g_object_set_data(G_OBJECT(btn_dissolve), "ui", ui);
            g_object_set_data(G_OBJECT(btn_dissolve), "banc_id", GINT_TO_POINTER(b_id));
            g_signal_connect(btn_dissolve, "clicked", G_CALLBACK(on_dissolve_banc_clicked), NULL);
            gtk_box_append(GTK_BOX(actions_box), btn_dissolve);

            GtkWidget *btn_split = gtk_button_new_with_label("✂️ Diviser");
            g_object_set_data(G_OBJECT(btn_split), "ui", ui);
            g_object_set_data(G_OBJECT(btn_split), "banc_id", GINT_TO_POINTER(b_id));
            g_signal_connect(btn_split, "clicked", G_CALLBACK(on_split_banc_clicked), NULL);
            gtk_box_append(GTK_BOX(actions_box), btn_split);

            gtk_box_append(GTK_BOX(item), actions_box);

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

   // Dynamically update canvas and background bounds if window resizes
   int canvas_w = gtk_widget_get_width(ui->canvas);
   int canvas_h = gtk_widget_get_height(ui->canvas);
   if (canvas_w > 50 && canvas_h > 50)
   {
      if (canvas_w != ui->config_canvas_width || canvas_h != ui->config_canvas_height)
      {
         ui->config_canvas_width = canvas_w;
         ui->config_canvas_height = canvas_h;
         if (ui->bg_widget)
         {
            gtk_widget_set_size_request(ui->bg_widget, canvas_w, canvas_h);
         }
      }
   }

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
         // Update fish animation frame if multiple frames exist
         if (p->chemin_frames[1] != NULL)
         {
            p->temps_depuis_frame += dt;
            if (p->temps_depuis_frame >= 0.25) // Change frame every 250ms
            {
               p->temps_depuis_frame = 0.0;
               int next_frame = p->frame_courante + 1;
               if (next_frame >= 3 || p->chemin_frames[next_frame] == NULL)
               {
                  next_frame = 0;
               }
               p->frame_courante = next_frame;
            }
         }

         // Get the image widget inside the box: skips the leader label
         GtkWidget *img_widget = gtk_widget_get_first_child(p->widget_image); 
         if (img_widget && GTK_IS_LABEL(img_widget)) 
         {
            img_widget = gtk_widget_get_next_sibling(img_widget); // this is the image
         }
         
         if (img_widget && GTK_IS_PICTURE(img_widget))
         {
            gtk_picture_set_filename(GTK_PICTURE(img_widget), p->chemin_frames[p->frame_courante] ? p->chemin_frames[p->frame_courante] : p->chemin_frames[0]);
            if (p->vx < 0.0)
            {
               gtk_widget_add_css_class(img_widget, "flipped");
            }
            else
            {
               gtk_widget_remove_css_class(img_widget, "flipped");
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

   for (GList *l = ui->species_configs; l; l = l->next)
   {
      SpeciesConfig *cfg = l->data;
      if (strcmp(cfg->type, "prey") == 0)
      {
         char label_buf[128];
         const char *emoji = "🐟";
         if (strcmp(cfg->nom, "Poisson-globe") == 0) emoji = "🐡";
         else if (strcmp(cfg->nom, "Poisson clown") == 0) emoji = "🐠";
         
         sprintf(label_buf, "%s  %s (Banc possible)", emoji, cfg->nom);
         GtkWidget *btn_sp = gtk_button_new_with_label(label_buf);
         g_object_set_data(G_OBJECT(btn_sp), "ui", ui);
         g_object_set_data(G_OBJECT(btn_sp), "species", cfg->nom);
         g_signal_connect(btn_sp, "clicked", G_CALLBACK(on_species_selected), NULL);
         gtk_box_append(GTK_BOX(box), btn_sp);
      }
   }

   // PRÉDATEURS header
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
         
         sprintf(label_buf, "%s  %s (Individuel)", emoji, cfg->nom);
         GtkWidget *btn_sp = gtk_button_new_with_label(label_buf);
         g_object_set_data(G_OBJECT(btn_sp), "ui", ui);
         g_object_set_data(G_OBJECT(btn_sp), "species", cfg->nom);
         g_signal_connect(btn_sp, "clicked", G_CALLBACK(on_species_selected), NULL);
         gtk_box_append(GTK_BOX(box), btn_sp);
      }
   }

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
            for (int i = 0; i < 4; i++) add_fish_programmatic(ui, cfg->nom, TRUE, ui->num_bancs);
         }
         else if (preys_found == 2)
         {
            ui->num_bancs++;
            for (int i = 0; i < 3; i++) add_fish_programmatic(ui, cfg->nom, TRUE, ui->num_bancs);
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

    // Dynamic species loading
    load_species_configs(ui);

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

   GtkWidget *btn_save = gtk_button_new_with_label("💾 Sauvegarder");
   g_signal_connect(btn_save, "clicked", G_CALLBACK(on_save_clicked), ui);
   gtk_box_append(GTK_BOX(header), btn_save);

   GtkWidget *btn_load = gtk_button_new_with_label("📂 Charger");
   g_signal_connect(btn_load, "clicked", G_CALLBACK(on_load_clicked), ui);
   gtk_box_append(GTK_BOX(header), btn_load);

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
   gtk_widget_set_vexpand(ui->canvas, TRUE);
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
