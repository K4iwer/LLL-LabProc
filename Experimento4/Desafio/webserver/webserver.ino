#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_do_grupinho_s";
const char* password = "12345678";

WebServer server(80);

const uint8_t LED_PWM_PIN = 6;
const uint8_t LED_PWM_RESOLUTION = 10;

const uint8_t SERVO_PWM_PIN = 7;
const uint8_t SERVO_PWM_RESOLUTION = 14;
const uint32_t SERVO_PWM_FREQUENCY = 50;
const uint16_t SERVO_MIN_US = 500;
const uint16_t SERVO_MAX_US = 2400;

uint32_t ledFrequency = 1000;
uint8_t ledDutyPercent = 50;
uint8_t servoAngle = 90;

uint32_t maxDuty(uint8_t resolution) {
    return (1UL << resolution) - 1;
}

bool setupPwm(uint8_t pin, uint32_t freq, uint8_t resolution) {
    return ledcAttach(pin, freq, resolution);
}

bool changeFrequency(uint8_t pin, uint32_t freq, uint8_t resolution) {
    return ledcChangeFrequency(pin, freq, resolution) != 0;
}

void writeDuty(uint8_t pin, uint32_t duty) {
    ledcWrite(pin, duty);
}

uint32_t ledDutyFromPercent(uint8_t percent) {
    percent = constrain(percent, 0, 100);
    return (maxDuty(LED_PWM_RESOLUTION) * percent) / 100;
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
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PWM Integrado ESP32-C3</title>
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
            width: min(92vw, 560px);
        }
        h1 { margin-bottom: 24px; }
        h2 { margin-top: 28px; }
        label { display: block; margin-top: 14px; }
        input, select {
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
        .status { margin-top: 22px; line-height: 1.6; }
        .divider { height: 1px; background: #55556f; margin: 26px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Controle PWM Integrado</h1>

        <form action="/set" method="GET">
            <h2>LED externo</h2>
            <label>Frequencia</label>
            <select name="freq">
)rawliteral";

    const uint32_t frequencies[] = {100, 500, 1000, 5000};
    for (uint8_t i = 0; i < 4; i++) {
        html += "<option value='" + String(frequencies[i]) + "'";
        if (frequencies[i] == ledFrequency) {
            html += " selected";
        }
        html += ">" + String(frequencies[i]) + " Hz</option>";
    }

    html += R"rawliteral(
            </select>
            <label>Duty cycle (%)</label>
)rawliteral";

    html += "<input type='range' "
        "name='duty' "
        "min='0' "
        "max='100' "
        "value='" + String(ledDutyPercent) + "' "
        "onchange='this.form.submit()'>";

    html += R"rawliteral(
            <div class="divider"></div>
            <h2>Servomotor</h2>
            <label>Angulo (0 a 180)</label>
)rawliteral";

    html += "<input type='range' "
        "name='angle' "
        "min='0' "
        "max='180' "
        "value='" + String(servoAngle) + "' "
        "onchange='this.form.submit()'>";

    html += R"rawliteral(
            <br>
            <button type="submit">Aplicar</button>
        </form>
        <div class="status">
)rawliteral";

    html += "LED: " + String(ledFrequency) + " Hz, duty " + String(ledDutyPercent) + "%<br>";
    html += "Servo: " + String(servoAngle) + " graus, pulso " + String(pulseFromAngle(servoAngle)) + " us";

    html += R"rawliteral(
        </div>
    </div>
</body>
</html>
)rawliteral";

    return html;
}

void applyLedPwm(uint32_t freq, uint8_t dutyPercent) {
    ledFrequency = freq;
    ledDutyPercent = constrain(dutyPercent, 0, 100);

    changeFrequency(LED_PWM_PIN, ledFrequency, LED_PWM_RESOLUTION);
    writeDuty(LED_PWM_PIN, ledDutyFromPercent(ledDutyPercent));

    Serial.print("LED PWM -> frequencia: ");
    Serial.print(ledFrequency);
    Serial.print(" Hz, duty: ");
    Serial.print(ledDutyPercent);
    Serial.println("%");
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
    uint32_t freq = server.arg("freq").toInt();
    int duty = server.arg("duty").toInt();
    int angle = server.arg("angle").toInt();

    if (freq != 100 && freq != 500 && freq != 1000 && freq != 5000) {
        server.send(400, "text/plain", "Frequencia invalida");
        return;
    }

    if (duty < 0 || duty > 100) {
        server.send(400, "text/plain", "Duty invalido");
        return;
    }

    if (angle < 0 || angle > 180) {
        server.send(400, "text/plain", "Angulo invalido");
        return;
    }

    applyLedPwm(freq, duty);
    applyServoAngle(angle);
    server.send(200, "text/html", paginaHTML());
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PWM_PIN, OUTPUT);
    digitalWrite(LED_PWM_PIN, HIGH);
    delay(400);
    digitalWrite(LED_PWM_PIN, LOW);
    delay(400);

    setupPwm(LED_PWM_PIN, ledFrequency, LED_PWM_RESOLUTION);
    setupPwm(SERVO_PWM_PIN, SERVO_PWM_FREQUENCY, SERVO_PWM_RESOLUTION);
    applyLedPwm(ledFrequency, ledDutyPercent);
    applyServoAngle(servoAngle);

    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();

    Serial.print("IP da ESP32: ");
    Serial.println(IP);

    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.begin();

    Serial.println("Servidor HTTP iniciado para controle PWM integrado");
}

void loop() {
    server.handleClient();
}
