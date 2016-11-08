/*

MindRace - Software do Ardu�no - Desenvolvido por Paulo Salvatore


VIS�O GERAL

- O programa inicia em um estado neutro, aguardando a��o do bot�o de in�cio.

- Ao acionar o bot�o de in�cio, o programa ir� iniciar uma corrida.

- A energiza��o da pista se basear� nos valores que o programa recebe pela porta serial.

- Caso a comunica��o entre o programa conectado ao MindWave e o ardu�no estiver comprometida, � poss�vel acionar
o bot�o de emerg�ncia para que o programa comece a gerar valores aleat�rios e energize a pista com base neles.

- Um outro sensor ir� efetuar o trabalho de contagem das voltas, sempre que o sensor for acionado, o programa ir�
contabilizar uma volta. Ao atingir o limite de voltas a corrida ser� finalizada e o programa ir� retornar ao status
inicial.	Mesmo que o sensor continue ativo diversas vezes seguidamente, ele s� ir� aceitar um novo valor quando
entrar em estado inativo. Exemplo: Se o carrinho parar na frente do sensor, enviando constantemente uma informa��o,
o sensor ir� considerar aquilo como apenas uma informa��o e s� ir� liberar uma nova entrada de informa��o quando o
carrinho sair da frente do sensor.

- O programa ir� imprimir o n�mero da volta atual na porta serial sempre que:
A corrida for iniciada;
O n�mero da volta conforme o sensor de voltas for alterado;
A corrida for finalizada;


LEDs DE CONTROLE

Existem 5 LEDs que indicam o status de alguns componentes do circuito:

- LED Verde - Status - Corrida Ativa
Quando tiver uma corrida ativa, esse LED ir� ficar aceso.
- LED Vermelho - Status - Nenhuma Corrida Ativa
Quando n�o tiver nenhuma corrida ativa, esse LED ir� ficar aceso.
- LED Laranja - Concentra��o - �ltima Concentra��o V�lida
Quando o programa receber um valor de concentra��o via porta serial, esse LED ir� acender enquanto esse valor
estiver ativo, caso um novo valor n�o seja recebido em um certo prazo de segundos, esse LED ir� apagar e o
valor de concentra��o ser� zerado.
- LED Amarelo - Emerg�ncia - Estado de Emerg�ncia acionado
Quando a chave de emerg�ncia for acionada esse LED ir� ficar aceso para representar que o estado de emerg�ncia
est� acionado na pista.
- LED Azul - Sensor - Estado do Sensor
Enquanto o sensor estiver obtendo algum valor esse LED ir� ficar aceso. Assim que o sensor parar de emitir
sinal, o LED apagar�. Enquanto o LED estiver ligado, n�o � poss�vel contabilizar novas voltas na pista.

*/

int botaoInicio = 6;
int chaveEmergencia = 7;
int sensor = 5;

int pistaPositivo = 2;
int pista = 3;
int pistaNegativo = 4;
int energiaPista;

int ledOn = 12;
int ledOff = 11;
int ledConcentracao = 10;
int ledEmergencia = 9;
int ledVoltas = 8;

bool corridaIniciada = false;

bool emergenciaAtivado = false;

bool botaoInicioLiberado = true;
bool emergenciaLiberado = true;
bool sensorLiberado = true;
unsigned long tempoSensor;
float delaySensor = 500;

int voltas = 0;
int maxVoltas = 3;
unsigned long tempoLedVoltas;

int concentracao = 0;
int concentracaoAleatoria = 0;
int concentracaoMin = 0;
int concentracaoMax = 100;
unsigned long tempoUltimaConcentracaoRecebida;
float delayUltimaConcentracao = 5000;
bool ledConcentracaoLigado = false;
int variacaoConcentracaoAleatoria[] = {10, 41};

int energizarIntervalo[] = {50, 95};

unsigned long tempoAtual;
unsigned long tempoUltimaAtualizacao;
float delayAtualizacao = 2000;

void setup()
{
	pinMode(botaoInicio, INPUT);
	pinMode(chaveEmergencia, INPUT);

	pinMode(sensor, INPUT);

	pinMode(pistaNegativo, OUTPUT);
	pinMode(pista, OUTPUT);
	pinMode(pistaPositivo, OUTPUT);

	pinMode(ledOn, OUTPUT);
	pinMode(ledOff, OUTPUT);
	pinMode(ledConcentracao, OUTPUT);
	pinMode(ledEmergencia, OUTPUT);
	pinMode(ledVoltas, OUTPUT);

	Serial.begin(9600);

	AtualizarLedsStatus();
}

void AtualizarLedsStatus()
{
	digitalWrite(ledOn, (corridaIniciada) ? HIGH : LOW);
	digitalWrite(ledOff, (corridaIniciada) ? LOW : HIGH);
}

