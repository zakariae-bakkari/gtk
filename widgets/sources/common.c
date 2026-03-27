#include "../headers/common.h"
#include <stdlib.h>
#include <string.h>
#include <glib.h>

// ====================== FONCTIONS UTILITAIRES ======================

void widget_style_init(WidgetStyle *style)
{
   if (!style)
      return;

   memset(style, 0, sizeof(WidgetStyle));

   // Valeurs par défaut
   style->bg_normal = NULL;
   style->fg_normal = NULL;
   style->epaisseur_bordure = 1;
   style->couleur_bordure = NULL;
   style->rayon_arrondi = 8;
   style->gras = FALSE;
   style->italique = FALSE;
   style->taille_texte_px = 0;
   style->couleur_bordure_error = NULL;
   style->bg_error = NULL;
}

void widget_style_free(WidgetStyle *style)
{
   if (!style)
      return;

   g_free(style->bg_normal);
   g_free(style->fg_normal);
   g_free(style->couleur_bordure);
   g_free(style->couleur_bordure_error);
   g_free(style->bg_error);

   // Réinitialiser à zéro pour éviter les double-free
   memset(style, 0, sizeof(WidgetStyle));
}

void widget_style_copy(WidgetStyle *dest, const WidgetStyle *src)
{
   if (!dest || !src)
      return;

   // Libérer l'ancien contenu de dest
   widget_style_free(dest);

   // Copier les valeurs
   if (src->bg_normal)
   {
      dest->bg_normal = malloc(strlen(src->bg_normal) + 1);
      strcpy(dest->bg_normal, src->bg_normal);
   }
   else
   {
      dest->bg_normal = NULL;
   }

   if (src->fg_normal)
   {
      dest->fg_normal = malloc(strlen(src->fg_normal) + 1);
      strcpy(dest->fg_normal, src->fg_normal);
   }
   else
   {
      dest->fg_normal = NULL;
   }
   dest->epaisseur_bordure = src->epaisseur_bordure;
   if (src->couleur_bordure)
   {
      dest->couleur_bordure = malloc(strlen(src->couleur_bordure) + 1);
      strcpy(dest->couleur_bordure, src->couleur_bordure);
   }
   else
   {
      dest->couleur_bordure = NULL;
   }
   dest->rayon_arrondi = src->rayon_arrondi;
   dest->gras = src->gras;
   dest->italique = src->italique;
   dest->taille_texte_px = src->taille_texte_px;
   if (src->couleur_bordure_error)
   {
      dest->couleur_bordure_error = malloc(strlen(src->couleur_bordure_error) + 1);
      strcpy(dest->couleur_bordure_error, src->couleur_bordure_error);
   }
   else
   {
      dest->couleur_bordure_error = NULL;
   }

   if (src->bg_error)
   {
      dest->bg_error = malloc(strlen(src->bg_error) + 1);
      strcpy(dest->bg_error, src->bg_error);
   }
   else
   {
      dest->bg_error = NULL;
   }
}
