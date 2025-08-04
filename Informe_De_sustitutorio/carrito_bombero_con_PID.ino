#include <Servo.h>

// --- Constantes ---
const int UMBRAL = 875;       // Umbral para detectar fuego
const int UMBRAL_CERCA = 400; // Umbral para fuego muy cercano

// --- Pines del Driver L298N ---
const int ENA = 6;
const int IN1 = 7;
const int IN2 = 8;
const int IN3 = 9;
const int IN4 = 10;
const int ENB = 11;

// --- Pin del Relé de la Bomba ---
const int BOMBA = 12;

// --- Pines de Sensores de Flama ---
const int IZQ = A0;
const int CENTRO = A1;
const int DER = A2;

// --- Pines Sensor Ultrasónico ---
const int trigPin = 5;
const int echoPin = 4;

// --- Servomotor ---
Servo servo;

// --- NUEVO: Variables para el Controlador PID del Servo ---

// - Kp: Corrige el error presente. Aumenta la velocidad de respuesta.
// - Ki: Corrige errores acumulados del pasado. Ayuda a eliminar el error estacionario.
// - Kd: Predice errores futuros. Amortigua la respuesta y previene oscilaciones.
float Kp = 0.07;
float Ki = 0.001;
float Kd = 0.04;

float error = 0;
float error_previo = 0;
float error_acumulado = 0; // NUEVA variable para el término Integral
float salida_pid = 0;
int posicion_servo = 90; // Posición inicial y de reposo del servo

// --- Variables Globales de Estado ---
int sensor_izquierdo;
int sensor_centro;
int sensor_derecho;
int distancia_actual = 0;
String modo_actual = "manual";
String estado_bomba = "apagada";
String estado_servo = "detenido";

// --- Variables para envío de datos por Serial ---
unsigned long tiempoPrevio = 0;
const long intervalo = 1000; // Intervalo para enviar datos (1 segundo)

void setup() {
  // Inicialización de pines de salida
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(BOMBA, OUTPUT);

  // Inicialización de pines de entrada
  pinMode(IZQ, INPUT);
  pinMode(CENTRO, INPUT);
  pinMode(DER, INPUT);

  // Servomotor
  servo.attach(2);
  servo.write(posicion_servo); // Iniciar servo en la posición central

  // Estado inicial
  digitalWrite(BOMBA, LOW);

  // Sensor Ultrasónico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Comunicación Serial
  Serial.begin(9600);
}

void loop() {
  // Leer todos los sensores en cada ciclo
  sensor_izquierdo = analogRead(IZQ);
  sensor_centro = analogRead(CENTRO);
  sensor_derecho = analogRead(DER);
  distancia_actual = obtenerDistancia();

  // Lógica de cambio de modo
  if (sensor_izquierdo < UMBRAL || sensor_centro < UMBRAL || sensor_derecho < UMBRAL) {
    if (modo_actual != "auto") {
        detener();
        modo_actual = "auto";
    }
    seguirFuego();
  } else {
    if (modo_actual != "manual") {
        detener();
        modo_actual = "manual";
    }
    procesarComandoSerial();
  }
  
  // Enviar datos al Wemos
  enviarDatosWemos();
  
  delay(50);
}

// --- Envío de datos al Wemos ---
void enviarDatosWemos() {
  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoPrevio >= intervalo) {
    tiempoPrevio = tiempoActual;
    String data = "D:" + String(sensor_izquierdo) + "," + String(sensor_centro) + "," + String(sensor_derecho) + "," + modo_actual + "," + estado_bomba + "," + estado_servo + "," + String(distancia_actual);
    Serial.println(data);
  }
}

// --- Funciones de Movimiento ---
void adelante() { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); analogWrite(ENA, 140); analogWrite(ENB, 110); }
void atras() { digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENA, 140); analogWrite(ENB, 110); }
void izquierda() { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); analogWrite(ENA, 100); analogWrite(ENB, 100); }
void derecha() { digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); analogWrite(ENA, 100); analogWrite(ENB, 100); }
void detener() { digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); analogWrite(ENA, 0); analogWrite(ENB, 0); }

