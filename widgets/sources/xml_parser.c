/**
 * xml_parser.c  — v2
 *
 * Parser XML léger pour construire des interfaces GTK à partir d'un fichier XML.
 *
 * Widgets supportés :
 *   <fenetre>          Fenêtre principale
 *   <conteneur>        GtkBox (vertical ou horizontal)
 *   <separateur>       GtkSeparator
 *   <texte>            GtkLabel
 *   <bouton>           GtkButton
 *   <bouton_checklist> GtkCheckButton
 *   <champ_motdepasse> GtkPasswordEntry
 *   <menu>             Barre de navigation + sous-menus
 *   <menu_item>        Item de menu (enfant de <menu> ou d'un autre <menu_item>)
 *   <dialog>           Fenêtre modale (affichée immédiatement)
 *
 * Usage :
 *   GtkWidget *w = xml_load_file("ui.xml", app);
 */

#include "../headers/xml_parser.h"

/* ---- headers de tes widgets ---- */
#include "../headers/fenetre.h"
#include "../headers/conteneur.h"
#include "../headers/texte.h"
#include "../headers/bouton.h"
#include "../headers/bouton_checklist.h"
#include "../headers/bouton_radio.h"
#include "../headers/champ_motdepasse.h"
#include "../headers/champ_texte.h"
#include "../headers/champ_nombre.h"
#include "../headers/champ_select.h"
#include "../headers/champ_zone_texte.h"
#include "../headers/slider.h"
#include "../headers/image.h"
#include "../headers/menu.h"
#include "../headers/dialog.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ================================================================
 *  UTILITAIRES INTERNES
 * ================================================================ */

/** Duplique une chaîne avec malloc (NULL-safe). */
static char *xstrdup(const char *s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s);
    char *d = malloc(len + 1);
    if (d)
        strcpy(d, s);
    return d;
}

/**
 * Remplace *field par une copie de newval.
 * Libère l'ancienne valeur si elle existe (compatible avec g_free ET free).
 * Ne fait rien si newval est NULL.
 */
static void set_str(char **field, const char *newval)
{
    if (!newval)
        return;
    free(*field);
    *field = xstrdup(newval);
}

/* ================================================================
 *  GESTION DES ATTRIBUTS
 * ================================================================ */

static XmlAttr *attr_new(const char *name, const char *value)
{
    XmlAttr *a = malloc(sizeof(XmlAttr));
    a->name = xstrdup(name);
    a->value = xstrdup(value);
    a->next = NULL;
    return a;
}

static void attr_free_list(XmlAttr *a)
{
    while (a)
    {
        XmlAttr *next = a->next;
        free(a->name);
        free(a->value);
        free(a);
        a = next;
    }
}

const char *xml_attr_get(const XmlNode *node, const char *name)
{
    if (!node || !name)
        return NULL;
    for (XmlAttr *a = node->attrs; a; a = a->next)
        if (strcmp(a->name, name) == 0)
            return a->value;
    return NULL;
}

/** Retourne la valeur booléenne d'un attribut, ou `def` si absent. */
static gboolean attr_bool(const XmlNode *n, const char *name, gboolean def)
{
    const char *v = xml_attr_get(n, name);
    if (!v)
        return def;
    return (strcmp(v, "true") == 0 || strcmp(v, "1") == 0 || strcmp(v, "oui") == 0)
               ? TRUE
               : FALSE;
}

/** Retourne la valeur entière d'un attribut, ou `def` si absent. */
static int attr_int(const XmlNode *n, const char *name, int def)
{
    const char *v = xml_attr_get(n, name);
    return v ? atoi(v) : def;
}

/* ================================================================
 *  GESTION DES NŒUDS
 * ================================================================ */

static XmlNodeType tag_to_type(const char *tag)
{
    if (strcmp(tag, "fenetre") == 0)
        return NODE_FENETRE;
    if (strcmp(tag, "conteneur") == 0)
        return NODE_CONTENEUR;
    if (strcmp(tag, "separateur") == 0)
        return NODE_SEPARATEUR;
    if (strcmp(tag, "texte") == 0)
        return NODE_TEXTE;
    if (strcmp(tag, "bouton") == 0)
        return NODE_BOUTON;
    if (strcmp(tag, "bouton_checklist") == 0)
        return NODE_BOUTON_CHECKLIST;
    if (strcmp(tag, "bouton_radio") == 0)
        return NODE_BOUTON_RADIO;
    if (strcmp(tag, "champ_motdepasse") == 0)
        return NODE_CHAMP_MOTDEPASSE;
    if (strcmp(tag, "champ_texte") == 0)
        return NODE_CHAMP_TEXTE;
    if (strcmp(tag, "champ_nombre") == 0)
        return NODE_CHAMP_NOMBRE;
    if (strcmp(tag, "champ_select") == 0)
        return NODE_CHAMP_SELECT;
    if (strcmp(tag, "champ_zone_texte") == 0)
        return NODE_CHAMP_ZONE_TEXTE;
    if (strcmp(tag, "slider") == 0)
        return NODE_SLIDER;
    if (strcmp(tag, "image") == 0)
        return NODE_IMAGE;
    if (strcmp(tag, "menu") == 0)
        return NODE_MENU;
    if (strcmp(tag, "menu_item") == 0)
        return NODE_MENU_ITEM;
    if (strcmp(tag, "dialog") == 0)
        return NODE_DIALOG;
    return NODE_UNKNOWN;
}

static XmlNode *node_new(const char *tag)
{
    XmlNode *n = calloc(1, sizeof(XmlNode));
    n->tag = xstrdup(tag);
    n->type = tag_to_type(tag);
    return n;
}

void xml_node_free(XmlNode *node)
{
    if (!node)
        return;
    xml_node_free(node->children);
    xml_node_free(node->next);
    attr_free_list(node->attrs);
    free(node->tag);
    free(node);
}

/* ================================================================
 *  LEXER / PARSER XML
 * ================================================================ */

typedef struct
{
    const char *src;
    size_t pos;
    size_t len;
} Lexer;

static void lex_init(Lexer *l, const char *src)
{
    l->src = src;
    l->pos = 0;
    l->len = strlen(src);
}
static char lex_peek(Lexer *l) { return l->pos < l->len ? l->src[l->pos] : '\0'; }
static char lex_next(Lexer *l) { return l->pos < l->len ? l->src[l->pos++] : '\0'; }
static void lex_skip_spaces(Lexer *l)
{
    while (l->pos < l->len && isspace((unsigned char)l->src[l->pos]))
        l->pos++;
}

static char *lex_read_ident(Lexer *l)
{
    size_t start = l->pos;
    while (l->pos < l->len)
    {
        char c = l->src[l->pos];
        if (isalnum((unsigned char)c) || c == '_' || c == '-' || c == ':')
            l->pos++;
        else
            break;
    }
    size_t len = l->pos - start;
    if (!len)
        return NULL;
    char *s = malloc(len + 1);
    memcpy(s, l->src + start, len);
    s[len] = '\0';
    return s;
}

static char *lex_read_value(Lexer *l)
{
    char q = lex_peek(l);
    if (q != '"' && q != '\'')
        return NULL;
    lex_next(l);
    size_t start = l->pos;
    while (l->pos < l->len && l->src[l->pos] != q)
        l->pos++;
    size_t len = l->pos - start;
    if (lex_peek(l) == q)
        lex_next(l);
    char *s = malloc(len + 1);
    memcpy(s, l->src + start, len);
    s[len] = '\0';
    return s;
}

static void lex_skip_comment(Lexer *l)
{
    while (l->pos + 2 < l->len)
    {
        if (l->src[l->pos] == '-' && l->src[l->pos + 1] == '-' && l->src[l->pos + 2] == '>')
        {
            l->pos += 3;
            return;
        }
        l->pos++;
    }
}

static XmlAttr *parse_attrs(Lexer *l, int *self_closing)
{
    XmlAttr *head = NULL, *tail = NULL;
    *self_closing = 0;
    while (1)
    {
        lex_skip_spaces(l);
        char c = lex_peek(l);
        if (!c)
            break;
        if (c == '/')
        {
            lex_next(l);
            if (lex_peek(l) == '>')
                lex_next(l);
            *self_closing = 1;
            break;
        }
        if (c == '>')
        {
            lex_next(l);
            break;
        }
        char *name = lex_read_ident(l);
        if (!name)
        {
            lex_next(l);
            continue;
        }
        lex_skip_spaces(l);
        char *value;
        if (lex_peek(l) != '=')
        {
            value = xstrdup("true");
        }
        else
        {
            lex_next(l); /* '=' */
            lex_skip_spaces(l);
            value = lex_read_value(l);
            if (!value)
                value = xstrdup("");
        }
        XmlAttr *a = attr_new(name, value);
        free(name);
        free(value);
        if (!head)
            head = tail = a;
        else
        {
            tail->next = a;
            tail = a;
        }
    }
    return head;
}

/* Prototype anticipé */
static XmlNode *parse_node(Lexer *l);

