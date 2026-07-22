import RPi.GPIO as GPIO
import time

# ---------------------------------------------------------------------------
# Teste ISOLADO do teclado matricial 4x4 (Experimento 9).
#
# O teclado 4x4 possui 4 linhas e 4 colunas. A varredura funciona assim:
#   1. Todas as linhas comecam em nivel ALTO.
#   2. Uma linha por vez e colocada em nivel BAIXO.
#   3. Le-se cada coluna; a coluna que estiver em BAIXO indica a tecla
#      pressionada no cruzamento (linha, coluna).
#
# As colunas usam resistor de pull-up interno, entao ficam em ALTO enquanto
# nenhuma tecla e pressionada.
# ---------------------------------------------------------------------------

# --- MAPEAMENTO DOS PINOS (numeracao BCM) ---
LINHAS = [5, 6, 13, 19]     # saidas
COLUNAS = [26, 21, 20, 16]  # entradas (pull-up)

# --- LAYOUT FISICO DO TECLADO ---
TECLAS = [
    ["1", "2", "3", "A"],
    ["4", "5", "6", "B"],
    ["7", "8", "9", "C"],
    ["*", "0", "#", "D"],
]


def configurar():
    """Configura linhas como saida e colunas como entrada com pull-up."""
    GPIO.setmode(GPIO.BCM)
    for linha in LINHAS:
        GPIO.setup(linha, GPIO.OUT)
        GPIO.output(linha, GPIO.HIGH)
    for coluna in COLUNAS:
        GPIO.setup(coluna, GPIO.IN, pull_up_down=GPIO.PUD_UP)


def ler_tecla():
    """Executa uma varredura e retorna a tecla pressionada ou None."""
    tecla_lida = None
    for i, linha in enumerate(LINHAS):
        GPIO.output(linha, GPIO.LOW)  # ativa a linha atual
        for j, coluna in enumerate(COLUNAS):
            if GPIO.input(coluna) == GPIO.LOW:  # coluna aterrada -> tecla
                tecla_lida = TECLAS[i][j]
        GPIO.output(linha, GPIO.HIGH)  # desativa a linha
    return tecla_lida


def ler_tecla_debounce():
    """Le uma tecla tratando repique (debounce) e soltura da tecla."""
    tecla = ler_tecla()
    if tecla is not None:
        time.sleep(0.03)              # aguarda estabilizar o contato
        if ler_tecla() == tecla:      # confirma a leitura
            # espera o usuario soltar a tecla para nao repetir
            while ler_tecla() is not None:
                time.sleep(0.02)
            return tecla
    return None


if __name__ == "__main__":
    try:
        configurar()
        print("Teste isolado do teclado matricial 4x4.")
        print("Pressione as teclas (Ctrl+C para sair).")
        while True:
            tecla = ler_tecla_debounce()
            if tecla is not None:
                print(f"Tecla pressionada: {tecla}")
            time.sleep(0.02)

    except KeyboardInterrupt:
        print("\nTeste do teclado interrompido pelo usuario.")

    finally:
        GPIO.cleanup()
        print("Pinos GPIO liberados.")
