import RPi.GPIO as GPIO
import time

# ---------------------------------------------------------------------------
# Teste ISOLADO do sensor ultrassonico HC-SR04 (Experimento 9).
#
# O sensor mede a distancia ate a porta/gaveta para inferir se a fechadura
# esta FECHADA (objeto proximo) ou ABERTA (objeto distante). Funcionamento:
#   1. Envia um pulso de 10 us no pino TRIG.
#   2. O sensor emite 8 ciclos ultrassonicos e coloca ECHO em ALTO.
#   3. ECHO permanece em ALTO pelo tempo de ida e volta do som.
#   4. distancia_cm = tempo_echo * 34300 / 2   (velocidade do som ~343 m/s)
#
# ATENCAO: o pino ECHO opera em 5 V. Use um divisor de tensao (ex.: 1 kOhm +
# 2 kOhm) para reduzir a ~3,3 V antes de ligar ao GPIO do Raspberry Pi.
# ---------------------------------------------------------------------------

# --- MAPEAMENTO DOS PINOS (numeracao BCM) ---
TRIG_PIN = 14
ECHO_PIN = 15

# Distancia (cm) abaixo da qual consideramos a porta FECHADA/TRANCADA
LIMIAR_FECHADA_CM = 8.0

# Tempo maximo de espera pelo eco, para nao travar se o sensor falhar
TIMEOUT_S = 0.05


def configurar():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(TRIG_PIN, GPIO.OUT)
    GPIO.setup(ECHO_PIN, GPIO.IN)
    GPIO.output(TRIG_PIN, GPIO.LOW)
    time.sleep(0.1)  # estabiliza o sensor


def medir_distancia():
    """Retorna a distancia em cm, ou None em caso de timeout."""
    # Pulso de disparo de 10 us
    GPIO.output(TRIG_PIN, GPIO.HIGH)
    time.sleep(0.00001)
    GPIO.output(TRIG_PIN, GPIO.LOW)

    # Aguarda ECHO subir (inicio do eco)
    inicio_espera = time.time()
    while GPIO.input(ECHO_PIN) == GPIO.LOW:
        if time.time() - inicio_espera > TIMEOUT_S:
            return None
    t_inicio = time.time()

    # Aguarda ECHO descer (fim do eco)
    inicio_espera = time.time()
    while GPIO.input(ECHO_PIN) == GPIO.HIGH:
        if time.time() - inicio_espera > TIMEOUT_S:
            return None
    t_fim = time.time()

    duracao = t_fim - t_inicio
    distancia = duracao * 34300 / 2  # cm
    return distancia


def porta_fechada():
    """True se a porta esta fechada (objeto dentro do limiar)."""
    distancia = medir_distancia()
    if distancia is None:
        return False
    return distancia <= LIMIAR_FECHADA_CM


if __name__ == "__main__":
    try:
        configurar()
        print("Teste isolado do sensor ultrassonico HC-SR04. Ctrl+C para sair.")
        while True:
            distancia = medir_distancia()
            if distancia is None:
                print("Falha na leitura (timeout).")
            else:
                estado = "FECHADA" if distancia <= LIMIAR_FECHADA_CM else "ABERTA"
                print(f"Distancia: {distancia:5.1f} cm  ->  Porta {estado}")
            time.sleep(0.5)

    except KeyboardInterrupt:
        print("\nTeste do sensor interrompido pelo usuario.")

    finally:
        GPIO.cleanup()
        print("Pinos GPIO liberados.")
