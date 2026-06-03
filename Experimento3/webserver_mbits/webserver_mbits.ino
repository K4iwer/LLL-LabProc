#include <WiFi.h>
#include <WebServer.h>

// Nome e senha do Wi-Fi
const char* ssid = "ESP32_do_grupinho_s";
const char* password = "12345678";

// Cria servidor HTTP na porta 80
WebServer server(80);

const int LED_BIT0 = 12;
const int LED_BIT1 = 13;
const int LED_BIT2 = 14;
const int LED_BIT3 = 15;

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
        <h1>Calculadora 16 bits</h1>

        <form action="/calcular" method="GET">

            <div>
                <label>Número A:</label><br>
                <input type="text" name="a" maxlength="16" minlength="16" pattern="[01]{16}" placeholder="Ex: 0000000000001010" required>
            </div>

            <div>
                <label>Número B:</label><br>
                <input type="text" name="b" maxlength="16" pattern="(^$)|([01]{16})" placeholder="Ex: Opcional">
            </div>

            <button type="submit" name="op" value="+">Add</button>
            <button type="submit" name="op" value="-">Sub</button>
            <div></div>
            <button type="submit" name="op" value="*">Mul</button>
            <button type="submit" name="op" value="/">Div</button>
            <div></div>
            <button type="submit" name="op" value="!">Fat</button>

        </form>
    </div> 
</body>
</html>
)rawliteral";

// Função chamada ao acessar "/"
void handleRoot() {
    server.send(200, "text/html", paginaHTML);
}

void escreverLeds(int valor) {
    uint8_t v = valor & 0x0F;

    digitalWrite(LED_BIT0, (v & 0x01) ? HIGH : LOW);
    digitalWrite(LED_BIT1, (v & 0x02) ? HIGH : LOW);
    digitalWrite(LED_BIT2, (v & 0x04) ? HIGH : LOW);
    digitalWrite(LED_BIT3, (v & 0x08) ? HIGH : LOW);
}

int fromBinary16Signed(String bin) {
    int v = strtol(bin.c_str(), NULL, 2);

    if (v >= 32768) {
        v -= 65536;
    }

    return v;
}

int CalculateFactorial(int n) {

    // overflow ou negativo
    if(n < 0 || n > 8) {
        return 99999;
    }

    int result = 1;

    for(;n > 1; n--) {
        result *= n;
    }

    return result;
}

// Função do cálculo
void handleCalculo() {

    String binA = server.arg("a");
    String binB = server.arg("b");
    String op   = server.arg("op");

    if(binB == "") {
        binB = "0";
    }

    // Converte binário -> inteiro
    int a = fromBinary16Signed(binA);
    int b = fromBinary16Signed(binB);

    int resultado;
    bool divisaoPorZero = false;

    unsigned long inicioOperacao = micros();

    if(op == "+") {
        resultado = a + b;
    } 
    else if (op == "-") {
        resultado = a - b;
    }
    else if (op == "*") {
        resultado = a * b;
    }
    else if (op == "/") {
        if (b == 0) {
            divisaoPorZero = true;
            resultado = 0;
        }
        else {
            resultado = a / b;
        }
    }
    else {
        resultado = CalculateFactorial(a);
    }

    unsigned long tempoOperacao = micros() - inicioOperacao;

    escreverLeds(resultado);
    
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

    if(divisaoPorZero || (op != "!" && (resultado > 32767 || resultado < -32768)) || (resultado > 65536)){
        String resultadoBin = toBinary16(resultado);
        
        resposta += R"rawliteral(
        <div class="container">
            <div class="alert">
        )rawliteral";

        if (divisaoPorZero) {
            resposta += "<h1>Divisao por zero!!!</h1>";
        }
        else {
            resposta += "<h1>Overflow ou Erro!!!</h1>";
        }

        resposta += R"rawliteral(
            </div>
        )rawliteral";

        resposta += "<h2>Binario: ";
        resposta += resultadoBin;
        resposta += "</h2>";

        resposta += "<h2>Tempo de operacao: ";
        resposta += String(tempoOperacao);
        resposta += " us</h2>";

        resposta += R"rawliteral(
        <br><a href='/'>Fazer outra operacao</a>
        </div>
        </body>
        </html>
        )rawliteral";

        server.send(400, "text/html", resposta);
        return;
    }

    // Resultado em binário
    String resultadoBin = toBinary16(resultado);

    resposta += "<div class=container><h1>Resultado</h1>";

    // Ajuste para que retorno fatorial
    if (op == "!") {
        op = "";
        binB = "";
    } 

    resposta += "<p>";
    resposta += binA + " " + op + " " + binB;
    resposta += "</p>";

    resposta += "<h2>Binario: ";
    resposta += resultadoBin;
    resposta += "</h2>";

    resposta += "<h2>Decimal: ";
    resposta += String(resultado);
    resposta += "</h2>";

    resposta += "<h2>Tempo de operacao: ";
    resposta += String(tempoOperacao);
    resposta += " us</h2>";

    resposta += "<br><a href='/'>Voltar</a></div>";
    resposta += "</body></html>";

    server.send(200, "text/html", resposta);
}

String toBinary16(int valor) {
    uint16_t v = valor & 0xFFFF;

    String s = "";
    for (int i = 15; i >= 0; i--) {
        s += (v & (1U << i)) ? "1" : "0";
    }

    return s;
}

void setup() {
    pinMode(LED_BIT0, OUTPUT);
    pinMode(LED_BIT1, OUTPUT);
    pinMode(LED_BIT2, OUTPUT);
    pinMode(LED_BIT3, OUTPUT);

    digitalWrite(LED_BIT0, LOW);
    digitalWrite(LED_BIT1, LOW);
    digitalWrite(LED_BIT2, LOW);
    digitalWrite(LED_BIT3, LOW);

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
