#ifndef BASSIN_PRIVATE_H
#define BASSIN_PRIVATE_H

#include <stdbool.h>
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

// Forward declaration of GLib's GList
typedef struct _GList GList;

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
   Widget widget;
   int image_index; // 1, 2 or 3
   double health_restore;
   int ticks_at_bottom;
} Food;

typedef struct _Banc
{
   int id;
   char *nom_espece;
   GList *poissons;      /* list of Poisson* */
   Poisson *leader;      /* pointer to Poisson */
   int parent_banc_id;   /* parent bank ID if split, or -1 */
   int split_cooldown;   /* ticks remaining before this banc can split again */
} Banc;

typedef struct
{
   Widget root;
   Widget canvas;
   GList *poissons; /* list of Poisson* (autonomous only) */
   GList *bancs;    /* list of Banc* */
   GList *foods;    /* list of Food* */
   int next_id;
   int next_food_id;
   Widget btn_remove;
   bool delete_mode;

   // Simulation control
   bool simulation_running;
   int num_bancs;
   unsigned int timer_id;
   double simulation_speed;

   // Sidebar elements
   Widget btn_tab_entites;
   Widget btn_tab_bancs;
   Widget scrolled_sidebar_list;
   Widget box_sidebar_content;
   Widget sidebar;
   int active_tab; // 0 = Entités, 1 = Bancs

   // Status bar labels
   Widget lbl_status_indicator;
   Widget lbl_stats_text;
   Widget lbl_elapsed_time;
   double elapsed_time;

   // Dialog reference (if open)
   Dialog active_dialog;
   char *dialog_selected_species;
   Widget dialog_school_frame;
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
   Widget bg_widget;
   Widget debug_overlay;
   bool debug_mode;
   bool zen_mode;
   bool sidebar_was_visible;

   // Display settings
   bool config_hide_health_bar;
   bool config_hide_fish_name;
   bool config_hide_status_bar;
   bool config_always_eat;

   // Settings Dialog fields & switch widgets
   Dialog settings_dialog;
   ChampTexte settings_txt_bg;
   ChampSelect settings_sel_canvas;
   Slider settings_sld_fish_size;
   Widget settings_sw_hide_health;
   Widget settings_sw_hide_name;
   Widget settings_sw_hide_status;
   Widget settings_sw_always_eat;
   GList *species_configs; /* list of SpeciesConfig* */
   void *bg_stream; // Added for video backgrounds (hides GtkMediaStream*)

   // Main UI widgets for Zen mode
   Widget header_widget;
   Widget sep_top_widget;
   Widget sep_bottom_widget;
   Widget status_bar_widget;

   // Play/Pause button reference
   Widget btn_play;

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
Widget get_fish_picture_widget(Poisson *p);

void apply_zen_mode(BassinUI *ui);
void apply_fish_visibility_configs(BassinUI *ui);
void apply_background(BassinUI *ui);
void on_toggle_zen_mode_clicked(Widget widget, void *user_data);

// Shared helper functions
SpeciesConfig *find_species_config(BassinUI *ui, const char *species_name);
int get_fish_level(BassinUI *ui, Poisson *p);
bool is_predator(BassinUI *ui, Poisson *p);
bool is_ally(BassinUI *ui, Poisson *p);
bool is_prey(BassinUI *ui, Poisson *p);
void spawn_floating_damage(BassinUI *ui, double x, double y, double damage);
void spawn_floating_heal(BassinUI *ui, double x, double y, double amount);
void create_poisson_widget(BassinUI *ui, Poisson *p);
void spawn_food(BassinUI *ui, double x, double y);
void on_throw_food_clicked(Widget widget, void *user_data);
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
void bassin_menu_init(BassinUI *ui, Widget header_box);
void update_play_pause_button(BassinUI *ui);
void on_play_pause_clicked(Widget widget, void *user_data);
void on_stop_control_clicked(Widget widget, void *user_data);
void on_restart_clicked(Widget widget, void *user_data);
void on_remove_poisson_btn_clicked(Widget widget, void *user_data);
void on_vider_clicked(Widget widget, void *user_data);

// Module: Sidebar
void update_sidebar_list(BassinUI *ui);
void update_status_bar(BassinUI *ui);
void on_tab_entites_clicked(Widget btn, void *user_data);
void on_tab_bancs_clicked(Widget btn, void *user_data);
void on_dissolve_banc_clicked(Widget btn, void *user_data);
void on_split_banc_clicked(Widget btn, void *user_data);
void on_merge_bancs_clicked(Widget btn, void *user_data);
void on_merge_reponse(int reponse, void *user_data);

// Module: Simulation
bool update_simulation(void *user_data);
void tick_floating_labels(BassinUI *ui);
void eat_fish(BassinUI *ui, Poisson *prey);
void on_debug_draw(Widget drawing_area, void *cr, int width, int height, void *user_data);

// Module: XML / File IO
void load_species_configs(BassinUI *ui);
void bassin_save_to_xml(BassinUI *ui, const char *filename);
void bassin_load_from_xml(BassinUI *ui, const char *filename);
void on_save_clicked(Widget widget, void *user_data);
void on_load_clicked(Widget widget, void *user_data);
void load_settings_from_xml(BassinUI *ui);
void save_settings_to_xml(BassinUI *ui);

// Module: Dialogs
void on_add_poisson_btn_clicked(Widget widget, void *user_data);
void on_settings_clicked(Widget widget, void *user_data);
void on_insertion_mode_changed(Widget widget, void *user_data);
void on_dialog_reponse(int reponse, void *user_data);
void add_fish_programmatic(BassinUI *ui, const char *species, bool in_banc, int target_banc_id);
void show_fish_details_dialog(BassinUI *ui, Poisson *p);
void open_species_details_dialog(BassinUI *ui, const char *species_name);
void open_create_species_dialog(BassinUI *ui, const char *edit_species_name);
void open_delete_species_confirmation(BassinUI *ui, const char *species_name);
void save_species_configs_to_xml(BassinUI *ui);
void on_toggle_sidebar_clicked(Widget widget, void *user_data);
void on_image_widget_destroy(Widget widget, void *user_data);
void show_shortcuts_help_dialog(BassinUI *ui);
void open_random_load_dialog(BassinUI *ui);

#endif // BASSIN_PRIVATE_H
