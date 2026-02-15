
#include "../headers/bouton_radio.h"
#include <stdio.h>
#include <string.h>

/* GSList para almacenar el grupo de radios - GTK4 usa GSList para los grupos */
static GSList *radio_groups = NULL;

/**
 * Fonction interne pour appliquer le CSS au bouton radio
 * Utilise gtk_widget_add_css_class (GTK4) au lieu des APIs dépréciées
 */
static void _bouton_radio_appliquer_css(BoutonRadio *config)
{
   if (!config->id_css || !config->widget)
      return;

   GtkCssProvider *provider = gtk_css_provider_new();
   char css_buffer[1024] = "";

   snprintf(css_buffer, sizeof(css_buffer),
            "#%s { "
            "   color: %s; "
            "   font-weight: %s; "
            "} "
            "#%s:hover { "
            "   color: %s; "
            "}",
            config->id_css,
            config->style.couleur_texte ? config->style.couleur_texte : "#000000",
            config->style.gras ? "bold" : "normal",
            config->id_css,
            config->style.couleur_texte_hover ? config->style.couleur_texte_hover : "#333333");

   gtk_css_provider_load_from_string(provider, css_buffer);

   // Ajouter le CSS à la feuille de style globale de l'application
   GtkStyleContext *context = gtk_widget_get_style_context(config->widget);
   if (context)
   {
      // En GTK4, on ajoute un CSS class au widget
      gtk_widget_add_css_class(config->widget, config->id_css);
   }

   g_object_unref(provider);
}

void bouton_radio_initialiser(BoutonRadio *config)
{
   if (!config)
      return;
   memset(config, 0, sizeof(BoutonRadio));

   config->id_css = "radio_defaut";
   config->label = "Option";
   config->pos_label = RADIO_LABEL_DROITE;
   config->est_actif = false;
   config->sensible = true;
   config->group_leader = NULL;

   // Style par défaut
   config->style.couleur_texte = "#000000";
   config->style.couleur_texte_hover = "#333333";
   config->style.couleur_point = NULL; // Laisse GTK gérer
   config->style.taille_texte_px = 0;  // Défaut
   config->style.gras = false;

   config->tooltip = NULL;
   config->on_toggled = NULL;
   config->user_data = NULL;
}

GtkWidget *bouton_radio_creer(BoutonRadio *config)
{
   if (!config)
      return NULL;

   // En GTK4, créer un GtkCheckButton qui agira comme un radio
   // Les radios sont groupés avec gtk_check_button_set_group()
   config->widget = gtk_check_button_new_with_label(config->label);

   // Définir le nom CSS AVANT d'appliquer les styles
   gtk_widget_set_name(config->widget, config->id_css);

   // Si un group_leader existe, ajouter ce checkbox au groupe
   if (config->group_leader)
   {
      gtk_check_button_set_group(GTK_CHECK_BUTTON(config->widget), config->group_leader);
   }

   // Définir l'état initial
   if (config->est_actif)
   {
      gtk_check_button_set_active(GTK_CHECK_BUTTON(config->widget), TRUE);
   }

   // Appliquer les styles CSS
   _bouton_radio_appliquer_css(config);

   // Définir la sensibilité
   gtk_widget_set_sensitive(config->widget, config->sensible);

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

void bouton_radio_set_groupe(BoutonRadio *config, GtkCheckButton *group_leader)
{
   if (!config || !config->widget || !group_leader)
      return;

   // Ajouter ce bouton au groupe du leader
   gtk_check_button_set_group(GTK_CHECK_BUTTON(config->widget), group_leader);
   config->group_leader = group_leader;
}

void bouton_radio_set_actif(BoutonRadio *config, bool actif)
{
   if (!config || !config->widget)
      return;

   gtk_check_button_set_active(GTK_CHECK_BUTTON(config->widget), actif);
   config->est_actif = actif;
}

bool bouton_radio_est_actif(BoutonRadio *config)
{
   if (!config || !config->widget)
      return false;

   return gtk_check_button_get_active(GTK_CHECK_BUTTON(config->widget));
}