// --- Procesamiento de Comandos ---
void procesarComandoSerial() {
  if (Serial.available()) {
    char comando = Serial.read();
    switch (comando) {
      case 'A': adelante(); break;
      case 'S': atras(); break;
      case 'I': izquierda(); break;
      case 'D': derecha(); break;
      case 'T': detener(); break;
      case 'B': 
        digitalWrite(BOMBA, HIGH); 
        estado_bomba = "encendida";
        break;
      case 'N': 
        digitalWrite(BOMBA, LOW);
        estado_bomba = "apagada";
        break;
    }
  }
}

// --- Lógica del Modo Automático ---
void seguirFuego() {
  // Si el robot está muy cerca, se detiene y activa el modo de extinción
  if (distancia_actual > 0 && distancia_actual < 20) {
    detener();
    apagarFuegoPID(); // Llamada a la nueva función con control PID
    return;
  }

  // Si los sensores detectan fuego muy cercano, activa el modo de extinción
  if (sensor_izquierdo < UMBRAL_CERCA || sensor_centro < UMBRAL_CERCA || sensor_derecho < UMBRAL_CERCA) {
    detener();
    apagarFuegoPID(); // Llamada a la nueva función con control PID
  } else if ((sensor_izquierdo < UMBRAL) || (sensor_centro < UMBRAL) || (sensor_derecho < UMBRAL)) {
    // Lógica para seguir el fuego
    if (sensor_centro < UMBRAL) adelante();
    else if (sensor_izquierdo < sensor_derecho) izquierda();
    else derecha();
  } else {
    detener();
  }
}

// --- Función de extinción con control PID ---
void apagarFuegoPID() {
  digitalWrite(BOMBA, HIGH);
  estado_bomba = "encendida";
  estado_servo = "movimiento";

  unsigned long tiempoInicio = millis();
  // Ejecuta el control PID durante 5 segundos para apuntar y extinguir
  while (millis() - tiempoInicio < 5000) {
    // Para una respuesta rápida del servo, leemos los sensores de nuevo aquí
    int val_izq = analogRead(IZQ);
    int val_der = analogRead(DER);

    // El error es la diferencia entre los sensores. Un valor más bajo indica más fuego.
    error = val_izq - val_der;

    // TÉRMINO INTEGRAL: Acumula el error a lo largo del tiempo.
    error_acumulado += error;

    // --- Cálculo de la salida del controlador PID ---
    salida_pid = (Kp * error) + (Ki * error_acumulado) + (Kd * (error - error_previo));

    // Actualizar la posición del servo
    posicion_servo -= salida_pid;

    // Limitar la posición del servo a un rango seguro (ej. 60-120 grados)
    posicion_servo = constrain(posicion_servo, 60, 120);

    // Enviar el comando al servo
    servo.write(posicion_servo);

    // Guardar el error actual para la siguiente iteración (para el término derivativo)
    error_previo = error;

    delay(20); // Pequeña pausa para estabilizar el sistema y el ciclo de control
  }

  // Al terminar, apagar la bomba y centrar el servo
  digitalWrite(BOMBA, LOW);
  estado_bomba = "apagada";
  estado_servo = "detenido";
  
  // Regresar el servo a la posición central de reposo
  posicion_servo = 90;
  servo.write(posicion_servo);
  
  // Reiniciar errores para la próxima activación
  error_previo = 0; 
  error_acumulado = 0;
}

int obtenerDistancia() {
  long duration;
  float distance;
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH, 30000); // Timeout 30ms
  distance = duration * 0.0343 / 2;

  if (distance > 400 || distance <= 0) {
    return 400;
  }
  return (int)distance;
}

