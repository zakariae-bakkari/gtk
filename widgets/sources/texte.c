#include "../headers/texte.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Convertisseur interne pour l'alignement GTK */
static GtkAlign _convertir_widget_align(WidgetAlignment a)
{
   switch (a)
   {
   case WIDGET_ALIGN_START:
      return GTK_ALIGN_START;
   case WIDGET_ALIGN_END:
      return GTK_ALIGN_END;
   case WIDGET_ALIGN_CENTER:
      return GTK_ALIGN_CENTER;
   case WIDGET_ALIGN_FILL:
   default:
      return GTK_ALIGN_FILL;
   }
}

/* Convertisseur pour l'alignement du texte */
static GtkJustification _convertir_text_align(TexteAlignement alignement)
{
   switch (alignement)
   {
   case TEXTE_ALIGN_LEFT:
      return GTK_JUSTIFY_LEFT;
   case TEXTE_ALIGN_CENTER:
      return GTK_JUSTIFY_CENTER;
   case TEXTE_ALIGN_RIGHT:
      return GTK_JUSTIFY_RIGHT;
   case TEXTE_ALIGN_JUSTIFY:
      return GTK_JUSTIFY_FILL;
   default:
      return GTK_JUSTIFY_LEFT;
   }
}

/* Génère le markup Pango basé sur le type de heading */
static void _generer_markup_heading(char *buffer, size_t buffer_size, const char *texte, TexteType type)
{
   char format[128];

   switch (type)
   {
   case TEXTE_H1:
      strcpy(format, "<span size='xx-large' weight='bold'>%s</span>");
      break;
   case TEXTE_H2:
      strcpy(format, "<span size='x-large' weight='bold'>%s</span>");
      break;
   case TEXTE_H3:
      strcpy(format, "<span size='large' weight='bold'>%s</span>");
      break;
   case TEXTE_H4:
      strcpy(format, "<span size='medium' weight='bold'>%s</span>");
      break;
   case TEXTE_H5:
      strcpy(format, "<span size='small' weight='bold'>%s</span>");
      break;
   case TEXTE_H6:
      strcpy(format, "<span size='x-small' weight='bold'>%s</span>");
      break;
   case TEXTE_SUBTITLE:
      strcpy(format, "<span size='large' weight='normal' style='italic'>%s</span>");
      break;
   case TEXTE_CAPTION:
      strcpy(format, "<span size='small' weight='normal' style='italic'>%s</span>");
      break;
   case TEXTE_NORMAL:
   default:
      strncpy(buffer, texte, buffer_size - 1);
      buffer[buffer_size - 1] = '\0';
      return;
   }

   snprintf(buffer, buffer_size, format, texte);
}

/* Applique le style CSS au widget */
static void _texte_appliquer_css(GtkWidget *widget, Texte *config)
{
   GtkCssProvider *provider = gtk_css_provider_new();
   char css_data[2048] = "";
   char style_body[2048] = "";

   // Couleur de fond
   if (config->couleur_fond)
   {
      char buf[64];
      snprintf(buf, 64, "background-color: %s; ", config->couleur_fond);
      strcat(style_body, buf);
   }

   // Couleur du texte
   if (config->couleur_texte)
   {
      char buf[64];
      snprintf(buf, 64, "color: %s; ", config->couleur_texte);
      strcat(style_body, buf);
   }

   // Police personnalisée
   if (config->famille_police || config->taille_police > 0)
   {
      char buf[128];
      if (config->famille_police && config->taille_police > 0)
      {
         snprintf(buf, 128, "font-family: %s; font-size: %dpt; ",
                  config->famille_police, config->taille_police);
      }
      else if (config->famille_police)
      {
         snprintf(buf, 128, "font-family: %s; ", config->famille_police);
      }
      else if (config->taille_police > 0)
      {
         snprintf(buf, 128, "font-size: %dpt; ", config->taille_police);
      }
      strcat(style_body, buf);
   }

   // Style gras et italique
   if (config->gras)
   {
      strcat(style_body, "font-weight: bold; ");
   }
   if (config->italique)
   {
      strcat(style_body, "font-style: italic; ");
   }

   // Décoration du texte
   switch (config->decoration)
   {
   case TEXTE_DECORATION_UNDERLINE:
      strcat(style_body, "text-decoration: underline; ");
      break;
   case TEXTE_DECORATION_OVERLINE:
      strcat(style_body, "text-decoration: overline; ");
      break;
   case TEXTE_DECORATION_STRIKETHROUGH:
      strcat(style_body, "text-decoration: line-through; ");
      break;
   case TEXTE_DECORATION_NONE:
   default:
      break;
   }

   // Bordure
   if (config->bordure_largeur > 0 && config->bordure_couleur)
   {
      char buf[128];
      snprintf(buf, 128, "border: %dpx solid %s; ",
               config->bordure_largeur, config->bordure_couleur);
      strcat(style_body, buf);
   }

   // Rayon de bordure
   if (config->bordure_rayon > 0)
   {
      char buf[64];
      snprintf(buf, 64, "border-radius: %dpx; ", config->bordure_rayon);
      strcat(style_body, buf);
   }

   // Marges
   if (config->marges.haut > 0 || config->marges.gauche > 0)
   {
      char buf[128];
      snprintf(buf, 128, "margin: %dpx %dpx %dpx %dpx; ",
               config->marges.haut, config->marges.droite,
               config->marges.bas, config->marges.gauche);
      strcat(style_body, buf);
   }

   // Si on a du style à appliquer
   if (strlen(style_body) > 0)
   {
      snprintf(css_data, sizeof(css_data), "label { %s }", style_body);

      gtk_css_provider_load_from_string(provider, css_data);
      GtkStyleContext *context = gtk_widget_get_style_context(widget);
      gtk_style_context_add_provider(context,
                                     GTK_STYLE_PROVIDER(provider),
                                     GTK_STYLE_PROVIDER_PRIORITY_USER);
   }

   g_object_unref(provider);
}

