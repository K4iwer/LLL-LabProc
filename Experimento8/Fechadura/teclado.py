import RPi.GPIO as GPIO
import time

# ---------------------------------------------------------------------------
# Modulo do teclado matricial 4x4 para a fechadura integrada (Experimento 9).
# Espera que GPIO.setmode(GPIO.BCM) ja tenha sido chamado pelo programa
# principal. Fornece configurar() e ler_tecla() com tratamento de debounce.
# ---------------------------------------------------------------------------

LINHAS = [5, 6, 13, 19]     # saidas
COLUNAS = [26, 21, 20, 16]  # entradas (pull-up)

TECLAS = [
    ["1", "2", "3", "A"],
    ["4", "5", "6", "B"],
    ["7", "8", "9", "C"],
    ["*", "0", "#", "D"],
]


def configurar():
    for linha in LINHAS:
        GPIO.setup(linha, GPIO.OUT)
        GPIO.output(linha, GPIO.HIGH)
    for coluna in COLUNAS:
        GPIO.setup(coluna, GPIO.IN, pull_up_down=GPIO.PUD_UP)


def _varrer():
    """Uma varredura das linhas; retorna a tecla pressionada ou None."""
    tecla = None
    for i, linha in enumerate(LINHAS):
        GPIO.output(linha, GPIO.LOW)
        for j, coluna in enumerate(COLUNAS):
            if GPIO.input(coluna) == GPIO.LOW:
                tecla = TECLAS[i][j]
        GPIO.output(linha, GPIO.HIGH)
    return tecla


def ler_tecla():
    """Retorna uma tecla ja com debounce e espera de soltura, ou None."""
    tecla = _varrer()
    if tecla is not None:
        time.sleep(0.03)
        if _varrer() == tecla:
            while _varrer() is not None:
                time.sleep(0.02)
            return tecla
    return None
