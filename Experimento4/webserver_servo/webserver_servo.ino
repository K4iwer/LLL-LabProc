#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_do_grupinho_s";
const char* password = "12345678";

WebServer server(80);

const uint8_t SERVO_PWM_PIN = 7;
const uint8_t SERVO_PWM_RESOLUTION = 14;
const uint32_t SERVO_PWM_FREQUENCY = 50;
const uint16_t SERVO_MIN_US = 500;
const uint16_t SERVO_MAX_US = 2400;

uint8_t servoAngle = 90;

uint32_t maxDuty(uint8_t resolution) {
    return (1UL << resolution) - 1;
}

bool setupPwm(uint8_t pin, uint32_t freq, uint8_t resolution) {
    return ledcAttach(pin, freq, resolution);
}

void writeDuty(uint8_t pin, uint32_t duty) {
    ledcWrite(pin, duty);
}

uint32_t pulseUsToDuty(uint16_t pulseUs) {
    const uint32_t periodUs = 1000000UL / SERVO_PWM_FREQUENCY;
    return (pulseUs * maxDuty(SERVO_PWM_RESOLUTION)) / periodUs;
}

uint16_t pulseFromAngle(uint8_t angle) {
    angle = constrain(angle, 0, 180);
    return SERVO_MIN_US + ((uint32_t)(SERVO_MAX_US - SERVO_MIN_US) * angle) / 180;
}

String paginaHTML() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>PWM Servo ESP32-C3</title>
    <style>
        body {
            font-family: Arial;
            background-color: #1e1e2f;
            color: white;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
        }
        .container {
            background-color: #2b2b40;
            padding: 36px;
            border-radius: 20px;
            box-shadow: 0px 0px 20px rgba(0,0,0,0.3);
            text-align: center;
            width: min(92vw, 460px);
        }
        h1 { margin-bottom: 24px; }
        label { display: block; margin-top: 18px; }
        input {
            font-size: 18px;
            padding: 10px;
            margin: 10px;
            width: 170px;
            text-align: center;
            border: none;
            border-radius: 10px;
        }
        button {
            font-size: 20px;
            padding: 10px 20px;
            margin: 18px 10px 10px;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            transition: 0.2s;
        }
        button:hover { transform: scale(1.05); }
        .status { margin-top: 18px; line-height: 1.6; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Controle PWM do Servo</h1>
        <form action="/set" method="GET">
            <label>Angulo (0 a 180)</label>
)rawliteral";

    html += "<input type='range' "
        "name='duty' "
        "min='0' "
        "max='180' "
        "value='90' "
        "onchange='this.form.submit()'>";

    html += R"rawliteral(
            <br>
            <button type="submit">Aplicar</button>
        </form>
        <div class="status">
)rawliteral";

    html += "Angulo atual: " + String(servoAngle) + " graus<br>";
    html += "Pulso atual: " + String(pulseFromAngle(servoAngle)) + " us<br>";
    html += "Frequencia: 50 Hz";

    html += R"rawliteral(
        </div>
    </div>
</body>
</html>
)rawliteral";

    return html;
}

void applyServoAngle(uint8_t angle) {
    servoAngle = constrain(angle, 0, 180);
    uint16_t pulseUs = pulseFromAngle(servoAngle);
    writeDuty(SERVO_PWM_PIN, pulseUsToDuty(pulseUs));

    Serial.print("Servo PWM -> angulo: ");
    Serial.print(servoAngle);
    Serial.print(" graus, pulso: ");
    Serial.print(pulseUs);
    Serial.println(" us");
}

void handleRoot() {
    server.send(200, "text/html", paginaHTML());
}

void handleSet() {
    int angle = server.arg("angle").toInt();

    if (angle < 0 || angle > 180) {
        server.send(400, "text/plain", "Angulo invalido");
        return;
    }

    applyServoAngle(angle);
    server.send(200, "text/html", paginaHTML());
}

void setup() {
    Serial.begin(115200);

    setupPwm(SERVO_PWM_PIN, SERVO_PWM_FREQUENCY, SERVO_PWM_RESOLUTION);
    applyServoAngle(servoAngle);

    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();

    Serial.print("IP da ESP32: ");
    Serial.println(IP);

    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.begin();

    Serial.println("Servidor HTTP iniciado para controle PWM do servo");
}

void loop() {
    server.handleClient();
}
