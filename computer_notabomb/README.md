# Équipe 7

- [Équipe 7](#équipe-7)
  - [Introduction](#introduction)
  - [1. Machine principale - NOTABOMB](#1-machine-principale---notabomb)
    - [1.1. Description](#11-description)
    - [1.2. Modèles 3D](#12-modèles-3d)
    - [1.3. Montage](#13-montage)
  - [2. Talkie-Walkie](#2-talkie-walkie)
    - [2.1. Description](#21-description)
    - [2.2. Modèles 3D](#22-modèles-3d)
    - [2.3. Montage](#23-montage)

## Introduction

Cette équipe est composée de :

- Mattéo LUQUE
- Xavier MAZIERE
- Nolann SANMARTI
- Achille GRAVOUIL

Notre projet est une composé de deux éléments : une machine principale et un talkie-walkie. Durant l'activité, les joueurs devront intéragir avec la machine principale pour débloquer une disquette, utilisée pour débloquer la boîte finale de l'escape game.

Le fonctionnement général est inspiré du jeu "Keep Talking and Nobody Explodes". Alors que l'un des joueurs sera devant la machine principale, les autres joueurs devront trouver les morceaux de manuels cachés (et scotchés) dans la salle pour aider le joueur devant la machine à résoudre les différentes énigmes. Le talkie-walkie est utilisé pour certaines énigmes, en fournissant des messages audio à décoder et utiliser.

## 1. Machine principale - NOTABOMB

### 1.1. Description

Le "Numerically Operated Terminal Assisted By Occasional Manual Brute-force" ou "NOTABOMB", est une machine inspiré de l'écran Atari SM124 dans son design, imprimée en 3D et équipée de composants électroniques opérés par deux arduinos. La machine permet de débloquer l'une des disquettes de l'escape game. Dans l'histoire entourant le jeu, elle fait parti des machines créées par l'entreprise VEEC, et est un processeur de calcul très avancé qui a malheureusement planté durant ses opérations. Les joueurs ayant besoin de la disquette qu'elle contient, ils devront effectuer les dernières instructions manuellement pour débloquer la machine et récupérer la disquette.

### 1.2. Modèles 3D

Dans le dossier `models` se trouvent les modèles de la machine principale au format stl.

Les différentes pièces sont :

- Socle, sur laquelle viennent s'imbriquer les autres pièces
- Arduinos, dans laquelle 2 arduinos sont placées côte à côte. Des emplacements sont prévus pour d'éventuelles batteries. Dans le cas où une alimentation filaire est utilisée, il sera nécessaire de percer des trous à travers cette pièce et le socle pour faire passer les câbles.
- Contrôles, de laquelle sortent les 4 boutons et sur laquelle sont inscrits les noms des boutons.
- Disquette, (porte-disquette) qui contient la disquette et le servo qui bloque la disquette en place. Il est nécessaire de percer un trou à travers cette pièce et la boîte pour faire passer les câbles du servo.
- Écran, qui contient les deux écrans LCD et les quatre LEDs.
- Boîte, qui referme la machine et sert de couvercle au tout. En termes d'impression, il s'agit de la pièce la plus complexe, dû à son design particulier.

Afin de prévisualiser les différentes pièces, il est possible de les consulter sur Onshape en suivant [ce lien.](https://cad.onshape.com/documents/be6f37f78814590b17c3a1c2/w/f588fba8efcb07c5e2274cb0/e/107cf3115431e2fff5c39ad0)

### 1.3. Montage

Dans l'ordre de montage, il faut :

1. Percer le trou pour le cable du servo
   1. Placer le `socle` à plat
   2. Imbriquer la pièce `arduinos` dans le `socle`
   3. Imbriquer la `boite` dans le `socle`
   4. Imbriquer le porte-`disquette` dans la `boite` et le `socle`
   5. Percer un trou à travers le porte-`disquette` et la `boite` pour faire passer les câbles du servo, au dessus de la pièce `arduinos`
   6. Retirer la pièce `disquette` et la `boite` du `socle`
   7. Retirer la pièce `boite` du `socle`
2. Montage final
   1. Imbriquer les arduinos et les batteries dans la pièce `arduinos`
   2. Imbriquer les LCDs et les LEDs dans l'`ecran`
   3. Imbriquer l'`ecran` dans le `socle`
   4. Faire passer les fils des boutons dans l'encoche du bas de l'`ecran`
   5. Imbriquer les boutons dans la pièce `controles` (peut nécessiter des fixations supplémentaires)
   6. Imbriquer la pièce `controles` dans le `socle`
   7. Imbriquer la `disquette` dans la `boite`
   8. Faire passer les câbles du servo à travers le trou percé précédemment (peut nécessiter des fils plutôt longs)
   9. Effectuer tout les branchements sur les arduinos
   10. Imbriquer la `boite` et la `disquette` dans le `socle`

## 2. Talkie-Walkie

### 2.1. Description

### 2.2. Modèles 3D

### 2.3. Montage
