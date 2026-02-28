#include "../headers/common.h"
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
   style->gras = false;
   style->italique = false;
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
   dest->bg_normal = src->bg_normal ? g_strdup(src->bg_normal) : NULL;
   dest->fg_normal = src->fg_normal ? g_strdup(src->fg_normal) : NULL;
   dest->epaisseur_bordure = src->epaisseur_bordure;
   dest->couleur_bordure = src->couleur_bordure ? g_strdup(src->couleur_bordure) : NULL;
   dest->rayon_arrondi = src->rayon_arrondi;
   dest->gras = src->gras;
   dest->italique = src->italique;
   dest->taille_texte_px = src->taille_texte_px;
   dest->couleur_bordure_error = src->couleur_bordure_error ? g_strdup(src->couleur_bordure_error) : NULL;
   dest->bg_error = src->bg_error ? g_strdup(src->bg_error) : NULL;
}