import cv2
import face_recognition
import RPi.GPIO as GPIO
import time

# --- CONFIGURAÇÕES DE HARDWARE ---
SERVO_PIN = 18
BUZZER_PIN = 22

GPIO.setmode(GPIO.BCM)
GPIO.setup(SERVO_PIN, GPIO.OUT)
GPIO.setup(BUZZER_PIN, GPIO.OUT)

pwm_servo = GPIO.PWM(SERVO_PIN, 50)
pwm_servo.start(2.5) # Inicia fechado (0 graus)

def abrir_fechadura():
    print("Acesso Permitido! Abrindo tranca...")
    # Bip de sucesso
    pwm_buzzer = GPIO.PWM(BUZZER_PIN, 2000)
    pwm_buzzer.start(50)
    time.sleep(0.2)
    pwm_buzzer.stop()
    
    # Gira o servo para 90 graus (Abre)
    pwm_servo.ChangeDutyCycle(7.5) 
    time.sleep(5) # Mantém aberto por 5 segundos
    
    print("Trancando novamente...")
    # Volta o servo para 0 graus (Tranca)
    pwm_servo.ChangeDutyCycle(2.5) 
    time.sleep(0.5)
    pwm_servo.ChangeDutyCycle(0) # Para o jitter

# --- TREINAMENTO DO ROSTO ---
print("Carregando banco de dados de rostos...")
# Precisa colocar uma foto do rosto
foto_dono = face_recognition.load_image_file("meu_rosto.jpg")

# Pega o 'hash' (encoding) do rosto na foto. Pega o índice [0] assumindo que há só 1 rosto lá
dono_encoding = face_recognition.face_encodings(foto_dono)[0]

rostos_conhecidos = [dono_encoding]
nomes_conhecidos = ["Usuario Autorizado"]

# --- INICIALIZAÇÃO DA CÂMERA ---
# 0 geralmente é a câmera USB padrão. Se tiver a câmera nativa do RPi conectada, a USB pode ser a 1.
video_capture = cv2.VideoCapture(0)

# Variável para processar apenas frame sim, frame não (otimização para RPi3)
processar_frame = True

print("Sistema de Câmera Iniciado. Pressione 'q' na janela de vídeo para sair.")

try:
    while True:
        # Pega um único frame de vídeo
        ret, frame = video_capture.read()
        
        if not ret:
            print("Erro ao ler a câmera.")
            break

        # Apenas processa o frame a cada 2 iterações para poupar a CPU da Raspberry
        if processar_frame:
            # Redimensiona o frame para 1/4 do tamanho para o reconhecimento facial ser mais rápido
            small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)

            # Converte a imagem de BGR (Padrão OpenCV) para RGB (Padrão face_recognition)
            # Nota: Nas versões mais recentes do face_recognition não exige, mas é mais seguro.
            rgb_small_frame = small_frame[:, :, ::-1]
            
            # Acha todos os rostos e seus encodings no frame de vídeo atual
            face_locations = face_recognition.face_locations(rgb_small_frame)
            face_encodings = face_recognition.face_encodings(rgb_small_frame, face_locations)

            for face_encoding in face_encodings:
                # Vê se o rosto capturado bate com o rosto salvo (tolerância padrão é 0.6)
                matches = face_recognition.compare_faces(rostos_conhecidos, face_encoding)
                
                if True in matches:
                    primeiro_match_index = matches.index(True)
                    nome = nomes_conhecidos[primeiro_match_index]
                    
                    # Chama a função que mexe no hardware
                    abrir_fechadura()
                    
                    # Evita que a fechadura fique abrindo loucamente, 
                    # limpa o buffer de vídeo acumulado durante os 5 segundos aberta
                    video_capture.grab() 

        # Inverte a variável para pular o processamento no próximo loop
        processar_frame = not processar_frame

        # Mostra o vídeo na tela (Isso consome processamento, apagar depois dos testes)
        cv2.imshow('Camera de Seguranca', frame)

        # Aperte a tecla 'q' no teclado para encerrar
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    print("\nEncerrado pelo teclado.")

finally:
    # Libera a câmera e os pinos
    video_capture.release()
    cv2.destroyAllWindows()
    pwm_servo.stop()
    GPIO.cleanup()
