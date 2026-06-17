#include <gtk/gtk.h>
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
// MAXIMIZE WIN32 via signal "realize"
// ════════════════════════════════════════════════════════════
#ifdef _WIN32
static void _win32_maximize_on_realize(GtkWidget *widget, gpointer data)
{
    (void)data;
    GdkSurface *surface = gtk_native_get_surface(GTK_NATIVE(widget));
    if (!surface) return;
    HWND hwnd = GDK_SURFACE_HWND(surface);
    if (hwnd) ShowWindow(hwnd, SW_SHOWMAXIMIZED);
}
#endif

// ════════════════════════════════════════════════════════════
// ICÔNE TASKBAR WINDOWS via .ico
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
    char css_data[4096];

    if (config->background_image != NULL) {
        char *uri = NULL;
        if (g_str_has_prefix(config->background_image, "file://") ||
            g_str_has_prefix(config->background_image, "resource://")) {
            uri = g_strdup(config->background_image);
        } else {
            char *absolute_path = g_canonicalize_filename(config->background_image, NULL);
            uri = g_filename_to_uri(absolute_path, NULL, NULL);
            g_free(absolute_path);
        }

        if (uri != NULL) {
            snprintf(css_data, sizeof(css_data),
                "window {\n"
                "  background-image: url('%s');\n"
                "  background-size: cover;\n"
                "  background-position: center;\n"
                "  background-repeat: no-repeat;\n"
                "}\n"
                "scrolledwindow, viewport {\n"
                "  background: transparent;\n"
                "  background-color: transparent;\n"
                "}\n",
                uri);
            g_free(uri);
        } else {
            snprintf(css_data, sizeof(css_data),
                "window { background-color: #000000; }\n");
        }
        fprintf(stdout, "[CSS] background applied successfully: %s\n", config->background_image);
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
Widget fenetre_creer(Fenetre *config, void *app)
{
    if (!config) return NULL;

    // ✅ WIN_TYPE_TOPLEVEL = fenêtre normale
    // ✅ WIN_TYPE_POPUP    = sans décoration
    config->wind = gtk_application_window_new(GTK_APPLICATION(app));

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

    if (config->demarrer_maximisee) {
        gtk_window_maximize(GTK_WINDOW(config->wind));
#ifdef _WIN32
        g_signal_connect(config->wind, "realize",
                         G_CALLBACK(_win32_maximize_on_realize), NULL);
#endif
    }

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

        if (config->content_min_width > 0) {
            gtk_scrolled_window_set_min_content_width(
                GTK_SCROLLED_WINDOW(config->scroll_widget),
                config->content_min_width);
        }
        if (config->content_min_height > 0) {
            gtk_scrolled_window_set_min_content_height(
                GTK_SCROLLED_WINDOW(config->scroll_widget),
                config->content_min_height);
        }

        gtk_window_set_child(GTK_WINDOW(config->wind), config->scroll_widget);
    }

    g_object_set_data(G_OBJECT(config->wind), "custom_struct", config);
    g_object_set_data(G_OBJECT(config->wind), "custom_type", "Fenetre");

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

void fenetre_set_scroll_overlay(Fenetre *config, bool overlay) {
    if (!config) return;
    config->scroll_overlay = overlay;
}

Widget fenetre_get_content_container(Fenetre *config) {
    if (!config) return NULL;
    if (config->scroll_mode != SCROLL_NONE && config->scroll_widget)
        return config->scroll_widget;
    return config->wind;
}

void fenetre_ajouter(Fenetre *config, Widget enfant) {
    if (!config || !config->wind || !enfant) return;
    if (config->scroll_mode != SCROLL_NONE && config->scroll_widget) {
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(config->scroll_widget), GTK_WIDGET(enfant));
    } else {
        gtk_window_set_child(GTK_WINDOW(config->wind), GTK_WIDGET(enfant));
    }
}

/* Background management & action helpers */
void fenetre_set_background_image(Widget window, const char *image_path) {
    if (!window || !image_path) return;
    GtkCssProvider *provider = gtk_css_provider_new();
    char css[2048];
    
    char absolute_path[512];
#ifdef _WIN32
    _fullpath(absolute_path, image_path, sizeof(absolute_path));
#else
    if (realpath(image_path, absolute_path) == NULL) {
        strncpy(absolute_path, image_path, sizeof(absolute_path));
    }
#endif
    
    /* Normalisation des slashes du chemin pour le moteur CSS */
    for (int i = 0; absolute_path[i] != '\0'; i++) {
        if (absolute_path[i] == '\\') {
            absolute_path[i] = '/';
        }
    }
    
    snprintf(css, sizeof(css),
        "window {\n"
        "  background-image: url('file:///%s');\n"
        "  background-size: cover;\n"
        "  background-repeat: no-repeat;\n"
        "  background-position: center;\n"
        "}\n"
        "box {\n"
        "  background-color: transparent;\n"
        "}\n"
        "label {\n"
        "  text-shadow: 1px 1px 3px rgba(0,0,0,0.8);\n"
        "}\n",
        absolute_path
    );
    
    gtk_css_provider_load_from_string(provider, css);
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(GTK_WIDGET(window)),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );
    g_object_unref(provider);
}

void fenetre_reset_background(Widget window) {
    if (!window) return;
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider,
        "window {\n"
        "  background-image: none;\n"
        "}\n"
    );
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(GTK_WIDGET(window)),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );
    g_object_unref(provider);
}

void action_quitter(Widget widget, void *data) {
    (void)widget;
    GtkWidget *win = NULL;
    if (data && GTK_IS_WIDGET(data)) {
        win = GTK_WIDGET(data);
    } else if (widget) {
        win = gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW);
    }
    
    if (win && GTK_IS_WINDOW(win)) {
        gtk_window_close(GTK_WINDOW(win));
    }
}


