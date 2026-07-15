import RPi.GPIO as GPIO
import time

# --- MAPEAMENTO DOS PINOS ---
SERVO_PIN = 18
BUZZER_PIN = 12
BTN_MAIS_PIN = 27
BTN_MENOS_PIN = 17

# --- VARIÁVEIS GLOBAIS DE CONTROLE ---
bpm = 60               
intervalo = 60.0 / bpm 
posicao_servo = 0     

# --- CONFIGURAÇÃO INICIAL DA PLACA ---
GPIO.setmode(GPIO.BCM)
GPIO.setup(SERVO_PIN, GPIO.OUT)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

GPIO.setup(BTN_MAIS_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(BTN_MENOS_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)

pwm_servo = GPIO.PWM(SERVO_PIN, 50)     
pwm_buzzer = GPIO.PWM(BUZZER_PIN, 2000) 

pwm_servo.start(0)
pwm_buzzer.start(0)

# --- FUNÇÕES DE CALLBACK DOS BOTÕES (Rodam em Threads separadas) ---
def aumentar_bpm(canal):
    global bpm, intervalo
    # Limita o máximo a 240 BPM para o servo conseguir acompanhar
    bpm = min(240, bpm + 10) 
    intervalo = 60.0 / bpm
    print(f"[+] Mais rápido! Novo BPM: {bpm} (Intervalo: {intervalo:.2f}s)")

def diminuir_bpm(canal):
    global bpm, intervalo
    # Limita o mínimo a 20 BPM
    bpm = max(20, bpm - 10)  
    intervalo = 60.0 / bpm
    print(f"[-] Mais devagar! Novo BPM: {bpm} (Intervalo: {intervalo:.2f}s)")

# --- CONFIGURAÇÃO DAS INTERRUPÇÕES ---
GPIO.add_event_detect(BTN_MAIS_PIN, GPIO.FALLING, callback=aumentar_bpm, bouncetime=200)
GPIO.add_event_detect(BTN_MENOS_PIN, GPIO.FALLING, callback=diminuir_bpm, bouncetime=200)

try:
    print(f"Metrônomo Iniciado a {bpm} BPM.")
    print("Aperte os botões para alterar a velocidade.")
    
    # --- Thread do Metrônomo ---
    while True:
        pwm_buzzer.ChangeDutyCycle(25) 
        
        if posicao_servo == 0:
            pwm_servo.ChangeDutyCycle(12.5)
            posicao_servo = 180
        else:
            pwm_servo.ChangeDutyCycle(2.5)
            posicao_servo = 0
            
        time.sleep(0.05)
        
        pwm_buzzer.ChangeDutyCycle(0)
        
        tempo_restante = intervalo - 0.05
        if tempo_restante > 0:
            time.sleep(tempo_restante)

except KeyboardInterrupt:
    print("\nMetrônomo desligado pelo usuário.")

finally:
    # Finalização segura
    pwm_servo.stop()
    pwm_buzzer.stop()
    GPIO.cleanup()
    print("Pinos GPIO liberados.")
