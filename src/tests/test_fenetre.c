#include <gtk/gtk.h>
#include "../headers/fenetre.h"
#include "../headers/bouton.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

// ✅ Chemins absolus construits dynamiquement
static char ICON_PATH[512] = {0};
static char IMAGE_PATH[512] = {0};

// ✅ Ajoute ce chemin
static char ICO_PATH[512] = {0};

static void init_chemins(void)
{
#ifdef _WIN32
    char exe_dir[512] = {0};
    GetModuleFileNameA(NULL, exe_dir, sizeof(exe_dir));
    char *last = strrchr(exe_dir, '\\');
    if (last)
        *last = '\0';
    last = strrchr(exe_dir, '\\');
    if (last)
        *last = '\0';

    snprintf(ICON_PATH, sizeof(ICON_PATH),
             "%s\\resources\\icons\\zcode.png", exe_dir);
    snprintf(IMAGE_PATH, sizeof(IMAGE_PATH),
             "%s\\resources\\images\\zcode.png", exe_dir);
    snprintf(ICO_PATH, sizeof(ICO_PATH),
             "%s\\resources\\icons\\zcode.ico", exe_dir); // ✅

    // Convertir \ en / pour GTK CSS
    for (char *p = ICON_PATH; *p; p++)
        if (*p == '\\')
            *p = '/';
    for (char *p = IMAGE_PATH; *p; p++)
        if (*p == '\\')
            *p = '/';
    // ICO_PATH garde les \ pour Win32 LoadImageA

    fprintf(stdout, "[INIT] ICON_PATH  = %s (%s)\n",
            ICON_PATH, g_file_test(ICON_PATH, G_FILE_TEST_EXISTS) ? "OUI" : "NON");
    fprintf(stdout, "[INIT] IMAGE_PATH = %s (%s)\n",
            IMAGE_PATH, g_file_test(IMAGE_PATH, G_FILE_TEST_EXISTS) ? "OUI" : "NON");
    fprintf(stdout, "[INIT] ICO_PATH   = %s (%s)\n",
            ICO_PATH, g_file_test(ICO_PATH, G_FILE_TEST_EXISTS) ? "OUI" : "NON");
    fflush(stdout);
#endif
}

// ✅ creer_fenetre_complete utilise ico_path pour la taskbar
static GtkWidget *creer_fenetre_complete(Fenetre *f, GtkApplication *app)
{
    f->icon_path = ICON_PATH; // PNG pour HeaderBar
    f->ico_path = ICO_PATH;   // ICO pour taskbar Windows

    GtkWidget *win = fenetre_creer(f, app);

    // Icône taskbar après 100ms (surface pas encore dispo avant)
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, f);

    // Position après 50ms
    if (f->position != WIN_POS_DEFAULT)
        g_timeout_add(50, (GSourceFunc)fenetre_appliquer_position, f);

    return win;
}

// ─── Appliquer position APRÈS affichage ──────────────────────
typedef struct
{
    Fenetre *f;
} PosData;

static gboolean _appliquer_position_idle(gpointer data)
{
    Fenetre *f = (Fenetre *)data;
    fenetre_appliquer_position(f);
    return G_SOURCE_REMOVE; // appel unique
}

static void appliquer_position_apres_affichage(Fenetre *f)
{
    // Délai pour laisser GTK réaliser la fenêtre
    g_timeout_add(50, _appliquer_position_idle, f);
}

// ─── Helper bouton ───────────────────────────────────────────
static GtkWidget *creer_btn(const char *id, const char *texte,
                            const char *bg, const char *hover,
                            GCallback cb, gpointer data)
{
    static int cpt = 0;
    cpt++;
    Bouton *btn = g_new0(Bouton, 1);
    char id_buffer[256];
    bouton_initialiser(btn);
    snprintf(id_buffer, sizeof(id_buffer), "%s_%d", id, cpt);
    btn->id_css = malloc(strlen(id_buffer) + 1);
    strcpy(btn->id_css, id_buffer);
    btn->texte = malloc(strlen(texte) + 1);
    strcpy(btn->texte, texte);
    btn->style.bg_normal = malloc(strlen(bg) + 1);
    strcpy(btn->style.bg_normal, bg);
    btn->style.bg_hover = malloc(strlen(hover) + 1);
    strcpy(btn->style.bg_hover, hover);
    btn->taille.mode = TAILLE_FIXE;
    btn->taille.largeur = 380;
    btn->taille.hauteur = 46;
    btn->on_clic = (BoutonAction)cb;
    btn->user_data = data;
    return bouton_creer(btn);
}

