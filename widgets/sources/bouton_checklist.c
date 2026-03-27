#include "../headers/bouton_checklist.h"
#include "../headers/common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Fonction interne pour appliquer le CSS à la case à cocher
 * Gère la couleur du texte et les états (normal/hover)
 */
static void _bouton_checklist_appliquer_css(BoutonChecklist *config)
{
   if (!config->id_css || !config->widget)
      return;

   GtkCssProvider *provider = gtk_css_provider_new();
   char css_buffer[1024] = "";

   snprintf(css_buffer, sizeof(css_buffer),
            "checkbutton#%s { "
            "   color: %s; "
            "   font-weight: %s; "
            "} "
            "checkbutton#%s:hover { "
            "   color: %s; "
            "}",
            config->id_css,
            config->style.couleur_texte ? config->style.couleur_texte : "#000000",
            config->style.gras ? "bold" : "normal",
            config->id_css,
            config->style.couleur_texte_hover ? config->style.couleur_texte_hover : "#333333");

   gtk_css_provider_load_from_string(provider, css_buffer);
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(config->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

   g_object_unref(provider);
}

void bouton_checklist_initialiser(BoutonChecklist *config)
{
   if (!config)
      return;
   memset(config, 0, sizeof(BoutonChecklist));

   config->id_css = malloc(strlen("checklist_defaut") + 1);
   strcpy(config->id_css, "checklist_defaut");
   config->label = malloc(strlen("Option") + 1);
   strcpy(config->label, "Option");
   config->pos_label = CHECKLIST_LABEL_DROITE;
   config->etat = CHECKLIST_UNCHECKED;
   config->est_actif = true;
   config->inconsistent = false;

   // Style par défaut
   config->style.couleur_texte = malloc(strlen("#000000") + 1);
   strcpy(config->style.couleur_texte, "#000000");
   config->style.couleur_texte_hover = malloc(strlen("#333333") + 1);
   strcpy(config->style.couleur_texte_hover, "#333333");
   config->style.taille_texte_px = 0; // Défaut
   config->style.gras = false;

   config->tooltip = NULL;
   config->on_toggled = NULL;
   config->user_data = NULL;
}

GtkWidget *bouton_checklist_creer(BoutonChecklist *config)
{
   if (!config)
      return NULL;

   // Créer le widget GtkCheckButton avec le label
   config->widget = gtk_check_button_new_with_label(config->label);

   // Définir le nom CSS AVANT d'appliquer les styles
   gtk_widget_set_name(config->widget, config->id_css);

   // Définir l'état initial
   if (config->etat == CHECKLIST_CHECKED)
   {
      gtk_check_button_set_active(GTK_CHECK_BUTTON(config->widget), TRUE);
   }
   else if (config->etat == CHECKLIST_INCONSISTENT)
   {
      gtk_check_button_set_inconsistent(GTK_CHECK_BUTTON(config->widget), TRUE);
   }
   else
   {
      gtk_check_button_set_active(GTK_CHECK_BUTTON(config->widget), FALSE);
   }

   // Appliquer les styles CSS
   _bouton_checklist_appliquer_css(config);

   // Définir la sensibilité (active ou pas)
   gtk_widget_set_sensitive(config->widget, config->est_actif);

   // Ajouter le tooltip s'il existe
   if (config->tooltip)
   {
      gtk_widget_set_tooltip_text(config->widget, config->tooltip);
   }

   // Connecter le signal "toggled" s'il existe un callback
   if (config->on_toggled)
   {
      g_signal_connect(config->widget, "toggled", G_CALLBACK(config->on_toggled), config->user_data);
   }

   return config->widget;
}

void bouton_checklist_set_etat(BoutonChecklist *config, BoutonChecklistEtat etat)
{
   if (!config || !config->widget)
      return;

   switch (etat)
   {
   case CHECKLIST_CHECKED:
      gtk_check_button_set_active(GTK_CHECK_BUTTON(config->widget), TRUE);
      gtk_check_button_set_inconsistent(GTK_CHECK_BUTTON(config->widget), FALSE);
      config->etat = CHECKLIST_CHECKED;
      break;

   case CHECKLIST_UNCHECKED:
      gtk_check_button_set_active(GTK_CHECK_BUTTON(config->widget), FALSE);
      gtk_check_button_set_inconsistent(GTK_CHECK_BUTTON(config->widget), FALSE);
      config->etat = CHECKLIST_UNCHECKED;
      break;

   case CHECKLIST_INCONSISTENT:
      gtk_check_button_set_inconsistent(GTK_CHECK_BUTTON(config->widget), TRUE);
      config->etat = CHECKLIST_INCONSISTENT;
      break;

   default:
      break;
   }
}

BoutonChecklistEtat bouton_checklist_get_etat(BoutonChecklist *config)
{
   if (!config || !config->widget)
      return CHECKLIST_UNCHECKED;

   // Vérifier d'abord l'état inconsistent
   if (gtk_check_button_get_inconsistent(GTK_CHECK_BUTTON(config->widget)))
   {
      return CHECKLIST_INCONSISTENT;
   }

   // Sinon retourner l'état actif/inactif
   if (gtk_check_button_get_active(GTK_CHECK_BUTTON(config->widget)))
   {
      return CHECKLIST_CHECKED;
   }
   else
   {
      return CHECKLIST_UNCHECKED;
   }
}
