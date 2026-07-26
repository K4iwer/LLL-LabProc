import RPi.GPIO as GPIO
import time

# --- MAPEAMENTO DO PINO (numeracao BCM) ---
BUZZER_PIN = 12

# --- CONFIGURACAO INICIAL DA PLACA ---
GPIO.setmode(GPIO.BCM)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

# O PWM e criado uma unica vez e reaproveitado; a frequencia e alterada
# em tempo de execucao para gerar tons diferentes.
pwm_buzzer = GPIO.PWM(BUZZER_PIN, 2000)
pwm_buzzer.start(0)


def emitir_tom(frequencia, duracao):
    """Emite um tom com a frequencia (Hz) e duracao (s) informadas."""
    pwm_buzzer.ChangeFrequency(frequencia)
    pwm_buzzer.ChangeDutyCycle(50)  # 50% -> onda quadrada, volume maximo
    time.sleep(duracao)
    pwm_buzzer.ChangeDutyCycle(0)   # silencia o buzzer


def bip_sucesso():
    """Dois bips curtos e agudos: acesso liberado."""
    emitir_tom(2500, 0.08)
    time.sleep(0.05)
    emitir_tom(2500, 0.08)


def bip_erro():
    """Um bip longo e grave: acesso negado."""
    emitir_tom(400, 0.6)


def bip_tecla():
    """Bip muito curto de confirmacao de tecla pressionada."""
    emitir_tom(1800, 0.03)


if __name__ == "__main__":
    try:
        print("Teste isolado do buzzer. Ctrl+C para sair.")
        while True:
            print("-> bip de tecla")
            bip_tecla()
            time.sleep(1)

            print("-> bip de SUCESSO")
            bip_sucesso()
            time.sleep(1)

            print("-> bip de ERRO")
            bip_erro()
            time.sleep(1.5)

    except KeyboardInterrupt:
        print("\nTeste do buzzer interrompido pelo usuario.")

    finally:
        pwm_buzzer.stop()
        GPIO.cleanup()
        print("Pinos GPIO liberados.")
