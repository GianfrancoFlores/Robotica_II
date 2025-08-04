#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// --- Configuración del Punto de Acceso (Access Point) ---
const char* ssid = "CarritoBombero_WIFI";
const char* password = "password123";

// --- Configuración de IP Fija ---
IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// --- Servidor Web ---
ESP8266WebServer server(80);

// --- Variables para almacenar datos del Arduino ---
int sensorIzq = 0;
int sensorCen = 0;
int sensorDer = 0;
int distancia = 0; // NUEVO
String modo = "desconectado";
String estadoBomba = "apagada";
String estadoServo = "detenido";

// --- Página Web (HTML + CSS + JavaScript) ---
String webpage = R"RAW_HTML(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>Control Carrito Bombero</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background-color: #2c3e50; color: white; text-align: center; margin: 0; padding: 15px; display: flex; flex-direction: column; justify-content: center; min-height: 100vh; box-sizing: border-box; }
        h1 { margin-top: 0; margin-bottom: 20px; }
        .control-grid { display: grid; grid-template-columns: repeat(3, 1fr); grid-template-rows: repeat(3, 1fr); gap: 15px; max-width: 300px; width: 100%; margin: 0 auto 20px auto; }
        .btn { padding: 20px; font-size: 24px; border: none; border-radius: 15px; background-color: #3498db; color: white; cursor: pointer; transition: background-color 0.2s; user-select: none; -webkit-tap-highlight-color: transparent; }
        .btn:active { background-color: #2980b9; }
        #adelante { grid-column: 2; grid-row: 1; }
        #izquierda { grid-column: 1; grid-row: 2; }
        #detener { grid-column: 2; grid-row: 2; background-color: #e74c3c; }
        #derecha { grid-column: 3; grid-row: 2; }
        #atras { grid-column: 2; grid-row: 3; }
        #bomba { width: 100%; max-width: 300px; margin: 0 auto; background-color: #f1c40f; color: #2c3e50; }
        .status-container { max-width: 300px; width: 100%; margin: 25px auto 0 auto; background-color: #34495e; border-radius: 15px; padding: 15px; box-sizing: border-box; }
        .status-container h2 { margin-top: 0; margin-bottom: 10px; font-size: 20px; color: #ecf0f1; }
        .status-info p { margin: 8px 0; font-size: 18px; }
        .status-info span { font-weight: bold; color: #f1c40f; text-transform: uppercase; }
        .sensor-readings { display: flex; justify-content: space-around; margin-top: 15px; background-color: #2c3e50; padding: 10px; border-radius: 10px; }
        .sensor-readings div { font-size: 16px; }
    </style>
</head>
<body>
    <h1>Control Wi-Fi</h1>
    <div class="control-grid">
        <button id="adelante" class="btn" onmousedown="sendCommand('A')" onmouseup="sendCommand('T')" ontouchstart="sendCommand('A')" ontouchend="sendCommand('T')">▲</button>
        <button id="izquierda" class="btn" onmousedown="sendCommand('I')" onmouseup="sendCommand('T')" ontouchstart="sendCommand('I')" ontouchend="sendCommand('T')">◀</button>
        <button id="detener" class="btn" onclick="sendCommand('T')">■</button>
        <button id="derecha" class="btn" onmousedown="sendCommand('D')" onmouseup="sendCommand('T')" ontouchstart="sendCommand('D')" ontouchend="sendCommand('T')">▶</button>
        <button id="atras" class="btn" onmousedown="sendCommand('S')" onmouseup="sendCommand('T')" ontouchstart="sendCommand('S')" ontouchend="sendCommand('T')">▼</button>
    </div>
    <button id="bomba" class="btn" onclick="toggleBomba()">BOMBA</button>

    <div class="status-container">
        <h2>Estado del Sistema</h2>
        <div class="status-info">
            <p>Modo: <span id="modo-val">--</span></p>
            <p>Bomba: <span id="bomba-val">--</span></p>
            <p>Servo: <span id="servo-val">--</span></p>
            <p>Distancia: <span id="dist-val">--</span> cm</p> <!-- NUEVO -->
        </div>
        <div class="sensor-readings">
            <div>IZQ: <span id="sensor-izq">--</span></div>
            <div>CEN: <span id="sensor-cen">--</span></div>
            <div>DER: <span id="sensor-der">--</span></div>
        </div>
    </div>

    <script>
        let bombaActivada = false;

        function sendCommand(cmd) { fetch('/control?cmd=' + cmd); }

        function toggleBomba() {
            bombaActivada = !bombaActivada;
            const cmd = bombaActivada ? 'B' : 'N';
            sendCommand(cmd);
            document.getElementById('bomba').style.backgroundColor = bombaActivada ? '#2ecc71' : '#f1c40f';
        }

        function updateStatus() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('modo-val').innerText = data.modo;
                    document.getElementById('bomba-val').innerText = data.bomba;
                    document.getElementById('servo-val').innerText = data.servo;
                    document.getElementById('dist-val').innerText = data.distancia; // NUEVO
                    document.getElementById('sensor-izq').innerText = data.izq;
                    document.getElementById('sensor-cen').innerText = data.cen;
                    document.getElementById('sensor-der').innerText = data.der;
                })
                .catch(error => console.error('Error al actualizar estado:', error));
        }

        setInterval(updateStatus, 1000);
        document.addEventListener('DOMContentLoaded', updateStatus);
    </script>
</body>
</html>
)RAW_HTML";

// --- Funciones del Servidor ---

void handleRoot() { server.send(200, "text/html", webpage); }

void handleControl() {
  String command = server.arg("cmd");
  if (command.length() > 0) {
    Serial.print(command.charAt(0));
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Error");
  }
}

void handleData() {
  // NUEVO: JSON con más datos
  String json = "{\"izq\": " + String(sensorIzq) +
                ", \"cen\": " + String(sensorCen) +
                ", \"der\": " + String(sensorDer) +
                ", \"modo\": \"" + modo + "\"" +
                ", \"bomba\": \"" + estadoBomba + "\"" +
                ", \"servo\": \"" + estadoServo + "\"" +
                ", \"distancia\": " + String(distancia) + "}";
  server.send(200, "application/json", json);
}

void handleNotFound() { server.send(404, "text/plain", "404: No encontrado"); }

void readSerial() {
  while (Serial.available() > 0) {
    String serialData = Serial.readStringUntil('\n');
    serialData.trim();

    if (serialData.startsWith("D:")) {
      String data = serialData.substring(2);
      
      // NUEVO: Parseo para 7 valores
      int p[6]; // Posiciones de las comas
      p[0] = data.indexOf(',');
      p[1] = data.indexOf(',', p[0] + 1);
      p[2] = data.indexOf(',', p[1] + 1);
      p[3] = data.indexOf(',', p[2] + 1);
      p[4] = data.indexOf(',', p[3] + 1);
      p[5] = data.indexOf(',', p[4] + 1);

      if (p[0]>0 && p[1]>0 && p[2]>0 && p[3]>0 && p[4]>0 && p[5]>0) {
        sensorIzq = data.substring(0, p[0]).toInt();
        sensorCen = data.substring(p[0] + 1, p[1]).toInt();
        sensorDer = data.substring(p[1] + 1, p[2]).toInt();
        modo = data.substring(p[2] + 1, p[3]);
        estadoBomba = data.substring(p[3] + 1, p[4]);
        estadoServo = data.substring(p[4] + 1, p[5]);
        distancia = data.substring(p[5] + 1).toInt();
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(100);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  
  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);

  server.begin();
}

void loop() {
  server.handleClient();
  readSerial();
}


