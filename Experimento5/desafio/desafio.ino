const int LDR_PIN = 0;
const int BOTAO_PIN = 4;

const int LIMIAR = 2000;

const unsigned long TEMPO_VERDE = 5000;
const unsigned long TEMPO_AMARELO = 2000;
const unsigned long TEMPO_VERMELHO = 5000;

volatile bool pedidoTravessia = false;

unsigned long tempoEstado = 0;

enum Estado {
    NORMAL_VERDE,
    NORMAL_AMARELO,
    NORMAL_VERMELHO,
    NOTURNO_AMARELO,
    NOTURNO_APAGADO
};

Estado estadoAtual = NORMAL_VERDE;

void IRAM_ATTR isrBotao()
{
    pedidoTravessia = true;
}

void setup()
{
    Serial.begin(115200);

    pinMode(BOTAO_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(BOTAO_PIN),
        isrBotao,
        FALLING
    );

    tempoEstado = millis();
}

void loop()
{
    unsigned long agora = millis();

    int luminosidade = analogRead(LDR_PIN);

    bool noite = luminosidade > LIMIAR;

    if (noite)
    {
        if (estadoAtual != NOTURNO_AMARELO &&
            estadoAtual != NOTURNO_APAGADO)
        {
            estadoAtual = NOTURNO_AMARELO;
            tempoEstado = agora;
        }
    }
    else
    {
        if (estadoAtual == NOTURNO_AMARELO ||
            estadoAtual == NOTURNO_APAGADO)
        {
            estadoAtual = NORMAL_VERDE;
            tempoEstado = agora;
        }
    }

    switch (estadoAtual)
    {
        case NORMAL_VERDE:

            rgbLedWrite(LED_BUILTIN, 0, 15, 0);

            if (pedidoTravessia)
            {
                pedidoTravessia = false;

                estadoAtual = NORMAL_AMARELO;
                tempoEstado = agora;
            }
            else if (agora - tempoEstado >= TEMPO_VERDE)
            {
                estadoAtual = NORMAL_AMARELO;
                tempoEstado = agora;
            }

            break;

        case NORMAL_AMARELO:

            rgbLedWrite(LED_BUILTIN, 15, 15, 0);

            if (agora - tempoEstado >= TEMPO_AMARELO)
            {
                estadoAtual = NORMAL_VERMELHO;
                tempoEstado = agora;
            }

            break;

        case NORMAL_VERMELHO:

            rgbLedWrite(LED_BUILTIN, 15, 0, 0);

            if (agora - tempoEstado >= TEMPO_VERMELHO)
            {
                estadoAtual = NORMAL_VERDE;
                tempoEstado = agora;
            }

            break;

        case NOTURNO_AMARELO:

            rgbLedWrite(LED_BUILTIN, 15, 15, 0);

            if (agora - tempoEstado >= 500)
            {
                estadoAtual = NOTURNO_APAGADO;
                tempoEstado = agora;
            }

            break;

        case NOTURNO_APAGADO:

            rgbLedWrite(LED_BUILTIN, 0, 0, 0);

            if (agora - tempoEstado >= 500)
            {
                estadoAtual = NOTURNO_AMARELO;
                tempoEstado = agora;
            }

            break;
    }
}
