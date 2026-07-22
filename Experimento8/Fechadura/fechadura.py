import hashlib
import time
import RPi.GPIO as GPIO

import buzzer
import teclado
import sensor
import tranca
from lcd_i2c import LcdI2c

# ===========================================================================
#  FECHADURA ELETRONICA - INTEGRACAO COMPLETA (Experimento 9)
#
#  Raspberry Pi 3 controlando:
#    - Teclado matricial 4x4  -> entrada da senha            (teclado.py)
#    - Display LCD 16x2 I2C    -> feedback visual             (lcd_i2c.py)
#    - Buzzer PWM              -> feedback sonoro             (buzzer.py)
#    - Sensor ultrassonico     -> verifica se a porta fechou  (sensor.py)
#    - Servomotor (tranca)     -> ferrolho da fechadura       (tranca.py)
#
#  Convencoes do teclado:
#    0-9 -> digitos da senha
#     #  -> confirma a senha digitada
#     *  -> apaga a senha digitada (limpa)
#     A  -> tranca manualmente
#
#  A senha NAO fica em texto puro no codigo: guarda-se apenas o hash SHA-256
#  (ver diagrama de sequencia em Experimento8/README.md). A senha digitada e
#  convertida em hash e comparada com o hash armazenado.
# ===========================================================================

# Hash SHA-256 da senha padrao "1234".
# Para gerar outro hash:
#   python3 -c "import hashlib;print(hashlib.sha256(b'NOVA_SENHA').hexdigest())"
SENHA_HASH = "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4"

ENDERECO_LCD = 0x27      # confirme com: i2cdetect -y 1

TAMANHO_MAX_SENHA = 8    # limite de digitos aceitos
MAX_TENTATIVAS = 3       # tentativas antes do bloqueio
TEMPO_BLOQUEIO = 30      # segundos de bloqueio apos exceder tentativas
TEMPO_ABERTA = 5         # segundos que a tranca fica destravada
INTERVALO_SENSOR = 1.0   # periodo de checagem de arrombamento (s)


def hash_senha(texto):
    """Retorna o hash SHA-256 (hexadecimal) da senha informada."""
    return hashlib.sha256(texto.encode()).hexdigest()


def mostrar_senha(lcd, buffer):
    """Atualiza o LCD mascarando os digitos com '*'."""
    lcd.mensagem("Digite a senha:", "*" * len(buffer))


def acesso_liberado(lcd):
    """Sequencia executada quando a senha esta correta."""
    print("[OK] Senha correta - acesso liberado.")
    buzzer.bip_sucesso()
    lcd.mensagem("Acesso Liberado", ":)")
    tranca.destravar()

    # Mantem destravada por alguns segundos, mostrando contagem regressiva.
    for restante in range(TEMPO_ABERTA, 0, -1):
        lcd.mensagem("Porta Aberta", f"Trancando: {restante}s")
        time.sleep(1)

    tranca.trancar()

    # Confirma pelo sensor que a porta realmente fechou antes de armar.
    if sensor.porta_fechada():
        lcd.mensagem("Sistema Travado", "Porta OK")
    else:
        lcd.mensagem("ATENCAO", "Feche a porta!")
        buzzer.bip_erro()
    time.sleep(1.5)


def acesso_negado(lcd, tentativas):
    """Sequencia executada quando a senha esta incorreta."""
    restantes = MAX_TENTATIVAS - tentativas
    print(f"[X] Senha incorreta. Tentativas restantes: {restantes}")
    buzzer.bip_erro()
    lcd.mensagem("Acesso Negado", f"Restam: {restantes}")
    time.sleep(1.5)


def bloquear(lcd):
    """Bloqueia o teclado por TEMPO_BLOQUEIO segundos apos exceder tentativas."""
    print("[!] Numero maximo de tentativas atingido. Sistema bloqueado.")
    buzzer.bip_bloqueio()
    for restante in range(TEMPO_BLOQUEIO, 0, -1):
        lcd.mensagem("BLOQUEADO", f"Aguarde: {restante}s")
        time.sleep(1)


def verificar_arrombamento(lcd):
    """Se a tranca esta armada mas o sensor ve a porta aberta, dispara alarme."""
    if tranca.esta_trancada() and not sensor.porta_fechada():
        print("[ALARME] Porta aberta com a fechadura travada!")
        lcd.mensagem("!! ALARME !!", "Porta forcada")
        buzzer.bip_bloqueio()
        time.sleep(1)
        return True
    return False


def main():
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)

    # Inicializa cada subsistema (ordem indiferente; todos usam BCM).
    lcd = LcdI2c(endereco=ENDERECO_LCD)
    buzzer.configurar()
    teclado.configurar()
    sensor.configurar()
    tranca.configurar()

    buffer = ""
    tentativas = 0
    ultimo_check = time.time()

    print("Fechadura eletronica iniciada. Ctrl+C para encerrar.")
    lcd.mensagem("Fechadura Elet.", "Digite a senha:")

    try:
        while True:
            # Verificacao periodica de arrombamento (nao a cada iteracao para
            # nao sobrecarregar o loop nem o sensor).
            if time.time() - ultimo_check >= INTERVALO_SENSOR:
                ultimo_check = time.time()
                if verificar_arrombamento(lcd):
                    buffer = ""
                    mostrar_senha(lcd, buffer)

            tecla = teclado.ler_tecla()
            if tecla is None:
                time.sleep(0.02)
                continue

            buzzer.bip_tecla()

            if tecla.isdigit():
                if len(buffer) < TAMANHO_MAX_SENHA:
                    buffer += tecla
                    mostrar_senha(lcd, buffer)

            elif tecla == "*":  # limpar
                buffer = ""
                mostrar_senha(lcd, buffer)

            elif tecla == "#":  # confirmar
                if hash_senha(buffer) == SENHA_HASH:
                    tentativas = 0
                    acesso_liberado(lcd)
                else:
                    tentativas += 1
                    if tentativas >= MAX_TENTATIVAS:
                        bloquear(lcd)
                        tentativas = 0
                    else:
                        acesso_negado(lcd, tentativas)
                buffer = ""
                lcd.mensagem("Fechadura Elet.", "Digite a senha:")

            elif tecla == "A":  # tranca manual
                tranca.trancar()
                lcd.mensagem("Trancado", "manualmente")
                time.sleep(1)
                buffer = ""
                lcd.mensagem("Fechadura Elet.", "Digite a senha:")

    except KeyboardInterrupt:
        print("\nFechadura encerrada pelo usuario.")

    finally:
        lcd.limpar()
        lcd.mensagem("Sistema", "Desligado")
        buzzer.liberar()
        tranca.liberar()
        GPIO.cleanup()
        print("Pinos GPIO liberados.")


if __name__ == "__main__":
    main()
