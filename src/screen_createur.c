#include "screen_createur.h"
#include "bassin.h"

extern void nav_to_accueil(void);

static Bassin    *s_bassin    = NULL;
static EntityType s_tool_type = FISH_NORMAL;
static GtkWidget *s_label_count = NULL;

static void update_count_label(void) {
    if (!s_label_count || !s_bassin) return;
    int fish   = 0, sharks = 0, objects = 0;
    for (int i = 0; i < s_bassin->nb_entities; i++) {
        Entity *e = s_bassin->entities[i];
        if (!e) continue;
        if (entity_is_predator(e->type))      sharks++;
        else if (entity_is_object(e->type))   objects++;
        else                                  fish++;
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "Poissons : %d | Requins : %d | Objets : %d",
             fish, sharks, objects);
    gtk_label_set_text(GTK_LABEL(s_label_count), buf);
}

static void on_bassin_click(GtkGestureClick *gc, int n, double x, double y, gpointer data) {
    (void)gc; (void)n; (void)data;
    if (!s_bassin) return;
    Entity *e = entity_create(s_tool_type, (float)x, (float)y);
    e->state = STATE_SWIMMING;
    e->vx = 1.5f * (rand() % 2 == 0 ? 1 : -1);
    e->vy = 0.5f * (rand() % 2 == 0 ? 1 : -1);
    e->angle = atan2(e->vy, e->vx);
    bassin_add_entity(s_bassin, e);
    update_count_label();
}

static void set_tool(GtkButton *btn, gpointer data) {
    (void)btn;
    s_tool_type = (EntityType)(intptr_t)data;
}

static void on_reset(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    bassin_clear(s_bassin);
    update_count_label();
}

static void on_save(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    bassin_save(s_bassin, "data/bassin.txt");
}

static void on_load(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    bassin_load(s_bassin, "data/bassin.txt");
    update_count_label();
}

static void on_simulate(GtkButton *btn, gpointer data) {
    (void)btn; (void)data;
    /* Donner une vélocité initiale à toutes les entités non-objets */
    for (int i = 0; i < s_bassin->nb_entities; i++) {
        Entity *e = s_bassin->entities[i];
        if (e && !entity_is_object(e->type) && e->vx == 0.0f) {
            e->state = STATE_SWIMMING;
            e->vx = 1.0f + (rand() % 20) * 0.1f;
            e->vy = -0.5f + (rand() % 10) * 0.1f;
            e->angle = atan2(e->vy, e->vx);
        }
    }
}

static GtkWidget *make_tool_btn(const char *label, EntityType type) {
    GtkWidget *btn = gtk_button_new_with_label(label);
    gtk_widget_set_size_request(btn, 140, 40);
    g_signal_connect(btn, "clicked", G_CALLBACK(set_tool), (gpointer)(intptr_t)type);
    return btn;
}

GtkWidget *screen_createur_create(void) {
    s_bassin = bassin_create(BASSIN_MODE_CREATEUR);
    bassin_start(s_bassin);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    /* ── Barre gauche (outils) ─────────────────────── */
    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_size_request(left, 160, -1);
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "box { background: rgba(0,30,60,0.85); padding: 10px; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(left),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    GtkWidget *lbl = gtk_label_new("🛠 Outils");
    gtk_box_append(GTK_BOX(left), lbl);

    gtk_box_append(GTK_BOX(left), make_tool_btn("🐟 Poisson",   FISH_NORMAL));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🦈 Requin",    SHARK_WHITE));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🐢 Tortue",    TURTLE_SEA));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🎐 Méduse",    JELLYFISH));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🪨 Rocher",    OBJ_ROCK));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🌿 Algue",     OBJ_ALGAE));
    gtk_box_append(GTK_BOX(left), make_tool_btn("🪸 Corail",    OBJ_CORAL));

    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(left), sep);

    GtkWidget *btn_reset = gtk_button_new_with_label("🔄 Reset");
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(on_reset), NULL);
    gtk_box_append(GTK_BOX(left), btn_reset);

    GtkWidget *btn_back = gtk_button_new_with_label("← Menu");
    g_signal_connect(btn_back, "clicked", G_CALLBACK(nav_to_accueil), NULL);
    gtk_box_append(GTK_BOX(left), btn_back);

    /* ── Zone centrale (bassin) ─────────────────────── */
    GtkWidget *center = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(center, TRUE);
    gtk_widget_set_vexpand(center, TRUE);

    GtkWidget *canvas = bassin_get_widget(s_bassin);
    gtk_widget_set_hexpand(canvas, TRUE);
    gtk_widget_set_vexpand(canvas, TRUE);

    /* Gestionnaire de clic pour placer les entités */
    GtkGesture *click = gtk_gesture_click_new();
    g_signal_connect(click, "pressed", G_CALLBACK(on_bassin_click), NULL);
    gtk_widget_add_controller(canvas, GTK_EVENT_CONTROLLER(click));

    gtk_box_append(GTK_BOX(center), canvas);

    /* Barre du bas */
    GtkWidget *bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p,
            "box { background: rgba(0,20,40,0.9); padding: 8px; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(bottom),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    GtkWidget *btn_save     = gtk_button_new_with_label("💾 Sauvegarder");
    GtkWidget *btn_load     = gtk_button_new_with_label("📂 Charger");
    GtkWidget *btn_simulate = gtk_button_new_with_label("▶ Simuler");

    g_signal_connect(btn_save,     "clicked", G_CALLBACK(on_save),     NULL);
    g_signal_connect(btn_load,     "clicked", G_CALLBACK(on_load),     NULL);
    g_signal_connect(btn_simulate, "clicked", G_CALLBACK(on_simulate), NULL);

    s_label_count = gtk_label_new("Poissons : 0 | Requins : 0 | Objets : 0");
    {
        GtkCssProvider *p = gtk_css_provider_new();
        gtk_css_provider_load_from_string(p, "label { color: white; }");
        gtk_style_context_add_provider(gtk_widget_get_style_context(s_label_count),
            GTK_STYLE_PROVIDER(p), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(p);
    }

    gtk_box_append(GTK_BOX(bottom), btn_save);
    gtk_box_append(GTK_BOX(bottom), btn_load);
    gtk_box_append(GTK_BOX(bottom), btn_simulate);
    gtk_box_append(GTK_BOX(bottom), s_label_count);

    gtk_box_append(GTK_BOX(center), bottom);

    /* ── Assemblage ─────────────────────────────────── */
    gtk_box_append(GTK_BOX(hbox), left);
    gtk_box_append(GTK_BOX(hbox), center);

    return hbox;
}