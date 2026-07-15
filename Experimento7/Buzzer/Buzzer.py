import RPi.GPIO as GPIO
import time
import threading

# Definição do pino
BUZZER_PIN = 12

SOM_ATIVADO = True 

# Configuração da placa
GPIO.setmode(GPIO.BCM)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

def escutar_teclado():
    global SOM_ATIVADO
    while True:
        try:
            entrada = input()
            if entrada.strip().lower() == 'm':
                SOM_ATIVADO = not SOM_ATIVADO
                
                estado = "LIGADO" if SOM_ATIVADO else "MUTADO"
                print(f"\n[COMANDO] -> Som {estado}!\n")
        except EOFError:
            break

def emitir_bip(frequencia=2000, duracao=0.2):
    if SOM_ATIVADO:
        pwm_buzzer = GPIO.PWM(BUZZER_PIN, frequencia)
        pwm_buzzer.start(50) 
        time.sleep(duracao) 
        pwm_buzzer.stop()

try:
    print("-> Digite 'm' e aperte ENTER a qualquer momento para mutar/desmutar.")
    thread = threading.Thread(target=escutar_teclado, daemon=True)
    thread.start()
    
    ciclo_atual = 1
    
    while True:
        print(f"Executando ciclo {ciclo_atual}...")
        time.sleep(2)
        
        emitir_bip() 
        
        ciclo_atual += 1

except KeyboardInterrupt:
    print("\nPrograma interrompido pelo usuário.")

finally:
    GPIO.cleanup()
    print("Pinos GPIO liberados.")
