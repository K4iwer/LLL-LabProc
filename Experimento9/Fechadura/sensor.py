import RPi.GPIO as GPIO
import time

# ---------------------------------------------------------------------------
# Modulo do sensor ultrassonico HC-SR04 para a fechadura integrada
# (Experimento 9). Detecta se a porta esta FECHADA (objeto proximo).
# Espera que GPIO.setmode(GPIO.BCM) ja tenha sido chamado pelo principal.
#
# ATENCAO: use divisor de tensao no pino ECHO (5 V -> 3,3 V).
# ---------------------------------------------------------------------------

TRIG_PIN = 23
ECHO_PIN = 24

LIMIAR_FECHADA_CM = 8.0
TIMEOUT_S = 0.05


def configurar():
    GPIO.setup(TRIG_PIN, GPIO.OUT)
    GPIO.setup(ECHO_PIN, GPIO.IN)
    GPIO.output(TRIG_PIN, GPIO.LOW)
    time.sleep(0.1)


def medir_distancia():
    """Distancia em cm, ou None em caso de timeout."""
    GPIO.output(TRIG_PIN, GPIO.HIGH)
    time.sleep(0.00001)
    GPIO.output(TRIG_PIN, GPIO.LOW)

    inicio = time.time()
    while GPIO.input(ECHO_PIN) == GPIO.LOW:
        if time.time() - inicio > TIMEOUT_S:
            return None
    t_inicio = time.time()

    inicio = time.time()
    while GPIO.input(ECHO_PIN) == GPIO.HIGH:
        if time.time() - inicio > TIMEOUT_S:
            return None
    t_fim = time.time()

    return (t_fim - t_inicio) * 34300 / 2


def porta_fechada():
    """True se a porta esta fechada (distancia dentro do limiar)."""
    distancia = medir_distancia()
    if distancia is None:
        return False
    return distancia <= LIMIAR_FECHADA_CM
