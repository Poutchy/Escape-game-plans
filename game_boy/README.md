# Gameboy

## Résumé du fonctionnement

Ce projet est une énigme interactive inspirée d'une Gameboy, utilisée dans un escape game. Le joueur doit entrer un code de 4 lettres à l'aide de boutons et de l'orientation de l'appareil. Selon le code saisi, un message s'affiche pour aider à résoudre l'énigme des disquettes.

L'appareil détecte son orientation grâce à l'accéléromètre/gyroscope et affiche une couleur correspondante sur l'écran LCD. Chaque orientation permet de sélectionner une lettre. Le joueur valide chaque lettre avec le bouton bleu et peut réinitialiser la saisie avec le bouton rouge.

## Schéma de branchement

- Grove LCD RGB Backlight : port I2C
- Grove 6-Axis Accelerometer & Gyroscope : port I2C
- Red LED Button : port D2
- Blue LED Button : port D4

## Utilisation
### Exemple de séquence de jeu

1. L'appareil démarre, l'écran affiche une couleur selon l'orientation.
2. L'utilisateur incline l'appareil pour choisir la couleur/lettre désirée.
3. Il appuie sur le bouton bleu pour valider la lettre (répéter 4 fois).
4. Le message correspondant au code s'affiche.
5. Pour recommencer, appuyer sur le bouton rouge.

### Couleur - Orientation
|Orientation|Couleur|Numéro|
|:------------:|:-----:|:-----:|
|Plat       |Vert       |1|
|Retourne   |Rouge      |2|
|Gauche     |Bleu       |3|
|Droite     |Jaune      |4|
|Avant      |Orange     |5|
|Arriere    |Magenta    |6|

#### Détail technique

L'accéléromètre/gyroscope détecte l'orientation de l'appareil. Chaque orientation correspond à une couleur affichée sur l'écran LCD et à une lettre pour le code. Le code est validé lorsque 4 lettres sont saisies.

### Glossaire
|Orientation                            |Code secret  |Texte|
|:-------------------------------------:|:-----------:|:-----:|
|Plat, Gauche, Avant, Retourne          |V - B - O - R|Cette disquette est placée avant OMVB|
|Retourne, Droite, Gauche, Plat         |R - B - J - V|BRJO est la première disquette|
|Avant, Arriere, Plat, Gauche           |O - M - V - B|Je suis placée juste après VBOR|
|Droite, Arriere, Retourne, Gauche      |J - M - R - B|VBOR est la dernière disquette|
|Avant, Droite, Plat, Arriere           |O - J - V - M|Je suis entre VBOR et BRJO|
|Gauche, Retourne, Droite, Avant        |B - R - B - O|Je suis placée après OMVB|
|Plat, Plat, Plat, Plat                 |V - V - V - V|Message 1  |X|
|Retourne, Retourne, Retourne, Retourne |R - R - R - R|Message 1  |X|

## Dépendances logicielles

- Bibliothèque Grove LCD RGB Backlight
- Bibliothèque Grove 6-Axis Accelerometer & Gyroscope
- Bibliothèque pour gestion des boutons