// static XmlNode *parse_node(Lexer *l)
// {
//     lex_skip_spaces(l);
//     while (l->pos < l->len && lex_peek(l) != '<')
//         lex_next(l);
//     if (l->pos >= l->len)
//         return NULL;
//     lex_next(l); /* '<' */
//
//     /* Commentaire <!-- */
//     if (l->pos + 2 < l->len && l->src[l->pos] == '!' && l->src[l->pos + 1] == '-' && l->src[l->pos + 2] == '-')
//     {
//         l->pos += 3;
//         lex_skip_comment(l);
//         return parse_node(l);
//     }
//
//     /* Prologue <?xml ?> */
//     if (lex_peek(l) == '?')
//     {
//         while (l->pos < l->len && !(l->src[l->pos] == '?' && l->src[l->pos + 1] == '>'))
//             l->pos++;
//         l->pos += 2;
//         return parse_node(l);
//     }
//
//     /* Balise fermante </tag> */
//     if (lex_peek(l) == '/')
//     {
//         while (l->pos < l->len && l->src[l->pos] != '>')
//             l->pos++;
//         if (lex_peek(l) == '>')
//             lex_next(l);
//         return NULL;
//     }
//
//     char *tag = lex_read_ident(l);
//     if (!tag)
//         return NULL;
//     XmlNode *node = node_new(tag);
//     free(tag);
//
//     int self_closing = 0;
//     node->attrs = parse_attrs(l, &self_closing);
//
//     if (!self_closing)
//     {
//         XmlNode *ch_head = NULL, *ch_tail = NULL;
//         while (1)
//         {
//             lex_skip_spaces(l);
//             if (l->pos + 1 < l->len && l->src[l->pos] == '<' && l->src[l->pos + 1] == '/')
//             {
//                 while (l->pos < l->len && l->src[l->pos] != '>')
//                     l->pos++;
//                 if (lex_peek(l) == '>')
//                     lex_next(l);
//                 break;
//             }
//             if (l->pos >= l->len)
//                 break;
//             XmlNode *child = parse_node(l);
//             if (!child)
//                 break;
//             if (!ch_head)
//                 ch_head = ch_tail = child;
//             else
//             {
//                 ch_tail->next = child;
//                 ch_tail = child;
//             }
//         }
//         node->children = ch_head;
//     }
//     return node;
// }

static XmlNode *parse_node(Lexer *l)
{
    lex_skip_spaces(l);

    /* Avance jusqu'au prochain '<', en ignorant silencieusement le texte brut.
       Attention : les nœuds texte entre balises sont perdus (non capturés). */
    while (l->pos < l->len && lex_peek(l) != '<')
        lex_next(l);

    /* Fin de flux : rien à parser. */
    if (l->pos >= l->len)
        return NULL;

    lex_next(l); /* consomme '<' */

    /* ── Commentaire <!-- ... --> ────────────────────────────────────────── */
    if (l->pos + 2 < l->len && l->src[l->pos] == '!' && l->src[l->pos + 1] == '-' &&l->src[l->pos + 2] == '-')
    {
        l->pos += 3; /* saute '!--' */
        lex_skip_comment(l);
        return parse_node(l); /* continue vers le nœud suivant */
    }

    /* ── Prologue <?xml ... ?> ───────────────────────────────────────────── */
    if (lex_peek(l) == '?')
    {
        /* ERREUR : l->src[l->pos + 1] est accédé sans vérifier que
           l->pos + 1 < l->len → débordement possible en fin de flux.
           CORRECTION : ajouter la garde sur la borne. */
        /* while (l->pos < l->len && !(l->src[l->pos] == '?' && l->src[l->pos + 1] == '>')) */
        while (l->pos + 1 < l->len &&
               !(l->src[l->pos] == '?' && l->src[l->pos + 1] == '>'))
            l->pos++;
        l->pos += 2; /* saute '?>' */
        return parse_node(l);
    }

    /* ── Balise fermante </tag> ──────────────────────────────────────────── */
    if (lex_peek(l) == '/')
    {
        /* Saute jusqu'au '>' de la balise fermante. */
        while (l->pos < l->len && l->src[l->pos] != '>')
            l->pos++;

        /* REMARQUE : le if ici est redondant — la boucle s'arrête exactement
           sur '>' ou sur la fin de flux. On peut remplacer par un lex_next
           inconditionnel protégé par la vérification de fin de flux. */
        if (l->pos < l->len && lex_peek(l) == '>')
            lex_next(l); /* consomme '>' */

        /* Retourne NULL comme sentinelle pour signaler une balise fermante. */
        return NULL;
    }

    /* ── Balise ouvrante <tag ...> ──────────────────────────────────────── */
    char *tag = lex_read_ident(l);
    if (!tag)
        return NULL; /* REMARQUE : aucune mémoire allouée ici, le return est sûr. */

    XmlNode *node = node_new(tag);
    free(tag); /* node_new a dû copier le nom ; on libère la copie temporaire. */

    int self_closing = 0;
    node->attrs = parse_attrs(l, &self_closing);

    if (!self_closing)
    {
        XmlNode *ch_head = NULL, *ch_tail = NULL;

        while (1)
        {
            lex_skip_spaces(l);

            /* Détection anticipée de </tag> pour éviter un appel récursif inutile. */
            if (l->pos + 1 < l->len &&
                l->src[l->pos]     == '<' &&
                l->src[l->pos + 1] == '/')
            {
                /* REMARQUE : le nom de la balise fermante n'est pas vérifié.
                   Un XML mal formé comme <a><b></a> serait accepté sans erreur. */
                while (l->pos < l->len && l->src[l->pos] != '>')
                    l->pos++;
                if (l->pos < l->len && lex_peek(l) == '>')
                    lex_next(l); /* consomme '>' */
                break;
            }

            if (l->pos >= l->len)
                break; /* Fin de flux inattendue (XML mal formé). */

            XmlNode *child = parse_node(l);

            /* parse_node retourne NULL sur balise fermante ou erreur :
               on sort de la boucle dans les deux cas. */
            if (!child)
                break;

            /* Ajout du fils en queue de liste chaînée. */
            if (!ch_head)
                ch_head = ch_tail = child;
            else
            {
                ch_tail->next = child;
                ch_tail = child;
            }
        }
        node->children = ch_head;
    }
    return node;
}

/* ================================================================
 *  API parse
 * ================================================================ */

XmlNode *xml_parser_parse_string(const char *xml)
{
    if (!xml)
        return NULL;
    Lexer l;
    lex_init(&l, xml);     // initialise le lexer avec la chaîne XML( lexer = analyseur lexical, c'est lui qui va parcourir la chaîne de caractères et extraire les éléments)
    return parse_node(&l); // parse_node est la fonction récursive qui va construire l'arbre de nœuds XML à partir de la chaîne d'entrée. Elle utilise le lexer pour lire les balises, les attributs et les enfants, et retourne un pointeur vers le nœud racine de l'arbre construit.
}

XmlNode *xml_parser_parse_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        fprintf(stderr, "[xml_parser] Impossible d'ouvrir : %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END); // placer le curseur à la fin du fichier
    long sz = ftell(f);    // obtenir la position actuelle (taille du fichier)
    rewind(f);             // remettre le curseur au début du fichier
    char *buf = malloc(sz + 1);
    if (!buf)
    {
        fclose(f);
        return NULL;
    }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    XmlNode *root = xml_parser_parse_string(buf);
    free(buf);
    return root;
}

/* ================================================================
 *  CONSTRUCTION DES WIDGETS
 * ================================================================ */

/* Prototype de la fonction récursive principale */
static GtkWidget *build_widget(const XmlNode *node, Conteneur *parent_ct,
                               GtkApplication *app, GtkWindow *parent_win);

/* ----------------------------------------------------------------
 *  FENETRE
 * ----------------------------------------------------------------
 * Correspond exactement à ce que fait main.c :
 *   1. fenetre_initialiser / fenetre_creer
 *   2. Crée un root_ct vertical sur le tas → gtk_window_set_child
 *   3. Tous les enfants XML sont ajoutés dans root_ct
 */
static GtkWidget *build_fenetre(const XmlNode *n, GtkApplication *app)
{
    Fenetre win;
    fenetre_initialiser(&win);

    /* Titre */
    const char *title = xml_attr_get(n, "title");
    if (!title)
        title = xml_attr_get(n, "titre");
    if (title)
        set_str(&win.title, title);

    /* Couleur de fond */
    const char *bg = xml_attr_get(n, "bgcolor");
    if (!bg)
        bg = xml_attr_get(n, "color_bg");
    if (!bg)
        bg = xml_attr_get(n, "background");
    if (bg)
        set_str(&win.color_bg, bg);

    /* Dimensions */
    win.taille.width = attr_int(n, "width", win.taille.width);
    win.taille.height = attr_int(n, "height", win.taille.height);

    /* Comportement */
    const char *res = xml_attr_get(n, "resizable");
    if (res)
        win.resizable = (strcmp(res, "true") == 0 || strcmp(res, "1") == 0);

    const char *maxim = xml_attr_get(n, "maximisee");
    if (!maxim)
        maxim = xml_attr_get(n, "maximized");
    if (maxim)
        win.demarrer_maximisee = (strcmp(maxim, "true") == 0 || strcmp(maxim, "1") == 0);

    /* Boutons de la fenêtre */
    const char *bfermer = xml_attr_get(n, "bouton_fermer");
    const char *bagrandir = xml_attr_get(n, "bouton_agrandir");
    const char *breduire = xml_attr_get(n, "bouton_reduire");
    if (bfermer)
        win.bouton_fermer = (strcmp(bfermer, "true") == 0 || strcmp(bfermer, "1") == 0);
    if (bagrandir)
        win.bouton_agrandir = (strcmp(bagrandir, "true") == 0 || strcmp(bagrandir, "1") == 0);
    if (breduire)
        win.bouton_reduire = (strcmp(breduire, "true") == 0 || strcmp(breduire, "1") == 0);

    /* Icônes */
    const char *icon = xml_attr_get(n, "icon");
    if (!icon)
        icon = xml_attr_get(n, "icon_path");
    if (icon)
        set_str(&win.icon_path, icon);

    const char *ico = xml_attr_get(n, "ico_path");
    if (ico)
        set_str(&win.ico_path, ico);

    /* Image de fond */
    const char *bgimg = xml_attr_get(n, "background_image");
    if (bgimg)
        set_str(&win.background_image, bgimg);

    /* Défilement */
    const char *scroll = xml_attr_get(n, "scroll");
    if (scroll)
    {
        if (strcmp(scroll, "vertical") == 0)
            fenetre_set_scrollable(&win, SCROLL_VERTICAL);
        else if (strcmp(scroll, "horizontal") == 0)
            fenetre_set_scrollable(&win, SCROLL_HORIZONTAL);
        else if (strcmp(scroll, "both") == 0)
            fenetre_set_scrollable(&win, SCROLL_BOTH);
    }
    int scroll_w = attr_int(n, "scroll_min_width", 0);
    int scroll_h = attr_int(n, "scroll_min_height", 0);
    if (scroll_w || scroll_h)
        fenetre_set_scroll_content_size(&win, scroll_w, scroll_h);

    GtkWidget *window = fenetre_creer(&win, app);

    /*
     * Crée un conteneur racine vertical sur le tas (comme dans main.c) :
     *   root_ct → root_box → gtk_window_set_child(window, root_box)
     * Tous les enfants XML vont dans root_ct.
     */
    Conteneur *root_ct = g_new0(Conteneur, 1);
    conteneur_initialiser(root_ct);
    root_ct->orientation = CONTENEUR_VERTICAL;
    root_ct->espacement = 0;
    GtkWidget *root_box = conteneur_creer(root_ct);
    gtk_widget_set_hexpand(root_box, TRUE);
    gtk_widget_set_vexpand(root_box, TRUE);
    if (win.scroll_widget)
        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(win.scroll_widget), root_box);
    else
        gtk_window_set_child(GTK_WINDOW(window), root_box);
    g_signal_connect_swapped(root_box, "destroy", G_CALLBACK(g_free), root_ct);

    /* Enfants XML → ajoutés dans root_ct */
    for (const XmlNode *child = n->children; child; child = child->next)
        build_widget(child, root_ct, app, GTK_WINDOW(window));

    return window;
}

