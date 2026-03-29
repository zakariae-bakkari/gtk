#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <gtk/gtk.h>

// ============================================================
//  Types de nœuds supportés
// ============================================================
typedef enum {
    NODE_FENETRE,
    NODE_CONTENEUR,
    NODE_TEXTE,
    NODE_BOUTON,
    NODE_BOUTON_CHECKLIST,
    NODE_CHAMP_MOTDEPASSE,
    NODE_MENU,
    NODE_MENU_ITEM,
    NODE_SEPARATEUR,
    NODE_DIALOG,
    NODE_UNKNOWN
} XmlNodeType;

// ============================================================
//  Attribut générique  (name="value")
// ============================================================
typedef struct XmlAttr {
    char *name;
    char *value;
    struct XmlAttr *next;
} XmlAttr;

// ============================================================
//  Nœud XML (arbre)
// ============================================================
typedef struct XmlNode {
    XmlNodeType  type;
    char        *tag;          // nom de balise brut ("fenetre", "conteneur"…)
    XmlAttr     *attrs;        // liste chaînée d'attributs
    struct XmlNode *children;  // premier enfant
    struct XmlNode *next;      // frère suivant
} XmlNode;

// ============================================================
//  API publique
// ============================================================

/**
 * Parse un fichier XML et retourne l'arbre de nœuds.
 * Retourne NULL en cas d'erreur (message imprimé sur stderr).
 */
XmlNode *xml_parser_parse_file(const char *path);

/**
 * Parse une chaîne XML en mémoire.
 */
XmlNode *xml_parser_parse_string(const char *xml);

/**
 * Libère récursivement l'arbre de nœuds.
 */
void xml_node_free(XmlNode *node);

/**
 * Recherche la valeur d'un attribut dans un nœud.
 * Retourne NULL si l'attribut n'existe pas.
 */
const char *xml_attr_get(const XmlNode *node, const char *name);

/**
 * Construit les widgets GTK depuis l'arbre XML parsé.
 * @param root      : nœud racine (doit être <fenetre>)
 * @param app       : GtkApplication
 * @return          : le GtkWidget de la fenêtre créée, ou NULL
 */
GtkWidget *xml_build_ui(XmlNode *root, GtkApplication *app);

/**
 * Raccourci : parse + build en une seule étape.
 */
GtkWidget *xml_load_file(const char *path, GtkApplication *app);

#endif /* XML_PARSER_H */
