# Documentation Technique - Calibrateur

## Vue d'ensemble du projet

Le "Calibrateur" est un objet à l'apparence de vieille radio, créé pour les besoins d'un escape game. Le principe est de régler 3 fréquences (obtenues préalablement par le biais d'énigmes) avec le potentiomètre. Une fois les 3 fréquences validées, un code est délivré.

## Composants matériels

### Matériel principal

- **Carte Arduino Uno R4** : Contrôleur principal du système
- **Potentiomètre** : Pour régler la valeur de la fréquence.
- **Buzzer** : Retour sonor lors de la validation d'une fréquence.
- **Écran LCD Grove RGB** : Affichage de l'étape actuelle, de la fréquence, et du code à la fin de l'énigme
- **Alimentation** : 5V

## Fonctionnement logique

### Séquence d'opération

1. **Réglage sur une fréquence** :
   - Le joueur configure sa fréquence en tournant le potentiomètre.

2. **Détection de la validité de la fréquence** :
   - Si la fréquence est correcte, un signal sonor et un clignotement de l'écran sont émis au bout d'une seconde.
   - Si la fréquence est mauvaise, rien ne se passe.

3. **Validation globale** :
   - Lorsque les trois fréquences sont configurées, l'affichage change et affiche un code à utiliser sur une autre énigme.

## Code Arduino

Le code principal se trouve dans le dossier `code/`

## Énigme à résoudre pour trouver les fréquences

### Fréquence 1/3

```
// CALCUL DE LA FRÉQUENCE ALPHA
Variable Frequence = 5
// Répéter l'action suivante 4 fois :
POUR ( Cycle allant de 1 à 4 ) FAIRE :
    Frequence = Frequence + (Cycle * Cycle)
FIN POUR
// Entrez le résultat final.
```

Résultat attendu : 35

### Fréquence 2/3

```
// CALCUL DE LA FRÉQUENCE BETA
Variable Frequence = 80
// Répéter l'action suivante tant que la condition est vraie :
TANT QUE ( Frequence > 65 ) FAIRE :
    Frequence = Frequence - 7
    Frequence = Frequence + 3
FIN TANT QUE
// Entrez le résultat final.
```

Résultat attendu : 64

### Fréquence 3/3

```
// CALCUL DE LA FRÉQUENCE GAMMA
Variable Frequence = 2
// Pour chaque phase, exécuter les micro-ajustements :
POUR ( Phase allant de 1 à 3 ) FAIRE :
    POUR ( Ajustement allant de 1 à Phase ) FAIRE :
        Frequence = Frequence + (Phase * Ajustement)
    FIN POUR
FIN POUR
// Entrez le résultat final.
```

Résultat attendu : 27
