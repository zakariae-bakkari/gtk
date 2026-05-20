#include "../headers/export_xml.h"
#include <stdio.h>
#include <string.h>

static const char *safe_str(const char *s) { return s ? s : ""; }

/* ================================================================== */
/* INIT                                                                 */
/* ================================================================== */
void export_context_init(ExportContext *ctx)
{
    memset(ctx, 0, sizeof(ExportContext));
}

/* ================================================================== */
/* AJOUT DES WIDGETS                                                    */
/* ================================================================== */
void export_ajouter_fenetre  (ExportContext *ctx, Fenetre         *w) { ctx->fenetre = w; }
void export_ajouter_menu     (ExportContext *ctx, Menu            *w) { ctx->menus[ctx->nb_menus++] = w; }
void export_ajouter_texte    (ExportContext *ctx, Texte           *w) { ctx->textes[ctx->nb_textes++] = w; }
void export_ajouter_conteneur(ExportContext *ctx, Conteneur       *w) { ctx->conteneurs[ctx->nb_conteneurs++] = w; }
void export_ajouter_bouton   (ExportContext *ctx, Bouton          *w) { ctx->boutons[ctx->nb_boutons++] = w; }
void export_ajouter_checklist(ExportContext *ctx, BoutonChecklist *w) { ctx->checklists[ctx->nb_checklists++] = w; }
void export_ajouter_radio    (ExportContext *ctx, BoutonRadio     *w) { ctx->radios[ctx->nb_radios++] = w; }
void export_ajouter_champ_texte (ExportContext *ctx, ChampTexte      *w) { ctx->champs_texte[ctx->nb_champs_texte++] = w; }
void export_ajouter_champ_mdp   (ExportContext *ctx, ChampMotDePasse *w) { ctx->champs_mdp[ctx->nb_champs_mdp++] = w; }
void export_ajouter_champ_nombre(ExportContext *ctx, ChampNombre     *w) { ctx->champs_nombre[ctx->nb_champs_nombre++] = w; }
void export_ajouter_champ_select(ExportContext *ctx, ChampSelect     *w) { ctx->champs_select[ctx->nb_champs_select++] = w; }
void export_ajouter_zone_texte  (ExportContext *ctx, ChampZoneTexte  *w) { ctx->zones_texte[ctx->nb_zones_texte++] = w; }
void export_ajouter_slider  (ExportContext *ctx, Slider          *w) { ctx->sliders[ctx->nb_sliders++] = w; }
void export_ajouter_dialog  (ExportContext *ctx, Dialog          *w) { ctx->dialogs[ctx->nb_dialogs++] = w; }
void export_ajouter_image   (ExportContext *ctx, Image           *w) { ctx->images[ctx->nb_images++] = w; }
void export_ajouter_video   (ExportContext *ctx, Video           *w) { ctx->videos[ctx->nb_videos++] = w; }

/* ================================================================== */
/* MENU ITEMS (recursif)                                                */
/* ================================================================== */
static void export_menu_item(FILE *f, MenuItem *item, int depth)
{
    if (!item) return;

    char indent[64];
    int spaces = (depth + 2) * 2;
    if (spaces >= (int)sizeof(indent)) spaces = (int)sizeof(indent) - 1;
    memset(indent, ' ', spaces);
    indent[spaces] = '\0';

    const char *type_str;
    switch (item->type)
    {
        case MENU_ITEM_SEPARATEUR: type_str = "separateur"; break;
        case MENU_ITEM_DESACTIVE:  type_str = "desactive";  break;
        default:                   type_str = "normal";     break;
    }

    const char *sous_orient = item->sous_menu_orientation == MENU_VERTICAL ? "vertical" : "horizontal";

    fprintf(f, "%s<menu_item id=\"%s\" texte=\"%s\"",
            indent,
            safe_str(item->id),
            safe_str(item->texte));

    if (item->nom_icone && item->nom_icone[0])
        fprintf(f, " icone=\"%s\"", item->nom_icone);
    if (item->tooltip && item->tooltip[0])
        fprintf(f, " tooltip=\"%s\"", item->tooltip);
    if (item->type != MENU_ITEM_NORMAL)
        fprintf(f, " type=\"%s\"", type_str);
    if (item->nb_sous_items > 0)
        fprintf(f, " sous_menu_orientation=\"%s\"", sous_orient);

    if (item->nb_sous_items > 0)
    {
        fprintf(f, ">\n");
        for (int i = 0; i < item->nb_sous_items; i++)
            export_menu_item(f, item->sous_items[i], depth + 1);
        fprintf(f, "%s</menu_item>\n", indent);
    }
    else
    {
        fprintf(f, " />\n");
    }
}

static void write_indent(FILE *f, int depth)
{
    for (int i = 0; i < depth; i++) {
        fprintf(f, "  ");
    }
}

