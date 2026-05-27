const int LED_BIT0 = 9;
const int LED_BIT1 = 8;
const int LED_BIT2 = 7;
const int LED_BIT3 = 6;

void escreverLeds(String valor) {

    digitalWrite(LED_BIT0, (valor[3] == '1') ? HIGH : LOW);
    digitalWrite(LED_BIT1, (valor[2] == '1')  ? HIGH : LOW);
    digitalWrite(LED_BIT2, (valor[1] == '1')  ? HIGH : LOW);
    digitalWrite(LED_BIT3, (valor[0] == '1')  ? HIGH : LOW);
}

int fromBinary4OnesComplement(String bin) {
    int v = strtol(bin.c_str(), NULL, 2);

    // é -0
    if (bin == "1111") {
        return 0;
    }

    // é positivo
    if (bin[0] == '0') {
        return v;
    }
    // é negativo
    v = (~v) & 0b1111; //inverte todos bits e pega os ultimos 4

    return -v;
}

String toBinary4(int valor) {
  // v tá em complemento de 2, pq e assim que a ESP guarda int
    int v = valor & 0b1111;

    int v_final;
    if (valor >= 0) {
      // pra positivos, complemento de 2 é igual a 1
      v_final = v;
    }
    else{
      v_final = (~(-v)) & 0b1111;
    }

    String s = "";

    for (int i = 3; i >= 0; i--) {
        s += (v_final & (1 << i)) ? "1" : "0";
    }

    return s;
}

String sum(String a, String b) {
  // Converte os caracteres ASCII ('0' ou '1') para os inteiros binários 0 ou 1
  int a3 = a[0] - '0', b3 = b[0] - '0';
  int a2 = a[1] - '0', b2 = b[1] - '0';
  int a1 = a[2] - '0', b1 = b[2] - '0';
  int a0 = a[3] - '0', b0 = b[3] - '0';

  int c0 = 0, c1 = 0, c2 = 0, c3 = 0;
  int r0, r1, r2, r3;

  r0 = (a0 ^ b0) & 1; 
  c0 = a0 & b0;

  r1 = (a1 ^ b1 ^ c0) & 1;
  c1 = (a1 + b1 + c0 >= 2) ? 1 : 0;

  r2 = (a2 ^ b2 ^ c1) & 1;
  c2 = (a2 + b2 + c1 >= 2) ? 1 : 0;

  r3 = (a3 ^ b3 ^ c2) & 1;
  c3 = (a3 + b3 + c2 >= 2) ? 1 : 0;

  String resultado = String(r3) + String(r2) + String(r1) + String(r0);

  if (c3 == 1) {
    return sum(resultado, "0001");
  } else {
    return resultado; 
  }
}

void invertComplementOne(String &bin) {
  for (int i = 0; i < 4; i++) {
    bin[i] = (bin[i] == '1') ? '0' : '1';
  }
}

void handleCalculo(String entrada) {

    entrada.trim();

    // Espera formato [4bits]op[4bits]
    // 1010+0011

    String binA = entrada.substring(0, 4);
    char op = entrada.charAt(4);
    String binB = entrada.substring(5, 9);

    String resultado;

    if (op == '+') {
      resultado = sum(binA, binB);
    }
    else {
      invertComplementOne(binB);
      resultado = sum(binA, binB);
    }

    escreverLeds(resultado);

    Serial.println("------------");
    Serial.print("A: ");
    Serial.println(binA);

    Serial.print("B: ");
    if (op=='-') {
      invertComplementOne(binB);
    }

    Serial.println(binB);

    Serial.print("Op: ");
    Serial.println(op);

    Serial.print("Resultado binario: ");
    Serial.println(resultado);

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

        handleCalculo(entrada);
    }
}