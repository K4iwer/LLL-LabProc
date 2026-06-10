#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32_do_grupinho_s";
const char* password = "12345678";

WebServer server(80);

const uint8_t LED_PWM_PIN = 6;
const uint8_t LED_PWM_RESOLUTION = 10;

uint32_t ledFrequency = 1000;
uint8_t ledDutyPercent = 50;

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

uint32_t dutyFromPercent(uint8_t percent) {
    percent = constrain(percent, 0, 100);
    return (maxDuty(LED_PWM_RESOLUTION) * percent) / 100;
}

String paginaHTML() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PWM LED ESP32-C3</title>
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
        .status { margin-top: 18px; line-height: 1.6; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Controle PWM do LED</h1>
        <form action="/set" method="GET">
            <label>Frequencia</label>
            <select name="freq">
)rawliteral";

    html += "<label>Frequencia (Hz)</label>";

    html += "<input type='range' "
            "name='freq' "
            "min='10' "
            "max='1000' "
            "value='" + String(ledFrequency) + "'>";

    html += "<p>" + String(ledFrequency) + " Hz</p>";
    
    html += "<input type='range' "
        "name='duty' "
        "min='0' "
        "max='100' "
        "value='" + String(ledDutyPercent) + "' "
        "onchange='this.form.submit()'>";

    html += R"rawliteral(
            <br>
            <button type="submit">Aplicar</button>
        </form>
        <div class="status">
)rawliteral";

    html += "Frequencia atual: " + String(ledFrequency) + " Hz<br>";
    html += "Duty atual: " + String(ledDutyPercent) + "%";

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
    writeDuty(LED_PWM_PIN, dutyFromPercent(ledDutyPercent));

    Serial.print("LED PWM -> frequencia: ");
    Serial.print(ledFrequency);
    Serial.print(" Hz, duty: ");
    Serial.print(ledDutyPercent);
    Serial.println("%");
}

void handleRoot() {
    server.send(200, "text/html", paginaHTML());
}

void handleSet() {
    uint32_t freq = server.arg("freq").toInt();
    uint8_t duty = server.arg("duty").toInt();

    if (freq < 10 || freq > 1000) {
        server.send(400, "text/plain", "Frequencia invalida");
        return;
    }

    if (duty > 100) {
        server.send(400, "text/plain", "Duty invalido");
        return;
    }

    applyLedPwm(freq, duty);
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
    applyLedPwm(ledFrequency, ledDutyPercent);

    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();

    Serial.print("IP da ESP32: ");
    Serial.println(IP);

    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.begin();

    Serial.println("Servidor HTTP iniciado para controle PWM do LED");
}

void loop() {
    server.handleClient();
}