static void recursive_xml_export(FILE *f, GtkWidget *widget, int depth, void **exported, int *nb_exported)
{
    if (!widget) return;

    const char *custom_type = (const char *)g_object_get_data(G_OBJECT(widget), "custom_type");
    void *custom_struct = g_object_get_data(G_OBJECT(widget), "custom_struct");

    // Check if this structure has already been exported
    if (custom_struct) {
        for (int i = 0; i < *nb_exported; i++) {
            if (exported[i] == custom_struct) {
                // We've already exported the wrapper or inner part of this custom structure.
                // Just traverse its GtkWidget children recursively without printing another XML tag.
                GtkWidget *child = gtk_widget_get_first_child(widget);
                while (child) {
                    recursive_xml_export(f, child, depth, exported, nb_exported);
                    child = gtk_widget_get_next_sibling(child);
                }
                return;
            }
        }
    }

    if (custom_type && custom_struct) {
        // Enregistrer la structure comme exportée
        exported[(*nb_exported)++] = custom_struct;

        if (strcmp(custom_type, "Conteneur") == 0) {
            Conteneur *c = (Conteneur *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<conteneur orientation=\"%s\" espacement=\"%d\"",
                    c->orientation == CONTENEUR_VERTICAL ? "vertical" : "horizontal",
                    c->espacement);
            if (c->id_css && c->id_css[0])
                fprintf(f, " id=\"%s\"", c->id_css);
            if (c->couleur_fond && c->couleur_fond[0])
                fprintf(f, " bgcolor=\"%s\"", c->couleur_fond);
            if (c->bordure_largeur > 0) {
                fprintf(f, " bordure_largeur=\"%d\"", c->bordure_largeur);
                fprintf(f, " bordure_couleur=\"%s\"", safe_str(c->bordure_couleur));
                fprintf(f, " bordure_rayon=\"%d\"", c->bordure_rayon);
            }
            if (c->padding.haut || c->padding.bas || c->padding.gauche || c->padding.droite) {
                fprintf(f, " padding_top=\"%d\"", c->padding.haut);
                fprintf(f, " padding_bottom=\"%d\"", c->padding.bas);
                fprintf(f, " padding_left=\"%d\"", c->padding.gauche);
                fprintf(f, " padding_right=\"%d\"", c->padding.droite);
            }
            if (c->marges.haut || c->marges.bas || c->marges.gauche || c->marges.droite) {
                fprintf(f, " margin_top=\"%d\"", c->marges.haut);
                fprintf(f, " margin_bottom=\"%d\"", c->marges.bas);
                fprintf(f, " margin_left=\"%d\"", c->marges.gauche);
                fprintf(f, " margin_right=\"%d\"", c->marges.droite);
            }
            if (c->scroll_mode != SCROLL_NONE) {
                const char *scroll_str = "vertical";
                if (c->scroll_mode == SCROLL_HORIZONTAL) scroll_str = "horizontal";
                else if (c->scroll_mode == SCROLL_BOTH) scroll_str = "both";
                fprintf(f, " scroll=\"%s\"", scroll_str);
            }
            if (c->scroll_min_width > 0)
                fprintf(f, " scroll_min_width=\"%d\"", c->scroll_min_width);
            if (c->scroll_min_height > 0)
                fprintf(f, " scroll_min_height=\"%d\"", c->scroll_min_height);
            fprintf(f, " scroll_overlay=\"%s\"", c->scroll_overlay ? "true" : "false");
            if (c->homogene)
                fprintf(f, " homogene=\"true\"");
            if (c->enfants_hexpand)
                fprintf(f, " hexpand=\"true\"");
            if (c->enfants_vexpand)
                fprintf(f, " vexpand=\"true\"");
            if (c->align_x != ALIGNEMENT_REMPLIR) {
                const char *ax = "fill";
                if (c->align_x == ALIGNEMENT_DEBUT) ax = "start";
                else if (c->align_x == ALIGNEMENT_FIN) ax = "end";
                else if (c->align_x == ALIGNEMENT_CENTRE) ax = "center";
                fprintf(f, " align_x=\"%s\"", ax);
            }
            if (c->align_y != ALIGNEMENT_REMPLIR) {
                const char *ay = "fill";
                if (c->align_y == ALIGNEMENT_DEBUT) ay = "start";
                else if (c->align_y == ALIGNEMENT_FIN) ay = "end";
                else if (c->align_y == ALIGNEMENT_CENTRE) ay = "center";
                fprintf(f, " align_y=\"%s\"", ay);
            }
            fprintf(f, ">\n");

            // Parcourir récursivement les enfants
            GtkWidget *child = gtk_widget_get_first_child(widget);
            while (child) {
                recursive_xml_export(f, child, depth + 1, exported, nb_exported);
                child = gtk_widget_get_next_sibling(child);
            }

            write_indent(f, depth);
            fprintf(f, "</conteneur>\n");
            return;
        }
        else if (strcmp(custom_type, "Menu") == 0) {
            Menu *m = (Menu *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<menu id=\"%s\" orientation=\"%s\" espacement=\"%d\" bg_barre=\"%s\" fg_item=\"%s\" bg_item_hover=\"%s\"",
                    safe_str(m->id_css),
                    m->orientation == MENU_HORIZONTAL ? "horizontal" : "vertical",
                    m->espacement,
                    safe_str(m->style.bg_barre),
                    safe_str(m->style.fg_item),
                    safe_str(m->style.bg_item_hover));
            if (m->nb_items > 0) {
                fprintf(f, ">\n");
                for (int j = 0; j < m->nb_items; j++)
                    export_menu_item(f, m->items[j], depth);
                write_indent(f, depth);
                fprintf(f, "</menu>\n");
            } else {
                fprintf(f, " />\n");
            }
            return;
        }
        else if (strcmp(custom_type, "Texte") == 0) {
            Texte *t = (Texte *)custom_struct;
            const char *type_str;
            switch (t->type) {
                case TEXTE_H1:       type_str = "h1";       break;
                case TEXTE_H2:       type_str = "h2";       break;
                case TEXTE_H3:       type_str = "h3";       break;
                case TEXTE_H4:       type_str = "h4";       break;
                case TEXTE_H5:       type_str = "h5";       break;
                case TEXTE_H6:       type_str = "h6";       break;
                case TEXTE_SUBTITLE: type_str = "subtitle"; break;
                case TEXTE_CAPTION:  type_str = "caption";  break;
                default:             type_str = "normal";   break;
            }
            const char *align_str;
            switch (t->alignement) {
                case TEXTE_ALIGN_CENTER:  align_str = "center";  break;
                case TEXTE_ALIGN_RIGHT:   align_str = "right";   break;
                case TEXTE_ALIGN_JUSTIFY: align_str = "justify"; break;
                default:                  align_str = "left";    break;
            }
            write_indent(f, depth);
            fprintf(f, "<texte type=\"%s\" texte=\"%s\" alignement=\"%s\"",
                    type_str, safe_str(t->texte), align_str);
            if (t->couleur_texte && t->couleur_texte[0])
                fprintf(f, " color=\"%s\"", t->couleur_texte);
            if (t->gras)
                fprintf(f, " bold=\"true\"");
            if (t->italique)
                fprintf(f, " italic=\"true\"");
            if (t->id_css && t->id_css[0])
                fprintf(f, " id=\"%s\"", t->id_css);
            fprintf(f, " />\n");
            return;
        }
        else if (strcmp(custom_type, "Bouton") == 0) {
            Bouton *b = (Bouton *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<bouton id=\"%s\" texte=\"%s\" bgcolor=\"%s\" color=\"%s\" bgcolor_hover=\"%s\" radius=\"%d\" actif=\"%s\"",
                    safe_str(b->id_css),
                    safe_str(b->texte),
                    safe_str(b->style.bg_normal),
                    safe_str(b->style.fg_normal),
                    safe_str(b->style.bg_hover),
                    b->style.rayon_arrondi,
                    b->est_actif ? "true" : "false");
            if (b->taille.largeur > 0) fprintf(f, " width=\"%d\"", b->taille.largeur);
            if (b->taille.hauteur > 0) fprintf(f, " height=\"%d\"", b->taille.hauteur);
            if (b->tooltip && b->tooltip[0]) fprintf(f, " tooltip=\"%s\"", b->tooltip);
            fprintf(f, " />\n");
            return;
        }
        else if (strcmp(custom_type, "BoutonChecklist") == 0) {
            BoutonChecklist *bc = (BoutonChecklist *)custom_struct;
            const char *etat_str;
            switch (bc->etat) {
                case CHECKLIST_CHECKED:      etat_str = "checked";      break;
                case CHECKLIST_INCONSISTENT: etat_str = "inconsistent"; break;
                default:                     etat_str = "unchecked";    break;
            }
            write_indent(f, depth);
            fprintf(f, "<bouton_checklist id=\"%s\" label=\"%s\" etat=\"%s\" actif=\"%s\"",
                    safe_str(bc->id_css),
                    safe_str(bc->label),
                    etat_str,
                    bc->est_actif ? "true" : "false");
            if (bc->tooltip && bc->tooltip[0]) fprintf(f, " tooltip=\"%s\"", bc->tooltip);
            fprintf(f, " />\n");
            return;
        }
        else if (strcmp(custom_type, "BoutonRadio") == 0) {
            BoutonRadio *br = (BoutonRadio *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<bouton_radio id=\"%s\" label=\"%s\" checked=\"%s\" sensible=\"%s\"",
                    safe_str(br->id_css),
                    safe_str(br->label),
                    br->est_actif ? "true" : "false",
                    br->sensible  ? "true" : "false");
            if (br->tooltip && br->tooltip[0]) fprintf(f, " tooltip=\"%s\"", br->tooltip);
            fprintf(f, " />\n");
            return;
        }
        else if (strcmp(custom_type, "ChampTexte") == 0) {
            ChampTexte *ct = (ChampTexte *)custom_struct;
            const char *type_str;
            switch (ct->type) {
                case CHAMP_TEXTE_TYPE_EMAIL:  type_str = "email";  break;
                case CHAMP_TEXTE_TYPE_URL:    type_str = "url";    break;
                case CHAMP_TEXTE_TYPE_SEARCH: type_str = "search"; break;
                default:                      type_str = "text";   break;
            }
            write_indent(f, depth);
            fprintf(f, "<champ_texte id=\"%s\" placeholder=\"%s\" type=\"%s\" required=\"%s\"",
                    safe_str(ct->id_css),
                    safe_str(ct->placeholder),
                    type_str,
                    ct->required ? "true" : "false");
            if (ct->max_length > 0)
                fprintf(f, " max_length=\"%d\"", ct->max_length);
            if (ct->policy.min_len > 0)
                fprintf(f, " min_len=\"%d\"", ct->policy.min_len);
            if (ct->policy.max_len > 0)
                fprintf(f, " max_len=\"%d\"", ct->policy.max_len);
            if (ct->policy.only_digits)
                fprintf(f, " only_digits=\"true\"");
            if (ct->policy.no_whitespace)
                fprintf(f, " no_whitespace=\"true\"");
            if (ct->size.width > 0)
                fprintf(f, " width=\"%d\"", ct->size.width);
            if (ct->size.height > 0)
                fprintf(f, " height=\"%d\"", ct->size.height);
            fprintf(f, " />\n");
            return;
        }
        else if (strcmp(custom_type, "ChampMotDePasse") == 0) {
            ChampMotDePasse *cm = (ChampMotDePasse *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<champ_motdepasse id=\"%s\" placeholder=\"%s\" required=\"%s\" />\n",
                    safe_str(cm->id_css),
                    safe_str(cm->placeholder),
                    cm->required ? "true" : "false");
            return;
        }
        else if (strcmp(custom_type, "ChampNombre") == 0) {
            ChampNombre *cn = (ChampNombre *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<champ_nombre id=\"%s\" min=\"%.2f\" max=\"%.2f\" valeur=\"%.2f\" step=\"%.2f\" digits=\"%u\" wrap=\"%s\" required=\"%s\"",
                    safe_str(cn->id_css),
                    cn->min, cn->max, cn->valeur, cn->step, cn->digits,
                    cn->wrap     ? "true" : "false",
                    cn->required ? "true" : "false");
            if (cn->size.width > 0) fprintf(f, " width=\"%d\"", cn->size.width);
            fprintf(f, " />\n");
            return;
        }
        else if (strcmp(custom_type, "ChampSelect") == 0) {
            ChampSelect *cs = (ChampSelect *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<champ_select id=\"%s\" selected_index=\"%d\">\n",
                    safe_str(cs->id_css), cs->selected_index);
            if (cs->model) {
                guint n = g_list_model_get_n_items(G_LIST_MODEL(cs->model));
                for (guint j = 0; j < n; j++) {
                    const char *txt = gtk_string_list_get_string(cs->model, j);
                    write_indent(f, depth + 1);
                    fprintf(f, "<option>%s</option>\n", safe_str(txt));
                }
            }
            write_indent(f, depth);
            fprintf(f, "</champ_select>\n");
            return;
        }
        else if (strcmp(custom_type, "ChampZoneTexte") == 0) {
            ChampZoneTexte *cz = (ChampZoneTexte *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<champ_zone_texte id=\"%s\" max_length=\"%d\" wrap_word=\"%s\" required=\"%s\" />\n",
                    safe_str(cz->id_css),
                    cz->max_length,
                    cz->wrap_word ? "true" : "false",
                    cz->required  ? "true" : "false");
            return;
        }
        else if (strcmp(custom_type, "Slider") == 0) {
            Slider *s = (Slider *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<slider id=\"%s\" min=\"%.2f\" max=\"%.2f\" valeur=\"%.2f\" step=\"%.2f\" afficher_valeur=\"%s\"",
                    safe_str(s->id_css),
                    s->min, s->max, s->valeur, s->step,
                    s->afficher_valeur ? "true" : "false");
            if (s->size.width > 0) fprintf(f, " width=\"%d\"", s->size.width);
            fprintf(f, " />\n");
            return;
        }
        else if (strcmp(custom_type, "Dialog") == 0) {
            Dialog *d = (Dialog *)custom_struct;
            const char *type_str;
            switch (d->type) {
                case DIALOG_INFO:          type_str = "info";          break;
                case DIALOG_SUCCES:        type_str = "succes";        break;
                case DIALOG_AVERTISSEMENT: type_str = "avertissement"; break;
                case DIALOG_ERREUR:        type_str = "erreur";        break;
                default:                   type_str = "personnalise";  break;
            }
            write_indent(f, depth);
            fprintf(f, "<dialog id=\"%s\" titre=\"%s\" message=\"%s\" type=\"%s\" modal=\"%s\" />\n",
                    safe_str(d->id_css),
                    safe_str(d->titre),
                    safe_str(d->message),
                    type_str,
                    d->modal ? "true" : "false");
            return;
        }
        else if (strcmp(custom_type, "Image") == 0) {
            Image *img = (Image *)custom_struct;
            const char *fit_str;
            switch (img->fit_mode) {
                case IMAGE_FIT_FILL:    fit_str = "fill";    break;
                case IMAGE_FIT_CONTAIN: fit_str = "contain"; break;
                case IMAGE_FIT_COVER:   fit_str = "cover";   break;
                default:                fit_str = "none";    break;
            }
            write_indent(f, depth);
            fprintf(f, "<image");
            if (img->source_type == IMAGE_SOURCE_FILE && img->file_path && img->file_path[0])
                fprintf(f, " src=\"%s\"", img->file_path);
            else if (img->source_type == IMAGE_SOURCE_ICON_NAME && img->icon_name && img->icon_name[0])
                fprintf(f, " icone=\"%s\"", img->icon_name);
            fprintf(f, " fit=\"%s\"", fit_str);
            if (img->width > 0)  fprintf(f, " width=\"%d\"",  img->width);
            if (img->height > 0) fprintf(f, " height=\"%d\"", img->height);
            if (img->legende && img->legende[0])
                fprintf(f, " legende=\"%s\"", img->legende);
            fprintf(f, " />\n");
            return;
        }
        else if (strcmp(custom_type, "Video") == 0) {
            Video *v = (Video *)custom_struct;
            write_indent(f, depth);
            fprintf(f, "<video");
            if (v->file_path && v->file_path[0])
                fprintf(f, " src=\"%s\"", v->file_path);
            else if (v->resource_path && v->resource_path[0])
                fprintf(f, " src=\"resource://%s\"", v->resource_path);
            if (v->width > 0)  fprintf(f, " width=\"%d\"",  v->width);
            if (v->height > 0) fprintf(f, " height=\"%d\"", v->height);
            fprintf(f, " autoplay=\"%s\"", v->autoplay  ? "true" : "false");
            fprintf(f, " loop=\"%s\"",     v->loop      ? "true" : "false");
            fprintf(f, " controls=\"%s\"", v->controles ? "true" : "false");
            if (v->legende && v->legende[0])
                fprintf(f, " legende=\"%s\"", v->legende);
            if (v->id_css && v->id_css[0])
                fprintf(f, " id=\"%s\"", v->id_css);
            fprintf(f, " />\n");
            return;
        }
    }

    // Si on a un GtkWidget standard (ou un custom_struct déjà traité), on traverse ses enfants transparentement
    GtkWidget *child = gtk_widget_get_first_child(widget);
    while (child) {
        recursive_xml_export(f, child, depth, exported, nb_exported);
        child = gtk_widget_get_next_sibling(child);
    }
}

