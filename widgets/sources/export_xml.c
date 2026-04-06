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

/* ================================================================== */
/* GENERATION                                                           */
/* ================================================================== */
void generer_fichier_interface(ExportContext *ctx)
{
    FILE *f = fopen("interface.txt", "w");
    if (!f)
    {
        printf("Erreur : impossible de creer le fichier interface.txt\n");
        return;
    }

    fprintf(f, "<interface>\n");

    /* ------------------------------------------------------------------ */
    /* FENETRE                                                              */
    /* ------------------------------------------------------------------ */
    if (ctx->fenetre)
    {
        fprintf(f,
                "  <fenetre title=\"%s\" width=\"%d\" height=\"%d\""
                " resizable=\"%s\" maximized=\"%s\" />\n",
                safe_str(ctx->fenetre->title),
                ctx->fenetre->taille.width,
                ctx->fenetre->taille.height,
                ctx->fenetre->resizable          ? "true" : "false",
                ctx->fenetre->demarrer_maximisee ? "true" : "false");
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
                " bg_barre=\"%s\" fg_item=\"%s\" bg_item_hover=\"%s\" />\n",
                safe_str(m->id_css),
                m->orientation == MENU_HORIZONTAL ? "horizontal" : "vertical",
                m->espacement,
                safe_str(m->style.bg_barre),
                safe_str(m->style.fg_item),
                safe_str(m->style.bg_item_hover));
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
            fprintf(f, " color=\"%s\"",  t->couleur_texte);
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

        if (b->taille.largeur > 0) fprintf(f, " width=\"%d\"",   b->taille.largeur);
        if (b->taille.hauteur > 0) fprintf(f, " height=\"%d\"",  b->taille.hauteur);
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

        fprintf(f,
                "  <bouton_radio id=\"%s\" label=\"%s\" actif=\"%s\" sensible=\"%s\"",
                safe_str(br->id_css),
                safe_str(br->label),
                br->est_actif ? "true" : "false",
                br->sensible  ? "true" : "false");

        if (br->tooltip && br->tooltip[0]) fprintf(f, " tooltip=\"%s\"", br->tooltip);
        fprintf(f, " />\n");
    }

    /* ------------------------------------------------------------------ */
    /* CHAMPS TEXTE                                                         */
    /* ------------------------------------------------------------------ */
    for (int i = 0; i < ctx->nb_champs_texte; i++)
    {
        ChampTexte *ct = ctx->champs_texte[i];
        if (!ct) continue;
        fprintf(f,
                "  <champ_texte id=\"%s\" placeholder=\"%s\" required=\"%s\" />\n",
                safe_str(ct->id_css),
                safe_str(ct->placeholder),
                ct->required ? "true" : "false");
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
        if (img->width > 0)  fprintf(f, " width=\"%d\"",   img->width);
        if (img->height > 0) fprintf(f, " height=\"%d\"",  img->height);
        if (img->legende && img->legende[0])
            fprintf(f, " legende=\"%s\"", img->legende);
        fprintf(f, " />\n");
    }

    fprintf(f, "</interface>\n");
    fclose(f);
    printf("Fichier interface.txt genere avec succes.\n");
}