/* ----------------------------------------------------------------
 *  CONTENEUR
 * ----------------------------------------------------------------
 * IMPORTANT : alloué sur le tas pour que les pointeurs enfants
 * restent valides après le retour de cette fonction.
 */
static GtkWidget *build_conteneur(const XmlNode *n, Conteneur *parent_ct,
                                  GtkApplication *app, GtkWindow *parent_win)
{
    Conteneur *ct = g_new0(Conteneur, 1);
    conteneur_initialiser(ct);

    /* Orientation */
    const char *orient = xml_attr_get(n, "orientation");
    if (orient)
    {
        if (strcmp(orient, "horizontal") == 0 || strcmp(orient, "h") == 0)
            ct->orientation = CONTENEUR_HORIZONTAL;
        else
            ct->orientation = CONTENEUR_VERTICAL;
    }

    /* Espacement entre enfants */
    const char *esp = xml_attr_get(n, "espacement");
    if (!esp)
        esp = xml_attr_get(n, "spacing");
    if (esp)
        ct->espacement = atoi(esp);

    /* Homogène */
    ct->homogene = attr_bool(n, "homogene", FALSE);

    /* Couleur fond */
    const char *bg = xml_attr_get(n, "bgcolor");
    if (!bg)
        bg = xml_attr_get(n, "couleur_fond");
    if (!bg)
        bg = xml_attr_get(n, "background");
    if (bg)
        set_str(&ct->couleur_fond, bg);

    /* Dimensions forcées */
    const char *w = xml_attr_get(n, "width");
    const char *h = xml_attr_get(n, "height");
    if (w)
        ct->taille.largeur = atoi(w);
    if (h)
        ct->taille.hauteur = atoi(h);

    /* hexpand / vexpand des enfants */
    ct->enfants_hexpand = attr_bool(n, "hexpand", FALSE);
    ct->enfants_vexpand = attr_bool(n, "vexpand", FALSE);

    /* Padding (espace intérieur) */
    const char *p = xml_attr_get(n, "padding");
    if (p)
    {
        int pv = atoi(p);
        ct->padding.haut = ct->padding.bas = ct->padding.gauche = ct->padding.droite = pv;
    }
    const char *pt = xml_attr_get(n, "padding_top");
    if (pt)
        ct->padding.haut = atoi(pt);
    const char *pb = xml_attr_get(n, "padding_bottom");
    if (pb)
        ct->padding.bas = atoi(pb);
    const char *pl = xml_attr_get(n, "padding_left");
    if (pl)
        ct->padding.gauche = atoi(pl);
    const char *pr = xml_attr_get(n, "padding_right");
    if (pr)
        ct->padding.droite = atoi(pr);

    /* Marges (espace extérieur) */
    const char *m = xml_attr_get(n, "margin");
    if (m)
    {
        int mv = atoi(m);
        ct->marges.haut = ct->marges.bas = ct->marges.gauche = ct->marges.droite = mv;
    }
    const char *mt = xml_attr_get(n, "margin_top");
    if (mt)
        ct->marges.haut = atoi(mt);
    const char *mb = xml_attr_get(n, "margin_bottom");
    if (mb)
        ct->marges.bas = atoi(mb);
    const char *mleft = xml_attr_get(n, "margin_left");
    if (mleft)
        ct->marges.gauche = atoi(mleft);
    const char *mright = xml_attr_get(n, "margin_right");
    if (mright)
        ct->marges.droite = atoi(mright);

    /* Bordure */
    const char *bl = xml_attr_get(n, "bordure_largeur");
    if (!bl)
        bl = xml_attr_get(n, "border_width");
    if (bl)
        ct->bordure_largeur = atoi(bl);

    const char *bc = xml_attr_get(n, "bordure_couleur");
    if (!bc)
        bc = xml_attr_get(n, "border_color");
    if (bc)
        set_str(&ct->bordure_couleur, bc);

    const char *br = xml_attr_get(n, "bordure_rayon");
    if (!br)
        br = xml_attr_get(n, "border_radius");
    if (br)
        ct->bordure_rayon = atoi(br);

    /* Alignement du conteneur dans son parent */
    const char *ax = xml_attr_get(n, "align_x");
    if (ax)
    {
        if (strcmp(ax, "debut") == 0 || strcmp(ax, "start") == 0)
            ct->align_x = ALIGNEMENT_DEBUT;
        else if (strcmp(ax, "fin") == 0 || strcmp(ax, "end") == 0)
            ct->align_x = ALIGNEMENT_FIN;
        else if (strcmp(ax, "centre") == 0 || strcmp(ax, "center") == 0)
            ct->align_x = ALIGNEMENT_CENTRE;
        else
            ct->align_x = ALIGNEMENT_REMPLIR;
    }
    const char *ay = xml_attr_get(n, "align_y");
    if (ay)
    {
        if (strcmp(ay, "debut") == 0 || strcmp(ay, "start") == 0)
            ct->align_y = ALIGNEMENT_DEBUT;
        else if (strcmp(ay, "fin") == 0 || strcmp(ay, "end") == 0)
            ct->align_y = ALIGNEMENT_FIN;
        else if (strcmp(ay, "centre") == 0 || strcmp(ay, "center") == 0)
            ct->align_y = ALIGNEMENT_CENTRE;
        else
            ct->align_y = ALIGNEMENT_REMPLIR;
    }

    /* Défilement */
    const char *scroll = xml_attr_get(n, "scroll");
    if (scroll)
    {
        if (strcmp(scroll, "vertical") == 0)
            conteneur_set_scrollable(ct, SCROLL_VERTICAL);
        else if (strcmp(scroll, "horizontal") == 0)
            conteneur_set_scrollable(ct, SCROLL_HORIZONTAL);
        else if (strcmp(scroll, "both") == 0)
            conteneur_set_scrollable(ct, SCROLL_BOTH);
    }
    int sw = attr_int(n, "scroll_min_width", 0);
    int sh = attr_int(n, "scroll_min_height", 0);
    if (sw || sh)
        conteneur_set_scroll_size(ct, sw, sh);
    ct->scroll_overlay = attr_bool(n, "scroll_overlay", TRUE);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&ct->id_css, id);

    GtkWidget *box = conteneur_creer(ct);

    /* Expand pour remplir le parent (comportement par défaut de main.c) */
    gtk_widget_set_hexpand(box, TRUE);
    gtk_widget_set_vexpand(box, TRUE);

    /* Libération automatique à la destruction du widget */
    g_signal_connect_swapped(box, "destroy", G_CALLBACK(g_free), ct);

    /* Ajout dans le parent */
    if (parent_ct)
        conteneur_ajouter(parent_ct, box);

    /* Enfants récursifs — ct est sur le tas, toujours valide */
    for (const XmlNode *child = n->children; child; child = child->next)
        build_widget(child, ct, app, parent_win);

    return box;
}

/* ----------------------------------------------------------------
 *  SEPARATEUR
 * ---------------------------------------------------------------- */
static GtkWidget *build_separateur(const XmlNode *n, Conteneur *parent_ct)
{
    const char *orient = xml_attr_get(n, "orientation");
    GtkOrientation o = GTK_ORIENTATION_HORIZONTAL;
    if (orient && (strcmp(orient, "vertical") == 0 || strcmp(orient, "v") == 0))
        o = GTK_ORIENTATION_VERTICAL;

    GtkWidget *sep = gtk_separator_new(o);
    if (parent_ct)
        conteneur_ajouter(parent_ct, sep);
    return sep;
}

/* ----------------------------------------------------------------
 *  TEXTE (GtkLabel)
 * ---------------------------------------------------------------- */
