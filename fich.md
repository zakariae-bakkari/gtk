# 🐠 Banc de Poisson — Description Détaillée du Projet GTK

## Vue d'ensemble

Un projet GTK interactif simulant un bassin océanique, avec trois modes de jeu distincts. Voici la description complète du design visuel, des ambiances, et des éléments multimédias pour chaque page.

---

## 🏠 Page 1 — Écran d'Accueil (Accueil)

### Background & Design Visuel
- **Fond animé** : Un océan profond en dégradé — bleu marine `#0A1628` en haut vers bleu turquoise `#0D4F6C` vers vert-bleu `#0A7A6E` en bas
- **Effets visuels** :
  - Bulles d'air qui montent lentement (cercles semi-transparents animés avec `cairo`)
  - Rayons de lumière qui traversent l'eau (effet "god rays" en diagonale, opacity animée)
  - Algues qui ondulent en bas de l'écran (dessin cairo avec courbes bézier)
  - Banc de petits poissons qui nage en arrière-plan (animation cairo automatique)
- **Logo / Titre** : "BANC DE POISSON" en grand, police style aquatique, avec effet de reflet d'eau (ombre portée bleutée), centré en haut
- **Décor de fond** : Rochers et coraux dessinés avec cairo en bas d'écran

### Les 3 Boutons Principaux
Chaque bouton est un **widget personnalisé** (struct GTK custom) avec :

| Bouton | Icône cairo | Couleur | Effet hover |
|--------|-------------|---------|-------------|
| 🎨 Mode Créateur | Pinceau + poisson | Vert émeraude `#2ECC71` | Glow vert + poisson qui frétille |
| 🦈 Mode Prédateur | Aileron de requin | Rouge corail `#E74C3C` | Glow rouge + ondulation agressive |
| 🐠 Mode Survie | Petit poisson fuyant | Orange `#F39C12` | Glow orange + animation de fuite |

- Boutons en forme de **bulles arrondies** avec bordure lumineuse
- Au survol (hover) : légère animation de "pulse" + son de bulle
- Chaque bouton a une petite **animation cairo** de son icône intégrée

### Sons
- **Musique de fond** : Ambiance océanique douce (sons de vagues, eau, poissons)
- **Son d'entrée** : Plouf à l'ouverture de la fenêtre
- Implémentation via **GStreamer** ou lecture de fichiers `.wav/.ogg` avec `g_spawn`

---

## 🎨 Page 2 — Mode Créateur

### Concept
Un **éditeur de bassin** : l'utilisateur place et configure des poissons dans le bassin, comme un niveau de jeu qu'il construit lui-même.

### Background & Design
- **Fond** : Même océan mais plus **clair et lumineux** (`#1A6B8A` à `#2ABFBF`), version "lagon peu profond"
- **Grille de placement** invisible mais active (snapping des poissons sur une grille logique)
- **Fond de sable** visible en bas avec texture pointillée (cairo)
- **Coraux colorés** décoratifs dessinés en cairo

### Interface & Widgets (structs GTK)

**Barre d'outils latérale gauche** (widget Panel personnalisé) :
- 🐟 **Bouton "Ajouter Poisson"** → clic sur le bassin pour placer
- 🪨 **Bouton "Ajouter Rocher"** → obstacle dans le bassin
- 🌿 **Bouton "Ajouter Algue"** → décoration + cachette
- 🦈 **Bouton "Ajouter Requin"** → danger pour les poissons
- 🗑️ **Bouton "Supprimer"** → clic sur un élément pour l'effacer
- 🔄 **Bouton "Reset"** → vide tout le bassin

**Panneau de propriétés droit** (widget PropertyPanel) :
- **Boutons Radio** pour le type de poisson (petit / moyen / grand)
- **Boutons Radio** pour la couleur (rouge, bleu, jaune, vert, orange)
- **Slider** pour la vitesse de nage
- **Checkbox** "Poisson peureux" (fuit les requins)
- **Checkbox** "Poisson agressif" (attaque les petits)

**Barre du bas** :
- 💾 **Bouton "Sauvegarder"** → écrit dans un fichier texte/JSON le contenu du bassin
- 📂 **Bouton "Charger"** → lit un fichier existant et reconstruit le bassin
- ▶️ **Bouton "Simuler"** → lance une animation des poissons placés
- 📊 **Label compteur** : "Poissons : 5 | Requins : 1 | Obstacles : 3"

