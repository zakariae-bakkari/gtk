#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/bouton.h"
#include "../../widgets/headers/texte.h"
#include "../../widgets/headers/export_xml.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    // 1. Initialize and Create the Custom Fenetre with title as file name
    static Fenetre fenetre;
    fenetre_initialiser(&fenetre);
    g_free(fenetre.title);
    fenetre.title = malloc(strlen("exam2") + 1);
    strcpy(fenetre.title, "exam2");
    fenetre.taille.width = 800;
    fenetre.taille.height = 600;
    fenetre.scroll_mode = SCROLL_VERTICAL;
    fenetre.scroll_overlay = TRUE;

    fenetre.icon_path = "resources/icons/zcode.png";
    fenetre.ico_path = "resources/icons/zcode.ico";

    GtkWidget *window = fenetre_creer(&fenetre, app);

    #ifdef _WIN32
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, &fenetre);
    #endif

    // 2. Initialize and Create the Custom Conteneur main_box by default
    static Conteneur main_box;
    conteneur_initialiser(&main_box);
    main_box.orientation = CONTENEUR_VERTICAL;
    main_box.espacement = 15;
    main_box.padding.haut = 15;
    main_box.padding.bas = 15;
    main_box.padding.gauche = 20;
    main_box.padding.droite = 20;
    main_box.enfants_hexpand = TRUE;
    main_box.enfants_vexpand = TRUE;

    GtkWidget *p_main_box = conteneur_creer(&main_box);
    fenetre_ajouter(&fenetre, p_main_box);

    // 3. XML export setup
    ExportContext ctx;
    export_context_init(&ctx);
    export_ajouter_fenetre(&ctx, &fenetre);
    export_ajouter_conteneur(&ctx, &main_box);

    // --- Place your other widgets here and add them to the export context:
    
    // Add title text
    static Texte title_label;
    texte_initialiser(&title_label);
    title_label.texte = g_strdup("--- TEST DE DÉFILEMENT (SCROLLING TEST) ---");
    title_label.type = TEXTE_H1;
    title_label.gras = TRUE;
    title_label.alignement = TEXTE_ALIGN_CENTER;
    GtkWidget *p_title = texte_creer(&title_label);
    conteneur_ajouter(&main_box, p_title);
    export_ajouter_texte(&ctx, &title_label);

    // Add subtitle text
    static Texte subtitle_label;
    texte_initialiser(&subtitle_label);
    subtitle_label.texte = g_strdup("Faites défiler vers le bas pour voir la liste de 20 étiquettes et boutons.");
    subtitle_label.type = TEXTE_SUBTITLE;
    subtitle_label.alignement = TEXTE_ALIGN_CENTER;
    GtkWidget *p_subtitle = texte_creer(&subtitle_label);
    conteneur_ajouter(&main_box, p_subtitle);
    export_ajouter_texte(&ctx, &subtitle_label);

    // Arrays to hold configurations (must be static or dynamically allocated to persist)
    static Texte items_text[20];
    static Bouton items_btn[20];

    for (int i = 0; i < 20; i++) {
        // Add a text element
        char label_buf[128];
        snprintf(label_buf, sizeof(label_buf), "Élément de texte #%d - Zone de défilement vertical en action", i + 1);
        
        texte_initialiser(&items_text[i]);
        items_text[i].texte = g_strdup(label_buf);
        items_text[i].type = TEXTE_NORMAL;
        
        GtkWidget *p_txt = texte_creer(&items_text[i]);
        conteneur_ajouter(&main_box, p_txt);
        export_ajouter_texte(&ctx, &items_text[i]);

        // Add an interactive button
        char btn_buf[128];
        snprintf(btn_buf, sizeof(btn_buf), "Bouton Interactif #%d", i + 1);
        
        bouton_initialiser(&items_btn[i]);
        items_btn[i].texte = g_strdup(btn_buf);
        items_btn[i].taille.mode = TAILLE_FIXE;
        items_btn[i].taille.largeur = 250;
        items_btn[i].taille.hauteur = 40;
        
        GtkWidget *p_btn = bouton_creer(&items_btn[i]);
        conteneur_ajouter(&main_box, p_btn);
        export_ajouter_bouton(&ctx, &items_btn[i]);
    }

    // 4. Generate XML interface file
    generer_fichier_interface(&ctx, "interface.txt");

    // 5. Present the window
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("fr.exam.exam2", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