void texte_initialiser(Texte *config)
{
   if (!config)
      return;

   config->widget = NULL;
   config->texte = NULL;
   config->texte_markup = NULL;
   config->use_markup = false;

   // Type et style par défaut
   config->type = TEXTE_NORMAL;
   config->alignement = TEXTE_ALIGN_LEFT;
   config->decoration = TEXTE_DECORATION_NONE;

   // Police par défaut
   config->famille_police = NULL;
   config->taille_police = 0;
   config->gras = false;
   config->italique = false;

   // Couleurs par défaut
   config->couleur_texte = NULL;
   config->couleur_fond = NULL;

   // Dimensions par défaut
   config->taille.largeur = -1;
   config->taille.hauteur = -1;
   config->marges.haut = 0;
   config->marges.bas = 0;
   config->marges.gauche = 0;
   config->marges.droite = 0;
   config->align_widget_x = WIDGET_ALIGN_FILL;
   config->align_widget_y = WIDGET_ALIGN_FILL;

   // Comportement par défaut
   config->selectable = false;
   config->wrap = false;
   config->wrap_width = -1;
   config->ellipsize = false;

   // Style par défaut
   widget_style_init(&config->style);
   config->id_css = NULL;

   // Bordure par défaut
   config->bordure_largeur = 0;
   config->bordure_couleur = NULL;
   config->bordure_rayon = 0;
}

GtkWidget *texte_creer(Texte *config)
{
   if (!config)
      return NULL;

   // Créer le widget label
   config->widget = gtk_label_new("");

   // Configurer le texte selon le type
   if (config->texte)
   {
      if (config->type != TEXTE_NORMAL || config->use_markup)
      {
         char markup_buffer[1024];

         if (config->use_markup && config->texte_markup)
         {
            gtk_label_set_markup(GTK_LABEL(config->widget), config->texte_markup);
         }
         else if (config->type != TEXTE_NORMAL)
         {
            _generer_markup_heading(markup_buffer, sizeof(markup_buffer),
                                    config->texte, config->type);
            gtk_label_set_markup(GTK_LABEL(config->widget), markup_buffer);
         }
         else
         {
            gtk_label_set_text(GTK_LABEL(config->widget), config->texte);
         }
      }
      else
      {
         gtk_label_set_text(GTK_LABEL(config->widget), config->texte);
      }
   }

   // Configurer l'alignement du texte
   gtk_label_set_justify(GTK_LABEL(config->widget),
                         _convertir_text_align(config->alignement));

   // Définir l'alignement du widget selon l'alignement du texte
   switch (config->alignement)
   {
   case TEXTE_ALIGN_LEFT:
      gtk_widget_set_halign(config->widget, GTK_ALIGN_START);
      break;
   case TEXTE_ALIGN_CENTER:
      gtk_widget_set_halign(config->widget, GTK_ALIGN_CENTER);
      break;
   case TEXTE_ALIGN_RIGHT:
      gtk_widget_set_halign(config->widget, GTK_ALIGN_END);
      break;
   case TEXTE_ALIGN_JUSTIFY:
      gtk_widget_set_halign(config->widget, GTK_ALIGN_FILL);
      break;
   }

   // Alignement vertical par défaut
   gtk_widget_set_valign(config->widget, GTK_ALIGN_START);

   // Note: Ne pas utiliser align_widget_x/y ici car cela override l'alignement du texte
   // Si vous voulez un contrôle précis du widget, utilisez les marges ou conteneurs

   // Dimensions
   if (config->taille.largeur != -1 || config->taille.hauteur != -1)
   {
      gtk_widget_set_size_request(config->widget,
                                  config->taille.largeur,
                                  config->taille.hauteur);
   }

   // Comportement du texte
   gtk_label_set_selectable(GTK_LABEL(config->widget), config->selectable);
   gtk_label_set_wrap(GTK_LABEL(config->widget), config->wrap);

   if (config->wrap && config->wrap_width > 0)
   {
      gtk_label_set_width_chars(GTK_LABEL(config->widget), config->wrap_width);
   }

   if (config->ellipsize)
   {
      gtk_label_set_ellipsize(GTK_LABEL(config->widget), PANGO_ELLIPSIZE_END);
   }

   // ID CSS
   if (config->id_css)
   {
      gtk_widget_set_name(config->widget, config->id_css);
   }

   // Appliquer le style CSS
   _texte_appliquer_css(config->widget, config);

   return config->widget;
}

