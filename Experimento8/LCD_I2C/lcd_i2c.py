import smbus2
import time

# ---------------------------------------------------------------------------
# Driver minimo para display LCD 16x2 (HD44780) via expansor I2C PCF8574.
#
# Este backpack I2C conecta os 8 bits do PCF8574 ao LCD da seguinte forma:
#   P0 -> RS      P1 -> RW      P2 -> EN      P3 -> luz de fundo
#   P4..P7 -> D4..D7 (o LCD opera em modo de 4 bits)
#
# Referencia da pinagem do backpack: datasheet do PCF8574 (NXP) e do
# controlador HD44780 (Hitachi).
# ---------------------------------------------------------------------------

# Bits de controle no barramento do PCF8574
LCD_CHR = 1          # envio de dado/caractere (RS = 1)
LCD_CMD = 0          # envio de comando       (RS = 0)
ENABLE = 0b00000100  # bit EN (P2)
LUZ_FUNDO = 0b00001000  # bit da luz de fundo (P3)

# Enderecos das duas linhas na memoria DDRAM do HD44780
LINHA_1 = 0x80
LINHA_2 = 0xC0


class LcdI2c:
    def __init__(self, endereco=0x27, barramento=1, colunas=16):
        self.endereco = endereco
        self.bus = smbus2.SMBus(barramento)
        self.colunas = colunas
        self._inicializar()

    def _escrever_byte(self, dado):
        """Escreve um byte cru no PCF8574 (ja com a luz de fundo ligada)."""
        self.bus.write_byte(self.endereco, dado | LUZ_FUNDO)

    def _pulso_enable(self, dado):
        """Gera o pulso no pino EN para o LCD registrar os 4 bits."""
        time.sleep(0.0005)
        self._escrever_byte(dado | ENABLE)
        time.sleep(0.0005)
        self._escrever_byte(dado & ~ENABLE)
        time.sleep(0.0001)

    def _enviar(self, valor, modo):
        """Envia um byte ao LCD em dois nibbles de 4 bits (nibble alto primeiro)."""
        alto = modo | (valor & 0xF0)
        baixo = modo | ((valor << 4) & 0xF0)
        for nibble in (alto, baixo):
            self._escrever_byte(nibble)
            self._pulso_enable(nibble)

    def _inicializar(self):
        """Sequencia de inicializacao do HD44780 em modo de 4 bits."""
        for comando in (0x33, 0x32, 0x06, 0x0C, 0x28, 0x01):
            self._enviar(comando, LCD_CMD)
        time.sleep(0.005)

    def limpar(self):
        """Apaga o display."""
        self._enviar(0x01, LCD_CMD)
        time.sleep(0.005)

    def escrever(self, texto, linha=1):
        """Escreve uma string em uma das linhas (1 ou 2), truncando/preenchendo."""
        endereco = LINHA_1 if linha == 1 else LINHA_2
        self._enviar(endereco, LCD_CMD)
        texto = texto.ljust(self.colunas)[: self.colunas]
        for caractere in texto:
            self._enviar(ord(caractere), LCD_CHR)

    def mensagem(self, linha1="", linha2=""):
        """Atalho para escrever as duas linhas de uma vez."""
        self.escrever(linha1, 1)
        self.escrever(linha2, 2)


if __name__ == "__main__":
    # Pequena demonstracao ao executar o proprio driver.
    lcd = LcdI2c()
    lcd.mensagem("LCD I2C OK", "Experimento 9")
    time.sleep(3)
    lcd.limpar()
