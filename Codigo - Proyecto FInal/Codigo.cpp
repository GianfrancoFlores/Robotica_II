#include <Servo.h>

// Umbral de detección de fuego
const int UMBRAL = 875;
// Umbral de fuego muy cercano
const int UMBRAL_CERCA = 400;

// Pines del driver L298N
const int ENA = 6;
const int IN1 = 7;
const int IN2 = 8;
const int IN3 = 9;
const int IN4 = 10;
const int ENB = 11;

// Pin del relé de la bomba
const int BOMBA = 12;

// Pines de los sensores de flama
const int IZQ = A0;
const int CENTRO = A1;
const int DER = A2;

// Pines sensor ultrasónico
const int trigPin = 5;
const int echoPin = 4;

// Servomotor
Servo servo;

// Variables para almacenar la lectura de los sensores
int sensor_izquierdo;
int sensor_centro;
int sensor_derecho;

// Modo actual bluetooth / automático
String modo_actual = "bluetooth";

// Variables para intervalo de envio de datos
unsigned long t_anterior = 0;
const unsigned long intervalo = 1000; // ms entre envíos

void setup() {
  // Inicialización de pines
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(BOMBA, OUTPUT);

  pinMode(IZQ, INPUT);
  pinMode(CENTRO, INPUT);
  pinMode(DER, INPUT);

  // Pin del servomotor
  servo.attach(2);

  // Bomba apagada
  digitalWrite(BOMBA, LOW);

  // Inicializar pines sensor US
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Inicializar Bluetooth
  Serial.begin(9600);
}

void loop() {
  sensor_izquierdo = analogRead(IZQ);
  sensor_centro = analogRead(CENTRO);
  sensor_derecho = analogRead(DER);

  if (sensor_izquierdo < UMBRAL || sensor_centro < UMBRAL || sensor_derecho < UMBRAL) {
    modo_actual = "auto";
    seguirFuego();
  } else {
    controlBluetooth();
  }

  // Enviar cada cierto intervalo
  if ((millis() - t_anterior) >= intervalo) {
    enviarDatosBT(sensor_izquierdo, sensor_centro, sensor_derecho, modo_actual);
    t_anterior = millis();
  }
  delay(50);
}

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
  analogWrite(ENA, 140);  // Velocidad
  analogWrite(ENB, 110);  // Velocidad
}

void izquierda() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 100);  // Velocidad
  analogWrite(ENB, 100);  // Velocidad
}

void derecha() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 100);  // Velocidad
  analogWrite(ENB, 100);  // Velocidad
}

void detener() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void controlBluetooth () {
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

void seguirFuego() {
  int distancia = obtenerDistancia();
  if (distancia > 0 && distancia < 20) {
    detener();
    apagarFuego();
  }

  if (sensor_izquierdo < UMBRAL_CERCA || sensor_centro < UMBRAL_CERCA || sensor_derecho < UMBRAL_CERCA) {
    //Los sensores están muy cerca al fuego
    detener();
    apagarFuego();

  } else if ((sensor_izquierdo < UMBRAL && sensor_izquierdo > UMBRAL_CERCA) ||
             (sensor_centro < UMBRAL && sensor_centro > UMBRAL_CERCA) ||
             (sensor_derecho < UMBRAL && sensor_derecho > UMBRAL_CERCA)) {
    // Buscar el sensor con el valor más bajo
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
    // Ningún sensor detecta fuego
    detener();
  }
}

void apagarFuego() {
  digitalWrite(BOMBA, HIGH); // Activa bomba de agua

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

  digitalWrite(BOMBA, LOW); // Desactiva bomba de agua
}

int obtenerDistancia() {
  long duration;
  float distance;

  // Enviar pulso de 10us
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Leer el tiempo que tarda el eco
  duration = pulseIn(echoPin, HIGH, 30000); // timeout opcional (30 ms ≈ 5 metros máx)

  // Calcular distancia
  distance = duration * 0.0343 / 2;

  return distance;
}

void enviarDatosBT(int s1, int s2, int s3, String modo) {
  String buffer;
  buffer = String(s1) + "," + String(s2) + "," + String(s3) + "," + ((modo == "auto") ? "a" : "m") + "\0";
  Serial.print(buffer);
}
