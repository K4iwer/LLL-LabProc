import cv2
import RPi.GPIO as GPIO
import time
import os

# --- CONFIGURAÇÕES DE HARDWARE ---
SERVO_PIN = 18
BUZZER_PIN = 22

GPIO.setmode(GPIO.BCM)
GPIO.setup(SERVO_PIN, GPIO.OUT)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

pwm_servo = GPIO.PWM(SERVO_PIN, 50)
pwm_buzzer = GPIO.PWM(BUZZER_PIN, 2000)

pwm_servo.start(2.5) # Inicia trancado (0 graus)

# Variável de estado para evitar abrir a porta milhares de vezes
porta_aberta = False

def abrir_fechadura():
    global porta_aberta
    if not porta_aberta:
        print("\n>>> ACESSO CONCEDIDO: ABRINDO FECHADURA <<<")
        porta_aberta = True
        
        # Bip de Sucesso (Dois bipes curtos)
        pwm_buzzer.start(50)
        time.sleep(0.1)
        pwm_buzzer.ChangeDutyCycle(0)
        time.sleep(0.1)
        pwm_buzzer.ChangeDutyCycle(50)
        time.sleep(0.1)
        pwm_buzzer.stop()
        
        # Abre o Servo (90 graus)
        pwm_servo.ChangeDutyCycle(7.5)
        
        # Deixa a porta aberta por 5 segundos
        time.sleep(5)
        
        print(">>> TRANCANDO FECHADURA <<<")
        pwm_servo.ChangeDutyCycle(2.5)
        time.sleep(0.5)
        pwm_servo.ChangeDutyCycle(0) # Remove jitter
        
        porta_aberta = False

# --- CONFIGURAÇÕES DE VISÃO COMPUTACIONAL ---
# Verifica se o modelo existe
if not os.path.exists('modelo_lbph_fechadura.yml'):
    print("ERRO: Modelo não encontrado. Rode o script de treinamento primeiro!")
    GPIO.cleanup()
    exit()

reconhecedor = cv2.face.LBPHFaceRecognizer_create()
reconhecedor.read('modelo_lbph_fechadura.yml')

# Classificador para achar o rosto na imagem
detector_face = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

# Nomes correspondentes aos IDs (ID 1 = Usuário Autorizado)
nomes = ['Desconhecido', 'Usuario_Autorizado'] 

cap = cv2.VideoCapture(0)

print("\n[INFO] Sistema de Segurança Iniciado. Pressione ESC na janela para sair.")

try:
    while True:
        ret, frame = cap.read()
        if not ret:
            break
            
        # O modelo LBPH processa imagens em tons de cinza
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        # Acha os rostos no frame
        rostos = detector_face.detectMultiScale(gray, scaleFactor=1.2, minNeighbors=5)

        for (x, y, w, h) in rostos:
            # Pega só o quadrado onde está o rosto
            roi_cinza = gray[y:y+h, x:x+w]
            
            # Tenta prever quem é
            id_previsto, confianca = reconhecedor.predict(roi_cinza)
            
            # A confiança no LBPH é uma "distância". 
            # 0 seria um match perfeito. Valores menores que 60-70 geralmente são bons.
            if confianca < 65:
                nome_identificado = nomes[id_previsto]
                texto_confianca = f" Conf.: {round(100 - confianca)}%"
                
                # Se achou o dono, aciona o hardware
                abrir_fechadura()
                
            else:
                nome_identificado = "Desconhecido"
                texto_confianca = f" Conf.: {round(100 - confianca)}%"
            
            # Desenha os resultados na tela (Visualização)
            cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0) if nome_identificado != "Desconhecido" else (0, 0, 255), 2)
            cv2.putText(frame, str(nome_identificado), (x+5, y-5), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
            cv2.putText(frame, str(texto_confianca), (x+5, y+h-5), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 0), 1)

        cv2.imshow('Fechadura Segura', frame)

        # Tecla ESC para sair
        if cv2.waitKey(10) & 0xFF == 27:
            break

except KeyboardInterrupt:
    print("\nEncerrado pelo usuário.")

finally:
    cap.release()
    cv2.destroyAllWindows()
    pwm_servo.stop()
    pwm_buzzer.stop()
    GPIO.cleanup()