static GtkWidget *build_texte(const XmlNode *n, Conteneur *parent_ct)
{
    Texte *t = g_new0(Texte, 1);
    texte_initialiser(t);

    /* Contenu */
    const char *txt = xml_attr_get(n, "texte");
    if (!txt)
        txt = xml_attr_get(n, "text");
    if (!txt)
        txt = xml_attr_get(n, "value");
    if (!txt)
        txt = xml_attr_get(n, "label");
    if (txt)
        set_str(&t->texte, txt);

    /* Markup Pango */
    const char *markup = xml_attr_get(n, "markup");
    if (markup)
    {
        set_str(&t->texte_markup, markup);
        t->use_markup = TRUE;
    }

    /* Type (h1..h6, normal, subtitle, caption) */
    const char *type = xml_attr_get(n, "type");
    if (type)
    {
        if (strcmp(type, "h1") == 0)
            t->type = TEXTE_H1;
        else if (strcmp(type, "h2") == 0)
            t->type = TEXTE_H2;
        else if (strcmp(type, "h3") == 0)
            t->type = TEXTE_H3;
        else if (strcmp(type, "h4") == 0)
            t->type = TEXTE_H4;
        else if (strcmp(type, "h5") == 0)
            t->type = TEXTE_H5;
        else if (strcmp(type, "h6") == 0)
            t->type = TEXTE_H6;
        else if (strcmp(type, "subtitle") == 0)
            t->type = TEXTE_SUBTITLE;
        else if (strcmp(type, "caption") == 0)
            t->type = TEXTE_CAPTION;
        else
            t->type = TEXTE_NORMAL;
    }

    /* Alignement texte */
    const char *align = xml_attr_get(n, "alignement");
    if (!align)
        align = xml_attr_get(n, "align");
    if (align)
    {
        if (strcmp(align, "center") == 0 || strcmp(align, "centre") == 0)
            t->alignement = TEXTE_ALIGN_CENTER;
        else if (strcmp(align, "right") == 0 || strcmp(align, "droite") == 0)
            t->alignement = TEXTE_ALIGN_RIGHT;
        else if (strcmp(align, "justify") == 0 || strcmp(align, "justifie") == 0)
            t->alignement = TEXTE_ALIGN_JUSTIFY;
        else
            t->alignement = TEXTE_ALIGN_LEFT;
    }

    /* Décoration */
    const char *deco = xml_attr_get(n, "decoration");
    if (deco)
    {
        if (strcmp(deco, "underline") == 0)
            t->decoration = TEXTE_DECORATION_UNDERLINE;
        else if (strcmp(deco, "overline") == 0)
            t->decoration = TEXTE_DECORATION_OVERLINE;
        else if (strcmp(deco, "strikethrough") == 0)
            t->decoration = TEXTE_DECORATION_STRIKETHROUGH;
        else
            t->decoration = TEXTE_DECORATION_NONE;
    }

    /* Couleurs */
    const char *col = xml_attr_get(n, "couleur");
    if (!col)
        col = xml_attr_get(n, "color");
    if (col)
        set_str(&t->couleur_texte, col);

    const char *bgc = xml_attr_get(n, "bgcolor");
    if (!bgc)
        bgc = xml_attr_get(n, "background");
    if (bgc)
        set_str(&t->couleur_fond, bgc);

    /* Police */
    const char *font = xml_attr_get(n, "police");
    if (!font)
        font = xml_attr_get(n, "font");
    if (font)
        set_str(&t->famille_police, font);

    const char *sz = xml_attr_get(n, "taille");
    if (!sz)
        sz = xml_attr_get(n, "size");
    if (sz)
        t->taille_police = atoi(sz);

    /* Attributs gras/italique — accepte les deux langues */
    t->gras = attr_bool(n, "gras", FALSE) || attr_bool(n, "bold", FALSE);
    t->italique = attr_bool(n, "italique", FALSE) || attr_bool(n, "italic", FALSE);

    /* Retour à la ligne */
    t->wrap = attr_bool(n, "wrap", FALSE);
    const char *ww = xml_attr_get(n, "wrap_width");
    if (ww)
        t->wrap_width = atoi(ww);

    /* Ellipsis */
    t->ellipsize = attr_bool(n, "ellipsize", FALSE);

    /* Sélectabilité */
    t->selectable = attr_bool(n, "selectable", FALSE);

    /* Dimensions widget */
    const char *tw = xml_attr_get(n, "width");
    const char *th = xml_attr_get(n, "height");
    if (tw)
        t->taille.largeur = atoi(tw);
    if (th)
        t->taille.hauteur = atoi(th);

    /* Marges */
    const char *mg = xml_attr_get(n, "margin");
    if (mg)
    {
        int mv = atoi(mg);
        t->marges.haut = t->marges.bas = t->marges.gauche = t->marges.droite = mv;
    }
    const char *mgt = xml_attr_get(n, "margin_top");
    if (mgt)
        t->marges.haut = atoi(mgt);
    const char *mgb = xml_attr_get(n, "margin_bottom");
    if (mgb)
        t->marges.bas = atoi(mgb);
    const char *mgl = xml_attr_get(n, "margin_left");
    if (mgl)
        t->marges.gauche = atoi(mgl);
    const char *mgr = xml_attr_get(n, "margin_right");
    if (mgr)
        t->marges.droite = atoi(mgr);

    /* Bordure */
    const char *bw = xml_attr_get(n, "border_width");
    if (bw)
        t->bordure_largeur = atoi(bw);
    const char *bco = xml_attr_get(n, "border_color");
    if (bco)
        t->bordure_couleur = xstrdup(bco);
    const char *brz = xml_attr_get(n, "border_radius");
    if (brz)
        t->bordure_rayon = atoi(brz);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&t->id_css, id);

    GtkWidget *w = texte_creer(t);

    /* Libère la struct Texte à la destruction du widget */
    g_signal_connect_swapped(w, "destroy", G_CALLBACK(g_free), t);

    if (parent_ct)
        conteneur_ajouter(parent_ct, w);
    return w;
}

/* ----------------------------------------------------------------
 *  BOUTON
 * ---------------------------------------------------------------- */
static GtkWidget *build_bouton(const XmlNode *n, Conteneur *parent_ct)
{
    Bouton b;
    bouton_initialiser(&b);

    /* Texte du bouton */
    const char *txt = xml_attr_get(n, "texte");
    if (!txt)
        txt = xml_attr_get(n, "text");
    if (!txt)
        txt = xml_attr_get(n, "label");
    if (txt)
        set_str(&b.texte, txt);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&b.id_css, id);

    /* Icône */
    const char *icon = xml_attr_get(n, "icone");
    if (!icon)
        icon = xml_attr_get(n, "icon");
    if (icon)
        b.nom_icone = xstrdup(icon);

    /* Position icône */
    const char *ipos = xml_attr_get(n, "icone_pos");
    if (ipos)
    {
        if (strcmp(ipos, "droite") == 0 || strcmp(ipos, "right") == 0)
            b.pos_icone = ICONE_DROITE;
        else if (strcmp(ipos, "haut") == 0 || strcmp(ipos, "top") == 0)
            b.pos_icone = ICONE_HAUT;
        else if (strcmp(ipos, "bas") == 0 || strcmp(ipos, "bottom") == 0)
            b.pos_icone = ICONE_BAS;
        else if (strcmp(ipos, "seule") == 0 || strcmp(ipos, "only") == 0)
            b.pos_icone = ICONE_SEULE;
        else
            b.pos_icone = ICONE_GAUCHE;
    }

    /* Couleurs */
    const char *bg = xml_attr_get(n, "bgcolor");
    if (!bg)
        bg = xml_attr_get(n, "bg");
    if (bg)
        set_str(&b.style.bg_normal, bg);

    const char *fg = xml_attr_get(n, "color");
    if (!fg)
        fg = xml_attr_get(n, "fg");
    if (fg)
        set_str(&b.style.fg_normal, fg);

    const char *bgh = xml_attr_get(n, "bgcolor_hover");
    if (!bgh)
        bgh = xml_attr_get(n, "bg_hover");
    if (bgh)
        set_str(&b.style.bg_hover, bgh);

    const char *fgh = xml_attr_get(n, "color_hover");
    if (!fgh)
        fgh = xml_attr_get(n, "fg_hover");
    if (fgh)
        set_str(&b.style.fg_hover, fgh);

    /* Bordure */
    const char *rad = xml_attr_get(n, "radius");
    if (!rad)
        rad = xml_attr_get(n, "bordure_rayon");
    if (!rad)
        rad = xml_attr_get(n, "border_radius");
    if (rad)
        b.style.rayon_arrondi = atoi(rad);

    const char *bw = xml_attr_get(n, "border_width");
    if (bw)
        b.style.epaisseur_bordure = atoi(bw);

    const char *bc = xml_attr_get(n, "border_color");
    if (bc)
        set_str(&b.style.couleur_bordure, bc);

    /* Police */
    b.style.gras = attr_bool(n, "gras", FALSE) || attr_bool(n, "bold", FALSE);
    b.style.italique = attr_bool(n, "italique", FALSE) || attr_bool(n, "italic", FALSE);
    b.style.taille_texte_px = attr_int(n, "taille_texte", 0);

    /* Dimensions */
    const char *bw2 = xml_attr_get(n, "width");
    const char *bh = xml_attr_get(n, "height");
    if (bw2 || bh)
    {
        b.taille.mode = TAILLE_FIXE;
        b.taille.largeur = bw2 ? atoi(bw2) : -1;
        b.taille.hauteur = bh ? atoi(bh) : -1;
    }

    /* Comportement */
    b.est_actif = attr_bool(n, "actif", TRUE);

    const char *tt = xml_attr_get(n, "tooltip");
    if (tt)
        b.tooltip = xstrdup(tt);

    /* Curseur */
    const char *cur = xml_attr_get(n, "curseur");
    if (!cur)
        cur = xml_attr_get(n, "cursor");
    if (cur)
    {
        if (strcmp(cur, "main") == 0 || strcmp(cur, "pointer") == 0)
            b.curseur = CURSEUR_MAIN;
        else if (strcmp(cur, "aide") == 0 || strcmp(cur, "help") == 0)
            b.curseur = CURSEUR_AIDE;
        else if (strcmp(cur, "attente") == 0 || strcmp(cur, "wait") == 0)
            b.curseur = CURSEUR_ATTENTE;
        else if (strcmp(cur, "croix") == 0 || strcmp(cur, "cross") == 0)
            b.curseur = CURSEUR_CROIX;
        else
            b.curseur = CURSEUR_DEFAUT;
    }

    GtkWidget *widget = bouton_creer(&b);
    if (parent_ct)
        conteneur_ajouter(parent_ct, widget);
    return widget;
}

/* ----------------------------------------------------------------
 *  BOUTON_CHECKLIST (GtkCheckButton)
 * ---------------------------------------------------------------- */