/* ================================================================== */
/* GENERATION                                                           */
/* ================================================================== */
void generer_fichier_interface(ExportContext *ctx, const char *chemin)
{
    FILE *f = fopen(chemin ? chemin : "interface.txt", "w");
    if (!f)
    {
        printf("Erreur : impossible de creer le fichier %s\n", chemin ? chemin : "interface.txt");
        return;
    }

    if (ctx && ctx->fenetre && ctx->fenetre->wind)
    {
        Fenetre *win = ctx->fenetre;

        const char *scroll_str = NULL;
        switch (win->scroll_mode)
        {
            case SCROLL_VERTICAL:   scroll_str = "vertical";   break;
            case SCROLL_HORIZONTAL: scroll_str = "horizontal"; break;
            case SCROLL_BOTH:       scroll_str = "both";       break;
            default:                scroll_str = NULL;         break;
        }

        fprintf(f,
                "<fenetre title=\"%s\" width=\"%d\" height=\"%d\""
                " resizable=\"%s\" maximisee=\"%s\"",
                safe_str(win->title),
                win->taille.width,
                win->taille.height,
                win->resizable          ? "true" : "false",
                win->demarrer_maximisee ? "true" : "false");

        if (scroll_str)
            fprintf(f, " scroll=\"%s\"", scroll_str);
        if (win->content_min_width > 0)
            fprintf(f, " scroll_min_width=\"%d\"", win->content_min_width);
        if (win->content_min_height > 0)
            fprintf(f, " scroll_min_height=\"%d\"", win->content_min_height);
        if (win->color_bg && win->color_bg[0])
            fprintf(f, " bgcolor=\"%s\"", win->color_bg);
        if (win->background_image && win->background_image[0])
            fprintf(f, " background_image=\"%s\"", win->background_image);
        if (win->icon_path && win->icon_path[0])
            fprintf(f, " icon_path=\"%s\"", win->icon_path);
        if (win->ico_path && win->ico_path[0])
            fprintf(f, " ico_path=\"%s\"", win->ico_path);

        fprintf(f, ">\n");

        GtkWidget *win_child = gtk_window_get_child(GTK_WINDOW(win->wind));
        if (win_child)
        {
            void *exported_ptrs[1024];
            int nb_exported_ptrs = 0;
            exported_ptrs[nb_exported_ptrs++] = win;

            recursive_xml_export(f, win_child, 1, exported_ptrs, &nb_exported_ptrs);
        }

        fprintf(f, "</fenetre>\n");
        fclose(f);
        printf("Fichier interface genere de maniere arborescente recursive : %s\n", chemin ? chemin : "interface.txt");
        return;
    }

    /* ------------------------------------------------------------------ */
    /* MENUS                                                                */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_menus; i++)
    {
        Menu *m = ctx->menus[i];
        if (!m) continue;
        fprintf(f,
                "  <menu id=\"%s\" orientation=\"%s\" espacement=\"%d\""
                " bg_barre=\"%s\" fg_item=\"%s\" bg_item_hover=\"%s\"",
                safe_str(m->id_css),
                m->orientation == MENU_HORIZONTAL ? "horizontal" : "vertical",
                m->espacement,
                safe_str(m->style.bg_barre),
                safe_str(m->style.fg_item),
                safe_str(m->style.bg_item_hover));

        if (m->nb_items > 0)
        {
            fprintf(f, ">\n");
            for (int j = 0; j < m->nb_items; j++)
                export_menu_item(f, m->items[j], 0);
            fprintf(f, "  </menu>\n");
        }
        else
        {
            fprintf(f, " />\n");
        }
    }

    /* ------------------------------------------------------------------ */
    /* TEXTES                                                               */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_textes; i++)
    {
        Texte *t = ctx->textes[i];
        if (!t) continue;

        const char *type_str;
        switch (t->type)
        {
            case TEXTE_H1:       type_str = "h1";       break;
            case TEXTE_H2:       type_str = "h2";       break;
            case TEXTE_H3:       type_str = "h3";       break;
            case TEXTE_H4:       type_str = "h4";       break;
            case TEXTE_H5:       type_str = "h5";       break;
            case TEXTE_H6:       type_str = "h6";       break;
            case TEXTE_SUBTITLE: type_str = "subtitle"; break;
            case TEXTE_CAPTION:  type_str = "caption";  break;
            default:             type_str = "normal";   break;
        }

        const char *align_str;
        switch (t->alignement)
        {
            case TEXTE_ALIGN_CENTER:  align_str = "center";  break;
            case TEXTE_ALIGN_RIGHT:   align_str = "right";   break;
            case TEXTE_ALIGN_JUSTIFY: align_str = "justify"; break;
            default:                  align_str = "left";    break;
        }

        fprintf(f, "  <texte type=\"%s\" texte=\"%s\" alignement=\"%s\"",
                type_str, safe_str(t->texte), align_str);

        if (t->couleur_texte && t->couleur_texte[0])
            fprintf(f, " color=\"%s\"", t->couleur_texte);
        if (t->gras)
            fprintf(f, " bold=\"true\"");
        if (t->italique)
            fprintf(f, " italic=\"true\"");
        if (t->id_css && t->id_css[0])
            fprintf(f, " id=\"%s\"", t->id_css);

        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* CONTENEURS                                                           */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_conteneurs; i++)
    {
        Conteneur *c = ctx->conteneurs[i];
        if (!c) continue;

        fprintf(f, "  <conteneur id=\"%s\" orientation=\"%s\" espacement=\"%d\"",
                safe_str(c->id_css),
                c->orientation == CONTENEUR_VERTICAL ? "vertical" : "horizontal",
                c->espacement);

        if (c->couleur_fond && c->couleur_fond[0])
            fprintf(f, " bgcolor=\"%s\"", c->couleur_fond);
        if (c->bordure_largeur > 0)
        {
            fprintf(f, " bordure_largeur=\"%d\"", c->bordure_largeur);
            fprintf(f, " bordure_couleur=\"%s\"", safe_str(c->bordure_couleur));
            fprintf(f, " bordure_rayon=\"%d\"",   c->bordure_rayon);
        }
        if (c->padding.haut || c->padding.bas || c->padding.gauche || c->padding.droite)
        {
            fprintf(f, " padding_top=\"%d\"",    c->padding.haut);
            fprintf(f, " padding_bottom=\"%d\"", c->padding.bas);
            fprintf(f, " padding_left=\"%d\"",   c->padding.gauche);
            fprintf(f, " padding_right=\"%d\"",  c->padding.droite);
        }
        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* BOUTONS                                                              */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_boutons; i++)
    {
        Bouton *b = ctx->boutons[i];
        if (!b) continue;

        fprintf(f,
                "  <bouton id=\"%s\" texte=\"%s\" bgcolor=\"%s\""
                " color=\"%s\" bgcolor_hover=\"%s\" radius=\"%d\" actif=\"%s\"",
                safe_str(b->id_css),
                safe_str(b->texte),
                safe_str(b->style.bg_normal),
                safe_str(b->style.fg_normal),
                safe_str(b->style.bg_hover),
                b->style.rayon_arrondi,
                b->est_actif ? "true" : "false");

        if (b->taille.largeur > 0) fprintf(f, " width=\"%d\"",  b->taille.largeur);
        if (b->taille.hauteur > 0) fprintf(f, " height=\"%d\"", b->taille.hauteur);
        if (b->tooltip && b->tooltip[0]) fprintf(f, " tooltip=\"%s\"", b->tooltip);

        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* BOUTONS CHECKLIST                                                    */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_checklists; i++)
    {
        BoutonChecklist *bc = ctx->checklists[i];
        if (!bc) continue;

        const char *etat_str;
        switch (bc->etat)
        {
            case CHECKLIST_CHECKED:      etat_str = "checked";      break;
            case CHECKLIST_INCONSISTENT: etat_str = "inconsistent"; break;
            default:                     etat_str = "unchecked";    break;
        }

        fprintf(f,
                "  <bouton_checklist id=\"%s\" label=\"%s\" etat=\"%s\" actif=\"%s\"",
                safe_str(bc->id_css),
                safe_str(bc->label),
                etat_str,
                bc->est_actif ? "true" : "false");

        if (bc->tooltip && bc->tooltip[0]) fprintf(f, " tooltip=\"%s\"", bc->tooltip);
        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* BOUTONS RADIO                                                        */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_radios; i++)
    {
        BoutonRadio *br = ctx->radios[i];
        if (!br) continue;

        /* checked = selected state, sensible = interactive state */
        fprintf(f,
                "  <bouton_radio id=\"%s\" label=\"%s\" checked=\"%s\" sensible=\"%s\"",
                safe_str(br->id_css),
                safe_str(br->label),
                br->est_actif ? "true" : "false",
                br->sensible  ? "true" : "false");

        if (br->tooltip && br->tooltip[0]) fprintf(f, " tooltip=\"%s\"", br->tooltip);
        /* groupe_nom cannot be exported: the group name is not stored in BoutonRadio */
        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* CHAMPS TEXTE                                                         */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_champs_texte; i++)
    {
        ChampTexte *ct = ctx->champs_texte[i];
        if (!ct) continue;

        const char *type_str;
        switch (ct->type)
        {
            case CHAMP_TEXTE_TYPE_EMAIL:  type_str = "email";  break;
            case CHAMP_TEXTE_TYPE_URL:    type_str = "url";    break;
            case CHAMP_TEXTE_TYPE_SEARCH: type_str = "search"; break;
            default:                      type_str = "text";   break;
        }

        fprintf(f,
                "  <champ_texte id=\"%s\" placeholder=\"%s\" type=\"%s\""
                " required=\"%s\"",
                safe_str(ct->id_css),
                safe_str(ct->placeholder),
                type_str,
                ct->required ? "true" : "false");

        if (ct->max_length > 0)
            fprintf(f, " max_length=\"%d\"", ct->max_length);
        if (ct->policy.min_len > 0)
            fprintf(f, " min_len=\"%d\"", ct->policy.min_len);
        if (ct->policy.max_len > 0)
            fprintf(f, " max_len=\"%d\"", ct->policy.max_len);
        if (ct->policy.only_digits)
            fprintf(f, " only_digits=\"true\"");
        if (ct->policy.no_whitespace)
            fprintf(f, " no_whitespace=\"true\"");
        if (ct->size.width > 0)
            fprintf(f, " width=\"%d\"", ct->size.width);
        if (ct->size.height > 0)
            fprintf(f, " height=\"%d\"", ct->size.height);

        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* CHAMPS MOT DE PASSE                                                  */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_champs_mdp; i++)
    {
        ChampMotDePasse *cm = ctx->champs_mdp[i];
        if (!cm) continue;
        fprintf(f,
                "  <champ_motdepasse id=\"%s\" placeholder=\"%s\" required=\"%s\" />\n",
                safe_str(cm->id_css),
                safe_str(cm->placeholder),
                cm->required ? "true" : "false");
    }

    /* ------------------------------------------------------------------ */
    /* CHAMPS NOMBRE                                                        */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_champs_nombre; i++)
    {
        ChampNombre *cn = ctx->champs_nombre[i];
        if (!cn) continue;
        fprintf(f,
                "  <champ_nombre id=\"%s\" min=\"%.2f\" max=\"%.2f\""
                " valeur=\"%.2f\" step=\"%.2f\" digits=\"%u\""
                " wrap=\"%s\" required=\"%s\"",
                safe_str(cn->id_css),
                cn->min, cn->max, cn->valeur, cn->step, cn->digits,
                cn->wrap     ? "true" : "false",
                cn->required ? "true" : "false");
        if (cn->size.width > 0) fprintf(f, " width=\"%d\"", cn->size.width);
        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* CHAMPS SELECT                                                        */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_champs_select; i++)
    {
        ChampSelect *cs = ctx->champs_select[i];
        if (!cs) continue;

        fprintf(f, "  <champ_select id=\"%s\" selected_index=\"%d\">\n",
                safe_str(cs->id_css), cs->selected_index);

        if (cs->model)
        {
            guint n = g_list_model_get_n_items(G_LIST_MODEL(cs->model));
            for (guint j = 0; j < n; j++)
            {
                const char *txt = gtk_string_list_get_string(cs->model, j);
                fprintf(f, "    <option>%s</option>\n", safe_str(txt));
            }
        }
        fprintf(f, "  </champ_select>\n");
    }

    /* ------------------------------------------------------------------ */
    /* ZONES TEXTE                                                          */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_zones_texte; i++)
    {
        ChampZoneTexte *cz = ctx->zones_texte[i];
        if (!cz) continue;
        fprintf(f,
                "  <champ_zone_texte id=\"%s\" max_length=\"%d\""
                " wrap_word=\"%s\" required=\"%s\" />\n",
                safe_str(cz->id_css),
                cz->max_length,
                cz->wrap_word ? "true" : "false",
                cz->required  ? "true" : "false");
    }

    /* ------------------------------------------------------------------ */
    /* SLIDERS                                                              */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_sliders; i++)
    {
        Slider *s = ctx->sliders[i];
        if (!s) continue;
        fprintf(f,
                "  <slider id=\"%s\" min=\"%.2f\" max=\"%.2f\""
                " valeur=\"%.2f\" step=\"%.2f\" afficher_valeur=\"%s\"",
                safe_str(s->id_css),
                s->min, s->max, s->valeur, s->step,
                s->afficher_valeur ? "true" : "false");
        if (s->size.width > 0) fprintf(f, " width=\"%d\"", s->size.width);
        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* DIALOGS                                                              */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_dialogs; i++)
    {
        Dialog *d = ctx->dialogs[i];
        if (!d) continue;

        const char *type_str;
        switch (d->type)
        {
            case DIALOG_INFO:          type_str = "info";          break;
            case DIALOG_SUCCES:        type_str = "succes";        break;
            case DIALOG_AVERTISSEMENT: type_str = "avertissement"; break;
            case DIALOG_ERREUR:        type_str = "erreur";        break;
            default:                   type_str = "personnalise";  break;
        }

        fprintf(f,
                "  <dialog id=\"%s\" titre=\"%s\" message=\"%s\""
                " type=\"%s\" modal=\"%s\" />\n",
                safe_str(d->id_css),
                safe_str(d->titre),
                safe_str(d->message),
                type_str,
                d->modal ? "true" : "false");
    }

    /* ------------------------------------------------------------------ */
    /* IMAGES                                                               */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_images; i++)
    {
        Image *img = ctx->images[i];
        if (!img) continue;

        const char *fit_str;
        switch (img->fit_mode)
        {
            case IMAGE_FIT_FILL:    fit_str = "fill";    break;
            case IMAGE_FIT_CONTAIN: fit_str = "contain"; break;
            case IMAGE_FIT_COVER:   fit_str = "cover";   break;
            default:                fit_str = "none";    break;
        }

        fprintf(f, "  <image");
        if (img->source_type == IMAGE_SOURCE_FILE && img->file_path && img->file_path[0])
            fprintf(f, " src=\"%s\"", img->file_path);
        else if (img->source_type == IMAGE_SOURCE_ICON_NAME && img->icon_name && img->icon_name[0])
            fprintf(f, " icone=\"%s\"", img->icon_name);
        fprintf(f, " fit=\"%s\"", fit_str);
        if (img->width > 0)  fprintf(f, " width=\"%d\"",  img->width);
        if (img->height > 0) fprintf(f, " height=\"%d\"", img->height);
        if (img->legende && img->legende[0])
            fprintf(f, " legende=\"%s\"", img->legende);
        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* VIDEOS                                                               */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_videos; i++)
    {
        Video *v = ctx->videos[i];
        if (!v) continue;

        fprintf(f, "  <video");
        if (v->file_path && v->file_path[0])
            fprintf(f, " src=\"%s\"", v->file_path);
        else if (v->resource_path && v->resource_path[0])
            fprintf(f, " src=\"resource://%s\"", v->resource_path);
        if (v->width > 0)  fprintf(f, " width=\"%d\"",  v->width);
        if (v->height > 0) fprintf(f, " height=\"%d\"", v->height);
        fprintf(f, " autoplay=\"%s\"", v->autoplay  ? "true" : "false");
        fprintf(f, " loop=\"%s\"",     v->loop      ? "true" : "false");
        fprintf(f, " controls=\"%s\"", v->controles ? "true" : "false");
        if (v->legende && v->legende[0])
            fprintf(f, " legende=\"%s\"", v->legende);
        if (v->id_css && v->id_css[0])
            fprintf(f, " id=\"%s\"", v->id_css);
        fprintf(f, " />\n");
    }

    fprintf(f, "</fenetre>\n");
    fclose(f);
    printf("Fichier interface genere : %s\n", chemin ? chemin : "interface.txt");
}

