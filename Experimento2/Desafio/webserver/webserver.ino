const int LED_BIT0 = 13;
const int LED_BIT1 = 12;
const int LED_BIT2 = 14;
const int LED_BIT3 = 27;

void escreverLeds(int valor) {
    uint8_t v = valor & 0x0F;

    digitalWrite(LED_BIT0, (v & 0x01) ? HIGH : LOW);
    digitalWrite(LED_BIT1, (v & 0x02) ? HIGH : LOW);
    digitalWrite(LED_BIT2, (v & 0x04) ? HIGH : LOW);
    digitalWrite(LED_BIT3, (v & 0x08) ? HIGH : LOW);
}

int fromBinary4Signed(String bin) {
    int v = strtol(bin.c_str(), NULL, 2);

    if (v >= 8) {
        v -= 16;
    }

    return v;
}

String toBinary4(int valor) {
    uint8_t v = valor & 0b1111;

    String s = "";

    for (int i = 3; i >= 0; i--) {
        s += (v & (1 << i)) ? "1" : "0";
    }

    return s;
}

void handleCalculo(String entrada) {

    entrada.trim();

    // Espera formato [4bits]op[4bits]
    // 1010+0011

    String binA = entrada.substring(0, 4);
    char op = entrada.charAt(5);
    String binB = entrada.substring(7, 11);

    int a = fromBinary4Signed(binA);
    int b = fromBinary4Signed(binB);

    int resultado;

    if (op == '+') {
        resultado = a + b;
    }
    else {
        resultado = a - b;
    }

    escreverLeds(resultado);

    Serial.println("------------");
    Serial.print("A: ");
    Serial.println(a);

    Serial.print("B: ");
    Serial.println(b);

    Serial.print("Resultado decimal: ");
    Serial.println(resultado);

    Serial.print("Resultado binario: ");
    Serial.println(toBinary4(resultado));

    if(resultado > 7 || resultado < -8) {
        Serial.println("OVERFLOW!");
    }

    Serial.println("------------");
}

void setup() {

    pinMode(LED_BIT0, OUTPUT);
    pinMode(LED_BIT1, OUTPUT);
    pinMode(LED_BIT2, OUTPUT);
    pinMode(LED_BIT3, OUTPUT);

    Serial.begin(115200);

    Serial.println("Calculadora binaria pronta!");
    Serial.println("Digite algo como:");
    Serial.println("1010 + 0011");
}

void loop() {

    if (Serial.available()) {

        String entrada = Serial.readStringUntil('\n');

        calcular(entrada);
    }
}