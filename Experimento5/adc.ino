#include <WiFi.h>
#include <WebServer.h>

// Nome e senha do Wi-Fi
const char* ssid = "ESP32_do_grupinho_s";
const char* password = "LLL";

// Cria servidor HTTP na porta 80
WebServer server(80);

// Define o pino do LDR (O GPIO 34 é excelente para ADC)
const int LDR_PIN = 34;

int valorLDRAtual = 0;

// Página HTML
String paginaHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Leitor de Sensor de Luminosidade</title>
    <style>
        body {
            font-family: Arial;
            background-color: #1e1e2f;
            color: white;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            background-color: #2b2b40;
            padding: 40px;
            border-radius: 20px;
            box-shadow: 0px 0px 20px rgba(0,0,0,0.3);
            text-align: center;
            min-width: 300px;
        }
        h1 { margin-bottom: 20px; font-size: 24px; }
        .valor-sensor {
            font-size: 48px;
            font-weight: bold;
            color: #4CAF50;
            margin: 20px 0;
            padding: 20px;
            background-color: #1e1e2f;
            border-radius: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Luminosidade Atual</h1>
        <div class="valor-sensor" id="valorLDR">--</div>
    </div> 

    <script>
        // Função para buscar o valor do LDR no ESP32
        function atualizarSensor() {
            fetch('/sensor')
                .then(response => response.text())
                .then(valor => {
                    document.getElementById('valorLDR').innerText = valor;
                })
                .catch(error => console.error('Erro ao ler sensor:', error));
        }

        setInterval(atualizarSensor, 1000);
        
        atualizarSensor();
    </script>
</body>
</html>
)rawliteral";

// Função chamada ao acessar "/"
void handleRoot() {
    server.send(200, "text/html", paginaHTML);
}

// Função chamada pelo JavaScript para entregar apenas o dado do ADC
void handleSensor() {
    server.send(200, "text/plain", String(valorLDRAtual)); // Retorna como texto simples
}

void setup() {
    Serial.begin(115200);

    // Configura os pinos
    pinMode(LED_BIT0, OUTPUT);
    pinMode(LED_BIT1, OUTPUT);
    pinMode(LED_BIT2, OUTPUT);
    pinMode(LED_BIT3, OUTPUT);

    // Access Point
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();

    Serial.print("IP da ESP32: ");
    Serial.println(IP);

    // Mapeamento das Rotas
    server.on("/", handleRoot);
    server.on("/sensor", handleSensor); // Rota do AJAX

    // Inicia servidor
    server.begin();
    Serial.println("Servidor HTTP iniciado");
}

void loop() {
    // Mantém o servidor escutando as requisições
    server.handleClient();

    // Não tenho certeza se isso conta como pooling
    valorLDRAtual = analogRead(LDR_PIN);
}