#include "../headers/fenetre.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <gdk/win32/gdkwin32.h>
#endif

// ════════════════════════════════════════════════════════════
// POSITION VIA WIN32
// ════════════════════════════════════════════════════════════
void fenetre_appliquer_position(Fenetre *config)
{
#ifdef _WIN32
    if (!config || !config->wind) return;

    GdkSurface *surface = gtk_native_get_surface(GTK_NATIVE(config->wind));
    if (!surface) return;
    HWND hwnd = GDK_SURFACE_HWND(surface);
    if (!hwnd) return;

    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);

    RECT rect;
    GetWindowRect(hwnd, &rect);
    int win_w = rect.right  - rect.left;
    int win_h = rect.bottom - rect.top;
    int x = rect.left;
    int y = rect.top;

    switch (config->position)
    {
        case WIN_POS_CENTER:
            x = (screen_w - win_w) / 2;
            y = (screen_h - win_h) / 2;
            fprintf(stdout, "[POS] CENTER: ecran=%dx%d fenetre=%dx%d -> (%d,%d)\n",
                screen_w, screen_h, win_w, win_h, x, y);
            break;

        case WIN_POS_MOUSE:
        {
            POINT pt;
            GetCursorPos(&pt);
            x = pt.x - win_w / 2;
            y = pt.y - win_h / 2;
            if (x < 0) x = 0;
            if (y < 0) y = 0;
            if (x + win_w > screen_w) x = screen_w - win_w;
            if (y + win_h > screen_h) y = screen_h - win_h;
            fprintf(stdout, "[POS] MOUSE: souris=(%ld,%ld) -> (%d,%d)\n",
                pt.x, pt.y, x, y);
            break;
        }

        case WIN_POS_CENTER_ON_PARENT:
            x = (screen_w - win_w) / 2;
            y = (screen_h - win_h) / 2;
            fprintf(stdout, "[POS] CENTER_ON_PARENT -> (%d,%d)\n", x, y);
            break;

        case WIN_POS_DEFAULT:
        default:
            fprintf(stdout, "[POS] DEFAULT: laisse au WM\n");
            fflush(stdout);
            return;
    }

    SetWindowPos(hwnd, NULL, x, y, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    fflush(stdout);
#endif
}

// ════════════════════════════════════════════════════════════
// ICÔNE TASKBAR WINDOWS via .ico  ✅ CORRECTION
// ════════════════════════════════════════════════════════════
void fenetre_appliquer_icone_taskbar(Fenetre *config)
{
#ifdef _WIN32
    if (!config || !config->wind) return;
    if (!config->ico_path) return;

    // Attendre que la surface soit disponible
    GdkSurface *surface = gtk_native_get_surface(GTK_NATIVE(config->wind));
    if (!surface) return;
    HWND hwnd = GDK_SURFACE_HWND(surface);
    if (!hwnd) return;

    // ✅ Charger le .ico via Win32 (PNG ne fonctionne pas pour la taskbar)
    HICON hicon_big = (HICON)LoadImageA(
        NULL, config->ico_path,
        IMAGE_ICON, 32, 32,
        LR_LOADFROMFILE | LR_DEFAULTSIZE);

    HICON hicon_small = (HICON)LoadImageA(
        NULL, config->ico_path,
        IMAGE_ICON, 16, 16,
        LR_LOADFROMFILE);

    if (hicon_big) {
        SendMessageA(hwnd, WM_SETICON, ICON_BIG,   (LPARAM)hicon_big);
        fprintf(stdout, "[ICON] Icone taskbar (32x32) appliquee: %s\n",
            config->ico_path);
    } else {
        fprintf(stdout, "[WARN] Impossible charger ico 32x32: %s (erreur=%lu)\n",
            config->ico_path, GetLastError());
    }

    if (hicon_small) {
        SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon_small);
        fprintf(stdout, "[ICON] Icone taskbar (16x16) appliquee\n");
    }

    fflush(stdout);
#endif
}