static GtkWidget *build_checklist(const XmlNode *n, Conteneur *parent_ct)
{
    BoutonChecklist chk;
    bouton_checklist_initialiser(&chk);

    const char *lbl = xml_attr_get(n, "label");
    if (!lbl)
        lbl = xml_attr_get(n, "texte");
    if (!lbl)
        lbl = xml_attr_get(n, "text");
    if (lbl)
        set_str(&chk.label, lbl);

    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&chk.id_css, id);

    /* État */
    const char *etat = xml_attr_get(n, "etat");
    if (!etat)
        etat = xml_attr_get(n, "checked");
    if (!etat)
        etat = xml_attr_get(n, "state");
    if (etat)
    {
        if (strcmp(etat, "true") == 0 || strcmp(etat, "1") == 0 || strcmp(etat, "checked") == 0)
            chk.etat = CHECKLIST_CHECKED;
        else if (strcmp(etat, "inconsistent") == 0 || strcmp(etat, "indeterminate") == 0)
            chk.etat = CHECKLIST_INCONSISTENT;
        else
            chk.etat = CHECKLIST_UNCHECKED;
    }

    /* Position du label */
    const char *lpos = xml_attr_get(n, "label_pos");
    if (lpos && (strcmp(lpos, "gauche") == 0 || strcmp(lpos, "left") == 0))
        chk.pos_label = CHECKLIST_LABEL_GAUCHE;

    /* Style */
    const char *col = xml_attr_get(n, "color");
    if (!col)
        col = xml_attr_get(n, "couleur");
    if (col)
        set_str(&chk.style.couleur_texte, col);

    chk.style.gras = attr_bool(n, "gras", FALSE) || attr_bool(n, "bold", FALSE);
    chk.style.taille_texte_px = attr_int(n, "taille_texte", 0);

    chk.est_actif = attr_bool(n, "actif", TRUE);

    const char *tt = xml_attr_get(n, "tooltip");
    if (tt)
        chk.tooltip = xstrdup(tt);

    GtkWidget *w = bouton_checklist_creer(&chk);
    if (parent_ct)
        conteneur_ajouter(parent_ct, w);
    return w;
}

/* ----------------------------------------------------------------
 *  CHAMP_MOTDEPASSE (GtkPasswordEntry)
 * ---------------------------------------------------------------- */
static GtkWidget *build_champ_motdepasse(const XmlNode *n, Conteneur *parent_ct)
{
    ChampMotDePasse *cfg = g_new0(ChampMotDePasse, 1);
    champ_motdepasse_initialiser(cfg);

    const char *ph = xml_attr_get(n, "placeholder");
    if (!ph)
        ph = xml_attr_get(n, "hint");
    if (ph)
        champ_motdepasse_set_placeholder(cfg, ph);

    int maxl = attr_int(n, "max_length", 0);
    if (!maxl)
        maxl = attr_int(n, "maxlength", 0);
    if (maxl)
        champ_motdepasse_set_max_length(cfg, maxl);

    cfg->required = attr_bool(n, "required", FALSE);
    cfg->reveal_toggle = attr_bool(n, "reveal", TRUE);
    cfg->sensitive = attr_bool(n, "actif", TRUE);

    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&cfg->id_css, id);

    cfg->size.width = attr_int(n, "width", 0);
    cfg->size.height = attr_int(n, "height", 0);

    /* Politique de validation */
    ChampPasswordPolicy pol = {0};
    pol.min_len = attr_int(n, "min_len", 0);
    pol.require_digit = attr_bool(n, "require_digit", FALSE);
    pol.require_upper = attr_bool(n, "require_upper", FALSE);
    pol.require_symbol = attr_bool(n, "require_symbol", FALSE);
    champ_motdepasse_set_policy(cfg, pol);

    /* Style du label d'erreur */
    cfg->show_error_label = attr_bool(n, "show_error", TRUE);
    const char *ecol = xml_attr_get(n, "erreur_couleur");
    if (!ecol)
        ecol = xml_attr_get(n, "error_color");
    if (ecol)
        cfg->erreur_couleur = xstrdup(ecol);
    cfg->erreur_taille_px = attr_int(n, "erreur_taille", 0);

    /* Style global du champ */
    const char *bg = xml_attr_get(n, "bgcolor");
    if (!bg)
        bg = xml_attr_get(n, "background");
    if (bg)
        cfg->style.bg_normal = xstrdup(bg);

    const char *col = xml_attr_get(n, "color");
    if (!col)
        col = xml_attr_get(n, "couleur");
    if (col)
        cfg->style.fg_normal = xstrdup(col);

    champ_motdepasse_creer(cfg);
    /* On ajoute cfg->container (le GtkBox vertical avec champ + label erreur) */
    if (parent_ct)
        conteneur_ajouter(parent_ct, cfg->container);

    /* Libération automatique */
    g_signal_connect_swapped(cfg->container, "destroy", G_CALLBACK(champ_motdepasse_free), cfg);
    g_signal_connect(cfg->container, "destroy", G_CALLBACK(g_free), cfg);

    return cfg->widget;
}

/* ----------------------------------------------------------------
 *  BOUTON_RADIO (GtkCheckButton en mode radio)
 * ----------------------------------------------------------------
 * IMPORTANT : les radios d'un même groupe partagent le leader.
 * Dans le XML, le 1er <bouton_radio> du groupe n'a pas d'attribut
 * "groupe" ; les suivants pointent vers l'id CSS du leader.
 *
 * Limitation parser : on ne peut pas résoudre les références croisées
 * d'id à la volée.  On gère donc le groupe via un attribut numérique
 * optionnel "groupe_leader_widget" stocké dans GObject data.
 * Stratégie choisie : le parser garde un pointeur statique sur le
 * dernier widget leader créé PAR CONTENEUR.  C'est suffisant pour
 * les cas courants (un seul groupe radio par conteneur).
 * Pour plusieurs groupes distincts, utilisez l'attribut "groupe_nom"
 * (chaîne arbitraire) : tous les radios avec la même valeur
 * groupe_nom sont liés.
 *
 * Implémentation simplifiée : on utilise une table de hachage légère
 * (tableau fixe de 32 entrées) nom_groupe → GtkCheckButton* leader.
 * ---------------------------------------------------------------- */

#define RADIO_GROUP_MAX 32
typedef struct
{
    char nom[64];
    GtkCheckButton *leader;
} RadioGroupEntry;
static RadioGroupEntry _radio_groups[RADIO_GROUP_MAX];
static int _radio_groups_count = 0;

static GtkCheckButton *radio_group_get(const char *nom)
{
    for (int i = 0; i < _radio_groups_count; i++)
        if (strcmp(_radio_groups[i].nom, nom) == 0)
            return _radio_groups[i].leader;
    return NULL;
}

static void radio_group_set(const char *nom, GtkCheckButton *leader)
{
    for (int i = 0; i < _radio_groups_count; i++)
    {
        if (strcmp(_radio_groups[i].nom, nom) == 0)
        {
            _radio_groups[i].leader = leader;
            return;
        }
    }
    if (_radio_groups_count < RADIO_GROUP_MAX)
    {
        strncpy(_radio_groups[_radio_groups_count].nom, nom, 63);
        _radio_groups[_radio_groups_count].nom[63] = '\0';
        _radio_groups[_radio_groups_count].leader = leader;
        _radio_groups_count++;
    }
}

static GtkWidget *build_bouton_radio(const XmlNode *n, Conteneur *parent_ct)
{
    BoutonRadio *cfg = g_new0(BoutonRadio, 1);
    bouton_radio_initialiser(cfg);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&cfg->id_css, id);

    /* Label */
    const char *lbl = xml_attr_get(n, "label");
    if (!lbl)
        lbl = xml_attr_get(n, "texte");
    if (!lbl)
        lbl = xml_attr_get(n, "text");
    if (lbl)
        set_str(&cfg->label, lbl);

    /* Position label */
    const char *lpos = xml_attr_get(n, "label_pos");
    if (lpos && (strcmp(lpos, "gauche") == 0 || strcmp(lpos, "left") == 0))
        cfg->pos_label = RADIO_LABEL_GAUCHE;

    /* État initial */
    cfg->est_actif = attr_bool(n, "actif", FALSE) || attr_bool(n, "checked", FALSE);

    /* Sensibilité */
    cfg->sensible = attr_bool(n, "sensible", TRUE);

    /* Tooltip */
    const char *tt = xml_attr_get(n, "tooltip");
    if (tt)
        cfg->tooltip = xstrdup(tt);

    /* Style */
    const char *col = xml_attr_get(n, "color");
    if (!col)
        col = xml_attr_get(n, "couleur");
    if (col)
        cfg->style.couleur_texte = xstrdup(col);

    const char *colh = xml_attr_get(n, "color_hover");
    if (colh)
        cfg->style.couleur_texte_hover = xstrdup(colh);

    const char *colp = xml_attr_get(n, "color_point");
    if (colp)
        cfg->style.couleur_point = xstrdup(colp);

    cfg->style.taille_texte_px = attr_int(n, "taille_texte", 0);
    cfg->style.gras = attr_bool(n, "gras", FALSE) || attr_bool(n, "bold", FALSE);

    /* Groupe radio — résolution par nom */
    const char *grp = xml_attr_get(n, "groupe_nom");
    if (!grp)
        grp = xml_attr_get(n, "group");
    if (grp)
    {
        GtkCheckButton *leader = radio_group_get(grp);
        if (leader)
        {
            /* Ce radio rejoint un groupe existant */
            cfg->group_leader = leader;
        }
        /* Sinon : ce sera le leader, on l'enregistre après création */
    }

    GtkWidget *w = bouton_radio_creer(cfg);

    /* Si leader d'un groupe nommé, l'enregistrer maintenant */
    if (grp && cfg->group_leader == NULL)
        radio_group_set(grp, GTK_CHECK_BUTTON(w));

    /* Libération automatique */
    g_signal_connect_swapped(w, "destroy", G_CALLBACK(g_free), cfg);

    if (parent_ct)
        conteneur_ajouter(parent_ct, w);
    return w;
}

/* ----------------------------------------------------------------
 *  CHAMP_TEXTE (GtkEntry)
 * ---------------------------------------------------------------- */
