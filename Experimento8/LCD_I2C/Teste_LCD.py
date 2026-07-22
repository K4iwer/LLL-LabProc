import time
from lcd_i2c import LcdI2c

# ---------------------------------------------------------------------------
# Teste ISOLADO do display LCD 16x2 via I2C (Experimento 9).
#
# Antes de rodar, habilite o I2C no Raspberry Pi (sudo raspi-config ->
# Interface Options -> I2C) e confirme o endereco do modulo com:
#       i2cdetect -y 1
# O endereco costuma ser 0x27 ou 0x3F. Ajuste abaixo se necessario.
# ---------------------------------------------------------------------------

ENDERECO_LCD = 0x27

if __name__ == "__main__":
    try:
        lcd = LcdI2c(endereco=ENDERECO_LCD)

        print("Teste isolado do LCD I2C. Ctrl+C para sair.")

        lcd.mensagem("Fechadura Elet.", "Iniciando...")
        time.sleep(2)

        lcd.mensagem("Digite a senha:", "____")
        time.sleep(2)

        lcd.mensagem("Acesso Liberado", ":)")
        time.sleep(2)

        lcd.mensagem("Acesso Negado", "Tente de novo")
        time.sleep(2)

        # Teste de contador dinamico
        for i in range(5, 0, -1):
            lcd.mensagem("Trancando em:", f"{i} s")
            time.sleep(1)

        lcd.limpar()
        print("Teste concluido.")

    except KeyboardInterrupt:
        print("\nTeste do LCD interrompido pelo usuario.")
