#ifndef POISSON_H
#define POISSON_H

#include "../../widgets/headers/common.h"
#include <stdbool.h>

typedef enum
{
   ETAT_NORMAL,
   ETAT_FUITE,
   ETAT_RALENTI,
   ETAT_HORS_CADRE,
   ETAT_MORT
} EtatPoisson;

typedef struct _Poisson
{
   int id;
   char *nom;
   int *niveaux_mangeable;
   int nb_niveaux_mangeable;

   double x, y;   // position in the bassin
   double vx, vy; // velocity in pixels per second
   double angle;  // angle of movement in degrees (0 = right, 90 = down, etc.)

   double vitesse_normale;
   double vitesse_fuite;
   double vitesse_ralentie;
   int taille;              // in pixels, used for display and collision
   int perimetre_detection; // radius in pixels for detecting other fish or obstacles

   EtatPoisson etat;
   double temps_hors_cadre;
   int cote_sortie;

   /* animation */
   char *chemin_frames[3];
   int frame_courante;
   double temps_depuis_frame;

   /* widget GTK (optionnel pour la structure de base) */
   Widget widget_image;
   bool visible;

   /* health and damage */
   double sante;
   double sante_max;
   double temps_effet_attaque;
   double degats_accumules;
   double temps_dernier_floating_damage;

   /* kill stats (predators only) */
   char *kills_espece[8];   /* species name (owned string) */
   int   kills_count[8];    /* kills per species */
   int   nb_kills_types;    /* number of distinct species eaten */

   /* last attacker reference (prey only, not owned) */
   struct _Poisson *dernier_attaquant;

   /* banc */
   int id_banc;
   bool est_leader;
} Poisson;

Poisson *poisson_new(const char *nom);
void poisson_free(Poisson *p);

void poisson_set_position(Poisson *p, double x, double y);
void poisson_set_widget(Poisson *p, Widget widget);
void poisson_set_default_frames(Poisson *p, const char *frame0, const char *frame1, const char *frame2);
void poisson_set_niveaux_mangeable(Poisson *p, const int *niveaux, int nb_niveaux);

#endif // POISSON_H