### Fichier de Bassin
Format texte structuré, ex. `bassin.txt` :
```
FISH 120 340 SMALL RED SPEED:2 FEARFUL:true
FISH 300 200 LARGE BLUE SPEED:1 AGGRESSIVE:true
SHARK 500 150 SPEED:3
ROCK 200 400
ALGAE 350 380
```

### Sons
- Clic de placement : son de "plouf" léger
- Sauvegarde : son de "bulle" satisfaisant
- Suppression : son de "glou glou"

---

## 🦈 Page 3 — Mode Prédateur

### Concept
Jeu d'action : le joueur **contrôle un requin** et doit **manger des poissons** pour scorer avant la fin du timer.

### Background & Design
- **Fond** : Océan **sombre et menaçant** — `#050D1A` à `#0A2240`, ambiance abysses
- **Effet de particules** : Petites particules lumineuses qui dérivent (plancton bioluminescent)
- **Vignette sombre** sur les bords (cairo, dégradé radial noir semi-transparent)
- **Effet de sang** temporaire quand un poisson est mangé (tache rouge cairo qui se dissipe)
- Ombres portées profondes sous chaque entité

### Interface & Widgets

**HUD (Heads-Up Display)** — widget HUD personnalisé en overlay :
- 🏆 **Score** : grand compteur en haut à gauche, style digital, couleur or `#F1C40F`
- ⏱️ **Timer** : compte à rebours en haut à droite (ex. 60 secondes), rouge quand < 10s
- ❤️ **Vies** : icônes de requins en haut au centre (si le requin sort du bassin = perd une vie)
- 🎯 **Multiplicateur** : "x2 COMBO" si mange plusieurs poissons vite

**Contrôles** : clavier (ZQSD ou flèches) ou souris (le requin suit le curseur)

**Poissons auto-générés** :
- Apparaissent depuis les bords de l'écran à intervalles aléatoires
- Différentes tailles = différents points (petit = 10pts, moyen = 25pts, grand = 50pts)
- Comportement de fuite : s'éloignent du requin (algorithme simple)

**Écran de fin** (widget GameOver) :
- Fond semi-transparent bleu sombre
- Score final en grand
- Boutons : "Rejouer" / "Menu Principal" / "Meilleur Score"

### Sons
- Musique : rythme tendu, percussions sous-marines
- Manger un poisson : son de "crunch" + splash
- Timer < 10s : battements de cœur qui s'accélèrent
- Game Over : son dramatique de descente

---

## 🐠 Page 4 — Mode Survie

### Concept
Jeu d'esquive : le joueur **contrôle un petit poisson** et doit **survivre** le plus longtemps possible face aux requins.

### Background & Design
- **Fond** : Océan de **récif coloré** — `#0D3B6E` à `#1A7A5E`, coraux lumineux
- **Décor riche** : Anémones, étoiles de mer, bulles, algues ondulantes (tous en cairo)
- **Effet de profondeur** : couches de parallaxe (éléments proches bougent plus vite que le fond)
- **Lumière dynamique** : halo de lumière autour du poisson joueur
- Quand un requin est proche → légère teinte rouge sur les bords (danger imminent)

### Interface & Widgets

**HUD Survie** :
- ⏱️ **Timer de survie** : compte en avant (combien de temps tu as survécu), en haut au centre
- ❤️ **Vie unique** : une seule vie, une barre de "peur" qui monte quand un requin est proche
- 🌊 **Vague** : indicateur de la vague actuelle ("Vague 3 — 4 requins")

**Mécaniques** :
- Requins qui apparaissent progressivement (1 requin → 2 → 3...)
- Powerups qui apparaissent aléatoirement :
  - 💨 **Boost de vitesse** (éclaire temporairement le poisson)
  - 🛡️ **Invincibilité courte** (poisson brille en blanc)
  - 🌀 **Ralentir les requins** (requins deviennent bleus/lents)
- Obstacles (rochers) pour se cacher

**Écran de fin** :
- "Tu as survécu X secondes !"
- Classement local (top 5 scores sauvegardés dans fichier)

### Sons
- Musique : mélodique mais avec tension croissante
- Requin proche : son grave de menace
- Powerup ramassé : son joyeux de bulle
- Mort : son triste de "glou glou" descendant

---

## 🔧 Architecture Technique Commune

