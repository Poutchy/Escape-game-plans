#define LIGHT_SENSOR_A0 A0
#define LIGHT_SENSOR_A1 A1
#define LIGHT_SENSOR_A2 A2
#define LED_BLANCHE_0 6
#define LED_BLANCHE_1 7
#define LED_BLANCHE_2 8
#define DISTANCEMIN 2
#define LIGHT_THRESHOLD 650
#define VALIDATION_DURATION 3000  // 3 secondes en ms

#include "Ultrasonic.h"

Ultrasonic ultrasonic0(2);
Ultrasonic ultrasonic1(3);
Ultrasonic ultrasonic2(4);

unsigned long lightTimer = 0;
bool timerStarted = false;
bool finished = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BLANCHE_0, OUTPUT);
  pinMode(LED_BLANCHE_1, OUTPUT);
  pinMode(LED_BLANCHE_2, OUTPUT);
}

void loop() {
  if (finished) return;  // Ne rien faire si déjà validé

  long cm0 = ultrasonic0.MeasureInCentimeters();
  long cm1 = ultrasonic1.MeasureInCentimeters();
  long cm2 = ultrasonic2.MeasureInCentimeters();

  Serial.println(String("Distance 0 : ") + cm0 + " cm");
  Serial.println(String("Distance 1 : ") + cm1 + " cm");
  Serial.println(String("Distance 2 : ") + cm2 + " cm");

  // Allumer LEDs si disquette détectée (distance <= 2 cm)
  digitalWrite(LED_BLANCHE_0, cm0 <= DISTANCEMIN ? HIGH : LOW);
  digitalWrite(LED_BLANCHE_1, cm1 <= DISTANCEMIN ? HIGH : LOW);
  digitalWrite(LED_BLANCHE_2, cm2 <= DISTANCEMIN ? HIGH : LOW);

  int lightValue0 = analogRead(LIGHT_SENSOR_A0);
  int lightValue1 = analogRead(LIGHT_SENSOR_A1);
  int lightValue2 = analogRead(LIGHT_SENSOR_A2);

  Serial.println(String("Luminosite 0: ") + lightValue0);
  Serial.println(String("Luminosite 1: ") + lightValue1);
  Serial.println(String("Luminosite 2: ") + lightValue2);

  // Vérifier si les 3 capteurs de lumière sont au-dessus du seuil
  bool allLightValid = (lightValue0 >= LIGHT_THRESHOLD) &&
                       (lightValue1 >= LIGHT_THRESHOLD) &&
                       (lightValue2 >= LIGHT_THRESHOLD);

  if (allLightValid) {
    if (!timerStarted) {
      // Démarrer le timer la première fois
      lightTimer = millis();
      timerStarted = true;
      Serial.println(">>> Lumière détectée, démarrage du timer...");
    } else if (millis() - lightTimer >= VALIDATION_DURATION) {
      // 3 secondes écoulées sans interruption → SUCCÈS
      Serial.println(">>> DISQUETTE VALIDE ! Bonne disquette détectée <<<");
      finished = true;

      // Laisser les LEDs allumées en signe de succès
      digitalWrite(LED_BLANCHE_0, HIGH);
      digitalWrite(LED_BLANCHE_1, HIGH);
      digitalWrite(LED_BLANCHE_2, HIGH);
    }
  } else {
    // Réinitialiser le timer si la condition est perdue
    if (timerStarted) {
      Serial.println(">>> Lumière perdue, timer réinitialisé.");
      timerStarted = false;
      lightTimer = 0;
    }
  }

  delay(100);
}