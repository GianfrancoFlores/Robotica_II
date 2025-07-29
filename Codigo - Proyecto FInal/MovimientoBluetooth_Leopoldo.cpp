// === Pines del Driver L298N ===
const int ENA = 6;
const int IN1 = 7;
const int IN2 = 8;
const int IN3 = 9;
const int IN4 = 10;
const int ENB = 11;

// === Variable para el modo actual ===
String modo_actual = "bluetooth";

// ==================== MOVIMIENTO ====================
void adelante() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 140);
  analogWrite(ENB, 110);
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

// ==================== CONTROL BLUETOOTH ====================
void controlBluetooth () {
  if (modo_actual != "bluetooth") {
    detener();
    modo_actual = "bluetooth";
  }

  if (Serial.available()) {
    char comando = Serial.read();
    switch (comando) {
      case 'A': adelante(); break;
      case 'S': atras(); break;
      case 'I': izquierda(); break;
      case 'D': derecha(); break;
      case 'T': detener(); break;
      default: break;
    }
  }
}