### Widgets Créés par Structures (obligatoire)
```c
typedef struct _FishWidget      FishWidget;       // Poisson animé
typedef struct _SharkWidget     SharkWidget;      // Requin animé  
typedef struct _BassinWidget    BassinWidget;     // Le bassin principal
typedef struct _HUDWidget       HUDWidget;        // Overlay score/timer
typedef struct _ButtonOcean     ButtonOcean;      // Bouton custom stylisé
typedef struct _RadioOcean      RadioOcean;       // Bouton radio custom
typedef struct _PanelWidget     PanelWidget;      // Panneau latéral
typedef struct _GameOverWidget  GameOverWidget;   // Écran de fin
```

### Dessin Cairo — Éléments Réutilisables
- `draw_fish(cr, x, y, size, color, angle)` — dessine un poisson orienté
- `draw_shark(cr, x, y, size, direction)` — dessine un requin
- `draw_bubble(cr, x, y, radius, alpha)` — bulle transparente
- `draw_algae(cr, x, y, height, sway)` — algue qui ondule
- `draw_coral(cr, x, y, type, color)` — corail décoratif
- `draw_water_rays(cr, width, height, time)` — rayons lumineux

### Animations
- `g_timeout_add(16, animate_callback, data)` — ~60 FPS
- Chaque entité a une position, vitesse, et phase d'animation
- Interpolation fluide avec fonctions easing

### Fichiers Ressources suggérés
```
/assets/sounds/
  ├── ambiance_ocean.ogg
  ├── bubble.wav
  ├── splash.wav
  ├── crunch.wav
  ├── gameover.wav
  └── powerup.wav
/assets/
  └── (tout dessiné en cairo, pas d'images externes nécessaires)
```

---

Ce projet GTK sera visuellement immersif grâce à **Cairo pour tout le rendu graphique**, des **animations fluides à 60fps**, et une **architecture propre basée sur des structs GTK personnalisés**. Chaque mode offre une expérience distincte tout en partageant le même univers océanique cohérent.






# 🐠 Types d'Entités — Banc de Poisson

## Entités Confirmées + Suggestions Complètes

---

## 🐟 Catégorie 1 — POISSONS (Proies / Neutres)

| # | Entité | Nom FR | Taille | Points (Prédateur) | Comportement | Spécialité |
|---|--------|--------|--------|-------------------|--------------|------------|
| 1 | 🐟 | **Poisson Normal** | Moyen | 10 pts | Nage en banc, fuit les requins | Basique, référence |
| 2 | 🐠 | **Poisson Clown** | Petit | 15 pts | Se cache dans les anémones | Immunisé si près d'une anémone |
| 3 | 🐡 | **Poisson Ballon** | Petit→Grand | 5 pts | Se gonfle quand menacé | Ingangeable gonflé, ralentit |
| 4 | 💙 | **Poisson Bleu** | Moyen | 20 pts | Nage très vite, imprévisible | Boost de vitesse soudain |
| 5 | 🟡 | **Poisson Jaune** | Petit | 8 pts | Nage en zigzag | Très difficile à attraper |
| 6 | 🔴 | **Poisson Rouge** | Petit | 12 pts | Suit les autres poissons | Bonus si mangé en groupe |
| 7 | ✨ | **Poisson Lanterne** | Petit | 25 pts | Actif la nuit / fond sombre | Illumine autour de lui |
| 8 | 🦋 | **Poisson Mandarin** | Petit | 30 pts | Rare, apparaît peu | Bonus score x3 si attrapé |
| 9 | 👁️ | **Poisson Fantôme** | Moyen | 40 pts | Semi-transparent, invisible par moments | Disparaît et réapparaît |
| 10 | 🎈 | **Poisson Bulle** | Petit | 10 pts | Flotte vers le haut lentement | Laisse des bulles derrière lui |

---

## 🐢 Catégorie 2 — TORTUES & LENTS

| # | Entité | Nom FR | Taille | Points | Comportement | Spécialité |
|---|--------|--------|--------|--------|--------------|------------|
| 11 | 🐢 | **Tortue de Mer** | Grand | 50 pts | Très lente, imperturbable | Dure 3 morsures avant de mourir |
| 12 | 🟤 | **Tortue Géante** | Très Grand | 80 pts | Quasi immobile | Bloque le passage comme obstacle |
| 13 | 🌊 | **Tortue Luth** | Grand | 60 pts | Migration lente en ligne droite | Traverse tout l'écran sans dévier |

---

## 🦈 Catégorie 3 — REQUINS & PRÉDATEURS (Ennemis en Mode Survie)