void AtualizarLedEmergencia()
{
	digitalWrite(ledEmergencia, (emergenciaAtivado) ? HIGH : LOW);
}

void AtualizarLedVoltas(bool desligar = false)
{
	digitalWrite(ledVoltas, (!sensorLiberado && !desligar) ? HIGH : LOW);
}

void PiscarLedConcentracao()
{
	digitalWrite(ledConcentracao, LOW);
	delay(100);
	digitalWrite(ledConcentracao, HIGH);
}

void IniciarCorrida()
{
	if (!corridaIniciada) {
		voltas = 0;
		corridaIniciada = true;
		ExibirVoltas();
		AtualizarLedsStatus();
	}
}

void EncerrarCorrida()
{
	corridaIniciada = false;
	AtualizarLedsStatus();
	delay(100);
	AtualizarLedVoltas(true);
}

void ContarVoltas()
{
	voltas++;

	ExibirVoltas();

	if (voltas == maxVoltas)
		EncerrarCorrida();
}

void ExibirVoltas()
{
	Serial.println(voltas);
}

void ChecarBotaoInicio()
{
	bool leituraBotaoInicio = digitalRead(botaoInicio);
	if (botaoInicioLiberado && leituraBotaoInicio)
	{
		IniciarCorrida();
		botaoInicioLiberado = false;
	}
	else if (!botaoInicioLiberado && !leituraBotaoInicio)
		botaoInicioLiberado = true;
}

void ChecarEmergencia()
{
	bool leituraEmergencia = digitalRead(chaveEmergencia);
	if (leituraEmergencia && !emergenciaAtivado)
	{
		emergenciaAtivado = true;
		AtualizarLedEmergencia();
	}
	else if (!leituraEmergencia && emergenciaAtivado)
	{
		emergenciaAtivado = false;
		AtualizarLedEmergencia();
		concentracao = 0;
	}
}

void ChecarSensor()
{
	if (!corridaIniciada)
		return;

	bool leituraSensor = digitalRead(sensor);
	if (sensorLiberado && leituraSensor)
	{
		sensorLiberado = false;
		AtualizarLedVoltas();
		ContarVoltas();
		tempoSensor = tempoAtual;
	}
	else if (!sensorLiberado && !leituraSensor && tempoAtual > tempoSensor + delaySensor) {
		sensorLiberado = true;
		AtualizarLedVoltas();
	}
}

void ValidarConcentracao()
{
	if (tempoAtual > tempoUltimaConcentracaoRecebida + delayUltimaConcentracao)
	{
		if (!emergenciaAtivado)
			concentracao = 0;

		if (ledConcentracaoLigado)
		{
			digitalWrite(ledConcentracao, LOW);
			ledConcentracaoLigado = false;
		}
	}
	else if (tempoUltimaConcentracaoRecebida != 0 && tempoAtual < tempoUltimaConcentracaoRecebida + delayUltimaConcentracao && !ledConcentracaoLigado)
	{
		digitalWrite(ledConcentracao, HIGH);
		ledConcentracaoLigado = true;
	}
}

void AtualizarConcentracao()
{
	if (Serial.available())
	{
		concentracao = Serial.parseInt();
		PiscarLedConcentracao();
		tempoUltimaConcentracaoRecebida = tempoAtual;
	}
}

void AtualizarConcentracaoAleatoria()
{
	if (tempoAtual > tempoUltimaAtualizacao + delayAtualizacao)
	{
		int modificador = random(variacaoConcentracaoAleatoria[0], variacaoConcentracaoAleatoria[1]);
		int multiplicador = random(0, 2) == 0 ? 1 : -1;

		concentracaoAleatoria = min(concentracaoMax, max(concentracaoMin, concentracao + modificador * multiplicador));

		tempoUltimaAtualizacao = tempoAtual;
	}
}

void EnergizarPista()
{
	if (emergenciaAtivado)
		concentracao = concentracaoAleatoria;

	int energia = (corridaIniciada) ? max(energizarIntervalo[0], concentracao * energizarIntervalo[1] / 100) : 0;

	if (energia != energiaPista)
	{
		digitalWrite(pistaPositivo, LOW);
		digitalWrite(pistaNegativo, HIGH);
		analogWrite(pista, energia);
		energiaPista = energia;
	}
}

void loop()
{
	tempoAtual = millis();

	ChecarBotaoInicio();
	ChecarEmergencia();
	ChecarSensor();

	ValidarConcentracao();
	AtualizarConcentracao();
	AtualizarConcentracaoAleatoria();

	EnergizarPista();

	delay(1); // Apenas para controlar o tempo de execu��o do programa
}

