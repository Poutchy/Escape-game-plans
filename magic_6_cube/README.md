Magic 6 Cube
=============

Description
-----------

"magic 6 cube" est un prototype de cube interactif conçu pour des escape-games. Il utilise un Arduino pour piloter un écran RGB, lire un capteur accéléromètre/gyroscope et gérer deux boutons (bleu et rouge). Le cube détecte les mouvements et fournit un retour visuel.

Connexions (général)
--------------------

- Écran RGB : bus I2C (SDA / SCL)
- Capteur accel/gyro : bus I2C (SDA / SCL)
- Bouton bleu : broches D4 / D5 (ou selon le sketch)
- Bouton rouge: broches D2 / D3

Consultez le dossier `schemes/` pour le schéma de câblage détaillé.

Structure du projet
-------------------

- `code/` : sketch Arduino principal (`sketch_mar2a.ino`)
- `models/` : modèles 3D
- `PDFs/` : documents uticomplémentaires
- `schemes/` : schémas électriques et diagrammes

Installation et utilisation
--------------------------

1. Ouvrir `code/sketch_mar2a.ino` dans l'IDE Arduino.
2. Installer les bibliothèques nécessaires (I2C, écran, MPU6050) si besoin.
3. Vérifier et ajuster les définitions de broches dans le sketch si nécessaire.
4. Téléverser le sketch sur l'Arduino et tester les entrées (boutons) et l'affichage.

Remarques
--------

- Les adresses I2C et les broches peuvent varier selon le matériel utilisé.
- Le schéma fourni dans `schemes/` est un diagramme simplifié pour guider le câblage.