// ─── Helper label ────────────────────────────────────────────
static GtkWidget *lbl(const char *txt)
{
    GtkWidget *l = gtk_label_new(txt);
    gtk_widget_set_halign(l, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(l, GTK_ALIGN_CENTER);
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_string(p,
                                      "label { color:#111; font-size:14px; font-weight:bold; }");
    gtk_style_context_add_provider(gtk_widget_get_style_context(l),
                                   GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(p);
    return l;
}

// ─── Callbacks internes ──────────────────────────────────────
static void on_restaurer(GtkWidget *w, gpointer data)
{
    gtk_window_unmaximize(GTK_WINDOW(((Fenetre *)data)->wind));
}
static void on_maximiser(GtkWidget *w, gpointer data)
{
    gtk_window_maximize(GTK_WINDOW(((Fenetre *)data)->wind));
}
static void on_fermer_popup(GtkWidget *w, gpointer data)
{
    if (GTK_IS_WINDOW(data))
        gtk_window_destroy(GTK_WINDOW(data));
}

// ════════════════════════════════════════════════════════════
// TEST 1 : Couleur fond + titre GAUCHE
// ════════════════════════════════════════════════════════════
static void test_basique(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 1 — Couleur fond") + 1);
    strcpy(f.title, "Test 1 — Couleur fond");
    f.taille.width = 420;
    f.taille.height = 220;
    f.color_bg = malloc(strlen("#d6eaf8") + 1);
    strcpy(f.color_bg, "#d6eaf8");
    f.titre_align = TITRE_ALIGN_GAUCHE;
    f.bouton_agrandir = false;
    f.bouton_reduire = false;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_child(GTK_WINDOW(win),
                         lbl("✅ color_bg=#d6eaf8\n✅ titre_align=GAUCHE\n✅ bouton_agrandir=false\n✅ bouton_reduire=false"));
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 2 : Titre CENTRE
// ════════════════════════════════════════════════════════════
static void test_titre_centre(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Titre Centré — Test 2") + 1);
    strcpy(f.title, "Titre Centré — Test 2");
    f.taille.width = 400;
    f.taille.height = 180;
    f.color_bg = malloc(strlen("#d5f5e3") + 1);
    strcpy(f.color_bg, "#d5f5e3");
    f.titre_align = TITRE_ALIGN_CENTRE;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_child(GTK_WINDOW(win), lbl("✅ titre_align=CENTRE"));
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 3 : Titre DROITE
// ════════════════════════════════════════════════════════════
static void test_titre_droite(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 3 — TITRE_ALIGN_DROITE") + 1);
    strcpy(f.title, "Test 3 — TITRE_ALIGN_DROITE");
    f.taille.width = 420;
    f.taille.height = 180;
    f.color_bg = malloc(strlen("#fdebd0") + 1);
    strcpy(f.color_bg, "#fdebd0");
    f.titre_align = TITRE_ALIGN_DROITE;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_child(GTK_WINDOW(win),
                         lbl("✅ f.titre_align = TITRE_ALIGN_DROITE"));
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 4 : Icône zcode.png partout
// ════════════════════════════════════════════════════════════
static void test_icone(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 4 — Icône zcode.png") + 1);
    strcpy(f.title, "Test 4 — Icône zcode.png");
    f.taille.width = 460;
    f.taille.height = 300;
    f.color_bg = malloc(strlen("#e8daef") + 1);
    strcpy(f.color_bg, "#e8daef");
    f.icon_path = ICON_PATH;
    GtkWidget *win = creer_fenetre_complete(&f, app);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    if (g_file_test(ICON_PATH, G_FILE_TEST_EXISTS))
    {
        GtkWidget *img = gtk_image_new_from_file(ICON_PATH);
        gtk_image_set_pixel_size(GTK_IMAGE(img), 80);
        gtk_box_append(GTK_BOX(box), img);
        gtk_box_append(GTK_BOX(box),
                       lbl("✅ Icône dans HeaderBar\n✅ Icône dans taskbar Windows\n✅ Aperçu 80px"));
    }
    else
    {
        char msg[300];
        snprintf(msg, sizeof(msg), "⚠️ Fichier non trouvé:\n%s", ICON_PATH);
        gtk_box_append(GTK_BOX(box), lbl(msg));
    }
    gtk_window_set_child(GTK_WINDOW(win), box);
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 5 : Icône + texte dans le titre
// ════════════════════════════════════════════════════════════
static void test_titre_avec_icone(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("ignoré") + 1);
    strcpy(f.title, "ignoré");
    f.taille.width = 440;
    f.taille.height = 200;
    f.color_bg = malloc(strlen("#fef9e7") + 1);
    strcpy(f.color_bg, "#fef9e7");
    GtkWidget *win = creer_fenetre_complete(&f, app);
    GtkWidget *header = gtk_window_get_titlebar(GTK_WINDOW(win));
    GtkWidget *tbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_widget_set_halign(tbox, GTK_ALIGN_CENTER);
    GtkWidget *icon = g_file_test(ICON_PATH, G_FILE_TEST_EXISTS)
                          ? gtk_image_new_from_file(ICON_PATH)
                          : gtk_image_new_from_icon_name("starred-symbolic");
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 20);
    gtk_box_append(GTK_BOX(tbox), icon);
    gtk_box_append(GTK_BOX(tbox), gtk_label_new("Mon Application"));
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), tbox);
    gtk_window_set_child(GTK_WINDOW(win),
                         lbl("✅ Icône zcode + texte dans HeaderBar\n✅ Widget titre personnalisé"));
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 6 : Background image  ✅ chemin absolu file:///
// ════════════════════════════════════════════════════════════
static void test_background_image(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 6 — Background Image") + 1);
    strcpy(f.title, "Test 6 — Background Image");
    f.taille.width = 500;
    f.taille.height = 350;

    // ✅ GTK CSS url() nécessite file:/// avec chemin absolu
    static char css_image_path[600];
    const char *src = NULL;
    if (g_file_test(IMAGE_PATH, G_FILE_TEST_EXISTS))
        src = IMAGE_PATH;
    else if (g_file_test(ICON_PATH, G_FILE_TEST_EXISTS))
        src = ICON_PATH;

    if (src)
    {
        snprintf(css_image_path, sizeof(css_image_path), "file:///%s", src);
        f.background_image = css_image_path;
        fprintf(stdout, "[TEST6] background_image = %s\n", css_image_path);
    }
    else
    {
        f.color_bg = malloc(strlen("#aed6f1") + 1);
        strcpy(f.color_bg, "#aed6f1");
        fprintf(stdout, "[TEST6] Aucune image, color_bg de secours\n");
    }
    fflush(stdout);

    GtkWidget *win = creer_fenetre_complete(&f, app);

    char msg[300];
    snprintf(msg, sizeof(msg),
             src ? "✅ background_image:\n%s" : "⚠️ Image non trouvée\ncolor_bg=#aed6f1",
             src ? src : "");
    GtkWidget *l = gtk_label_new(msg);
    gtk_widget_set_halign(l, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(l, GTK_ALIGN_CENTER);
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_string(p, src
                                             ? "label{color:white;font-size:13px;font-weight:bold;"
                                               "background-color:rgba(0,0,0,0.6);padding:10px 16px;border-radius:8px;}"
                                             : "label{color:#111;font-size:14px;font-weight:bold;}");
    gtk_style_context_add_provider(gtk_widget_get_style_context(l),
                                   GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(p);
    gtk_window_set_child(GTK_WINDOW(win), l);
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 7 : Scroll vertical
// ════════════════════════════════════════════════════════════
static void test_scroll_vertical(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 7 — Scroll Vertical") + 1);
    strcpy(f.title, "Test 7 — Scroll Vertical");
    f.taille.width = 400;
    f.taille.height = 280;
    f.color_bg = malloc(strlen("#eaf4fb") + 1);
    strcpy(f.color_bg, "#eaf4fb");
    f.scroll_mode = SCROLL_VERTICAL;
    f.scroll_overlay = true;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_margin_top(box, 10);
    gtk_widget_set_margin_start(box, 15);
    for (int i = 1; i <= 30; i++)
    {
        char txt[64];
        snprintf(txt, sizeof(txt), "Ligne %d — scroll vertical", i);
        GtkWidget *l = gtk_label_new(txt);
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, "label{color:#1a1a1a;}");
        gtk_style_context_add_provider(gtk_widget_get_style_context(l),
                                       GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
        gtk_box_append(GTK_BOX(box), l);
    }
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(fenetre_get_content_container(&f)), box);
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 8 : Scroll horizontal
// ════════════════════════════════════════════════════════════
static void test_scroll_horizontal(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 8 — Scroll Horizontal") + 1);
    strcpy(f.title, "Test 8 — Scroll Horizontal");
    f.taille.width = 300;
    f.taille.height = 180;
    f.color_bg = malloc(strlen("#e9f7ef") + 1);
    strcpy(f.color_bg, "#e9f7ef");
    f.scroll_mode = SCROLL_HORIZONTAL;
    f.scroll_overlay = false;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_set_margin_top(box, 20);
    for (int i = 1; i <= 25; i++)
    {
        char txt[32];
        snprintf(txt, sizeof(txt), " Col%d ", i);
        GtkWidget *l = gtk_label_new(txt);
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, "label{color:#1a1a1a;}");
        gtk_style_context_add_provider(gtk_widget_get_style_context(l),
                                       GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
        gtk_box_append(GTK_BOX(box), l);
    }
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(fenetre_get_content_container(&f)), box);
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 9 : Scroll both
// ════════════════════════════════════════════════════════════
static void test_scroll_both(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 9 — Scroll Both") + 1);
    strcpy(f.title, "Test 9 — Scroll Both");
    f.taille.width = 320;
    f.taille.height = 260;
    f.color_bg = malloc(strlen("#fdfefe") + 1);
    strcpy(f.color_bg, "#fdfefe");
    f.scroll_mode = SCROLL_BOTH;
    f.scroll_overlay = true;
    f.content_min_width = 600;
    f.content_min_height = 500;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    for (int r = 0; r < 18; r++)
    {
        for (int c = 0; c < 12; c++)
        {
            char txt[24];
            snprintf(txt, sizeof(txt), "R%d-C%d", r, c);
            GtkWidget *l = gtk_label_new(txt);
            GtkCssProvider *p = gtk_css_provider_new();
            gtk_css_provider_load_from_string(p, "label{color:#111;}");
            gtk_style_context_add_provider(gtk_widget_get_style_context(l),
                                           GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
            g_object_unref(p);
            gtk_grid_attach(GTK_GRID(grid), l, c, r, 1, 1);
        }
    }
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(fenetre_get_content_container(&f)), grid);
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 10 : Maximisée
// ════════════════════════════════════════════════════════════
static void test_maximisee(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 10 — Maximisée") + 1);
    strcpy(f.title, "Test 10 — Maximisée");
    f.demarrer_maximisee = true;
    f.color_bg = malloc(strlen("#f2f3f4") + 1);
    strcpy(f.color_bg, "#f2f3f4");
    GtkWidget *win = creer_fenetre_complete(&f, app);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), lbl("✅ demarrer_maximisee=true"));
    gtk_box_append(GTK_BOX(box),
                   creer_btn("r1", "Restaurer", "#e74c3c", "#c0392b", G_CALLBACK(on_restaurer), &f));
    gtk_box_append(GTK_BOX(box),
                   creer_btn("m1", "Re-maximiser", "#27ae60", "#1e8449", G_CALLBACK(on_maximiser), &f));
    gtk_window_set_child(GTK_WINDOW(win), box);
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 11 : Sans boutons
// ════════════════════════════════════════════════════════════
static void test_sans_boutons(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 11 — Sans boutons") + 1);
    strcpy(f.title, "Test 11 — Sans boutons");
    f.taille.width = 400;
    f.taille.height = 200;
    f.color_bg = malloc(strlen("#fef5e7") + 1);
    strcpy(f.color_bg, "#fef5e7");
    f.bouton_fermer = false;
    f.bouton_agrandir = false;
    f.bouton_reduire = false;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_child(GTK_WINDOW(win),
                         lbl("✅ bouton_fermer=false\n✅ bouton_agrandir=false\n✅ bouton_reduire=false\nFermez avec Alt+F4"));
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 12 : Non redimensionnable
// ════════════════════════════════════════════════════════════
static void test_non_redim(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 12 — Non redimensionnable") + 1);
    strcpy(f.title, "Test 12 — Non redimensionnable");
    f.taille.width = 420;
    f.taille.height = 190;
    f.color_bg = malloc(strlen("#eafaf1") + 1);
    strcpy(f.color_bg, "#eafaf1");
    f.resizable = false;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_child(GTK_WINDOW(win),
                         lbl("✅ resizable=false\n✅ Bouton maximiser masqué auto"));
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 13 : Popup
// ════════════════════════════════════════════════════════════
static void test_popup(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 13 — Popup") + 1);
    strcpy(f.title, "Test 13 — Popup");
    f.taille.width = 340;
    f.taille.height = 200;
    f.color_bg = malloc(strlen("#f9ebea") + 1);
    strcpy(f.color_bg, "#f9ebea");
    f.type = WIN_TYPE_POPUP;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_decorated(GTK_WINDOW(win), FALSE);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_bottom(box, 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box),
                   lbl("✅ WIN_TYPE_POPUP\n✅ Sans décoration\n✅ Fermeture corrigée"));
    gtk_box_append(GTK_BOX(box),
                   creer_btn("cp", "Fermer ce popup", "#e74c3c", "#c0392b",
                             G_CALLBACK(on_fermer_popup), win));
    gtk_window_set_child(GTK_WINDOW(win), box);
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 14 : Position WIN_POS_CENTER  ✅ Win32 réel
// ════════════════════════════════════════════════════════════
static void test_position_center(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 14 — Position CENTER") + 1);
    strcpy(f.title, "Test 14 — Position CENTER");
    f.taille.width = 420;
    f.taille.height = 200;
    f.color_bg = malloc(strlen("#d6eaf8") + 1);
    strcpy(f.color_bg, "#d6eaf8");
    f.position = WIN_POS_CENTER;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_child(GTK_WINDOW(win),
                         lbl("✅ WIN_POS_CENTER\nFenêtre centrée via Win32 SetWindowPos\nAprès 50ms délai"));
    gtk_window_present(GTK_WINDOW(win));
    // ✅ Position appliquée après affichage via g_timeout
    appliquer_position_apres_affichage(&f);
}

// ════════════════════════════════════════════════════════════
// TEST 15 : Position WIN_POS_MOUSE  ✅ Win32 réel
// ════════════════════════════════════════════════════════════
static void test_position_mouse(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 15 — Position MOUSE") + 1);
    strcpy(f.title, "Test 15 — Position MOUSE");
    f.taille.width = 380;
    f.taille.height = 200;
    f.color_bg = malloc(strlen("#d5f5e3") + 1);
    strcpy(f.color_bg, "#d5f5e3");
    f.position = WIN_POS_MOUSE;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_child(GTK_WINDOW(win),
                         lbl("✅ WIN_POS_MOUSE\nFenêtre centrée sur la souris\nvia Win32 GetCursorPos"));
    gtk_window_present(GTK_WINDOW(win));
    appliquer_position_apres_affichage(&f);
}

// ════════════════════════════════════════════════════════════
// TEST 16 : Position WIN_POS_CENTER_ON_PARENT  ✅ Win32
// ════════════════════════════════════════════════════════════
static void test_position_center_parent(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 16 — Position CENTER_ON_PARENT") + 1);
    strcpy(f.title, "Test 16 — Position CENTER_ON_PARENT");
    f.taille.width = 380;
    f.taille.height = 200;
    f.color_bg = malloc(strlen("#fdebd0") + 1);
    strcpy(f.color_bg, "#fdebd0");
    f.position = WIN_POS_CENTER_ON_PARENT;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_child(GTK_WINDOW(win),
                         lbl("✅ WIN_POS_CENTER_ON_PARENT\nCentrée sur l'écran principal\nvia Win32"));
    gtk_window_present(GTK_WINDOW(win));
    appliquer_position_apres_affichage(&f);
}

// ════════════════════════════════════════════════════════════
// TEST 17 : Position WIN_POS_DEFAULT
// ════════════════════════════════════════════════════════════
static void test_position_default(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 17 — Position DEFAULT") + 1);
    strcpy(f.title, "Test 17 — Position DEFAULT");
    f.taille.width = 380;
    f.taille.height = 180;
    f.color_bg = malloc(strlen("#e8daef") + 1);
    strcpy(f.color_bg, "#e8daef");
    f.position = WIN_POS_DEFAULT;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    gtk_window_set_child(GTK_WINDOW(win),
                         lbl("✅ WIN_POS_DEFAULT\nPosition laissée au WM Windows"));
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 18 : Taille + id
// ════════════════════════════════════════════════════════════
static void test_taille_id(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 18 — Taille & ID") + 1);
    strcpy(f.title, "Test 18 — Taille & ID");
    f.taille.width = 600;
    f.taille.height = 400;
    f.color_bg = malloc(strlen("#ebf5fb") + 1);
    strcpy(f.color_bg, "#ebf5fb");
    f.id = 42;
    GtkWidget *win = creer_fenetre_complete(&f, app);
    char txt[128];
    snprintf(txt, sizeof(txt),
             "✅ taille.width=%d\n✅ taille.height=%d\n✅ id=%d",
             f.taille.width, f.taille.height, f.id);
    gtk_window_set_child(GTK_WINDOW(win), lbl(txt));
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// TEST 19 : fenetre_set_scrollable() helpers
// ════════════════════════════════════════════════════════════
static void test_set_scrollable(GtkWidget *w, gpointer data)
{
    GtkApplication *app = (GtkApplication *)data;
    static Fenetre f;
    fenetre_initialiser(&f);
    f.title = malloc(strlen("Test 19 — set_scrollable()") + 1);
    strcpy(f.title, "Test 19 — set_scrollable()");
    f.taille.width = 380;
    f.taille.height = 260;
    f.color_bg = malloc(strlen("#fdfefe") + 1);
    strcpy(f.color_bg, "#fdfefe");
    fenetre_set_scrollable(&f, SCROLL_VERTICAL);
    fenetre_set_scroll_overlay(&f, FALSE);
    fenetre_set_scroll_content_size(&f, 0, 900);
    GtkWidget *win = creer_fenetre_complete(&f, app);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_start(box, 15);
    gtk_widget_set_margin_top(box, 10);
    GtkWidget *info = gtk_label_new(
        "SPECIAL: barres TOUJOURS visibles\nHauteur min=900px via helpers");
    GtkCssProvider *pi = gtk_css_provider_new();
    gtk_css_provider_load_from_string(pi,
                                      "label{color:#c0392b;font-weight:bold;font-size:13px;}");
    gtk_style_context_add_provider(gtk_widget_get_style_context(info),
                                   GTK_STYLE_PROVIDER(pi), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(pi);
    gtk_box_append(GTK_BOX(box), info);
    for (int i = 1; i <= 25; i++)
    {
        char txt[64];
        snprintf(txt, sizeof(txt), "Item %d — set_scrollable()", i);
        GtkWidget *l = gtk_label_new(txt);
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, "label{color:#111;}");
        gtk_style_context_add_provider(gtk_widget_get_style_context(l),
                                       GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
        gtk_box_append(GTK_BOX(box), l);
    }
    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(fenetre_get_content_container(&f)), box);
    gtk_window_present(GTK_WINDOW(win));
}

// ════════════════════════════════════════════════════════════
// FENÊTRE PRINCIPALE
// ════════════════════════════════════════════════════════════


static void activate(GtkApplication *app, gpointer user_data)
{
    init_chemins();

    static Fenetre principale;
    fenetre_initialiser(&principale);
    principale.title = malloc(strlen("Tests Fenetre — Tous les tests") + 1);
    strcpy(principale.title, "Tests Fenetre — Tous les tests");
    principale.taille.width = 500;
    principale.taille.height = 780;
    principale.color_bg = malloc(strlen("#f4f6f7") + 1);
    strcpy(principale.color_bg, "#f4f6f7");
    principale.scroll_mode = SCROLL_VERTICAL;

    GtkWidget *win = creer_fenetre_complete(&principale, app);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(box, 15);
    gtk_widget_set_margin_bottom(box, 15);
    gtk_widget_set_margin_start(box, 55);
    gtk_widget_set_margin_end(box, 55);

    GtkWidget *titre = gtk_label_new("── Choisissez un test ──");
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_string(p,
                                      "label{color:#1a1a1a;font-size:15px;font-weight:bold;}");
    gtk_style_context_add_provider(gtk_widget_get_style_context(titre),
                                   GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(p);
    gtk_widget_set_halign(titre, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(box), titre);

    struct
    {
        const char *id, *txt, *bg, *hov;
        GCallback cb;
    } tests[] = {
        {"t1", "Test 1  — Couleur fond + titre GAUCHE", "#2980b9", "#1a5276", G_CALLBACK(test_basique)},
        {"t2", "Test 2  — Titre CENTRE", "#27ae60", "#1e8449", G_CALLBACK(test_titre_centre)},
        {"t3", "Test 3  — Titre DROITE", "#d35400", "#a04000", G_CALLBACK(test_titre_droite)},
        {"t4", "Test 4  — Icône zcode.png partout", "#7d3c98", "#6c3483", G_CALLBACK(test_icone)},
        {"t5", "Test 5  — Icône + texte dans titre", "#c0392b", "#922b21", G_CALLBACK(test_titre_avec_icone)},
        {"t6", "Test 6  — Background image", "#1a5276", "#154360", G_CALLBACK(test_background_image)},
        {"t7", "Test 7  — Scroll vertical", "#117a65", "#0e6655", G_CALLBACK(test_scroll_vertical)},
        {"t8", "Test 8  — Scroll horizontal", "#b7950b", "#9a7d0a", G_CALLBACK(test_scroll_horizontal)},
        {"t9", "Test 9  — Scroll both + taille min", "#784212", "#6e2c00", G_CALLBACK(test_scroll_both)},
        {"t10", "Test 10 — Démarrer maximisée", "#2c3e50", "#1a252f", G_CALLBACK(test_maximisee)},
        {"t11", "Test 11 — Sans boutons de contrôle", "#922b21", "#7b241c", G_CALLBACK(test_sans_boutons)},
        {"t12", "Test 12 — Non redimensionnable", "#1f618d", "#1a5276", G_CALLBACK(test_non_redim)},
        {"t13", "Test 13 — Popup sans décoration", "#6c3483", "#5b2c6f", G_CALLBACK(test_popup)},
        {"t14", "Test 14 — Position CENTER ✅ Win32", "#0e6655", "#0b5345", G_CALLBACK(test_position_center)},
        {"t15", "Test 15 — Position MOUSE ✅ Win32", "#4a235a", "#3b1a4a", G_CALLBACK(test_position_mouse)},
        {"t16", "Test 16 — Position CENTER_ON_PARENT ✅ Win32", "#7e5109", "#6e4509", G_CALLBACK(test_position_center_parent)},
        {"t17", "Test 17 — Position DEFAULT", "#1b4332", "#145a32", G_CALLBACK(test_position_default)},
        {"t18", "Test 18 — Taille 600x400 + id=42", "#2c3e50", "#1a252f", G_CALLBACK(test_taille_id)},
        {"t19", "Test 19 — set_scrollable() helpers", "#922b21", "#7b241c", G_CALLBACK(test_set_scrollable)},
    };

    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; i++)
        gtk_box_append(GTK_BOX(box),
                       creer_btn(tests[i].id, tests[i].txt,
                                 tests[i].bg, tests[i].hov,
                                 tests[i].cb, app));

    gtk_scrolled_window_set_child(
        GTK_SCROLLED_WINDOW(fenetre_get_content_container(&principale)), box);
    gtk_window_present(GTK_WINDOW(win));
}

int main(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new(
        "com.test.fenetre", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
