#ifndef EXPORT_XML_H
#define EXPORT_XML_H

#include <gtk/gtk.h>
#include "fenetre.h"
#include "conteneur.h"
#include "bouton.h"
#include "bouton_checklist.h"
#include "bouton_radio.h"
#include "champ_texte.h"
#include "champ_motdepasse.h"
#include "champ_nombre.h"
#include "champ_select.h"
#include "champ_zone_texte.h"
#include "slider.h"
#include "dialog.h"
#include "menu.h"
#include "texte.h"
#include "image.h"

/* ------------------------------------------------------------------ */
/* Contexte d'export — accumule les widgets avant génération           */
/* ------------------------------------------------------------------ */
typedef struct
{
    Fenetre         *fenetre;

    Menu            *menus[32];           int nb_menus;
    Texte           *textes[64];          int nb_textes;
    Conteneur       *conteneurs[32];      int nb_conteneurs;
    Bouton          *boutons[32];         int nb_boutons;
    BoutonChecklist *checklists[32];      int nb_checklists;
    BoutonRadio     *radios[32];          int nb_radios;
    ChampTexte      *champs_texte[32];    int nb_champs_texte;
    ChampMotDePasse *champs_mdp[32];      int nb_champs_mdp;
    ChampNombre     *champs_nombre[32];   int nb_champs_nombre;
    ChampSelect     *champs_select[32];   int nb_champs_select;
    ChampZoneTexte  *zones_texte[32];     int nb_zones_texte;
    Slider          *sliders[32];         int nb_sliders;
    Dialog          *dialogs[32];         int nb_dialogs;
    Image           *images[32];          int nb_images;
} ExportContext;

/* ------------------------------------------------------------------ */
/* Prototypes                                                           */
/* ------------------------------------------------------------------ */
void export_context_init    (ExportContext *ctx);

void export_ajouter_fenetre  (ExportContext *ctx, Fenetre         *w);
void export_ajouter_menu     (ExportContext *ctx, Menu            *w);
void export_ajouter_texte    (ExportContext *ctx, Texte           *w);
void export_ajouter_conteneur(ExportContext *ctx, Conteneur       *w);
void export_ajouter_bouton   (ExportContext *ctx, Bouton          *w);
void export_ajouter_checklist(ExportContext *ctx, BoutonChecklist *w);
void export_ajouter_radio    (ExportContext *ctx, BoutonRadio     *w);
void export_ajouter_champ_texte  (ExportContext *ctx, ChampTexte      *w);
void export_ajouter_champ_mdp    (ExportContext *ctx, ChampMotDePasse *w);
void export_ajouter_champ_nombre (ExportContext *ctx, ChampNombre     *w);
void export_ajouter_champ_select (ExportContext *ctx, ChampSelect     *w);
void export_ajouter_zone_texte   (ExportContext *ctx, ChampZoneTexte  *w);
void export_ajouter_slider   (ExportContext *ctx, Slider          *w);
void export_ajouter_dialog   (ExportContext *ctx, Dialog          *w);
void export_ajouter_image    (ExportContext *ctx, Image           *w);

void generer_fichier_interface(ExportContext *ctx);

#endif // EXPORT_XML_H