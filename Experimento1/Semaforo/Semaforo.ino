void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // amarelo = vermelho + verde
  rgbLedWrite(LED_BUILTIN, 64, 64, 0);
  delay(1000);

  // apaga
  rgbLedWrite(LED_BUILTIN, 0, 0, 0);
  delay(1000);
}