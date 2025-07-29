// === Pines de sensores de flama ===
const int IZQ = A0;
const int CENTRO = A1;
const int DER = A2;

// === Pines del sensor ultrasónico ===
const int trigPin = 5;
const int echoPin = 4;

// === Umbrales de detección ===
const int UMBRAL = 970;
const int UMBRAL_CERCA = 400;

// === Variables globales de sensores ===
int sensor_izquierdo;
int sensor_centro;
int sensor_derecho;

// === Variable modo (compartida con Bluetooth) ===
extern String modo_actual;

// Función detener (declarada externamente)
extern void detener();
extern void adelante();
extern void izquierda();
extern void derecha();
extern void apagarFuego();

// ==================== MODO AUTOMÁTICO: SEGUIR FUEGO ====================
void seguirFuego() {
  int distancia = obtenerDistancia();
  if (distancia > 0 && distancia < 20) {
    detener();
    apagarFuego();
  }

  if (sensor_izquierdo < UMBRAL_CERCA || sensor_centro < UMBRAL_CERCA || sensor_derecho < UMBRAL_CERCA) {
    detener();
    apagarFuego();
  } else if ((sensor_izquierdo < UMBRAL && sensor_izquierdo > UMBRAL_CERCA) ||
             (sensor_centro < UMBRAL && sensor_centro > UMBRAL_CERCA) ||
             (sensor_derecho < UMBRAL && sensor_derecho > UMBRAL_CERCA)) {

    int minValor = sensor_centro;
    adelante();
    if (sensor_izquierdo < minValor) {
      minValor = sensor_izquierdo;
      izquierda();
    }
    if (sensor_derecho < minValor) {
      minValor = sensor_derecho;
      derecha();
    }
  } else {
    detener();
  }
}

// ==================== SENSOR ULTRASÓNICO ====================
int obtenerDistancia() {
  long duration;
  float distance;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000); // timeout opcional
  distance = duration * 0.0343 / 2;

  return distance;
}
