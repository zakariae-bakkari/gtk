#include <gtk/gtk.h>
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/texte.h"
#include <stdio.h>

static void on_activate(GtkApplication *app, gpointer user_data)
{
   // 1. Configuration de la fenêtre
   Fenetre fenetre_config;
   fenetre_initialiser(&fenetre_config);
   fenetre_config.title = "Test du Widget Texte - Headings et Alignements";
   fenetre_config.taille.width = 900;
   fenetre_config.taille.height = 700;
   // fenetre_config.demarrer_maximisee = true;
   fenetre_config.titre_align = TITRE_ALIGN_DROITE;
   // fenetre_config.type = WIN_TYPE_POPUP;    // Supprimé : non supporté en GTK4
   // fenetre_config.position = WIN_POS_MOUSE; // Supprimé : non supporté en GTK4
   fenetre_set_scrollable(&fenetre_config, SCROLL_VERTICAL);

   GtkWidget *window = fenetre_creer(&fenetre_config);
   gtk_application_add_window(app, GTK_WINDOW(window));

   // Conteneur principal
   Conteneur conteneur_principal;
   conteneur_initialiser(&conteneur_principal);
   conteneur_principal.orientation = CONTENEUR_VERTICAL;
   conteneur_principal.espacement = 20;
   conteneur_principal.padding.haut = 20;
   conteneur_principal.padding.bas = 20;
   conteneur_principal.padding.gauche = 20;
   conteneur_principal.padding.droite = 20;

   GtkWidget *main_container = conteneur_creer(&conteneur_principal);

   // ===== SECTION 1: HEADINGS H1 à H6 =====
   Texte titre_section1;
   texte_initialiser(&titre_section1);
   titre_section1.texte = "DÉMONSTRATION DES HEADINGS";
   titre_section1.type = TEXTE_H1;
   titre_section1.alignement = TEXTE_ALIGN_CENTER;
   titre_section1.couleur_texte = "#2196F3";
   titre_section1.marges.bas = 10;

   GtkWidget *widget_titre1 = texte_creer(&titre_section1);
   conteneur_ajouter(&conteneur_principal, widget_titre1);

   // Conteneur pour les headings
   Conteneur section_headings;
   conteneur_initialiser(&section_headings);
   section_headings.orientation = CONTENEUR_VERTICAL;
   section_headings.espacement = 10;
   section_headings.couleur_fond = "#F5F5F5";
   section_headings.bordure_largeur = 1;
   section_headings.bordure_couleur = "#DDD";
   section_headings.bordure_rayon = 8;
   section_headings.padding.haut = 15;
   section_headings.padding.bas = 15;
   section_headings.padding.gauche = 15;
   section_headings.padding.droite = 15;

   GtkWidget *container_headings = conteneur_creer(&section_headings);

   // H1
   Texte h1;
   texte_initialiser(&h1);
   h1.texte = "Heading H1 - Le plus grand titre";
   h1.type = TEXTE_H1;
   h1.couleur_texte = "#1976D2";

   GtkWidget *widget_h1 = texte_creer(&h1);
   conteneur_ajouter(&section_headings, widget_h1);

   // H2
   Texte h2;
   texte_initialiser(&h2);
   h2.texte = "Heading H2 - Titre de section";
   h2.type = TEXTE_H2;
   h2.couleur_texte = "#388E3C";

   GtkWidget *widget_h2 = texte_creer(&h2);
   conteneur_ajouter(&section_headings, widget_h2);

   // H3
   Texte h3;
   texte_initialiser(&h3);
   h3.texte = "Heading H3 - Sous-section";
   h3.type = TEXTE_H3;
   h3.couleur_texte = "#F57C00";

   GtkWidget *widget_h3 = texte_creer(&h3);
   conteneur_ajouter(&section_headings, widget_h3);

   // H4, H5, H6
   Texte h4;
   texte_initialiser(&h4);
   h4.texte = "Heading H4 - Titre moyen";
   h4.type = TEXTE_H4;
   h4.couleur_texte = "#7B1FA2";

   GtkWidget *widget_h4 = texte_creer(&h4);
   conteneur_ajouter(&section_headings, widget_h4);

   Texte h5;
   texte_initialiser(&h5);
   h5.texte = "Heading H5 - Petit titre";
   h5.type = TEXTE_H5;
   h5.couleur_texte = "#C2185B";

   GtkWidget *widget_h5 = texte_creer(&h5);
   conteneur_ajouter(&section_headings, widget_h5);

   Texte h6;
   texte_initialiser(&h6);
   h6.texte = "Heading H6 - Le plus petit titre";
   h6.type = TEXTE_H6;
   h6.couleur_texte = "#5D4037";

   GtkWidget *widget_h6 = texte_creer(&h6);
   conteneur_ajouter(&section_headings, widget_h6);

   conteneur_ajouter(&conteneur_principal, container_headings);

   // ===== SECTION 2: ALIGNEMENTS =====
   Texte titre_section2;
   texte_initialiser(&titre_section2);
   titre_section2.texte = "ALIGNEMENTS DU TEXTE";
   titre_section2.type = TEXTE_H2;
   titre_section2.alignement = TEXTE_ALIGN_CENTER;
   titre_section2.couleur_texte = "#E91E63";
   titre_section2.marges.haut = 20;
   titre_section2.marges.bas = 10;

   GtkWidget *widget_titre2 = texte_creer(&titre_section2);
   conteneur_ajouter(&conteneur_principal, widget_titre2);

   // Conteneur pour les alignements
   Conteneur section_alignements;
   conteneur_initialiser(&section_alignements);
   section_alignements.orientation = CONTENEUR_VERTICAL;
   section_alignements.espacement = 15;
   section_alignements.couleur_fond = "#E8F5E8";
   section_alignements.bordure_largeur = 2;
   section_alignements.bordure_couleur = "#4CAF50";
   section_alignements.bordure_rayon = 8;
   section_alignements.padding.haut = 15;
   section_alignements.padding.bas = 15;
   section_alignements.padding.gauche = 15;
   section_alignements.padding.droite = 15;

   GtkWidget *container_alignements = conteneur_creer(&section_alignements);

   // Texte aligné à gauche
   Texte texte_gauche;
   texte_initialiser(&texte_gauche);
   texte_gauche.texte = "Ce texte est aligné à GAUCHE (par défaut)";
   texte_gauche.alignement = TEXTE_ALIGN_LEFT;
   texte_gauche.couleur_fond = "#FFEB3B";
   texte_gauche.bordure_largeur = 1;
   texte_gauche.bordure_couleur = "#F57F17";
   texte_gauche.bordure_rayon = 4;
   texte_gauche.marges.haut = 5;
   texte_gauche.marges.bas = 5;

   GtkWidget *widget_gauche = texte_creer(&texte_gauche);
   conteneur_ajouter(&section_alignements, widget_gauche);

   // Texte centré
   Texte texte_centre;
   texte_initialiser(&texte_centre);
   texte_centre.texte = "Ce texte est CENTRÉ";
   texte_centre.alignement = TEXTE_ALIGN_CENTER;
   texte_centre.couleur_fond = "#E1F5FE";
   texte_centre.bordure_largeur = 1;
   texte_centre.bordure_couleur = "#0277BD";
   texte_centre.bordure_rayon = 4;
   texte_centre.marges.haut = 5;
   texte_centre.marges.bas = 5;

   GtkWidget *widget_centre = texte_creer(&texte_centre);
   conteneur_ajouter(&section_alignements, widget_centre);

   // Texte aligné à droite
   Texte texte_droite;
   texte_initialiser(&texte_droite);
   texte_droite.texte = "Ce texte est aligné à DROITE";
   texte_droite.alignement = TEXTE_ALIGN_RIGHT;
   texte_droite.couleur_fond = "#FCE4EC";
   texte_droite.bordure_largeur = 1;
   texte_droite.bordure_couleur = "#C2185B";
   texte_droite.bordure_rayon = 4;
   texte_droite.marges.haut = 5;
   texte_droite.marges.bas = 5;

   GtkWidget *widget_droite = texte_creer(&texte_droite);
   conteneur_ajouter(&section_alignements, widget_droite);

   // Texte justifié avec wrap
   Texte texte_justifie;
   texte_initialiser(&texte_justifie);
   texte_justifie.texte = "Ce texte est JUSTIFIÉ et utilise le retour à la ligne automatique. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";
   texte_justifie.alignement = TEXTE_ALIGN_JUSTIFY;
   texte_justifie.wrap = true;
   texte_justifie.wrap_width = 60;
   texte_justifie.couleur_fond = "#F3E5F5";
   texte_justifie.bordure_largeur = 1;
   texte_justifie.bordure_couleur = "#9C27B0";
   texte_justifie.bordure_rayon = 4;
   texte_justifie.marges.haut = 5;
   texte_justifie.marges.bas = 5;

   GtkWidget *widget_justifie = texte_creer(&texte_justifie);
   conteneur_ajouter(&section_alignements, widget_justifie);

   conteneur_ajouter(&conteneur_principal, container_alignements);

   // ===== SECTION 3: STYLES ET DÉCORATIONS =====
   Texte titre_section3;
   texte_initialiser(&titre_section3);
   titre_section3.texte = "STYLES ET DÉCORATIONS";
   titre_section3.type = TEXTE_H2;
   titre_section3.alignement = TEXTE_ALIGN_CENTER;
   titre_section3.couleur_texte = "#FF5722";
   titre_section3.marges.haut = 20;
   titre_section3.marges.bas = 10;

   GtkWidget *widget_titre3 = texte_creer(&titre_section3);
   conteneur_ajouter(&conteneur_principal, widget_titre3);

   // Conteneur pour les styles
   Conteneur section_styles;
   conteneur_initialiser(&section_styles);
   section_styles.orientation = CONTENEUR_VERTICAL;
   section_styles.espacement = 12;
   section_styles.couleur_fond = "#FFF3E0";
   section_styles.bordure_largeur = 2;
   section_styles.bordure_couleur = "#FF9800";
   section_styles.bordure_rayon = 8;
   section_styles.padding.haut = 15;
   section_styles.padding.bas = 15;
   section_styles.padding.gauche = 15;
   section_styles.padding.droite = 15;

   GtkWidget *container_styles = conteneur_creer(&section_styles);

   // Texte gras et italique
   Texte texte_gras_italique;
   texte_initialiser(&texte_gras_italique);
   texte_gras_italique.texte = "Texte en GRAS et ITALIQUE avec police personnalisée";
   texte_gras_italique.gras = true;
   texte_gras_italique.italique = true;
   texte_gras_italique.famille_police = "Arial";
   texte_gras_italique.taille_police = 14;
   texte_gras_italique.couleur_texte = "#1565C0";

   GtkWidget *widget_gras_italique = texte_creer(&texte_gras_italique);
   conteneur_ajouter(&section_styles, widget_gras_italique);

   // Texte souligné
   Texte texte_souligne;
   texte_initialiser(&texte_souligne);
   texte_souligne.texte = "Ce texte est SOULIGNÉ";
   texte_souligne.decoration = TEXTE_DECORATION_UNDERLINE;
   texte_souligne.couleur_texte = "#2E7D32";
   texte_souligne.taille_police = 12;

   GtkWidget *widget_souligne = texte_creer(&texte_souligne);
   conteneur_ajouter(&section_styles, widget_souligne);

   // Texte barré
   Texte texte_barre;
   texte_initialiser(&texte_barre);
   texte_barre.texte = "Ce texte est BARRÉ";
   texte_barre.decoration = TEXTE_DECORATION_STRIKETHROUGH;
   texte_barre.couleur_texte = "#D32F2F";

   GtkWidget *widget_barre = texte_creer(&texte_barre);
   conteneur_ajouter(&section_styles, widget_barre);

   // Sous-titre et caption
   Texte sous_titre;
   texte_initialiser(&sous_titre);
   sous_titre.texte = "Ceci est un sous-titre";
   sous_titre.type = TEXTE_SUBTITLE;
   sous_titre.couleur_texte = "#6A1B9A";

   GtkWidget *widget_sous_titre = texte_creer(&sous_titre);
   conteneur_ajouter(&section_styles, widget_sous_titre);

   Texte caption;
   texte_initialiser(&caption);
   caption.texte = "Ceci est une légende/caption";
   caption.type = TEXTE_CAPTION;
   caption.couleur_texte = "#424242";

   GtkWidget *widget_caption = texte_creer(&caption);
   conteneur_ajouter(&section_styles, widget_caption);

   conteneur_ajouter(&conteneur_principal, container_styles);

   // ===== SECTION 4: TEXTE SÉLECTIONNABLE =====
   Texte titre_section4;
   texte_initialiser(&titre_section4);
   titre_section4.texte = "TEXTE SÉLECTIONNABLE";
   titre_section4.type = TEXTE_H2;
   titre_section4.alignement = TEXTE_ALIGN_CENTER;
   titre_section4.couleur_texte = "#795548";
   titre_section4.marges.haut = 20;
   titre_section4.marges.bas = 10;

   GtkWidget *widget_titre4 = texte_creer(&titre_section4);
   conteneur_ajouter(&conteneur_principal, widget_titre4);

   // Texte sélectionnable
   Texte texte_selectionnable;
   texte_initialiser(&texte_selectionnable);
   texte_selectionnable.texte = "Ce texte peut être SÉLECTIONNÉ avec la souris. Essayez de le sélectionner!";
   texte_selectionnable.selectable = true;
   texte_selectionnable.couleur_fond = "#E8EAF6";
   texte_selectionnable.bordure_largeur = 1;
   texte_selectionnable.bordure_couleur = "#3F51B5";
   texte_selectionnable.bordure_rayon = 4;
   texte_selectionnable.marges.haut = 10;
   texte_selectionnable.marges.bas = 10;
   texte_selectionnable.wrap = true;

   GtkWidget *widget_selectionnable = texte_creer(&texte_selectionnable);
   conteneur_ajouter(&conteneur_principal, widget_selectionnable);

   // Instructions
   Texte instructions;
   texte_initialiser(&instructions);
   instructions.texte = "💡 Instructions: Explorez les différents styles de texte ci-dessus. Le texte de la dernière section peut être sélectionné!";
   instructions.couleur_fond = "#E0F2F1";
   instructions.couleur_texte = "#00695C";
   instructions.bordure_largeur = 1;
   instructions.bordure_couleur = "#009688";
   instructions.bordure_rayon = 6;
   instructions.marges.haut = 20;
   instructions.wrap = true;

   GtkWidget *widget_instructions = texte_creer(&instructions);
   conteneur_ajouter(&conteneur_principal, widget_instructions);

   // Attacher le conteneur principal à la fenêtre
   if (fenetre_config.scroll_mode != SCROLL_NONE && fenetre_config.scroll_widget)
   {
      gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fenetre_config.scroll_widget), main_container);
   }
   else
   {
      gtk_window_set_child(GTK_WINDOW(window), main_container);
   }

   gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
   GtkApplication *app = gtk_application_new("com.example.test.texte", G_APPLICATION_DEFAULT_FLAGS);
   g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

   int status = g_application_run(G_APPLICATION(app), argc, argv);
   g_object_unref(app);

   return status;
}