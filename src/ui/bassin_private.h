#ifndef BASSIN_PRIVATE_H
#define BASSIN_PRIVATE_H

#include <gtk/gtk.h>
#include "../modele/poisson.h"
#include "../../widgets/headers/common.h"
#include "../../widgets/headers/menu.h"
#include "../../widgets/headers/bouton.h"
#include "../../widgets/headers/dialog.h"
#include "../../widgets/headers/slider.h"
#include "../../widgets/headers/champ_select.h"
#include "../../widgets/headers/bouton_radio.h"
#include "../../widgets/headers/texte.h"

// Forward declaration of SpeciesConfig
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
   int level;
   char *diet[16]; // list of species names that this fish can eat
   int nb_diet;
} SpeciesConfig;

typedef struct
{
   GtkWidget *root;
   GtkWidget *canvas;
   GList *poissons; /* list of Poisson* */
   int next_id;
   GtkWidget *btn_remove;
   gboolean delete_mode;

   // Simulation control
   gboolean simulation_running;
   int num_bancs;
   guint timer_id;
   double simulation_speed;

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

   // UI widgets references
   GtkWidget *bg_widget;
   GtkWidget *debug_overlay;
   gboolean debug_mode;

   // Settings Dialog fields
   Dialog settings_dialog;
   ChampSelect settings_sel_bg;
   ChampSelect settings_sel_canvas;
   Slider settings_sld_fish_size;
   GList *species_configs; /* list of SpeciesConfig* */
} BassinUI;

// Shared helper functions
SpeciesConfig *find_species_config(BassinUI *ui, const char *species_name);
int get_fish_level(BassinUI *ui, Poisson *p);
gboolean is_predator(BassinUI *ui, Poisson *p);
gboolean is_ally(BassinUI *ui, Poisson *p);
gboolean is_prey(BassinUI *ui, Poisson *p);
void spawn_floating_damage(BassinUI *ui, double x, double y, double damage);
void create_poisson_widget(BassinUI *ui, Poisson *p);
Poisson *find_nearest_prey(BassinUI *ui, Poisson *predator);
Poisson *find_nearest_predator(BassinUI *ui, Poisson *fish);

// Module: Menu & Actions
void bassin_menu_init(BassinUI *ui, GtkWidget *header_box);
void update_play_pause_button(BassinUI *ui);
void on_play_pause_clicked(GtkWidget *widget, gpointer user_data);
void on_restart_clicked(GtkWidget *widget, gpointer user_data);
void on_remove_poisson_btn_clicked(GtkWidget *widget, gpointer user_data);
void on_vider_clicked(GtkWidget *widget, gpointer user_data);

// Module: Sidebar
void update_sidebar_list(BassinUI *ui);
void update_status_bar(BassinUI *ui);
void on_tab_entites_clicked(GtkButton *btn, gpointer user_data);
void on_tab_bancs_clicked(GtkButton *btn, gpointer user_data);
void on_dissolve_banc_clicked(GtkButton *btn, gpointer user_data);
void on_split_banc_clicked(GtkButton *btn, gpointer user_data);
void on_merge_bancs_clicked(GtkButton *btn, gpointer user_data);
void on_merge_reponse(int reponse, gpointer user_data);

// Module: Simulation
gboolean update_simulation(gpointer user_data);
void eat_fish(BassinUI *ui, Poisson *prey);
void on_debug_draw(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data);

// Module: XML / File IO
void load_species_configs(BassinUI *ui);
void bassin_save_to_xml(BassinUI *ui, const char *filename);
void bassin_load_from_xml(BassinUI *ui, const char *filename);
void on_save_clicked(GtkWidget *widget, gpointer user_data);
void on_load_clicked(GtkWidget *widget, gpointer user_data);

// Module: Dialogs
void on_add_poisson_btn_clicked(GtkWidget *widget, gpointer user_data);
void on_settings_clicked(GtkWidget *widget, gpointer user_data);
void on_insertion_mode_changed(GtkCheckButton *widget, gpointer user_data);
void on_dialog_reponse(int reponse, gpointer user_data);
void add_fish_programmatic(BassinUI *ui, const char *species, gboolean in_banc, int target_banc_id);

#endif // BASSIN_PRIVATE_H