static GtkWidget *build_champ_texte(const XmlNode *n, Conteneur *parent_ct)
{
    ChampTexte *cfg = g_new0(ChampTexte, 1);
    champ_texte_initialiser(cfg);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&cfg->id_css, id);

    /* Placeholder */
    const char *ph = xml_attr_get(n, "placeholder");
    if (!ph)
        ph = xml_attr_get(n, "hint");
    if (ph)
        champ_texte_set_placeholder(cfg, ph);

    /* Longueur max */
    int maxl = attr_int(n, "max_length", 0);
    if (!maxl)
        maxl = attr_int(n, "maxlength", 0);
    if (maxl)
        champ_texte_set_max_length(cfg, maxl);

    /* Comportement */
    cfg->required = attr_bool(n, "required", FALSE);
    cfg->editable = attr_bool(n, "editable", TRUE);
    cfg->sensitive = attr_bool(n, "actif", TRUE);

    /* Type d'entrée */
    const char *type = xml_attr_get(n, "type");
    if (type)
    {
        if (strcmp(type, "email") == 0)
            cfg->type = CHAMP_TEXTE_TYPE_EMAIL;
        else if (strcmp(type, "url") == 0)
            cfg->type = CHAMP_TEXTE_TYPE_URL;
        else if (strcmp(type, "search") == 0)
            cfg->type = CHAMP_TEXTE_TYPE_SEARCH;
        else
            cfg->type = CHAMP_TEXTE_TYPE_TEXT;
    }

    /* Politique de validation */
    ChampTextePolicy pol = {0};
    pol.min_len = attr_int(n, "min_len", 0);
    pol.max_len = attr_int(n, "max_len", 0);
    pol.no_whitespace = attr_bool(n, "no_whitespace", FALSE);
    pol.no_digits = attr_bool(n, "no_digits", FALSE);
    pol.only_digits = attr_bool(n, "only_digits", FALSE);
    const char *pat = xml_attr_get(n, "pattern");
    if (pat)
        pol.pattern = pat; /* pointe dans l'arbre XML (durée de vie suffisante) */
    champ_texte_set_policy(cfg, pol);

    /* Dimensions */
    int w = attr_int(n, "width", 0);
    int h = attr_int(n, "height", 0);
    if (w || h)
        champ_texte_set_size(cfg, w, h);

    /* Icônes */
    const char *ico1 = xml_attr_get(n, "icon_primary");
    const char *ico2 = xml_attr_get(n, "icon_secondary");
    if (ico1 || ico2)
        champ_texte_set_icons(cfg, ico1, ico2);

    /* Style */
    const char *bg = xml_attr_get(n, "bgcolor");
    if (!bg)
        bg = xml_attr_get(n, "background");
    if (bg)
        cfg->style.bg_normal = xstrdup(bg);

    const char *fg = xml_attr_get(n, "color");
    if (!fg)
        fg = xml_attr_get(n, "couleur");
    if (fg)
        cfg->style.fg_normal = xstrdup(fg);

    /* Label d'erreur */
    cfg->show_error_label = attr_bool(n, "show_error", TRUE);
    const char *ecol = xml_attr_get(n, "erreur_couleur");
    if (!ecol)
        ecol = xml_attr_get(n, "error_color");
    if (ecol)
        cfg->erreur_couleur = xstrdup(ecol);
    cfg->erreur_taille_px = attr_int(n, "erreur_taille", 0);

    /* Création : retourne cfg->container (GtkBox vertical) */
    GtkWidget *container = champ_texte_creer(cfg);

    if (parent_ct)
        conteneur_ajouter(parent_ct, container);

    /* Libération automatique */
    g_signal_connect_swapped(container, "destroy", G_CALLBACK(champ_texte_free), cfg);
    g_signal_connect(container, "destroy", G_CALLBACK(g_free), cfg);

    return cfg->widget; /* retourne le GtkEntry pour usage avancé */
}

/* ----------------------------------------------------------------
 *  CHAMP_NOMBRE (GtkSpinButton)
 * ---------------------------------------------------------------- */
static GtkWidget *build_champ_nombre(const XmlNode *n, Conteneur *parent_ct)
{
    ChampNombre *cfg = g_new0(ChampNombre, 1);
    champ_nombre_initialiser(cfg);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&cfg->id_css, id);

    /* Bornes & valeur */
    const char *vmin = xml_attr_get(n, "min");
    const char *vmax = xml_attr_get(n, "max");
    if (vmin)
        cfg->min = atof(vmin);
    if (vmax)
        cfg->max = atof(vmax);

    const char *vval = xml_attr_get(n, "valeur");
    if (!vval)
        vval = xml_attr_get(n, "value");
    if (vval)
        cfg->valeur = atof(vval);

    const char *vstep = xml_attr_get(n, "step");
    if (vstep)
        cfg->step = atof(vstep);

    cfg->digits = (guint)attr_int(n, "digits", 0);
    cfg->wrap = attr_bool(n, "wrap", FALSE);
    cfg->required = attr_bool(n, "required", FALSE);

    /* Taille */
    int w = attr_int(n, "width", 0);
    int h = attr_int(n, "height", 0);
    if (w || h)
        champ_nombre_set_size(cfg, w, h);

    /* Style */
    const char *bg = xml_attr_get(n, "bgcolor");
    if (!bg)
        bg = xml_attr_get(n, "background");
    if (bg)
        cfg->style.bg_normal = xstrdup(bg);

    const char *fg = xml_attr_get(n, "color");
    if (!fg)
        fg = xml_attr_get(n, "couleur");
    if (fg)
        cfg->style.fg_normal = xstrdup(fg);

    cfg->style.rayon_arrondi = attr_int(n, "radius", 0);
    cfg->style.epaisseur_bordure = attr_int(n, "border_width", 0);
    cfg->style.gras = attr_bool(n, "gras", FALSE) || attr_bool(n, "bold", FALSE);
    cfg->style.taille_texte_px = attr_int(n, "taille_texte", 0);

    GtkWidget *w_spin = champ_nombre_creer(cfg);

    if (parent_ct)
        conteneur_ajouter(parent_ct, w_spin);

    g_signal_connect_swapped(w_spin, "destroy", G_CALLBACK(champ_nombre_free), cfg);
    g_signal_connect(w_spin, "destroy", G_CALLBACK(g_free), cfg);

    return w_spin;
}

/* ----------------------------------------------------------------
 *  CHAMP_SELECT (GtkDropDown)
 * ---------------------------------------------------------------- */
static GtkWidget *build_champ_select(const XmlNode *n, Conteneur *parent_ct)
{
    ChampSelect *cfg = g_new0(ChampSelect, 1);
    champ_select_initialiser(cfg);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&cfg->id_css, id);

    /* Items : attribut "items" séparé par des virgules OU enfants <option> */
    const char *items_str = xml_attr_get(n, "items");
    if (items_str)
    {
        /* Parse "Item1,Item2,Item3" */
        char *copy = xstrdup(items_str);
        char *tok = strtok(copy, ",");
        while (tok)
        {
            /* Trim espaces de tête */
            while (*tok == ' ')
                tok++;
            champ_select_add_item(cfg, tok);
            tok = strtok(NULL, ",");
        }
        free(copy);
    }

    /* Enfants <option texte="…"/> ou <option>Texte</option> (tag "option") */
    for (const XmlNode *ch = n->children; ch; ch = ch->next)
    {
        if (!ch->tag)
            continue;
        if (strcmp(ch->tag, "option") == 0)
        {
            const char *opt = xml_attr_get(ch, "texte");
            if (!opt)
                opt = xml_attr_get(ch, "text");
            if (!opt)
                opt = xml_attr_get(ch, "value");
            if (opt)
                champ_select_add_item(cfg, opt);
        }
    }

    /* Index sélectionné */
    cfg->selected_index = attr_int(n, "selected", -1);
    if (cfg->selected_index < 0)
        cfg->selected_index = attr_int(n, "index", 0);

    cfg->required = attr_bool(n, "required", FALSE);
    cfg->enable_search = attr_bool(n, "search", FALSE);

    /* Taille */
    int w = attr_int(n, "width", 0);
    int h = attr_int(n, "height", 0);
    if (w || h)
        champ_select_set_size(cfg, w, h);

    /* Style */
    const char *bg = xml_attr_get(n, "bgcolor");
    if (!bg)
        bg = xml_attr_get(n, "background");
    if (bg)
        cfg->style.bg_normal = xstrdup(bg);

    const char *fg = xml_attr_get(n, "color");
    if (!fg)
        fg = xml_attr_get(n, "couleur");
    if (fg)
        cfg->style.fg_normal = xstrdup(fg);

    cfg->style.rayon_arrondi = attr_int(n, "radius", 0);
    cfg->style.epaisseur_bordure = attr_int(n, "border_width", 0);

    GtkWidget *w_dd = champ_select_creer(cfg);

    if (parent_ct)
        conteneur_ajouter(parent_ct, w_dd);

    g_signal_connect_swapped(w_dd, "destroy", G_CALLBACK(champ_select_free), cfg);
    g_signal_connect(w_dd, "destroy", G_CALLBACK(g_free), cfg);

    return w_dd;
}

/* ----------------------------------------------------------------
 *  CHAMP_ZONE_TEXTE (GtkTextView)
 * ---------------------------------------------------------------- */
static GtkWidget *build_champ_zone_texte(const XmlNode *n, Conteneur *parent_ct)
{
    ChampZoneTexte *cfg = g_new0(ChampZoneTexte, 1);
    champ_zone_texte_initialiser(cfg);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&cfg->id_css, id);

    /* Contenu initial */
    const char *txt = xml_attr_get(n, "texte");
    if (!txt)
        txt = xml_attr_get(n, "text");
    if (!txt)
        txt = xml_attr_get(n, "value");
    if (txt)
        champ_zone_texte_set_texte(cfg, txt);

    /* Contraintes */
    int maxl = attr_int(n, "max_length", 0);
    if (!maxl)
        maxl = attr_int(n, "maxlength", 0);
    if (maxl)
        champ_zone_texte_set_max_length(cfg, maxl);

    cfg->wrap_word = attr_bool(n, "wrap_word", TRUE);
    cfg->sensitive = attr_bool(n, "actif", TRUE);
    cfg->required = attr_bool(n, "required", FALSE);

    /* Taille */
    int w = attr_int(n, "width", 0);
    int h = attr_int(n, "height", 0);
    if (w || h)
        champ_zone_texte_set_size(cfg, w, h);

    /* Style */
    const char *bg = xml_attr_get(n, "bgcolor");
    if (!bg)
        bg = xml_attr_get(n, "background");
    if (bg)
        cfg->style.bg_normal = xstrdup(bg);

    const char *fg = xml_attr_get(n, "color");
    if (!fg)
        fg = xml_attr_get(n, "couleur");
    if (fg)
        cfg->style.fg_normal = xstrdup(fg);

    cfg->style.rayon_arrondi = attr_int(n, "radius", 0);
    cfg->style.epaisseur_bordure = attr_int(n, "border_width", 0);

    GtkWidget *w_tv = champ_zone_texte_creer(cfg);

    if (parent_ct)
        conteneur_ajouter(parent_ct, w_tv);

    g_signal_connect_swapped(w_tv, "destroy", G_CALLBACK(champ_zone_texte_free), cfg);
    g_signal_connect(w_tv, "destroy", G_CALLBACK(g_free), cfg);

    return w_tv;
}

