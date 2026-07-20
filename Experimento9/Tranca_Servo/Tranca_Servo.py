import RPi.GPIO as GPIO
import time

# ---------------------------------------------------------------------------
# Teste ISOLADO do atuador da tranca (Experimento 9).
#
# A tranca da fechadura e um servomotor SG90 que gira um ferrolho:
#   - posicao TRANCADA  -> 0 graus   (duty ~2,5 %)
#   - posicao DESTRAVADA-> 90 graus  (duty ~7,5 %)
#
# O servo e controlado por PWM de 50 Hz (periodo de 20 ms). A largura do
# pulso define o angulo: ~0,5 ms = 0 graus, ~2,5 ms = 180 graus.
#
# Obs.: o mesmo esquema pode ser trocado por um rele acionando um solenoide
# de fechadura 12 V; nesse caso basta usar GPIO.output(PINO, HIGH/LOW).
# ---------------------------------------------------------------------------

# --- MAPEAMENTO DO PINO (numeracao BCM) ---
SERVO_PIN = 18

# Ciclos de trabalho (duty cycle) para cada posicao
DUTY_TRANCADA = 2.5    # ~0 graus
DUTY_DESTRAVADA = 7.5  # ~90 graus

GPIO.setmode(GPIO.BCM)
GPIO.setup(SERVO_PIN, GPIO.OUT)

pwm_servo = GPIO.PWM(SERVO_PIN, 50)  # 50 Hz
pwm_servo.start(0)


def _mover(duty):
    """Move o servo ate o duty informado e depois relaxa (evita tremor)."""
    pwm_servo.ChangeDutyCycle(duty)
    time.sleep(0.5)            # tempo para o servo chegar a posicao
    pwm_servo.ChangeDutyCycle(0)  # para de enviar pulsos -> servo silencioso


def trancar():
    print("Tranca -> TRANCADA")
    _mover(DUTY_TRANCADA)


def destravar():
    print("Tranca -> DESTRAVADA")
    _mover(DUTY_DESTRAVADA)


if __name__ == "__main__":
    try:
        print("Teste isolado da tranca (servo). Ctrl+C para sair.")
        while True:
            destravar()
            time.sleep(2)
            trancar()
            time.sleep(2)

    except KeyboardInterrupt:
        print("\nTeste da tranca interrompido pelo usuario.")

    finally:
        pwm_servo.stop()
        GPIO.cleanup()
        print("Pinos GPIO liberados.")