// ════════════════════════════════════════════════════════════
// CSS BACKGROUND
// ════════════════════════════════════════════════════════════
static void _fenetre_appliquer_css(GtkWidget *window, Fenetre *config)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    char css_data[2048];

    if (config->background_image != NULL) {
        // ✅ GTK4 CSS url() nécessite file:/// sur Windows
        // Si le chemin commence déjà par file:/// on le garde tel quel
        if (strncmp(config->background_image, "file:///", 8) == 0) {
            snprintf(css_data, sizeof(css_data),
                "window { "
                "  background-image: url('%s'); "
                "  background-size: cover; "
                "  background-position: center; "
                "  background-repeat: no-repeat; "
                "}",
                config->background_image);
        } else {
            // Construire le chemin file:///
            snprintf(css_data, sizeof(css_data),
                "window { "
                "  background-image: url('file:///%s'); "
                "  background-size: cover; "
                "  background-position: center; "
                "  background-repeat: no-repeat; "
                "}",
                config->background_image);
        }
        fprintf(stdout, "[CSS] background: %s\n", css_data);
        fflush(stdout);
    } else if (config->color_bg != NULL) {
        snprintf(css_data, sizeof(css_data),
            "window { background-color: %s; }",
            config->color_bg);
    } else {
        g_object_unref(provider);
        return;
    }

    gtk_css_provider_load_from_string(provider, css_data);
    GtkStyleContext *context = gtk_widget_get_style_context(window);
    gtk_style_context_add_provider(context,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

// ════════════════════════════════════════════════════════════
// INITIALISER
// ════════════════════════════════════════════════════════════
void fenetre_initialiser(Fenetre *config)
{
    if (!config) return;
    config->wind             = NULL;
    config->scroll_widget    = NULL;
    config->title = malloc(strlen("Application") + 1);
    strcpy(config->title, "Application");
    config->titre_align      = TITRE_ALIGN_CENTRE;
    config->icon_path        = NULL;
    config->ico_path         = NULL;   // ✅ NOUVEAU
    config->type             = WIN_TYPE_TOPLEVEL;
    config->resizable        = true;
    config->demarrer_maximisee = false;
    config->bouton_fermer    = true;
    config->bouton_agrandir  = true;
    config->bouton_reduire   = true;
    config->scroll_mode      = SCROLL_NONE;
    config->scroll_overlay   = true;
    config->content_min_width  = 0;
    config->content_min_height = 0;
    config->taille.width     = 800;
    config->taille.height    = 600;
    config->color_bg         = NULL;
    config->background_image = NULL;
    config->position         = WIN_POS_DEFAULT;
    config->id               = 0;
}

// ════════════════════════════════════════════════════════════
// CRÉER
// ════════════════════════════════════════════════════════════
GtkWidget *fenetre_creer(Fenetre *config, GtkApplication *app)
{
    if (!config) return NULL;

    // ✅ WIN_TYPE_TOPLEVEL = fenêtre normale
    // ✅ WIN_TYPE_POPUP    = sans décoration
    config->wind = gtk_application_window_new(app);

    if (config->type == WIN_TYPE_POPUP) {
        gtk_window_set_decorated(GTK_WINDOW(config->wind), FALSE);
        fprintf(stdout, "[DEBUG] WIN_TYPE_POPUP: sans decoration\n");
    } else {
        gtk_window_set_decorated(GTK_WINDOW(config->wind), TRUE);
        fprintf(stdout, "[DEBUG] WIN_TYPE_TOPLEVEL: fenetre normale\n");
    }

    gtk_window_set_default_size(GTK_WINDOW(config->wind),
        config->taille.width, config->taille.height);
    gtk_window_set_resizable(GTK_WINDOW(config->wind), config->resizable);

    if (config->demarrer_maximisee)
        gtk_window_maximize(GTK_WINDOW(config->wind));

    // --- HEADERBAR ---
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_title_buttons(GTK_HEADER_BAR(header_bar), TRUE);

    char layout_desc[100];
    strcpy(layout_desc, ":");
    if (config->bouton_reduire)
        strcat(layout_desc, "minimize,");
    if (config->bouton_agrandir && config->resizable)
        strcat(layout_desc, "maximize,");
    if (config->bouton_fermer)
        strcat(layout_desc, "close");

    size_t len = strlen(layout_desc);
    if (len > 1 && layout_desc[len - 1] == ',')
        layout_desc[len - 1] = '\0';

    fprintf(stdout, "[DEBUG] Boutons: '%s'\n", layout_desc);
    gtk_header_bar_set_decoration_layout(GTK_HEADER_BAR(header_bar), layout_desc);

    // --- TITRE ---
    GtkWidget *custom_title = gtk_label_new(config->title);
    float xalign = 0.5f;
    if (config->titre_align == TITRE_ALIGN_GAUCHE) xalign = 0.0f;
    if (config->titre_align == TITRE_ALIGN_DROITE) xalign = 1.0f;
    gtk_label_set_xalign(GTK_LABEL(custom_title), xalign);
    gtk_widget_set_hexpand(custom_title, TRUE);
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header_bar), custom_title);

    // ✅ Icône PNG dans la HeaderBar (si icon_path fourni)
    if (config->icon_path && g_file_test(config->icon_path, G_FILE_TEST_EXISTS)) {
        GtkWidget *img = gtk_image_new_from_file(config->icon_path);
        gtk_image_set_pixel_size(GTK_IMAGE(img), 22);
        gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), img);
        fprintf(stdout, "[ICON] HeaderBar icon: %s\n", config->icon_path);
    } else if (config->icon_path) {
        // Nom d'icône système
        gtk_window_set_icon_name(GTK_WINDOW(config->wind), config->icon_path);
    }

    gtk_window_set_titlebar(GTK_WINDOW(config->wind), header_bar);

    // --- CSS ---
    _fenetre_appliquer_css(config->wind, config);

    // --- SCROLL ---
    if (config->scroll_mode != SCROLL_NONE) {
        config->scroll_widget = gtk_scrolled_window_new();

        GtkPolicyType h_policy = GTK_POLICY_NEVER;
        GtkPolicyType v_policy = GTK_POLICY_NEVER;

        switch (config->scroll_mode) {
            case SCROLL_HORIZONTAL: h_policy = GTK_POLICY_AUTOMATIC; break;
            case SCROLL_VERTICAL:   v_policy = GTK_POLICY_AUTOMATIC; break;
            case SCROLL_BOTH:
                h_policy = GTK_POLICY_AUTOMATIC;
                v_policy = GTK_POLICY_AUTOMATIC;
                break;
            default: break;
        }

        gtk_scrolled_window_set_policy(
            GTK_SCROLLED_WINDOW(config->scroll_widget), h_policy, v_policy);
        gtk_scrolled_window_set_overlay_scrolling(
            GTK_SCROLLED_WINDOW(config->scroll_widget), config->scroll_overlay);

        if (config->content_min_width > 0 || config->content_min_height > 0) {
            gtk_widget_set_size_request(config->scroll_widget,
                config->content_min_width  > 0 ? config->content_min_width  : -1,
                config->content_min_height > 0 ? config->content_min_height : -1);
        }

        gtk_window_set_child(GTK_WINDOW(config->wind), config->scroll_widget);
    }

    return config->wind;
}

// ════════════════════════════════════════════════════════════
// HELPERS SCROLL
// ════════════════════════════════════════════════════════════
void fenetre_set_scrollable(Fenetre *config, WidgetScrollMode mode) {
    if (!config) return;
    config->scroll_mode = mode;
}

void fenetre_set_scroll_content_size(Fenetre *config, int min_width, int min_height) {
    if (!config) return;
    config->content_min_width  = min_width;
    config->content_min_height = min_height;
}

void fenetre_set_scroll_overlay(Fenetre *config, gboolean overlay) {
    if (!config) return;
    config->scroll_overlay = overlay;
}

GtkWidget *fenetre_get_content_container(Fenetre *config) {
    if (!config) return NULL;
    if (config->scroll_mode != SCROLL_NONE && config->scroll_widget)
        return config->scroll_widget;
    return config->wind;
}
