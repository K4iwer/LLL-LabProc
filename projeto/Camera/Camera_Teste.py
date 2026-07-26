import cv2

# Tenta abrir a câmera USB
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("ERRO: Não foi possível acessar a câmera USB.")
    exit()

print("Câmera acessada com sucesso! Pressione 'q' na janela de vídeo para sair.")

while True:
    # Captura frame por frame
    ret, frame = cap.read()
    
    if not ret:
        print("Erro ao capturar o frame.")
        break

    # Mostra o frame na tela
    cv2.imshow('Teste de Camera OpenCV', frame)

    # Espera a tecla 'q' ser pressionada para sair
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Libera a câmera e fecha as janelas
cap.release()
cv2.destroyAllWindows()
