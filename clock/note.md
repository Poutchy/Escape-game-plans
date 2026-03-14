# Notes de fabrication de la clock.

## Communication LoRaWAN

Pour utiliser convenablement les différentes communications, plusieurs choses sont à connaître.

- la mise en place d'un serveur RedNode est nécessaire. Par souci de simplicité, un a été fait par l'université.
- les requêtes des objets de la salle doivent contenir un unique octet, qui correspond à l'event demandé

  - 01 qui informe d'une erreur et donc qui retranche 1 minute à la clock
  - 00 qui signifie la fin de la partie

## Information par rapport à la clock en elle-même

La clock ne possède pas de trous pour ajouter de câbles. Il est possible d'ajouter des piles directement à l'intérieur si l'on souhaite rendre l'objet portatif, mais il est parfaitement possible d'ajouter 2 trous pour ajouter des câbles à l'objet en lui-même.

## Le code

Les 2 dossiers de code reprennent les codes à ajouter sur les cartes Arduino.

- la carte possédant l'outil de communication doit avoir le code LoRaWAN_Class_C
- l'autre carte doit avoir le code sketch_mar5d

## Le montage

Les cartes ont été designées de manière à ce qu'un shield soit apposé au-dessus. Il faut donc faire attention à ce que tous les ports soient entrés en plus.

