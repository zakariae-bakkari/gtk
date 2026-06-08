# Structure du dossier `src`

Ce document décrit l'organisation et le rôle de chaque fichier présent dans le dossier `src` de l'application. L'architecture suit une approche modulaire séparant l'interface utilisateur, la logique de simulation, les actions, et les modèles de données.

## `src/` (Racine)
* **`main.c`** : Point d'entrée principal de l'application. Initialise GTK4, configure la fenêtre principale et gère la navigation entre les différentes vues (Accueil, Bassin, etc.).
* **`test_conteneur.c`** : Programme de test unitaire indépendant pour vérifier le comportement du widget `Conteneur`.
* **`xml_runner.c`** : Outil utilitaire permettant de charger et d'afficher directement une interface générée depuis un fichier XML.

---

## `src/actions/`
Ce dossier contient la logique des événements, des interactions utilisateur et des menus.
* **`bassin_interactions.c` & `.h`** : Gère les interactions directes avec le bassin (clics sur les poissons, glisser-déposer, raccourcis clavier, contrôle manuel d'un poisson).
* **`bassin_menu.c` & `.h`** : Implémente la barre de menu supérieure du bassin (boutons Play/Pause, Ajouter, Vider, Paramètres, Aide).
* **`bassin_sidebar.c` & `.h`** : Gère la barre latérale droite du bassin, l'affichage des listes d'entités (poissons) et des bancs, ainsi que les statistiques en temps réel.

---

## `src/core/`
Composants système globaux et transversaux.
* **`sound.c` & `.h`** : Moteur audio de l'application. Fournit une API simple (`sound_play`) pour jouer des effets sonores (bulles, morsures, alertes, etc.) via l'API Windows Multimedia (`winmm`).

---

## `src/dialogs/`
Vues modales et formulaires interactifs.
* **`bassin_dialogs.c` & `.h`** : Regroupe tous les formulaires modaux liés à la simulation :
  * Création / Modification d'une espèce de poisson.
  * Ajout manuel de poissons ou de bancs.
  * Génération aléatoire d'entités.
  * Panneau de configuration du bassin (taille, fond, options visuelles).
  * Fiches détails / Inspecteur d'un poisson ou d'une espèce.

---

## `src/modele/`
Modèles de données fondamentaux.
* **`poisson.c` & `.h`** : Définit la structure `Poisson`, les attributs physiques (santé, vitesse, taille) et les fonctions de base (création, positionnement, frames d'animation).

---

## `src/pages/`
Vues principales (écrans) de l'application.
* **`screen_bassin.c` & `.h`** : Construit l'interface principale de la simulation (Canvas central, layout, overlay de debug). Ce fichier se limite désormais à la construction de la vue (SRP).
* **`screen_bassin_helpers.c` & `.h`** : Fonctions utilitaires liées à la vue du bassin (création des widgets visuels des poissons, effets visuels de dégâts/soin, spawn de nourriture, gestion du mode Zen).
* **`screen_home.c` & `.h`** : Construit l'écran d'accueil de l'application (vidéo de fond, boutons de navigation vers les autres modes). Remplace l'ancien `screen_stubs.c`.

---

## `src/simulation/`
Moteur de simulation et logique métier de l'aquarium.
* **`bassin_simulation.c`** : Contient le cœur de la boucle de simulation (`update_simulation`). Calcule la physique, les comportements de fuite, d'attaque, l'algorithme de flocking (boids) pour les bancs, et la consommation de nourriture.
* **`bassin_xml.c`** : Gère la sauvegarde et le chargement de l'état de la simulation (poissons, canvas) et des configurations des espèces via des fichiers XML.
* **`bassin_private.h`** : En-tête interne regroupant les structures partagées (`BassinUI`, `SpeciesConfig`) et exposant les fonctions entre les différents modules du bassin (actions, dialogs, pages, simulation).

---

## `src/tests/`
Programmes de tests de validation pour les composants de l'application.
* **`test_*.c`** : Fichiers indépendants permettant de tester de manière isolée chaque widget personnalisé développé (boutons, champs de texte, conteneurs, fenêtres, images, menus, sliders).
* **`runbat.c`** : Outil potentiel de lancement/test.
* **`exam.c`** & **`exams/`** : Fichiers d'examens ou de prototypes liés au cadre éducatif du projet.