/* ----------------------------------------------------------------
 *  SLIDER (GtkScale)
 * ---------------------------------------------------------------- */
static GtkWidget *build_slider(const XmlNode *n, Conteneur *parent_ct)
{
    Slider *cfg = g_new0(Slider, 1);
    slider_initialiser(cfg);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&cfg->id_css, id);

    /* Bornes & valeur */
    const char *vmin = xml_attr_get(n, "min");
    const char *vmax = xml_attr_get(n, "max");
    if (vmin)
        cfg->min = atof(vmin);
    if (vmax)
        cfg->max = atof(vmax);

    const char *vval = xml_attr_get(n, "valeur");
    if (!vval)
        vval = xml_attr_get(n, "value");
    if (vval)
        cfg->valeur = atof(vval);

    const char *vstep = xml_attr_get(n, "step");
    if (vstep)
        cfg->step = atof(vstep);

    cfg->digits = (guint)attr_int(n, "digits", 0);

    /* Orientation */
    const char *orient = xml_attr_get(n, "orientation");
    if (orient && (strcmp(orient, "vertical") == 0 || strcmp(orient, "v") == 0))
        cfg->orientation = SLIDER_VERTICAL;
    else
        cfg->orientation = SLIDER_HORIZONTAL;

    /* Comportement */
    cfg->afficher_valeur = attr_bool(n, "afficher_valeur", FALSE) || attr_bool(n, "show_value", FALSE);
    cfg->afficher_label = attr_bool(n, "afficher_label", FALSE) || attr_bool(n, "show_label", FALSE);
    cfg->inverser = attr_bool(n, "inverser", FALSE) || attr_bool(n, "invert", FALSE);
    cfg->sensitive = attr_bool(n, "actif", TRUE);

    /* Marques */
    const char *mpos = xml_attr_get(n, "marques");
    if (mpos)
    {
        if (strcmp(mpos, "dessus") == 0)
            cfg->marques_pos = SLIDER_MARQUES_DESSUS;
        else if (strcmp(mpos, "dessous") == 0)
            cfg->marques_pos = SLIDER_MARQUES_DESSOUS;
        else if (strcmp(mpos, "les_deux") == 0)
            cfg->marques_pos = SLIDER_MARQUES_LES_DEUX;
        else
            cfg->marques_pos = SLIDER_MARQUES_AUCUNE;
    }
    const char *mstep = xml_attr_get(n, "marques_step");
    if (mstep)
        cfg->marques_step = atof(mstep);

    /* Taille */
    int w = attr_int(n, "width", 0);
    int h = attr_int(n, "height", 0);
    if (w || h)
        slider_set_size(cfg, w, h);

    /* Style */
    const char *bg = xml_attr_get(n, "bgcolor");
    if (bg)
        cfg->style.bg_normal = xstrdup(bg);
    const char *fg = xml_attr_get(n, "color");
    if (fg)
        cfg->style.fg_normal = xstrdup(fg);

    GtkWidget *w_sld = slider_creer(cfg);

    if (parent_ct)
        conteneur_ajouter(parent_ct, w_sld);

    g_signal_connect_swapped(w_sld, "destroy", G_CALLBACK(slider_free), cfg);
    g_signal_connect(w_sld, "destroy", G_CALLBACK(g_free), cfg);

    return w_sld;
}

/* ----------------------------------------------------------------
 *  IMAGE (GtkPicture)
 * ---------------------------------------------------------------- */
static GtkWidget *build_image(const XmlNode *n, Conteneur *parent_ct)
{
    Image *cfg = g_new0(Image, 1);
    image_initialiser(cfg);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&cfg->id_css, id);

    /* Source */
    const char *src = xml_attr_get(n, "src");
    if (!src)
        src = xml_attr_get(n, "fichier");
    if (!src)
        src = xml_attr_get(n, "file");
    if (src)
    {
        image_set_from_file(cfg, src);
    }
    else
    {
        const char *icon = xml_attr_get(n, "icone");
        if (!icon)
            icon = xml_attr_get(n, "icon");
        if (icon)
            image_set_from_icon_name(cfg, icon);
    }

    /* Légende */
    const char *leg = xml_attr_get(n, "legende");
    if (!leg)
        leg = xml_attr_get(n, "caption");
    if (leg)
        image_set_legende(cfg, leg);

    /* Dimensions */
    int w = attr_int(n, "width", 0);
    int h = attr_int(n, "height", 0);
    if (w || h)
        image_set_size(cfg, w, h);

    /* Mode de redimensionnement */
    const char *fit = xml_attr_get(n, "fit");
    if (fit)
    {
        if (strcmp(fit, "fill") == 0)
            image_set_fit_mode(cfg, IMAGE_FIT_FILL);
        else if (strcmp(fit, "contain") == 0)
            image_set_fit_mode(cfg, IMAGE_FIT_CONTAIN);
        else if (strcmp(fit, "cover") == 0)
            image_set_fit_mode(cfg, IMAGE_FIT_COVER);
        else
            image_set_fit_mode(cfg, IMAGE_FIT_NONE);
    }

    /* Comportement */
    cfg->can_shrink = attr_bool(n, "can_shrink", FALSE);
    cfg->sensitive = attr_bool(n, "actif", TRUE);

    /* Alignement */
    const char *ha = xml_attr_get(n, "align");
    if (!ha)
        ha = xml_attr_get(n, "halign");
    if (ha)
    {
        if (strcmp(ha, "centre") == 0 || strcmp(ha, "center") == 0)
            image_set_halign(cfg, WIDGET_ALIGN_CENTER);
        else if (strcmp(ha, "fin") == 0 || strcmp(ha, "end") == 0)
            image_set_halign(cfg, WIDGET_ALIGN_END);
        else if (strcmp(ha, "debut") == 0 || strcmp(ha, "start") == 0)
            image_set_halign(cfg, WIDGET_ALIGN_START);
        else
            image_set_halign(cfg, WIDGET_ALIGN_FILL);
    }

    /* Style */
    cfg->rayon_arrondi = attr_int(n, "radius", 0);

    const char *lc = xml_attr_get(n, "legende_couleur");
    if (lc)
        cfg->legende_couleur = xstrdup(lc);
    cfg->legende_taille_px = attr_int(n, "legende_taille", 0);

    /* Création : retourne cfg->container */
    GtkWidget *container = image_creer(cfg);

    if (parent_ct)
        conteneur_ajouter(parent_ct, container);

    g_signal_connect_swapped(container, "destroy", G_CALLBACK(image_free), cfg);
    g_signal_connect(container, "destroy", G_CALLBACK(g_free), cfg);

    return cfg->widget; /* GtkPicture */
}

/* ----------------------------------------------------------------
 *  MENU + MENU_ITEM
 * ----------------------------------------------------------------
 * Construit récursivement les MenuItems depuis l'arbre XML.
 */
static MenuItem *build_menu_item_node(const XmlNode *child)
{
    const char *item_id = xml_attr_get(child, "id");
    const char *item_txt = xml_attr_get(child, "texte");
    if (!item_txt)
        item_txt = xml_attr_get(child, "text");
    if (!item_txt)
        item_txt = xml_attr_get(child, "label");
    const char *item_icon = xml_attr_get(child, "icone");
    if (!item_icon)
        item_icon = xml_attr_get(child, "icon");
    const char *item_tt = xml_attr_get(child, "tooltip");

    /* Type d'item */
    MenuItemType itype = MENU_ITEM_NORMAL;
    const char *type = xml_attr_get(child, "type");
    if (type)
    {
        if (strcmp(type, "separateur") == 0 || strcmp(type, "separator") == 0)
            itype = MENU_ITEM_SEPARATEUR;
        else if (strcmp(type, "desactive") == 0 || strcmp(type, "disabled") == 0)
            itype = MENU_ITEM_DESACTIVE;
    }

    MenuItem *item;
    if (itype == MENU_ITEM_SEPARATEUR)
        item = menu_item_separateur();
    else
    {
        item = menu_item_creer(item_id ? item_id : "",
                               item_txt ? item_txt : "",
                               item_icon,
                               itype);
        if (item_tt)
            item->tooltip = xstrdup(item_tt);
    }

    /* Orientation du sous-menu de cet item */
    const char *so = xml_attr_get(child, "sous_menu_orientation");
    if (!so)
        so = xml_attr_get(child, "submenu_orient");
    if (so)
    {
        if (strcmp(so, "horizontal") == 0 || strcmp(so, "h") == 0)
            item->sous_menu_orientation = MENU_HORIZONTAL;
        else
            item->sous_menu_orientation = MENU_VERTICAL;
    }

    /* Sous-items récursifs */
    for (const XmlNode *sub = child->children; sub; sub = sub->next)
    {
        if (sub->type != NODE_MENU_ITEM)
            continue;
        MenuItem *sub_item = build_menu_item_node(sub);
        if (sub_item)
            menu_item_ajouter_sous_item(item, sub_item);
    }

    return item;
}

