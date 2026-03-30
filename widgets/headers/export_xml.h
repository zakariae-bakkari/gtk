#ifndef EXPORT_TEXTE_H
#define EXPORT_TEXTE_H

#include "fenetre.h"
#include "champ_texte.h"
#include "champ_motdepasse.h"
#include "champ_select.h"
#include "champ_zone_texte.h"
#include "slider.h"

void generer_fichier_interface(Fenetre *fenetre,
                               ChampTexte *champ_nom,
                               ChampMotDePasse *champ_mdp,
                               ChampSelect *champ_select,
                               ChampZoneTexte *zone_texte,
                               Slider *slider);

#endif // EXPORT_TEXTE_H