#include "../headers/export_xml.h"
#include <stdio.h>

static const char *safe_str(const char *s)
{
    return s ? s : "";
}

void generer_fichier_interface(Fenetre *fenetre,
                               ChampTexte *champ_nom,
                               ChampMotDePasse *champ_mdp,
                               ChampSelect *champ_select,
                               ChampZoneTexte *zone_texte,
                               Slider *slider)
{
    FILE *f = fopen("interface.txt", "w");
    if (!f)
    {
        printf("Erreur : impossible de creer le fichier interface.txt\n");
        return;
    }

    fprintf(f, "<interface>\n");

    if (fenetre)
    {
        fprintf(f,
                "  <fenetre title=\"%s\" width=\"%d\" height=\"%d\" resizable=\"%s\" maximized=\"%s\" />\n",
                safe_str(fenetre->title),
                fenetre->taille.width,
                fenetre->taille.height,
                fenetre->resizable ? "true" : "false",
                fenetre->demarrer_maximisee ? "true" : "false");
    }

    if (champ_nom)
    {
        fprintf(f,
                "  <champ_texte id=\"%s\" placeholder=\"%s\" required=\"%s\" />\n",
                safe_str(champ_nom->id_css),
                safe_str(champ_nom->placeholder),
                champ_nom->required ? "true" : "false");
    }

    if (champ_mdp)
    {
        fprintf(f,
                "  <champ_motdepasse id=\"%s\" placeholder=\"%s\" required=\"%s\" />\n",
                safe_str(champ_mdp->id_css),
                safe_str(champ_mdp->placeholder),
                champ_mdp->required ? "true" : "false");
    }

    if (champ_select)
    {
        fprintf(f,
                "  <champ_select id=\"%s\" selected_index=\"%d\">\n",
                safe_str(champ_select->id_css),
                champ_select->selected_index);

        if (champ_select->model)
        {
            guint n = g_list_model_get_n_items(G_LIST_MODEL(champ_select->model));
            for (guint i = 0; i < n; i++)
            {
                const char *txt = gtk_string_list_get_string(champ_select->model, i);
                fprintf(f, "    <option>%s</option>\n", safe_str(txt));
            }
        }

        fprintf(f, "  </champ_select>\n");
    }

    if (zone_texte)
    {
        fprintf(f,
                "  <champ_zone_texte id=\"%s\" max_length=\"%d\" wrap_word=\"%s\" required=\"%s\" />\n",
                safe_str(zone_texte->id_css),
                zone_texte->max_length,
                zone_texte->wrap_word ? "true" : "false",
                zone_texte->required ? "true" : "false");
    }

    if (slider)
    {
        fprintf(f,
                "  <slider id=\"%s\" min=\"%.2f\" max=\"%.2f\" valeur=\"%.2f\" step=\"%.2f\" />\n",
                safe_str(slider->id_css),
                slider->min,
                slider->max,
                slider->valeur,
                slider->step);
    }

    fprintf(f, "</interface>\n");
    fclose(f);

    printf("Fichier interface.txt genere avec succes.\n");
}