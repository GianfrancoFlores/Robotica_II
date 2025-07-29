#include <Servo.h>

// Cambios Leopoldo - Movimiento y control Bluetooth
// Pines del driver L298N
const int ENA = 6;
const int IN1 = 7;
const int IN2 = 8;
const int IN3 = 9;
const int IN4 = 10;
const int ENB = 11;

// Variable para modo actual
String modo_actual = "bluetooth";

void setup() {
  // Inicialización de pines de motor
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Iniciar comunicación serial (Bluetooth)
  Serial.begin(9600);
}

void loop() {
  controlBluetooth();
}

void controlBluetooth() {
  if (modo_actual != "bluetooth") {
    detener();
    modo_actual = "bluetooth";
  }

  if (Serial.available()) {
    char comando = Serial.read();

    switch (comando) {
      case 'A':  // Avanzar
        adelante();
        break;
      case 'S':  // Retroceder
        atras();
        break;
      case 'I':  // Girar a la izquierda
        izquierda();
        break;
      case 'D':  // Girar a la derecha
        derecha();
        break;
      case 'T':  // Detenerse
        detener();
        break;
      default:
        break;
    }
  }
}

/* Funciones de movimiento */
void adelante() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 140);  // Velocidad
  analogWrite(ENB, 110);  // Velocidad
}

void atras() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 140);
  analogWrite(ENB, 110);
}

void izquierda() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 100);
  analogWrite(ENB, 100);
}

void derecha() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 100);
  analogWrite(ENB, 100);
}

void detener() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

//control de bomba de agua

void apagarFuego() {
  digitalWrite(BOMBA, HIGH);  // Encender bomba

  for (int i = 90; i <= 110; i++) {
    servo.write(i);
    delay(10);
  }
  for (int i = 110; i >= 70; i--) {
    servo.write(i);
    delay(10);
  }
  for (int i = 70; i <= 90; i++) {
    servo.write(i);
    delay(10);
  }

  digitalWrite(BOMBA, LOW);   // Apagar bomba
}

// Deteccion de fuego - Gianfranco

void seguirFuego() {
  int distancia = obtenerDistancia();

  if ((distancia > 0 && distancia < 20) || 
      sensor_izquierdo < UMBRAL_CERCA || 
      sensor_centro < UMBRAL_CERCA || 
      sensor_derecho < UMBRAL_CERCA) {
    detener();
    apagarFuego();
  } else if ((sensor_izquierdo < UMBRAL && sensor_izquierdo > UMBRAL_CERCA) || 
             (sensor_centro < UMBRAL && sensor_centro > UMBRAL_CERCA) || 
             (sensor_derecho < UMBRAL && sensor_derecho > UMBRAL_CERCA)) {

    if (sensor_izquierdo < sensor_centro && sensor_izquierdo < sensor_derecho)
      izquierda();
    else if (sensor_derecho < sensor_centro && sensor_derecho < sensor_izquierdo)
      derecha();
    else
      adelante();

  } else {
    detener();
  }
}

int obtenerDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  float distance = duration * 0.0343 / 2;
  return distance;
}