// Fonctions utilitaires

void texte_set_text(Texte *config, const char *nouveau_texte)
{
   if (!config || !config->widget)
      return;

   g_free(config->texte);
   if (nouveau_texte)
   {
      config->texte = malloc(strlen(nouveau_texte) + 1);
      strcpy(config->texte, nouveau_texte);
   }
   else
   {
      config->texte = NULL;
   }

   if (config->type != TEXTE_NORMAL)
   {
      char markup_buffer[1024];
      _generer_markup_heading(markup_buffer, sizeof(markup_buffer),
                              nouveau_texte, config->type);
      gtk_label_set_markup(GTK_LABEL(config->widget), markup_buffer);
   }
   else
   {
      gtk_label_set_text(GTK_LABEL(config->widget), nouveau_texte);
   }
}

void texte_set_markup(Texte *config, const char *markup)
{
   if (!config || !config->widget)
      return;

   g_free(config->texte_markup);
   if (markup)
   {
      config->texte_markup = malloc(strlen(markup) + 1);
      strcpy(config->texte_markup, markup);
   }
   else
   {
      config->texte_markup = NULL;
   }
   config->use_markup = true;
   gtk_label_set_markup(GTK_LABEL(config->widget), markup);
}

void texte_set_type(Texte *config, TexteType type)
{
   if (!config)
      return;

   config->type = type;

   if (config->widget && config->texte)
   {
      texte_set_text(config, config->texte);
   }
}

void texte_set_alignement(Texte *config, TexteAlignement alignement)
{
   if (!config)
      return;

   config->alignement = alignement;

   if (config->widget)
   {
      gtk_label_set_justify(GTK_LABEL(config->widget),
                            _convertir_text_align(alignement));

      switch (alignement)
      {
      case TEXTE_ALIGN_LEFT:
         gtk_widget_set_halign(config->widget, GTK_ALIGN_START);
         break;
      case TEXTE_ALIGN_CENTER:
         gtk_widget_set_halign(config->widget, GTK_ALIGN_CENTER);
         break;
      case TEXTE_ALIGN_RIGHT:
         gtk_widget_set_halign(config->widget, GTK_ALIGN_END);
         break;
      case TEXTE_ALIGN_JUSTIFY:
         gtk_widget_set_halign(config->widget, GTK_ALIGN_FILL);
         break;
      }
   }
}

void texte_set_police(Texte *config, const char *famille, int taille, gboolean gras, gboolean italique)
{
   if (!config)
      return;

   g_free(config->famille_police);
   if (famille)
   {
      config->famille_police = malloc(strlen(famille) + 1);
      strcpy(config->famille_police, famille);
   }
   else
   {
      config->famille_police = NULL;
   }
   config->taille_police = taille;
   config->gras = gras;
   config->italique = italique;

   if (config->widget)
   {
      _texte_appliquer_css(config->widget, config);
   }
}

void texte_set_couleurs(Texte *config, const char *couleur_texte, const char *couleur_fond)
{
   if (!config)
      return;

   g_free(config->couleur_texte);
   if (couleur_texte)
   {
      config->couleur_texte = malloc(strlen(couleur_texte) + 1);
      strcpy(config->couleur_texte, couleur_texte);
   }
   else
   {
      config->couleur_texte = NULL;
   }

   g_free(config->couleur_fond);
   if (couleur_fond)
   {
      config->couleur_fond = malloc(strlen(couleur_fond) + 1);
      strcpy(config->couleur_fond, couleur_fond);
   }
   else
   {
      config->couleur_fond = NULL;
   }

   if (config->widget)
   {
      _texte_appliquer_css(config->widget, config);
   }
}

void texte_set_wrap(Texte *config, gboolean wrap, int width)
{
   if (!config)
      return;

   config->wrap = wrap;
   config->wrap_width = width;

   if (config->widget)
   {
      gtk_label_set_wrap(GTK_LABEL(config->widget), wrap);
      if (wrap && width > 0)
      {
         gtk_label_set_width_chars(GTK_LABEL(config->widget), width);
      }
   }
}

void texte_set_selectable(Texte *config, gboolean selectable)
{
   if (!config)
      return;

   config->selectable = selectable;

   if (config->widget)
   {
      gtk_label_set_selectable(GTK_LABEL(config->widget), selectable);
   }
}
