# Documentation Technique - Super Ordinateur pour Escape Game

## Vue d'ensemble du projet

Le "Super Ordinateur" est un composant électronique développé pour un escape game, basé sur la plateforme Arduino. Ce système simule un ordinateur vintage qui nécessite l'insertion de trois disquettes spécifiques pour valider une étape du jeu. Le système utilise des capteurs optiques et ultrasoniques pour vérifier l'authenticité des disquettes, et communique via LoRaOne avec un minuteur central pour signaler le succès ou l'échec de l'opération.

## Composants matériels

### Matériel principal
- **Carte Arduino** : Contrôleur principal du système
- **3 Unités de lecture de carte** : Chacune composée de :
  - Capteur ultrasonique pour détecter la présence de la disquette.
  - LED infrarouge ou visible pour l'émission de lumière.
  - Capteur de lumière (phototransistor ou LDR) pour détecter la transmission de lumière à travers le trou de la disquette.
- **Module LoRaOne** : Pour la communication sans fil avec le minuteur.
- **Alimentation** : 5V/3.3V selon les composants utilisés.

### Disquettes
- Chaque disquette est une carte plastique avec un trou percé à une position spécifique.
- Seules les disquettes avec le trou correctement positionné permettront la transmission de lumière.

## Fonctionnement logique

### Séquence d'opération

1. **Insertion de la disquette** :
   - Le joueur insère une disquette dans l'une des trois unités de lecture.
   - Le capteur ultrasonique détecte la présence de la disquette (mesure de distance).

2. **Activation de la LED** :
   - Une fois la présence détectée, la LED correspondante s'allume.
   - La lumière émise passe à travers le trou de la disquette si celui-ci est aligné avec la LED.

3. **Détection de la lumière** :
   - Le capteur de lumière détecte si la lumière traverse le trou.
   - Si oui, le capteur enregistre un signal positif pour cette unité.

4. **Validation globale** :
   - Lorsque les trois unités ont des disquettes insérées :
     - Si tous les trois capteurs détectent de la lumière : signal positif envoyé via LoRaOne.
     - Si au moins un capteur ne détecte pas de lumière : signal négatif envoyé via LoRaOne.

## Code Arduino

Le code principal se trouve dans le dossier `code/`

## Maintenance et dépannage

- Vérifier l'alignement des trous des disquettes avec les LEDs.
- Calibrer les seuils des capteurs de lumière.
- Assurer une alimentation stable pour éviter les faux positifs.
