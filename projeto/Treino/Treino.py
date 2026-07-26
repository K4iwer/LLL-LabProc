import cv2
import os
import numpy as np

# Configurações
pasta_dataset = 'dataset'
nome_dono = 'Usuario_Autorizado'

# Cria a pasta do dataset se não existir
if not os.path.exists(pasta_dataset):
    os.makedirs(pasta_dataset)

# Carrega o classificador de rostos padrão do OpenCV (Haar Cascade)
detector_face = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

cap = cv2.VideoCapture(0)

print("\n[INFO] Olhe para a câmera e aguarde. Capturando fotos para o treinamento...")
contador_fotos = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break
        
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    # Detecta rostos na imagem em tons de cinza
    rostos = detector_face.detectMultiScale(gray, scaleFactor=1.2, minNeighbors=5)

    for (x, y, w, h) in rostos:
        # Desenha um retângulo azul ao redor do rosto
        cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
        contador_fotos += 1
        
        # Salva a imagem cortada (só o rosto) na pasta
        caminho_arquivo = os.path.join(pasta_dataset, f"{nome_dono}_{contador_fotos}.jpg")
        cv2.imwrite(caminho_arquivo, gray[y:y+h, x:x+w])

        cv2.imshow('Capturando...', frame)

    # Pressione ESC para parar ou espera até tirar 30 fotos
    tecla = cv2.waitKey(100) & 0xFF
    if tecla == 27 or contador_fotos >= 30:
        break

cap.release()
cv2.destroyAllWindows()
print("\n[INFO] Captura concluída. Iniciando o treinamento do modelo LBPH...")

# --- FASE DE TREINAMENTO ---
# Necessário opencv-contrib-python
# Comando: pip install opencv-contrib-python

try:
    reconhecedor = cv2.face.LBPHFaceRecognizer_create()
except AttributeError:
    print("\nERRO: Você precisa instalar a versão contrib do OpenCV!")
    print("Rode no terminal: pip install opencv-contrib-python")
    exit()

faces_treino = []
ids = []

# Varre a pasta de dataset
for arquivo in os.listdir(pasta_dataset):
    if arquivo.endswith(".jpg"):
        caminho = os.path.join(pasta_dataset, arquivo)
        
        # Lê a imagem em tons de cinza
        img_gray = cv2.imread(caminho, cv2.IMREAD_GRAYSCALE)
        
        # Como temos apenas 1 usuário autorizado, o ID será sempre 1
        faces_treino.append(img_gray)
        ids.append(1)

# Treina o reconhecedor
print("[INFO] Treinando o modelo...")
reconhecedor.train(faces_treino, np.array(ids))

# Salva o modelo matemático treinado
reconhecedor.save('modelo_lbph_fechadura.yml')
print(f"[INFO] Treinamento finalizado. Modelo salvo como 'modelo_lbph_fechadura.yml' com {len(faces_treino)} faces.")
