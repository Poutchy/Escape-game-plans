# Magic 6 Cube

## Description

**Magic 6 Cube** est un prototype de cube interactif inspiré du jouet **Magic 8 Ball**.  
Il est conçu pour être utilisé dans des **escape games**.

L'objet utilise un **Arduino** pour piloter un écran LCD RGB, lire un **capteur accéléromètre / gyroscope** et gérer deux boutons.

Le cube détecte son **orientation dans l’espace** et affiche une **couleur correspondante sur l’écran LCD**.  
Le joueur doit entrer un **code de 4 couleurs** en inclinant le cube et en validant chaque choix avec un bouton.

Selon la combinaison saisie, un **message s’affiche** et fournit un indice pour résoudre une énigme.

---

# Connexions

Les composants sont reliés de la manière suivante :

- **Grove LCD RGB Backlight** : bus I2C
- **Grove 6-Axis Accelerometer & Gyroscope** : bus I2C
- **Red LED Button** : broche D2
- **Blue LED Button** : broche D4

Consultez le dossier **`schemes/`** pour le schéma de câblage détaillé.

---

# Installation et utilisation

1. Ouvrir `code/sketch_mar2a.ino` dans **Arduino IDE**.
2. Installer les bibliothèques nécessaires :
   - I2C
   - Grove LCD RGB Backlight
   - MPU6050 (accéléromètre / gyroscope)
3. Vérifier les broches utilisées pour les boutons dans le sketch.
4. Téléverser le programme sur l’Arduino.
5. Tester les boutons et l’affichage.

---

# Fonctionnement

Le cube utilise un **accéléromètre / gyroscope** pour détecter son orientation.

Chaque orientation correspond à :

- une **couleur affichée**
- une **lettre du code**

### Séquence d’utilisation

1. Incliner le cube pour sélectionner une couleur.
2. Appuyer sur le **bouton bleu** pour valider.
3. Répéter l’opération **4 fois**.
4. Lire le **message affiché**.
5. Appuyer sur le **bouton rouge** pour recommencer.

---

# Tableau des orientations

| Orientation | Couleur | Numéro |
|:------------:|:-----:|:-----:|
| Plat | Vert | 1 |
| Retourné | Rouge | 2 |
| Gauche | Bleu | 3 |
| Droite | Jaune | 4 |
| Avant | Orange | 5 |
| Arrière | Magenta | 6 |

---

# Messages / indices

| Orientation | Code secret | Texte | |
|:-------------------------------------:|:-----------:|:-----:|:-:|
| Plat, Gauche, Avant, Retourne | V - B - O - R | Les clés encore utilisables contiennent la lettre B | ✅3 |
| Avant, Arrière, Plat, Gauche | O - M - V - B | La cle 3 commence par V | ✅2 |
| Droite, Arrière, Retourne, Gauche | J - M - R - B | La cle 1 commence par une lettre plus proche de A que celle de la cle 2 | ✅1 |
| Avant, Droite, Plat, Arrière | O - J - V - M | Une seule cle commence par une voyelle | ❌ |
| Retourne, Droite, Gauche, Plat | R - B - J - V | Les cles valides ne commencent pas par B | ❌ |
| Gauche, Retourne, Droite, Avant | B - R - B - O | Une cle valide ne commence jamais par R | ❌ |
| Plat, Plat, Plat, Plat | V - V - V - V | Message 1 | X |
| Retourne, Retourne, Retourne, Retourne | R - R - R - R | Message 1 | X |

---

# Remarques

- Les **adresses I2C** peuvent varier selon le matériel utilisé.
- Les **broches des boutons** peuvent être modifiées dans le sketch Arduino.
- Le schéma présent dans **`schemes/`** est un diagramme simplifié pour aider au câblage.