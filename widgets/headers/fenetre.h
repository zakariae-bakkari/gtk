#ifndef FENETRE_H
#define FENETRE_H

#include "conteneur.h"
#include "common.h"

typedef enum {
    TITRE_ALIGN_GAUCHE = 0,
    TITRE_ALIGN_CENTRE = 1,
    TITRE_ALIGN_DROITE = 2
} FenetreTitreAlign;

typedef enum {
    WIN_POS_CENTER,
    WIN_POS_MOUSE,
    WIN_POS_CENTER_ON_PARENT,
    WIN_POS_DEFAULT
} FenetrePosition;

typedef enum {
    WIN_TYPE_TOPLEVEL,
    WIN_TYPE_POPUP
} FenetreType;

typedef struct {
    int width;
    int height;
} FenetreTaille;

typedef struct {
    Widget wind;
    Widget scroll_widget;

    char *title;
    FenetreTitreAlign titre_align;
    char *icon_path;     // ← chemin PNG pour la HeaderBar
    char *ico_path;      // ← chemin ICO pour la taskbar Windows ✅ NOUVEAU

    FenetreType type;
    bool resizable;
    bool demarrer_maximisee;

    bool bouton_fermer;
    bool bouton_agrandir;
    bool bouton_reduire;

    WidgetScrollMode scroll_mode;
    bool scroll_overlay;
    int content_min_width;
    int content_min_height;

    FenetreTaille taille;
    char *color_bg;
    char *background_image;
    FenetrePosition position;
    int id;
} Fenetre;

void fenetre_initialiser(Fenetre *config);
Widget fenetre_creer(Fenetre *config, void *app);
void fenetre_appliquer_position(Fenetre *config);
void fenetre_appliquer_icone_taskbar(Fenetre *config);  // ✅ NOUVEAU
void fenetre_set_scrollable(Fenetre *config, WidgetScrollMode mode);
void fenetre_set_scroll_content_size(Fenetre *config, int min_width, int min_height);
void fenetre_set_scroll_overlay(Fenetre *config, bool overlay);
Widget fenetre_get_content_container(Fenetre *config);
void fenetre_ajouter(Fenetre *config, Widget enfant);

/* Background & Common Action Helpers */
void fenetre_set_background_image(Widget window, const char *image_path);
void fenetre_reset_background(Widget window);
void action_quitter(Widget widget, void *data);

#endif