#include <gtk/gtk.h>
#include "../../widgets/headers/fenetre.h"
#include "../../widgets/headers/conteneur.h"
#include "../../widgets/headers/bouton.h"
#include "../../widgets/headers/texte.h"

/* Helper : crée un label de titre de section */
static GtkWidget *make_title(const char *markup)
{
    Texte t;
    texte_initialiser(&t);
    t.texte = (char *)markup;
    t.type = TEXTE_H2;
    t.couleur_texte = "#1A237E";
    t.marges.haut = 18;
    t.marges.bas = 4;
    return texte_creer(&t);
}

/* Helper : crée un label de description */
static GtkWidget *make_desc(const char *text)
{
    Texte t;
    texte_initialiser(&t);
    t.texte = (char *)text;
    t.type = TEXTE_SUBTITLE;
    t.couleur_texte = "#546E7A";
    t.wrap = TRUE;
    t.alignement = TEXTE_ALIGN_LEFT;
    return texte_creer(&t);
}

/* Helper : crée un label normal centré */
static GtkWidget *make_label(const char *text)
{
    Texte t;
    texte_initialiser(&t);
    t.texte = (char *)text;
    t.type = TEXTE_NORMAL;
    t.alignement = TEXTE_ALIGN_CENTER;
    return texte_creer(&t);
}

/* Helper : crée un label normal aligné à gauche */
static GtkWidget *make_label_left(const char *text)
{
    Texte t;
    texte_initialiser(&t);
    t.texte = (char *)text;
    t.type = TEXTE_NORMAL;
    t.alignement = TEXTE_ALIGN_LEFT;
    return texte_creer(&t);
}

