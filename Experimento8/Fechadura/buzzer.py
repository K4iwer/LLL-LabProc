import RPi.GPIO as GPIO
import time

# ---------------------------------------------------------------------------
# Modulo do buzzer para a fechadura integrada (Experimento 9).
# Espera que GPIO.setmode(GPIO.BCM) ja tenha sido chamado pelo programa
# principal. Fornece configurar() e os padroes de bip usados pela fechadura.
# ---------------------------------------------------------------------------

BUZZER_PIN = 12

_pwm = None


def configurar():
    global _pwm
    GPIO.setup(BUZZER_PIN, GPIO.OUT)
    _pwm = GPIO.PWM(BUZZER_PIN, 2000)
    _pwm.start(0)


def _tom(frequencia, duracao):
    _pwm.ChangeFrequency(frequencia)
    _pwm.ChangeDutyCycle(50)
    time.sleep(duracao)
    _pwm.ChangeDutyCycle(0)


def bip_tecla():
    """Bip curto de confirmacao de tecla."""
    _tom(1800, 0.03)


def bip_sucesso():
    """Dois bips agudos: acesso liberado."""
    _tom(2500, 0.08)
    time.sleep(0.05)
    _tom(2500, 0.08)


def bip_erro():
    """Bip longo e grave: acesso negado."""
    _tom(400, 0.6)


def bip_bloqueio():
    """Tres bips graves: sistema bloqueado por tentativas."""
    for _ in range(3):
        _tom(300, 0.25)
        time.sleep(0.1)


def liberar():
    if _pwm is not None:
        _pwm.stop()
