#include "poisson.h"
#include <stdlib.h>
#include <string.h>

Poisson *poisson_new(const char *nom)
{
   Poisson *p = (Poisson *)malloc(sizeof(Poisson));
   if (!p)
      return NULL;
   memset(p, 0, sizeof(Poisson));

   p->id = -1;
   if (nom)
      p->nom = strdup(nom);
   else
      p->nom = strdup("Poisson");

   p->niveaux_mangeable = (int *)malloc(5 * sizeof(int));
   p->niveaux_mangeable[0] = 0; /* Default: can eat level 0 (plankton) */
   p->nb_niveaux_mangeable = 1;
   p->x = p->y = 0.0;
   p->vx = 50.0; /* px/s par défaut */
   p->vy = 0.0;
   p->angle = 0.0;

   p->vitesse_normale = 50.0;
   p->vitesse_fuite = 80.0;
   p->vitesse_ralentie = 25.0;
   p->taille = 32;
   p->perimetre_detection = 100;

   p->etat = ETAT_NORMAL;
   p->temps_hors_cadre = 0;
   p->cote_sortie = 0;
   p->sante = 100.0;
   p->sante_max = 100.0;
   p->temps_effet_attaque = 0.0;

   /* kill stats */
   for (int i = 0; i < 8; i++) { p->kills_espece[i] = NULL; p->kills_count[i] = 0; }
   p->nb_kills_types = 0;
   p->dernier_attaquant = NULL;

   p->chemin_frames[0] = NULL;
   p->chemin_frames[1] = NULL;
   p->chemin_frames[2] = NULL;
   p->frame_courante = 0;
   p->temps_depuis_frame = 0.0;

   p->widget_image = NULL;
   p->visible = true;

   p->id_banc = -1;
   p->est_leader = false;

   return p;
}

void poisson_free(Poisson *p)
{
   if (!p)
      return;
   if (p->nom)
      free(p->nom);
   if (p->niveaux_mangeable)
      free(p->niveaux_mangeable);
   for (int i = 0; i < 3; ++i)
   {
      if (p->chemin_frames[i])
         free(p->chemin_frames[i]);
   }
   for (int i = 0; i < p->nb_kills_types; i++)
   {
      if (p->kills_espece[i]) free(p->kills_espece[i]);
   }
   /* Do not unref widget here: UI owner must handle widget lifecycle */
   free(p);
}

void poisson_set_position(Poisson *p, double x, double y)
{
   if (!p)
      return;
   p->x = x;
   p->y = y;
}

void poisson_set_widget(Poisson *p, GtkWidget *widget)
{
   if (!p)
      return;
   p->widget_image = widget;
}

void poisson_set_default_frames(Poisson *p, const char *frame0, const char *frame1, const char *frame2)
{
   if (!p)
      return;
   if (p->chemin_frames[0])
      free(p->chemin_frames[0]);
   if (p->chemin_frames[1])
      free(p->chemin_frames[1]);
   if (p->chemin_frames[2])
      free(p->chemin_frames[2]);

   p->chemin_frames[0] = frame0 ? strdup(frame0) : NULL;
   p->chemin_frames[1] = frame1 ? strdup(frame1) : NULL;
   p->chemin_frames[2] = frame2 ? strdup(frame2) : NULL;
}

void poisson_set_niveaux_mangeable(Poisson *p, const int *niveaux, int nb_niveaux)
{
   if (!p || !niveaux || nb_niveaux <= 0)
      return;
   if (p->niveaux_mangeable)
      free(p->niveaux_mangeable);
   p->niveaux_mangeable = (int *)malloc(nb_niveaux * sizeof(int));
   memcpy(p->niveaux_mangeable, niveaux, nb_niveaux * sizeof(int));
   p->nb_niveaux_mangeable = nb_niveaux;
}
