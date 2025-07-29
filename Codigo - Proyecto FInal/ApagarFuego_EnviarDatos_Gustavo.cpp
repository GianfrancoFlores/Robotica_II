#include <Servo.h>

// === Pin del relé de la bomba ===
const int BOMBA = 12;

// === Servo motor para aspersión ===
Servo servo;

// === Variable modo y sensores (externas) ===
extern String modo_actual;
extern int sensor_izquierdo;
extern int sensor_centro;
extern int sensor_derecho;

// ==================== APAGAR FUEGO ====================
void apagarFuego() {
  digitalWrite(BOMBA, HIGH);

  for (int i = 90; i < 110; i++) {
    servo.write(i);
    delay(10);
  }
  for (int i = 110; i > 70; i--) {
    servo.write(i);
    delay(10);
  }
  for (int i = 70; i < 90; i++) {
    servo.write(i);
    delay(10);
  }

  digitalWrite(BOMBA, LOW);
}

// ==================== ENVÍO DE DATOS BLUETOOTH ====================
void enviarDatosBT(int s1, int s2, int s3, String modo) {
  String buffer;
  buffer = String(s1) + "," + String(s2) + "," + String(s3) + "," + ((modo == "auto") ? "a" : "m") + "\0";
  Serial.print(buffer);
}