/* Helper : crée un bouton coloré avec texte */
static GtkWidget *make_colored_btn(const char *label_text, const char *color)
{
    Bouton b;
    bouton_initialiser(&b);
    b.texte = (char *)label_text;
    b.style.bg_normal = (char *)color;
    b.style.bg_hover = "#555555";
    b.style.fg_normal = "white";
    b.taille.mode = TAILLE_AUTO;
    return bouton_creer(&b);
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
    /* ================================================================
     * FENÊTRE PRINCIPALE  —  scrollable verticalement
     * ================================================================ */
    Fenetre win;
    fenetre_initialiser(&win);
    win.title = "Test complet des attributs Conteneur";
    win.taille.width = 750;
    win.taille.height = 600;
    fenetre_set_scrollable(&win, SCROLL_VERTICAL); // Activer le défilement vertical cette fonction exist dans fenetre.c et utilise l'enum partagé WidgetScrollMode pour configurer le défilement de la fenêtre

    GtkWidget *window = fenetre_creer(&win);
    gtk_application_add_window(app, GTK_WINDOW(window)); // Ajouter la fenêtre à l'application

    /* Racine : conteneur vertical qui recevra toutes les sections */
    Conteneur root;
    conteneur_initialiser(&root);
    root.orientation = CONTENEUR_VERTICAL; // ← orientation
    root.espacement = 12;                  // ← espacement
    root.padding.haut = 20;                // ← padding
    root.padding.bas = 20;
    root.padding.gauche = 20;
    root.padding.droite = 20;
    root.couleur_fond = "#FAFAFA";
    GtkWidget *root_widget = conteneur_creer(&root);

    /* ----------------------------------------------------------------
     * 1. ORIENTATION
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>1. orientation</big></b>"));
    gtk_box_append(GTK_BOX(root_widget),
                   make_desc("CONTENEUR_VERTICAL (haut) et CONTENEUR_HORIZONTAL (bas)"));

    // Vertical
    Conteneur c_vert;
    conteneur_initialiser(&c_vert);
    c_vert.orientation = CONTENEUR_VERTICAL; // ← orientation = VERTICAL
    c_vert.espacement = 6;
    c_vert.couleur_fond = "#E3F2FD";
    c_vert.bordure_largeur = 1;
    c_vert.bordure_couleur = "#1565C0";
    c_vert.bordure_rayon = 6;
    c_vert.padding.haut = c_vert.padding.bas =
        c_vert.padding.gauche = c_vert.padding.droite = 8;
    GtkWidget *w_vert = conteneur_creer(&c_vert);
    conteneur_ajouter(&c_vert, make_label("Item A"));
    conteneur_ajouter(&c_vert, make_label("Item B"));
    conteneur_ajouter(&c_vert, make_label("Item C"));
    gtk_box_append(GTK_BOX(root_widget), w_vert);

    // Horizontal
    Conteneur c_horiz;
    conteneur_initialiser(&c_horiz);
    c_horiz.orientation = CONTENEUR_HORIZONTAL; // ← orientation = HORIZONTAL
    c_horiz.espacement = 12;
    c_horiz.couleur_fond = "#E8F5E9";
    c_horiz.bordure_largeur = 1;
    c_horiz.bordure_couleur = "#2E7D32";
    c_horiz.bordure_rayon = 6;
    c_horiz.padding.haut = c_horiz.padding.bas =
        c_horiz.padding.gauche = c_horiz.padding.droite = 8;
    GtkWidget *w_horiz = conteneur_creer(&c_horiz);
    conteneur_ajouter(&c_horiz, make_label("Item A"));
    conteneur_ajouter(&c_horiz, make_label("Item B"));
    conteneur_ajouter(&c_horiz, make_label("Item C"));
    gtk_box_append(GTK_BOX(root_widget), w_horiz);

    /* ----------------------------------------------------------------
     * 2. ESPACEMENT
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>2. espacement</big></b>"));
    gtk_box_append(GTK_BOX(root_widget),
                   make_desc("Gauche = espacement 0 px  |  Droite = espacement 30 px"));

    Conteneur row_esp;
    conteneur_initialiser(&row_esp);
    row_esp.orientation = CONTENEUR_HORIZONTAL;
    row_esp.espacement = 16;
    GtkWidget *w_row_esp = conteneur_creer(&row_esp);

    Conteneur esp0;
    conteneur_initialiser(&esp0);
    esp0.orientation = CONTENEUR_HORIZONTAL;
    esp0.espacement = 0; // ← espacement = 0
    esp0.couleur_fond = "#FFF9C4";
    esp0.bordure_largeur = 1;
    esp0.bordure_couleur = "#F9A825";
    esp0.bordure_rayon = 4;
    esp0.padding.haut = esp0.padding.bas = esp0.padding.gauche = esp0.padding.droite = 6;
    GtkWidget *w_esp0 = conteneur_creer(&esp0);
    conteneur_ajouter(&esp0, make_colored_btn("A", "#F57F17"));
    conteneur_ajouter(&esp0, make_colored_btn("B", "#F57F17"));
    conteneur_ajouter(&esp0, make_colored_btn("C", "#F57F17"));

    Conteneur esp30;
    conteneur_initialiser(&esp30);
    esp30.orientation = CONTENEUR_HORIZONTAL;
    esp30.espacement = 30; // ← espacement = 30
    esp30.couleur_fond = "#FCE4EC";
    esp30.bordure_largeur = 1;
    esp30.bordure_couleur = "#C62828";
    esp30.bordure_rayon = 4;
    esp30.padding.haut = esp30.padding.bas = esp30.padding.gauche = esp30.padding.droite = 6;
    GtkWidget *w_esp30 = conteneur_creer(&esp30);
    conteneur_ajouter(&esp30, make_colored_btn("A", "#B71C1C"));
    conteneur_ajouter(&esp30, make_colored_btn("B", "#B71C1C"));
    conteneur_ajouter(&esp30, make_colored_btn("C", "#B71C1C"));

    conteneur_ajouter(&row_esp, w_esp0);
    conteneur_ajouter(&row_esp, w_esp30);
    gtk_box_append(GTK_BOX(root_widget), w_row_esp);

    /* ----------------------------------------------------------------
     * 3. HOMOGENE
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>3. homogene</big></b>"));
    gtk_box_append(GTK_BOX(root_widget),
                   make_desc("homogene=FALSE (gauche) vs homogene=TRUE — tous les enfants ont la même largeur (droite)"));

    Conteneur row_hom;
    conteneur_initialiser(&row_hom);
    row_hom.orientation = CONTENEUR_HORIZONTAL;
    row_hom.espacement = 16;
    GtkWidget *w_row_hom = conteneur_creer(&row_hom);

    Conteneur hom_false;
    conteneur_initialiser(&hom_false);
    hom_false.orientation = CONTENEUR_HORIZONTAL;
    hom_false.homogene = FALSE; // ← homogene = FALSE
    hom_false.espacement = 4;
    hom_false.couleur_fond = "#E8EAF6";
    hom_false.bordure_largeur = 1;
    hom_false.bordure_couleur = "#3949AB";
    hom_false.bordure_rayon = 4;
    hom_false.padding.haut = hom_false.padding.bas =
        hom_false.padding.gauche = hom_false.padding.droite = 6;
    GtkWidget *w_hom_false = conteneur_creer(&hom_false);
    conteneur_ajouter(&hom_false, make_colored_btn("Court", "#3949AB"));
    conteneur_ajouter(&hom_false, make_colored_btn("Beaucoup plus long", "#3949AB"));

    Conteneur hom_true;
    conteneur_initialiser(&hom_true);
    hom_true.orientation = CONTENEUR_HORIZONTAL;
    hom_true.homogene = TRUE; // ← homogene = TRUE
    hom_true.espacement = 4;
    hom_true.couleur_fond = "#EDE7F6";
    hom_true.bordure_largeur = 1;
    hom_true.bordure_couleur = "#6A1B9A";
    hom_true.bordure_rayon = 4;
    hom_true.padding.haut = hom_true.padding.bas =
        hom_true.padding.gauche = hom_true.padding.droite = 6;
    GtkWidget *w_hom_true = conteneur_creer(&hom_true);
    conteneur_ajouter(&hom_true, make_colored_btn("Court", "#6A1B9A"));
    conteneur_ajouter(&hom_true, make_colored_btn("Beaucoup plus long", "#6A1B9A"));

    conteneur_ajouter(&row_hom, w_hom_false);
    conteneur_ajouter(&row_hom, w_hom_true);
    gtk_box_append(GTK_BOX(root_widget), w_row_hom);

    /* ----------------------------------------------------------------
     * 4. TAILLE  (largeur / hauteur forcées)
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>4. taille (largeur / hauteur)</big></b>"));
    gtk_box_append(GTK_BOX(root_widget),
                   make_desc("Conteneur forcé à 300 × 80 px"));

    Conteneur c_taille;
    conteneur_initialiser(&c_taille);
    c_taille.orientation = CONTENEUR_VERTICAL;
    c_taille.taille.largeur = 300; // ← taille.largeur
    c_taille.taille.hauteur = 80;  // ← taille.hauteur
    c_taille.couleur_fond = "#FFF3E0";
    c_taille.bordure_largeur = 2;
    c_taille.bordure_couleur = "#E65100";
    c_taille.bordure_rayon = 6;
    GtkWidget *w_taille = conteneur_creer(&c_taille);
    conteneur_ajouter(&c_taille, make_label("300 × 80 px"));
    gtk_box_append(GTK_BOX(root_widget), w_taille);

    /* ----------------------------------------------------------------
     * 5. ALIGNEMENT  (align_x / align_y)
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>5. align_x / align_y</big></b>"));
    gtk_box_append(GTK_BOX(root_widget),
                   make_desc("DEBUT | CENTRE | FIN | REMPLIR  (horizontal)"));

    const char *align_labels[] = {"DEBUT", "CENTRE", "FIN", "REMPLIR"};
    ConteneurAlignement align_vals[] = {
        ALIGNEMENT_DEBUT, ALIGNEMENT_CENTRE, ALIGNEMENT_FIN, ALIGNEMENT_REMPLIR};
    const char *align_colors[] = {"#1565C0", "#2E7D32", "#6A1B9A", "#BF360C"};

    for (int i = 0; i < 4; i++)
    {
        Conteneur ca;
        conteneur_initialiser(&ca);
        ca.orientation = CONTENEUR_HORIZONTAL;
        ca.align_x = align_vals[i];     // ← align_x
        ca.align_y = ALIGNEMENT_CENTRE; // ← align_y
        ca.taille.hauteur = 40;
        ca.couleur_fond = "#FAFAFA";
        ca.bordure_largeur = 1;
        ca.bordure_couleur = align_colors[i];
        ca.bordure_rayon = 4;
        ca.padding.gauche = ca.padding.droite = 8;
        GtkWidget *wca = conteneur_creer(&ca);
        conteneur_ajouter(&ca, make_colored_btn(align_labels[i], align_colors[i]));
        gtk_box_append(GTK_BOX(root_widget), wca);
    }

    /* ----------------------------------------------------------------
     * 6. EXPANSION DES ENFANTS  (enfants_hexpand / enfants_vexpand)
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>6. enfants_hexpand / enfants_vexpand</big></b>"));
    gtk_box_append(GTK_BOX(root_widget),
                   make_desc("hexpand=FALSE (gauche) — boutons restent compacts\nhexpand=TRUE (droite) — boutons s'étirent"));

    Conteneur row_exp;
    conteneur_initialiser(&row_exp);
    row_exp.orientation = CONTENEUR_HORIZONTAL;
    row_exp.espacement = 16;
    GtkWidget *w_row_exp = conteneur_creer(&row_exp);

    Conteneur exp_false;
    conteneur_initialiser(&exp_false);
    exp_false.orientation = CONTENEUR_HORIZONTAL;
    exp_false.enfants_hexpand = FALSE; // ← enfants_hexpand = FALSE
    exp_false.espacement = 4;
    exp_false.couleur_fond = "#E0F2F1";
    exp_false.bordure_largeur = 1;
    exp_false.bordure_couleur = "#00695C";
    exp_false.bordure_rayon = 4;
    exp_false.padding.haut = exp_false.padding.bas =
        exp_false.padding.gauche = exp_false.padding.droite = 6;
    GtkWidget *w_exp_false = conteneur_creer(&exp_false);
    conteneur_ajouter(&exp_false, make_colored_btn("Btn1", "#00695C"));
    conteneur_ajouter(&exp_false, make_colored_btn("Btn2", "#00695C"));

    Conteneur exp_true;
    conteneur_initialiser(&exp_true);
    exp_true.orientation = CONTENEUR_HORIZONTAL;
    exp_true.enfants_hexpand = TRUE; // ← enfants_hexpand = TRUE
    exp_true.espacement = 4;
    exp_true.couleur_fond = "#E8F5E9";
    exp_true.bordure_largeur = 1;
    exp_true.bordure_couleur = "#1B5E20";
    exp_true.bordure_rayon = 4;
    exp_true.padding.haut = exp_true.padding.bas =
        exp_true.padding.gauche = exp_true.padding.droite = 6;
    GtkWidget *w_exp_true = conteneur_creer(&exp_true);
    // Apply hexpand manually to the children for this demo
    GtkWidget *b1 = make_colored_btn("Btn1", "#1B5E20");
    GtkWidget *b2 = make_colored_btn("Btn2", "#1B5E20");
    gtk_widget_set_hexpand(b1, TRUE);
    gtk_widget_set_halign(b1, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(b2, TRUE);
    gtk_widget_set_halign(b2, GTK_ALIGN_FILL);
    conteneur_ajouter(&exp_true, b1);
    conteneur_ajouter(&exp_true, b2);

    conteneur_ajouter(&row_exp, w_exp_false);
    conteneur_ajouter(&row_exp, w_exp_true);
    gtk_box_append(GTK_BOX(root_widget), w_row_exp);

    /* ----------------------------------------------------------------
     * 7. MARGES  (marges haut / bas / gauche / droite)
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>7. marges</big></b>"));
    gtk_box_append(GTK_BOX(root_widget),
                   make_desc("marges : haut=30  bas=5  gauche=60  droite=5"));

    Conteneur c_marges;
    conteneur_initialiser(&c_marges);
    c_marges.orientation = CONTENEUR_VERTICAL;
    c_marges.marges.haut = 30;   // ← marges.haut
    c_marges.marges.bas = 5;     // ← marges.bas
    c_marges.marges.gauche = 60; // ← marges.gauche
    c_marges.marges.droite = 5;  // ← marges.droite
    c_marges.couleur_fond = "#E1F5FE";
    c_marges.bordure_largeur = 2;
    c_marges.bordure_couleur = "#0277BD";
    c_marges.bordure_rayon = 4;
    GtkWidget *w_marges = conteneur_creer(&c_marges);
    conteneur_ajouter(&c_marges, make_label("marge haut=30 | gauche=60"));
    gtk_box_append(GTK_BOX(root_widget), w_marges);

    /* ----------------------------------------------------------------
     * 8. PADDING  (padding haut / bas / gauche / droite)
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>8. padding</big></b>"));
    gtk_box_append(GTK_BOX(root_widget),
                   make_desc("padding : haut=30  bas=5  gauche=50  droite=5"));

    Conteneur c_pad;
    conteneur_initialiser(&c_pad);
    c_pad.orientation = CONTENEUR_VERTICAL;
    c_pad.padding.haut = 30;   // ← padding.haut
    c_pad.padding.bas = 5;     // ← padding.bas
    c_pad.padding.gauche = 50; // ← padding.gauche
    c_pad.padding.droite = 5;  // ← padding.droite
    c_pad.couleur_fond = "#F3E5F5";
    c_pad.bordure_largeur = 2;
    c_pad.bordure_couleur = "#7B1FA2";
    c_pad.bordure_rayon = 4;
    GtkWidget *w_pad = conteneur_creer(&c_pad);
    conteneur_ajouter(&c_pad, make_label("padding haut=30 | gauche=50"));
    gtk_box_append(GTK_BOX(root_widget), w_pad);

    /* ----------------------------------------------------------------
     * 9. STYLE  (id_css / couleur_fond / bordure_largeur / couleur / rayon)
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>9. id_css · couleur_fond · bordure_*</big></b>"));
    gtk_box_append(GTK_BOX(root_widget),
                   make_desc("Trois conteneurs : fond différent, bordure épaisse, grand rayon arrondi"));

    Conteneur row_style;
    conteneur_initialiser(&row_style);
    row_style.orientation = CONTENEUR_HORIZONTAL;
    row_style.espacement = 12;
    GtkWidget *w_row_style = conteneur_creer(&row_style);

    // Fond coloré uniquement
    Conteneur s1;
    conteneur_initialiser(&s1);
    s1.orientation = CONTENEUR_VERTICAL;
    s1.id_css = "box_s1";        // ← id_css
    s1.couleur_fond = "#FFEB3B"; // ← couleur_fond
    s1.bordure_largeur = 0;      // ← bordure_largeur = 0 (aucune)
    s1.padding.haut = s1.padding.bas =
        s1.padding.gauche = s1.padding.droite = 10;
    GtkWidget *ws1 = conteneur_creer(&s1);
    conteneur_ajouter(&s1, make_label("fond jaune\nsans bordure"));
    conteneur_ajouter(&row_style, ws1);

    // Bordure épaisse
    Conteneur s2;
    conteneur_initialiser(&s2);
    s2.orientation = CONTENEUR_VERTICAL;
    s2.id_css = "box_s2";
    s2.couleur_fond = "#FFFFFF";
    s2.bordure_largeur = 5;         // ← bordure_largeur = 5
    s2.bordure_couleur = "#E91E63"; // ← bordure_couleur
    s2.bordure_rayon = 0;           // ← bordure_rayon = 0
    s2.padding.haut = s2.padding.bas =
        s2.padding.gauche = s2.padding.droite = 10;
    GtkWidget *ws2 = conteneur_creer(&s2);
    conteneur_ajouter(&s2, make_label("bordure 5px\nrayon=0"));
    conteneur_ajouter(&row_style, ws2);

    // Grand rayon arrondi
    Conteneur s3;
    conteneur_initialiser(&s3);
    s3.orientation = CONTENEUR_VERTICAL;
    s3.id_css = "box_s3";
    s3.couleur_fond = "#B2EBF2";
    s3.bordure_largeur = 2;
    s3.bordure_couleur = "#00838F";
    s3.bordure_rayon = 30; // ← bordure_rayon = 30
    s3.padding.haut = s3.padding.bas =
        s3.padding.gauche = s3.padding.droite = 14;
    GtkWidget *ws3 = conteneur_creer(&s3);
    conteneur_ajouter(&s3, make_label("rayon arrondi\n= 30 px"));
    conteneur_ajouter(&row_style, ws3);

    gtk_box_append(GTK_BOX(root_widget), w_row_style);

    /* ----------------------------------------------------------------
     * 10. DÉFILEMENT  (scroll_mode / scroll_min_width / scroll_min_height / scroll_overlay)
     * ---------------------------------------------------------------- */
    gtk_box_append(GTK_BOX(root_widget),
                   make_title("<b><big>10. scroll_mode · scroll_min_width/height · scroll_overlay</big></b>"));

    // 10a. SCROLL_VERTICAL
    gtk_box_append(GTK_BOX(root_widget), make_desc("SCROLL_VERTICAL — hauteur visible = 120 px, contenu = 400 px"));

    Conteneur sv;
    conteneur_initialiser(&sv);
    sv.orientation = CONTENEUR_VERTICAL;
    sv.espacement = 6;
    sv.couleur_fond = "#E8F5E9";
    sv.bordure_largeur = 2;
    sv.bordure_couleur = "#388E3C";
    sv.bordure_rayon = 6;
    sv.padding.haut = sv.padding.bas = sv.padding.gauche = sv.padding.droite = 6;
    conteneur_set_scrollable(&sv, SCROLL_VERTICAL); // ← scroll_mode = VERTICAL
    conteneur_set_scroll_size(&sv, 0, 120);         // ← scroll_min_height = 120
    conteneur_set_scroll_overlay(&sv, FALSE);       // ← scroll_overlay = FALSE (barres classiques)
    GtkWidget *wsv = conteneur_creer(&sv);
    for (int i = 1; i <= 12; i++)
    {
        char t[40];
        snprintf(t, sizeof(t), "Ligne verticale %d", i);
        conteneur_ajouter(&sv, make_label_left(t));
    }
    gtk_box_append(GTK_BOX(root_widget), wsv);

    // 10b. SCROLL_HORIZONTAL
    gtk_box_append(GTK_BOX(root_widget), make_desc("SCROLL_HORIZONTAL — largeur visible = 400 px, contenu = 900 px"));

    Conteneur sh;
    conteneur_initialiser(&sh);
    sh.orientation = CONTENEUR_HORIZONTAL;
    sh.espacement = 10;
    sh.couleur_fond = "#E3F2FD";
    sh.bordure_largeur = 2;
    sh.bordure_couleur = "#1565C0";
    sh.bordure_rayon = 6;
    sh.padding.haut = sh.padding.bas = sh.padding.gauche = sh.padding.droite = 6;
    conteneur_set_scrollable(&sh, SCROLL_HORIZONTAL); // ← scroll_mode = HORIZONTAL
    conteneur_set_scroll_size(&sh, 400, 0);           // ← scroll_min_width = 400
    conteneur_set_scroll_overlay(&sh, TRUE);          // ← scroll_overlay = TRUE (barres overlay)
    GtkWidget *wsh = conteneur_creer(&sh);
    for (int i = 1; i <= 10; i++)
    {
        char t[40];
        snprintf(t, sizeof(t), "Colonne %d (large)", i);
        Texte tx;
        texte_initialiser(&tx);
        tx.texte = g_strdup(t);
        tx.type = TEXTE_NORMAL;
        tx.alignement = TEXTE_ALIGN_CENTER;
        tx.taille.largeur = 90;
        GtkWidget *lbl = texte_creer(&tx);
        conteneur_ajouter(&sh, lbl);
    }
    gtk_box_append(GTK_BOX(root_widget), wsh);

    // 10c. SCROLL_BOTH
    gtk_box_append(GTK_BOX(root_widget), make_desc("SCROLL_BOTH — zone visible 300×120 px, contenu 700×400 px"));

    Conteneur sb;
    conteneur_initialiser(&sb);
    sb.orientation = CONTENEUR_VERTICAL;
    sb.espacement = 6;
    sb.couleur_fond = "#FFF3E0";
    sb.bordure_largeur = 2;
    sb.bordure_couleur = "#E65100";
    sb.bordure_rayon = 6;
    sb.padding.haut = sb.padding.bas = sb.padding.gauche = sb.padding.droite = 6;
    conteneur_set_scrollable(&sb, SCROLL_BOTH); // ← scroll_mode = BOTH
    conteneur_set_scroll_size(&sb, 300, 120);   // ← scroll_min_width + height
    conteneur_set_scroll_overlay(&sb, TRUE);
    GtkWidget *wsb = conteneur_creer(&sb);
    for (int i = 1; i <= 10; i++)
    {
        char t[80];
        snprintf(t, sizeof(t), "Ligne %d — texte très large pour forcer le défilement horizontal aussi", i);
        Texte tx;
        texte_initialiser(&tx);
        tx.texte = g_strdup(t);
        tx.type = TEXTE_NORMAL;
        tx.alignement = TEXTE_ALIGN_LEFT;
        tx.taille.largeur = 700;
        GtkWidget *lbl = texte_creer(&tx);
        conteneur_ajouter(&sb, lbl);
    }
    gtk_box_append(GTK_BOX(root_widget), wsb);

    /* ================================================================
     * ATTACHEMENT FINAL
     * ================================================================ */
    if (win.scroll_mode != SCROLL_NONE && win.scroll_widget)
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(win.scroll_widget), root_widget);
    else
        gtk_window_set_child(GTK_WINDOW(window), root_widget);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *app = gtk_application_new("com.example.conteneur.test", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
