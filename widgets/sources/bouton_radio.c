#include "../headers/bouton_radio.h"
#include "../headers/common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static GtkCheckButton *bouton_radio_ensure_group_leader(BoutonRadio *config)
{
   if (!config)
      return NULL;

   if (config->group_leader)
      return config->group_leader;

   GtkWidget *leader = gtk_check_button_new();
   gtk_widget_set_visible(leader, FALSE);
   gtk_widget_set_can_focus(leader, FALSE);
   gtk_check_button_set_active(GTK_CHECK_BUTTON(leader), FALSE);
   return GTK_CHECK_BUTTON(leader);
}

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

   // BUG FIX: the provider was built but never actually applied — add it to the style context
   gtk_style_context_add_provider(
       gtk_widget_get_style_context(config->widget),
       GTK_STYLE_PROVIDER(provider),
       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

   g_object_unref(provider);
}

void bouton_radio_initialiser(BoutonRadio *config)
{
   if (!config)
      return;
   memset(config, 0, sizeof(BoutonRadio));

   config->id_css = malloc(strlen("radio_defaut") + 1);
   strcpy(config->id_css, "radio_defaut");
   config->label = malloc(strlen("Option") + 1);
   strcpy(config->label, "Option");
   config->pos_label = RADIO_LABEL_DROITE;
   config->est_actif = false;
   config->sensible = true;
   config->group_leader = NULL;

   // Style par défaut
   config->style.couleur_texte = malloc(strlen("#000000") + 1);
   strcpy(config->style.couleur_texte, "#000000");
   config->style.couleur_texte_hover = malloc(strlen("#333333") + 1);
   strcpy(config->style.couleur_texte_hover, "#333333");
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

   // GTK4: les radios sont implémentés avec GtkCheckButton groupés
   config->widget = gtk_check_button_new_with_label(config->label);

   // Définir le nom CSS AVANT d'appliquer les styles
   gtk_widget_set_name(config->widget, config->id_css);

   // Rejoindre un groupe réel pour obtenir l'indicateur radio.
   GtkCheckButton *leader = bouton_radio_ensure_group_leader(config);
   if (leader)
      gtk_check_button_set_group(GTK_CHECK_BUTTON(config->widget), leader);

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

   g_object_set_data(G_OBJECT(config->widget), "custom_struct", config);
   g_object_set_data(G_OBJECT(config->widget), "custom_type", "BoutonRadio");

   return config->widget;
}

void bouton_radio_set_groupe(BoutonRadio *config, GtkCheckButton *group_leader)
{
   if (!config || !group_leader)
      return;

   config->group_leader = group_leader;
   if (config->widget)
   {
      gtk_check_button_set_group(GTK_CHECK_BUTTON(config->widget), config->group_leader);
   }
}

void bouton_radio_set_actif(BoutonRadio *config, gboolean actif)
{
   if (!config || !config->widget)
      return;

   gtk_check_button_set_active(GTK_CHECK_BUTTON(config->widget), actif);
   config->est_actif = actif;
}

gboolean bouton_radio_est_actif(BoutonRadio *config)
{
   if (!config || !config->widget)
      return FALSE;

   return gtk_check_button_get_active(GTK_CHECK_BUTTON(config->widget));
}
