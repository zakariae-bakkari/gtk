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
#include "../../widgets/headers/image.h"
#include "../../widgets/headers/champ_texte.h"
#include "../core/sound.h"

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
   double health;
} SpeciesConfig;

typedef struct
{
   double x, y;
   double vy;
   int id;
   GtkWidget *widget;
   int image_index; // 1, 2 or 3
   double health_restore;
   int ticks_at_bottom;
} Food;

typedef struct _Banc
{
   int id;
   char *nom_espece;
   GList *poissons;  /* list of Poisson* */
   Poisson *leader;  /* pointer to Poisson */
} Banc;

typedef struct
{
   GtkWidget *root;
   GtkWidget *canvas;
   GList *poissons; /* list of Poisson* (autonomous only) */
   GList *bancs;    /* list of Banc* */
   GList *foods;    /* list of Food* */
   int next_id;
   int next_food_id;
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
   GtkWidget *sidebar;
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
   gboolean zen_mode;
   gboolean sidebar_was_visible;

   // Display settings
   gboolean config_hide_health_bar;
   gboolean config_hide_fish_name;
   gboolean config_hide_status_bar;
   gboolean config_always_eat;

   // Settings Dialog fields & switch widgets
   Dialog settings_dialog;
   ChampTexte settings_txt_bg;
   ChampSelect settings_sel_canvas;
   Slider settings_sld_fish_size;
   GtkWidget *settings_sw_hide_health;
   GtkWidget *settings_sw_hide_name;
   GtkWidget *settings_sw_hide_status;
   GtkWidget *settings_sw_always_eat;
   GList *species_configs; /* list of SpeciesConfig* */
   GtkMediaStream *bg_stream; // Added for video backgrounds

   // Main UI widgets for Zen mode
   GtkWidget *header_widget;
   GtkWidget *sep_top_widget;
   GtkWidget *sep_bottom_widget;
   GtkWidget *status_bar_widget;

   // Play/Pause button reference
   GtkWidget *btn_play;

   // Customizable shortcuts configurations
   char *shortcut_play;
   char *shortcut_zen;
   char *shortcut_debug;
   char *shortcut_settings;
   char *shortcut_add;
   char *shortcut_sidebar;
   char *shortcut_restart;
   char *shortcut_food;

   // Settings dialog shortcut fields
   ChampTexte settings_txt_shortcut_play;
   ChampTexte settings_txt_shortcut_zen;
   ChampTexte settings_txt_shortcut_debug;
   ChampTexte settings_txt_shortcut_settings;
   ChampTexte settings_txt_shortcut_add;
   ChampTexte settings_txt_shortcut_sidebar;
   ChampTexte settings_txt_shortcut_restart;
   ChampTexte settings_txt_shortcut_food;

   // Playable fish fields
   Poisson *controlled_fish;
   double time_since_last_alert;

   // Floating damage/heal/kill labels (ticked by update_simulation, no per-label timer)
   GList *floating_labels; /* list of FloatingDamage* */
} BassinUI;

void spawn_floating_kill(BassinUI *ui, double x, double y);
void update_fish_widget_tags(BassinUI *ui, Poisson *p);
GtkWidget *get_fish_picture_widget(Poisson *p);

void apply_zen_mode(BassinUI *ui);
void apply_fish_visibility_configs(BassinUI *ui);
void apply_background(BassinUI *ui);
void on_toggle_zen_mode_clicked(GtkWidget *widget, gpointer user_data);

// Shared helper functions
SpeciesConfig *find_species_config(BassinUI *ui, const char *species_name);
int get_fish_level(BassinUI *ui, Poisson *p);
gboolean is_predator(BassinUI *ui, Poisson *p);
gboolean is_ally(BassinUI *ui, Poisson *p);
gboolean is_prey(BassinUI *ui, Poisson *p);
void spawn_floating_damage(BassinUI *ui, double x, double y, double damage);
void spawn_floating_heal(BassinUI *ui, double x, double y, double amount);
void create_poisson_widget(BassinUI *ui, Poisson *p);
void spawn_food(BassinUI *ui, double x, double y);
void on_throw_food_clicked(GtkWidget *widget, gpointer user_data);
Poisson *find_nearest_prey(BassinUI *ui, Poisson *predator);
Poisson *find_nearest_predator(BassinUI *ui, Poisson *fish);

// Shoal helpers
GList *bassin_get_all_poissons(BassinUI *ui);
Banc *bassin_find_banc(BassinUI *ui, int banc_id);
Banc *bassin_get_or_create_banc(BassinUI *ui, int banc_id, const char *species_name);
void bassin_remove_poisson_from_banc(BassinUI *ui, Poisson *p);
void bassin_add_poisson_to_banc(BassinUI *ui, Poisson *p, int banc_id);
void bassin_add_poisson(BassinUI *ui, Poisson *p);
void bassin_tick_bancs_behavior(BassinUI *ui, double dt);

// Module: Menu & Actions
void bassin_menu_init(BassinUI *ui, GtkWidget *header_box);
void update_play_pause_button(BassinUI *ui);
void on_play_pause_clicked(GtkWidget *widget, gpointer user_data);
void on_stop_control_clicked(GtkWidget *widget, gpointer user_data);
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
void tick_floating_labels(BassinUI *ui);
void eat_fish(BassinUI *ui, Poisson *prey);
void on_debug_draw(GtkDrawingArea *drawing_area, cairo_t *cr, int width, int height, gpointer user_data);

// Module: XML / File IO
void load_species_configs(BassinUI *ui);
void bassin_save_to_xml(BassinUI *ui, const char *filename);
void bassin_load_from_xml(BassinUI *ui, const char *filename);
void on_save_clicked(GtkWidget *widget, gpointer user_data);
void on_load_clicked(GtkWidget *widget, gpointer user_data);
void load_settings_from_xml(BassinUI *ui);
void save_settings_to_xml(BassinUI *ui);

// Module: Dialogs
void on_add_poisson_btn_clicked(GtkWidget *widget, gpointer user_data);
void on_settings_clicked(GtkWidget *widget, gpointer user_data);
void on_insertion_mode_changed(GtkCheckButton *widget, gpointer user_data);
void on_dialog_reponse(int reponse, gpointer user_data);
void add_fish_programmatic(BassinUI *ui, const char *species, gboolean in_banc, int target_banc_id);
void show_fish_details_dialog(BassinUI *ui, Poisson *p);
void open_species_details_dialog(BassinUI *ui, const char *species_name);
void open_create_species_dialog(BassinUI *ui, const char *edit_species_name);
void open_delete_species_confirmation(BassinUI *ui, const char *species_name);
void save_species_configs_to_xml(BassinUI *ui);
void on_toggle_sidebar_clicked(GtkWidget *widget, gpointer user_data);
void on_image_widget_destroy(GtkWidget *widget, gpointer user_data);
void show_shortcuts_help_dialog(BassinUI *ui);
void open_random_load_dialog(BassinUI *ui);

#endif // BASSIN_PRIVATE_H
