import RPi.GPIO as GPIO
import time

# ---------------------------------------------------------------------------
# Modulo do atuador da tranca (servomotor) para a fechadura integrada
# (Experimento 9). Espera GPIO.setmode(GPIO.BCM) chamado pelo principal.
# ---------------------------------------------------------------------------

SERVO_PIN = 18

DUTY_TRANCADA = 2.5    # ~0 graus
DUTY_DESTRAVADA = 7.5  # ~90 graus

_pwm = None
_trancada = True


def configurar():
    global _pwm
    GPIO.setup(SERVO_PIN, GPIO.OUT)
    _pwm = GPIO.PWM(SERVO_PIN, 50)
    _pwm.start(0)
    trancar()  # comeca sempre trancada


def _mover(duty):
    _pwm.ChangeDutyCycle(duty)
    time.sleep(0.5)
    _pwm.ChangeDutyCycle(0)  # relaxa o servo (evita tremor/ruido)


def trancar():
    global _trancada
    _mover(DUTY_TRANCADA)
    _trancada = True


def destravar():
    global _trancada
    _mover(DUTY_DESTRAVADA)
    _trancada = False


def esta_trancada():
    return _trancada


def liberar():
    if _pwm is not None:
        _pwm.stop()
