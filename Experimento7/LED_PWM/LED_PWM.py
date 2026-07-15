import RPi.GPIO as GPIO
import time

# Definição do pino do LED (usando a numeração BCM)
LED_PIN = 17

# Configuração inicial da placa
GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)

# Inicializa o PWM no pino do LED com uma frequência inicial de 100 Hz
# e define o ciclo de trabalho (Duty Cycle) inicial em 0% (apagado)
pwm_led = GPIO.PWM(LED_PIN, 100)
pwm_led.start(0)

# Lista de frequências (em Hz)
frequencias = [10, 50, 100, 500, 1000]

try:
    print("Iniciando o teste de frequências com RPi.GPIO.")
    
    # Loop principal para rodar o teste continuamente
    while True:
        for freq in frequencias:
            print(f"Testando Frequência atual: {freq} Hz")
            
            pwm_led.ChangeFrequency(freq)
            
            for duty in range(0, 101, 5):
                pwm_led.ChangeDutyCycle(duty)
                time.sleep(0.04)
                
            for duty in range(100, -1, -5):
                pwm_led.ChangeDutyCycle(duty)
                time.sleep(0.04)
                
            print(f"Fim do teste para {freq} Hz. Próxima em 1s...\n")
            time.sleep(1)

except KeyboardInterrupt:
    print("\nTeste interrompido pelo usuário.")

finally:
    pwm_led.stop()
    GPIO.cleanup()
    print("Pinos GPIO liberados com sucesso.")