| # | Entité | Nom FR | Taille | Dangerosité | Comportement | Spécialité |
|---|--------|--------|--------|-------------|--------------|------------|
| 14 | 🦈 | **Requin Blanc** | Grand | ⭐⭐⭐⭐ | Chasse directement le joueur | Rapide, impitoyable |
| 15 | 🔵 | **Requin Baleine** | Énorme | ⭐⭐ | Avale tout sur son passage | Aspire les poissons dans sa gueule |
| 16 | 🌀 | **Requin Marteau** | Grand | ⭐⭐⭐ | Tourne en cercles | Attaque latéralement |
| 17 | ⚡ | **Requin Mako** | Moyen | ⭐⭐⭐⭐⭐ | Le plus rapide de tous | Sprint soudain, imprévisible |
| 18 | 🐋 | **Orque** | Très Grand | ⭐⭐⭐⭐ | Chasse en groupe (2-3) | Stratégie de groupe, encerclement |
| 19 | 🟠 | **Requin Tigre** | Grand | ⭐⭐⭐⭐ | Mange tout (poissons + tortues) | Ne discrimine pas les proies |
| 20 | 👻 | **Requin Fantôme** | Moyen | ⭐⭐⭐ | Invisible jusqu'à 2 tiles | Apparaît soudainement |

---

## 🌊 Catégorie 4 — CRÉATURES SPÉCIALES (Suggestions Originales)

| # | Entité | Nom FR | Taille | Rôle | Comportement | Spécialité |
|---|--------|--------|--------|------|--------------|------------|
| 21 | 🦑 | **Calmar** | Moyen | Neutre/Fuite | Propulsion par jet d'encre | Laisse un nuage noir (obstacle temporaire) |
| 22 | 🐙 | **Pieuvre** | Moyen | Neutre | Se fixe sur les rochers | Change de couleur (camouflage) |
| 23 | 🎐 | **Méduse** | Petit | Danger passif | Dérive avec le courant | Paralyse le joueur si touché |
| 24 | ⭐ | **Étoile de Mer** | Petit | Décor/Bonus | Immobile sur le sable | Collectible → +vie ou +powerup |
| 25 | 🦀 | **Crabe** | Petit | Obstacle | Se déplace latéralement | Pince les entités qui passent dessus |
| 26 | 🦞 | **Homard** | Petit | Obstacle | Garde son territoire | Attaque si on s'approche trop |
| 27 | 🐬 | **Dauphin** | Grand | Allié | Aide le joueur (Mode Survie) | Repousse les requins proches |
| 28 | 🐳 | **Baleine** | Énorme | Neutre | Migration lente | Crée des vagues qui dévient les autres |
| 29 | 🦐 | **Crevette** | Minuscule | Décor/Bonus | Swarm de 10-20 | En groupe = gros bonus score |
| 30 | 🐡 | **Poisson Pierre** | Petit | Piège | Immobile, ressemble à un rocher | Empoisonne si touché |
| 31 | ⚡ | **Anguille Électrique** | Long | Danger passif | Nage en serpentin | Zone électrique autour d'elle |
| 32 | 🌙 | **Raie Manta** | Très Grand | Neutre/Bonus | Vol plané gracieux | Monter dessus = transport rapide |
| 33 | 🔱 | **Narval** | Grand | Rare/Allié | Apparaît rarement | Corne = détruit les requins proches |
| 34 | 💎 | **Poisson Cristal** | Petit | Rare/Bonus | Transparent, scintillant | +100 pts, très rare |

---

## 🏗️ Catégorie 5 — OBJETS & ENVIRONNEMENT (Mode Créateur)

| # | Objet | Description | Effet en jeu |
|---|-------|-------------|--------------|
| 35 | 🪨 | **Rocher** | Obstacle statique | Bloque le passage |
| 36 | 🌿 | **Algue** | Décor ondulant | Cachette pour petits poissons |
| 37 | 🪸 | **Corail** | Décor coloré | Zone de refuge (requins évitent) |
| 38 | 🌀 | **Courant Marin** | Flux directionnel | Pousse les entités dans une direction |
| 39 | 🌡️ | **Zone Chaude** | Bulle de chaleur | Accélère les poissons dedans |
| 40 | ❄️ | **Zone Froide** | Courant froid | Ralentit tout le monde |
| 41 | 💡 | **Lumière Abyssale** | Lampe sous-marine | Attire les poissons la nuit |
| 42 | 🕳️ | **Grotte** | Caverne dans le rocher | Cachette totale, requins ne rentrent pas |
| 43 | ⚓ | **Ancre** | Objet coulé | Obstacle + décor |
| 44 | 🚢 | **Épave** | Bateau coulé | Zone complexe, plusieurs cachettes |