static GtkWidget *build_menu(const XmlNode *n, Conteneur *parent_ct)
{
    Menu *m = g_new0(Menu, 1);
    menu_initialiser(m);

    /* Orientation barre principale */
    const char *orient = xml_attr_get(n, "orientation");
    if (orient)
    {
        if (strcmp(orient, "horizontal") == 0 || strcmp(orient, "h") == 0)
            m->orientation = MENU_HORIZONTAL;
        else
            m->orientation = MENU_VERTICAL;
    }

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&m->id_css, id);

    /* Espacement */
    const char *esp = xml_attr_get(n, "espacement");
    if (!esp)
        esp = xml_attr_get(n, "spacing");
    if (esp)
        m->espacement = atoi(esp);

    /* Taille de la barre */
    m->size.width = attr_int(n, "width", 0);
    m->size.height = attr_int(n, "height", 0);

    /* Style — barre */
    const char *bb = xml_attr_get(n, "bg_barre");
    if (bb)
        set_str(&m->style.bg_barre, bb);

    const char *bcb = xml_attr_get(n, "border_color");
    if (bcb)
        set_str(&m->style.couleur_bordure, bcb);
    m->style.epaisseur_bordure = attr_int(n, "border_width", 0);
    m->style.rayon_arrondi = attr_int(n, "radius", 0);

    /* Style — items */
    const char *bi = xml_attr_get(n, "bg_item");
    if (bi)
        set_str(&m->style.bg_item, bi);

    const char *fi = xml_attr_get(n, "fg_item");
    if (fi)
        set_str(&m->style.fg_item, fi);

    const char *bih = xml_attr_get(n, "bg_item_hover");
    if (bih)
        set_str(&m->style.bg_item_hover, bih);

    const char *fih = xml_attr_get(n, "fg_item_hover");
    if (fih)
        set_str(&m->style.fg_item_hover, fih);

    const char *bia = xml_attr_get(n, "bg_item_actif");
    if (!bia)
        bia = xml_attr_get(n, "bg_active");
    if (bia)
        set_str(&m->style.bg_item_actif, bia);

    m->style.rayon_item = attr_int(n, "item_radius", 0);
    m->style.taille_texte_px = attr_int(n, "taille_texte", 0);
    m->style.gras = attr_bool(n, "gras", FALSE) || attr_bool(n, "bold", FALSE);

    /* Style — séparateur */
    const char *sc = xml_attr_get(n, "sep_color");
    if (!sc)
        sc = xml_attr_get(n, "couleur_separateur");
    if (sc)
        set_str(&m->style.couleur_separateur, sc);

    /* Style — popover */
    const char *bp = xml_attr_get(n, "bg_popover");
    if (bp)
        set_str(&m->style.bg_popover, bp);

    /* Enfants → menu_items */
    for (const XmlNode *child = n->children; child; child = child->next)
    {
        if (child->type != NODE_MENU_ITEM)
            continue;
        MenuItem *item = build_menu_item_node(child);
        if (item)
            menu_ajouter_item(m, item);
    }

    GtkWidget *w = menu_creer(m);
    g_signal_connect_swapped(w, "destroy", G_CALLBACK(menu_free), m);
    if (parent_ct)
        conteneur_ajouter(parent_ct, w);
    return w;
}

/* ----------------------------------------------------------------
 *  DIALOG
 * ---------------------------------------------------------------- */
static GtkWidget *build_dialog(const XmlNode *n, GtkWindow *parent_win)
{
    Dialog *cfg = g_new0(Dialog, 1);
    dialog_initialiser(cfg);

    cfg->parent = parent_win;

    /* Titre */
    const char *titre = xml_attr_get(n, "titre");
    if (!titre)
        titre = xml_attr_get(n, "title");
    if (titre)
        set_str(&cfg->titre, titre);

    /* Message */
    const char *msg = xml_attr_get(n, "message");
    if (!msg)
        msg = xml_attr_get(n, "texte");
    if (!msg)
        msg = xml_attr_get(n, "text");
    if (msg)
        set_str(&cfg->message, msg);

    /* Type (icône + couleur header) */
    const char *type = xml_attr_get(n, "type");
    if (type)
    {
        if (strcmp(type, "info") == 0)
            cfg->type = DIALOG_INFO;
        else if (strcmp(type, "succes") == 0 || strcmp(type, "success") == 0)
            cfg->type = DIALOG_SUCCES;
        else if (strcmp(type, "avertissement") == 0 || strcmp(type, "warning") == 0)
            cfg->type = DIALOG_AVERTISSEMENT;
        else if (strcmp(type, "erreur") == 0 || strcmp(type, "error") == 0)
            cfg->type = DIALOG_ERREUR;
        else
            cfg->type = DIALOG_PERSONNALISE;
    }

    /* Boutons preset */
    const char *boutons = xml_attr_get(n, "boutons");
    if (!boutons)
        boutons = xml_attr_get(n, "buttons");
    if (boutons)
    {
        if (strcmp(boutons, "ok") == 0)
            cfg->boutons_preset = DIALOG_BOUTONS_OK;
        else if (strcmp(boutons, "ok_annuler") == 0 || strcmp(boutons, "ok_cancel") == 0)
            cfg->boutons_preset = DIALOG_BOUTONS_OK_ANNULER;
        else if (strcmp(boutons, "oui_non") == 0 || strcmp(boutons, "yes_no") == 0)
            cfg->boutons_preset = DIALOG_BOUTONS_OUI_NON;
        else if (strcmp(boutons, "oui_non_annuler") == 0)
            cfg->boutons_preset = DIALOG_BOUTONS_OUI_NON_ANNULER;
    }

    /* Style du dialog */
    const char *bg_h = xml_attr_get(n, "bg_header");
    if (bg_h)
        set_str(&cfg->style.bg_header, bg_h);
    const char *fg_h = xml_attr_get(n, "fg_header");
    if (fg_h)
        set_str(&cfg->style.fg_header, fg_h);
    const char *bg_b = xml_attr_get(n, "bg_corps");
    if (bg_b)
        set_str(&cfg->style.bg_corps, bg_b);
    const char *bg_f = xml_attr_get(n, "bg_footer");
    if (bg_f)
        set_str(&cfg->style.bg_footer, bg_f);
    const char *bg_bp = xml_attr_get(n, "bg_bouton_principal");
    if (bg_bp)
        set_str(&cfg->style.bg_bouton_principal, bg_bp);
    cfg->style.rayon_arrondi = attr_int(n, "radius", 0);
    cfg->style.epaisseur_bordure = attr_int(n, "border_width", 0);
    const char *bd_col = xml_attr_get(n, "border_color");
    if (bd_col)
        set_str(&cfg->style.couleur_bordure, bd_col);
    cfg->style.titre_gras = attr_bool(n, "titre_gras", TRUE);
    cfg->style.taille_titre = attr_int(n, "taille_titre", 0);

    /* ID CSS */
    const char *id = xml_attr_get(n, "id");
    if (id)
        set_str(&cfg->id_css, id);

    /* Comportement */
    cfg->modal = attr_bool(n, "modal", TRUE);
    cfg->fermeture_croix = attr_bool(n, "fermeture_croix", TRUE);

    /* Taille */
    cfg->taille.width = attr_int(n, "width", 0);
    cfg->taille.height = attr_int(n, "height", 0);

    dialog_creer(cfg);
    dialog_afficher(cfg);
    g_signal_connect_swapped(cfg->window, "destroy", G_CALLBACK(dialog_free), cfg);
    return cfg->window;
}

/* ================================================================
 *  DISPATCH PRINCIPAL
 * ================================================================ */

static GtkWidget *build_widget(const XmlNode *node, Conteneur *parent_ct,
                               GtkApplication *app, GtkWindow *parent_win)
{
    if (!node)
        return NULL;

    switch (node->type)
    {
    case NODE_FENETRE:
        return build_fenetre(node, app);

    case NODE_CONTENEUR:
        return build_conteneur(node, parent_ct, app, parent_win);

    case NODE_SEPARATEUR:
        return build_separateur(node, parent_ct);

    case NODE_TEXTE:
        return build_texte(node, parent_ct);

    case NODE_BOUTON:
        return build_bouton(node, parent_ct);

    case NODE_BOUTON_CHECKLIST:
        return build_checklist(node, parent_ct);

    case NODE_BOUTON_RADIO:
        return build_bouton_radio(node, parent_ct);

    case NODE_CHAMP_MOTDEPASSE:
        return build_champ_motdepasse(node, parent_ct);

    case NODE_CHAMP_TEXTE:
        return build_champ_texte(node, parent_ct);

    case NODE_CHAMP_NOMBRE:
        return build_champ_nombre(node, parent_ct);

    case NODE_CHAMP_SELECT:
        return build_champ_select(node, parent_ct);

    case NODE_CHAMP_ZONE_TEXTE:
        return build_champ_zone_texte(node, parent_ct);

    case NODE_SLIDER:
        return build_slider(node, parent_ct);

    case NODE_IMAGE:
        return build_image(node, parent_ct);

    case NODE_MENU:
        return build_menu(node, parent_ct);

    case NODE_DIALOG:
        return build_dialog(node, parent_win);

    case NODE_MENU_ITEM:
        /* Géré directement par build_menu → build_menu_item_node */
        return NULL;

    case NODE_UNKNOWN:
    default:
        fprintf(stderr, "[xml_parser] Balise inconnue ignorée : <%s>\n",
                node->tag ? node->tag : "?");
        return NULL;
    }
}

/* ================================================================
 *  API PUBLIQUE
 * ================================================================ */

GtkWidget *xml_build_ui(XmlNode *root, GtkApplication *app)
{
    if (!root)
        return NULL;

    /* La racine est directement <fenetre> */
    if (root->type == NODE_FENETRE)
        return build_fenetre(root, app);

    /* Sinon cherche le premier <fenetre> parmi les enfants */
    for (XmlNode *child = root->children; child; child = child->next)
        if (child->type == NODE_FENETRE)
            return build_fenetre(child, app);

    fprintf(stderr, "[xml_parser] Aucune balise <fenetre> trouvée dans le XML.\n");
    return NULL;
}

GtkWidget *xml_load_file(const char *path, GtkApplication *app)
{
    XmlNode *root = xml_parser_parse_file(path);
    if (!root)
        return NULL;
    GtkWidget *w = xml_build_ui(root, app);
    xml_node_free(root);
    return w;
}