static void traverser_et_enregistrer(GtkWidget *widget, ExportContext *ctx)
{
    if (!widget) return;

    const char *custom_type = (const char *)g_object_get_data(G_OBJECT(widget), "custom_type");
    void *custom_struct = g_object_get_data(G_OBJECT(widget), "custom_struct");

    if (custom_type && custom_struct)
    {
        if (strcmp(custom_type, "Fenetre") == 0) {
            ctx->fenetre = (Fenetre *)custom_struct;
        } else if (strcmp(custom_type, "Menu") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_menus; i++) {
                if (ctx->menus[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_menus < 32) {
                export_ajouter_menu(ctx, (Menu *)custom_struct);
            }
        } else if (strcmp(custom_type, "Texte") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_textes; i++) {
                if (ctx->textes[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_textes < 64) {
                export_ajouter_texte(ctx, (Texte *)custom_struct);
            }
        } else if (strcmp(custom_type, "Conteneur") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_conteneurs; i++) {
                if (ctx->conteneurs[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_conteneurs < 32) {
                export_ajouter_conteneur(ctx, (Conteneur *)custom_struct);
            }
        } else if (strcmp(custom_type, "Bouton") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_boutons; i++) {
                if (ctx->boutons[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_boutons < 32) {
                export_ajouter_bouton(ctx, (Bouton *)custom_struct);
            }
        } else if (strcmp(custom_type, "BoutonChecklist") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_checklists; i++) {
                if (ctx->checklists[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_checklists < 32) {
                export_ajouter_checklist(ctx, (BoutonChecklist *)custom_struct);
            }
        } else if (strcmp(custom_type, "BoutonRadio") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_radios; i++) {
                if (ctx->radios[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_radios < 32) {
                export_ajouter_radio(ctx, (BoutonRadio *)custom_struct);
            }
        } else if (strcmp(custom_type, "ChampTexte") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_champs_texte; i++) {
                if (ctx->champs_texte[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_champs_texte < 32) {
                export_ajouter_champ_texte(ctx, (ChampTexte *)custom_struct);
            }
        } else if (strcmp(custom_type, "ChampMotDePasse") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_champs_mdp; i++) {
                if (ctx->champs_mdp[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_champs_mdp < 32) {
                export_ajouter_champ_mdp(ctx, (ChampMotDePasse *)custom_struct);
            }
        } else if (strcmp(custom_type, "ChampNombre") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_champs_nombre; i++) {
                if (ctx->champs_nombre[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_champs_nombre < 32) {
                export_ajouter_champ_nombre(ctx, (ChampNombre *)custom_struct);
            }
        } else if (strcmp(custom_type, "ChampSelect") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_champs_select; i++) {
                if (ctx->champs_select[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_champs_select < 32) {
                export_ajouter_champ_select(ctx, (ChampSelect *)custom_struct);
            }
        } else if (strcmp(custom_type, "ChampZoneTexte") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_zones_texte; i++) {
                if (ctx->zones_texte[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_zones_texte < 32) {
                export_ajouter_zone_texte(ctx, (ChampZoneTexte *)custom_struct);
            }
        } else if (strcmp(custom_type, "Slider") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_sliders; i++) {
                if (ctx->sliders[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_sliders < 32) {
                export_ajouter_slider(ctx, (Slider *)custom_struct);
            }
        } else if (strcmp(custom_type, "Dialog") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_dialogs; i++) {
                if (ctx->dialogs[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_dialogs < 32) {
                export_ajouter_dialog(ctx, (Dialog *)custom_struct);
            }
        } else if (strcmp(custom_type, "Image") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_images; i++) {
                if (ctx->images[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_images < 32) {
                export_ajouter_image(ctx, (Image *)custom_struct);
            }
        } else if (strcmp(custom_type, "Video") == 0) {
            int deja_la = 0;
            for (int i = 0; i < ctx->nb_videos; i++) {
                if (ctx->videos[i] == custom_struct) { deja_la = 1; break; }
            }
            if (!deja_la && ctx->nb_videos < 32) {
                export_ajouter_video(ctx, (Video *)custom_struct);
            }
        }
    }

    // Parcourir récursivement les enfants
    GtkWidget *child = gtk_widget_get_first_child(widget);
    while (child)
    {
        traverser_et_enregistrer(child, ctx);
        child = gtk_widget_get_next_sibling(child);
    }
}

void xml_export_window(GtkWidget *window, const char *chemin_export)
{
    ExportContext ctx;
    export_context_init(&ctx);

    // Parcourir récursivement pour enregistrer les widgets dans le contexte
    traverser_et_enregistrer(window, &ctx);

    // Si on a trouvé la fenêtre principale mais qu'elle n'est pas encore enregistrée sous Fenetre
    if (!ctx.fenetre) {
        Fenetre *win_struct = (Fenetre *)g_object_get_data(G_OBJECT(window), "custom_struct");
        if (win_struct) {
            ctx.fenetre = win_struct;
        }
    }

    // Générer le fichier XML
    generer_fichier_interface(&ctx, chemin_export);
}