---

## 📊 Tableau de Stats pour le Code

```c
typedef enum {
    FISH_NORMAL, FISH_CLOWN, FISH_BALLOON, FISH_BLUE,
    FISH_LANTERN, FISH_GHOST, FISH_MANDARIN,
    TURTLE_SEA, TURTLE_GIANT, TURTLE_LEATHERBACK,
    SHARK_WHITE, SHARK_WHALE, SHARK_HAMMER,
    SHARK_MAKO, ORCA, SHARK_TIGER,
    SQUID, OCTOPUS, JELLYFISH, STARFISH,
    CRAB, LOBSTER, DOLPHIN, WHALE,
    SHRIMP, STONEFISH, EEL, MANTA_RAY,
    NARWHAL, CRYSTAL_FISH
} EntityType;

typedef struct {
    EntityType  type;
    char*       name_fr;
    float       size;          // 1.0 = normal
    float       speed;         // tiles/sec
    int         health;        // nb de hits
    int         score_value;   // points si mangé
    int         danger_level;  // 0=neutre, 5=très dangereux
    gboolean    is_predator;
    gboolean    is_collectible;
    gboolean    has_special;   // a une capacité spéciale
} EntityConfig;
```

---

## 🎮 Suggestions de Gameplay par Entité

### Combos & Interactions Spéciales
- 🐬 **Dauphin + Requin** → Le dauphin fonce sur le requin et le repousse
- 🦑 **Calmar menacé** → Jet d'encre = zone noire temporaire sur l'écran
- 🐡 **Poisson Ballon + Requin** → Le requin le recrache (trop gros)
- 🌀 **Courant + Méduse** → La méduse dérive et devient un mur de danger
- 🐢 **Tortue + Grotte** → La tortue bloque l'entrée = protection totale
- ⭐ **3 Étoiles de Mer** → Combinaison = vie bonus
- 🔱 **Narval rare** → Apparition = musique spéciale + effets lumineux

### Niveaux de Rareté (Mode Créateur)
```
⚪ COMMUN    → Poisson Normal, Rocher, Algue
🟢 PEU RARE  → Poisson Clown, Tortue, Requin Blanc
🔵 RARE      → Orque, Dauphin, Raie Manta, Calmar
🟣 ÉPIQUE    → Narval, Poisson Fantôme, Anguille Électrique  
🟡 LÉGENDAIRE→ Poisson Cristal, Requin Fantôme, Baleine
```

---

Avec ces **44 entités** réparties en 5 catégories, le projet couvre tous les modes de jeu et offre une grande richesse d'interactions. Chaque entité peut être représentée en **Cairo** avec ses propres fonctions de dessin et son comportement défini dans une struct `EntityConfig`.


### pour les poisson en peux cree un guideurs du groupe





banc_de_poisson/
├── Makefile                        ← Build complet GTK4
├── src/
│   ├── main.c                      ← Entrée app, GtkStack, 4 écrans
│   ├── entities.c                  ← Table des 44 entités
│   ├── draw.c                      ← Toutes les fonctions Cairo
│   ├── bassin.c                    ← Widget bassin animé
│   ├── game.c                      ← Logique Prédateur + Survie
│   ├── screen_accueil.c            ← Écran menu principal
│   ├── screen_createur.c           ← Éditeur complet
│   ├── screen_jeux.c               ← Prédateur + Survie HUD
│   └── widgets/
│       ├── fenetre.c / conteneur.c / bouton.c
│       ├── dialog.c / menu.c / image.c
│       ├── slider.c / texte.c / common.c
│       ├── champ_texte.c / champ_nombre.c
│       ├── champ_select.c / champ_motdepasse.c
│       ├── champ_zone_texte.c / bouton_radio.c
│       ├── bouton_checklist.c
│       ├── export_xml.c / xml_parser.c
├── data/
│   ├── bassin.txt                  ← Fichier texte du bassin
│   └── bassin.xml                  ← Format XML du bassin
Tâches suivantes :

Task 2 → Finir la compilation (corriger les erreurs)
Task 3 → Tester le Mode Créateur (placement d'entités, save/load)
Task 4 → Logique IA des poissons (banc, fuite, chasse)
Task 5 → Mode Prédateur complet avec scoring
Task 6 → Mode Survie avec vagues et powerups