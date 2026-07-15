import RPi.GPIO as GPIO
import time

# Definição do pino do servo
SERVO_PIN = 18

# Configuração da placa
GPIO.setmode(GPIO.BCM)
GPIO.setup(SERVO_PIN, GPIO.OUT)

# Inicializa o PWM na frequência de 50Hz
pwm_servo = GPIO.PWM(SERVO_PIN, 50)
pwm_servo.start(0)

def definir_angulo(angulo):
    duty = (angulo / 18.0) + 2.5
    
    pwm_servo.ChangeDutyCycle(duty)
    
    time.sleep(0.5)

try:
    print("Controle do Servomotor iniciado. Pressione CTRL+C para parar.")
    
    while True:
        print("Movendo para 0°")
        definir_angulo(0)
        time.sleep(1.5)
        
        print("Movendo para 90°")
        definir_angulo(90)
        time.sleep(1.5)
        
        print("Movendo para 180°")
        definir_angulo(180)
        time.sleep(1.5)

except KeyboardInterrupt:
    print("\nPrograma interrompido pelo usuário.")

finally:
    pwm_servo.stop()
    GPIO.cleanup()
    print("Pinos GPIO liberados.")