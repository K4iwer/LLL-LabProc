#include <WiFi.h>
#include <WebServer.h>

// Nome e senha do Wi-Fi
const char* ssid = "ESP32_do_grupinho_8";
const char* password = "LLLrebolando";

// Cria servidor HTTP na porta 80
WebServer server(80);

// Página HTML
String paginaHTML = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <meta charset="UTF-8">
    <title>Calculadora Binária</title>

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
        }

        h1 {
            margin-bottom: 30px;
        }

        input {
            font-size: 20px;

            padding: 10px;

            margin: 10px;

            width: 150px;

            text-align: center;

            border: none;

            border-radius: 10px;
        }

        button {

            font-size: 20px;

            padding: 10px 20px;

            margin: 10px;

            border: none;

            border-radius: 10px;

            cursor: pointer;

            transition: 0.2s;
        }

        button:hover {
            transform: scale(1.05);
        }
    </style>
</head>

<body>
    <div class="container">
        <h1>Calculadora 4 bits</h1>

        <form action="/calcular" method="GET">

            <div>
                <label>Número A:</label><br>
                <input type="text" name="a" maxlength="4" placeholder="Ex: 1010">
            </div>

            <div>
                <label>Número B:</label><br>
                <input type="text" name="b" maxlength="4" placeholder="Ex: 0011">
            </div>

            <button type="submit" name="op" value="+">Add</button>
            <button type="submit" name="op" value="-">Sub</button>

        </form>
    </div> 
</body>
</html>
)rawliteral";

// Função chamada ao acessar "/"
void handleRoot() {
    server.send(200, "text/html", paginaHTML);
}

// Função do cálculo
void handleCalculo() {

    String binA = server.arg("a");
    String binB = server.arg("b");
    String op   = server.arg("op");

    // Converte binário -> inteiro
    int a = strtol(binA.c_str(), NULL, 2);
    int b = strtol(binB.c_str(), NULL, 2);

    int resultado;

    if(op == "+") {
        resultado = a + b;
    }
    else {
        resultado = a - b;
    }

    String resposta = R"rawliteral(
        <!DOCTYPE html>
        <html>

        <head>
            <meta charset="UTF-8">

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
            }

            h1 {
                margin-bottom: 30px;
            }

            input {
                font-size: 20px;

                padding: 10px;

                margin: 10px;

                width: 150px;

                text-align: center;

                border: none;

                border-radius: 10px;
            }

            button {

                font-size: 20px;

                padding: 10px 20px;

                margin: 10px;

                border: none;

                border-radius: 10px;

                cursor: pointer;

                transition: 0.2s;
            }

            button:hover {
                transform: scale(1.05);
            }
        </style>

        </head>

        <body>
        )rawliteral";

    if(resultado > 7 || resultado < -8){
        resposta += R"rawliteral(
        <div class="container">
            <div class=alert>
                <h1>Overflow!</h1>
            </div>
            <br><a href='/'>Fazer outra operacao</a>
        )rawliteral";
        server.send(400, "text/html", resposta);
    }

    // Resultado em binário
    String resultadoBin = String(resultado, BIN);

    resposta += "<div class=container><h1>Resultado</h1>";

    resposta += "<p>";
    resposta += binA + " " + op + " " + binB;
    resposta += "</p>";

    resposta += "<h2>Binario: ";
    resposta += resultadoBin;
    resposta += "</h2>";

    resposta += "<h2>Decimal: ";
    resposta += String(resultado);
    resposta += "</h2>";

    resposta += "<br><a href='/'>Voltar</a><\div>";

    server.send(200, "text/html", resposta);
}


void setup() {
    Serial.begin(115200);

    // Cria Access Point
    WiFi.softAP(ssid, password);

    // IP da ESP32
    IPAddress IP = WiFi.softAPIP();

    Serial.print("IP da ESP32: ");
    Serial.println(IP);

    // Rota principal
    server.on("/", handleRoot);

    // Receber dados do formulário
    server.on("/calcular", handleCalculo);

    // Inicia servidor
    server.begin();

    Serial.println("Servidor HTTP iniciado");
}

void loop() {
    server.handleClient();
}