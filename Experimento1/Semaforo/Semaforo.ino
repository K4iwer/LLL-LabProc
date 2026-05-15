void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  fullCycle();
  // piscante();
}

void piscante() {
    // amarelo = vermelho + verde
  rgbLedWrite(LED_BUILTIN, 64, 64, 0);
  delay(1000);

  // apaga
  rgbLedWrite(LED_BUILTIN, 0, 0, 0);
  delay(1000);
}

void fullCycle() {
  // verde
  rgbLedWrite(LED_BUILTIN, 0, 128, 0);
  delay(3000);

  // amarelo = vermelho + verde
  rgbLedWrite(LED_BUILTIN, 64, 64, 0);
  delay(1000);

  // vermelho
  rgbLedWrite(LED_BUILTIN, 128, 0, 0);
  delay(4000);
}