#include <gtk/gtk.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "pages/screen_bassin.h"
#include "pages/screen_home.h"
#include "fenetre.h"
#include "core/app_runner.h"

#ifdef _WIN32
static void _set_cwd_to_project_root(void)
{
    CHAR path[MAX_PATH];
    if (!GetModuleFileNameA(NULL, path, MAX_PATH)) return;

    char *sep = strrchr(path, '\\');
    while (sep)
    {
        *sep = '\0';
        CHAR check[MAX_PATH];
        snprintf(check, MAX_PATH, "%s\\resources", path);
        DWORD attrs = GetFileAttributesA(check);
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            SetCurrentDirectoryA(path);
            return;
        }
        sep = strrchr(path, '\\');
    }
}

static void _setup_gstreamer_paths(void)
{
    /* Release build: plugins bundled next to exe */
    if (GetFileAttributesA("lib\\gstreamer-1.0") != INVALID_FILE_ATTRIBUTES)
    {
        CHAR abs[MAX_PATH];
        if (GetFullPathNameA("lib\\gstreamer-1.0", MAX_PATH, abs, NULL))
            g_setenv("GST_PLUGIN_PATH", abs, FALSE);
        return;
    }
    /* Development: standard MSYS2 location */
    const char *msys2 = "C:\\msys64\\mingw64\\lib\\gstreamer-1.0";
    if (GetFileAttributesA(msys2) != INVALID_FILE_ATTRIBUTES)
        g_setenv("GST_PLUGIN_PATH", msys2, FALSE);
}
#endif

Fenetre g_app_window;

static void show_screen(Widget child)
{
    if (!g_app_window.wind)
    {
        return;
    }

    fenetre_ajouter(&g_app_window, child);
}

void nav_to_accueil(void)
{
    show_screen(screen_accueil_create());
}

void nav_to_bassin(void)
{
    show_screen(screen_bassin_create());
}

static void activate(GtkApplication *app, gpointer user_data)
{
    (void)user_data;

    fenetre_initialiser(&g_app_window);

    if (g_app_window.title)
        free(g_app_window.title);
    g_app_window.title = strdup("DEEP SHARK ATTACK");

    g_app_window.demarrer_maximisee = true;

    g_app_window.icon_path = "resources/icons/app.png";
    g_app_window.ico_path = "resources/icons/app.ico";

    fenetre_creer(&g_app_window, app);
#ifdef _WIN32
    g_timeout_add(100, (GSourceFunc)fenetre_appliquer_icone_taskbar, &g_app_window);
#endif
    nav_to_accueil();

    gtk_window_present(GTK_WINDOW(g_app_window.wind));
}

int main(int argc, char *argv[])
{
#ifdef _WIN32
    _set_cwd_to_project_root();
    _setup_gstreamer_paths();
#endif
    return runner("fr.banc_poisson.app", G_CALLBACK(activate), NULL, argc, argv);
}