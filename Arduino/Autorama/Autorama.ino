/*

MindRace - Software do Arduino - Desenvolvido por Paulo Salvatore


VIS�O GERAL

- O programa inicia em um estado neutro, aguardando a��o do bot�o de in�cio.

- Ao acionar o bot�o de in�cio, o programa ir� iniciar uma corrida.

- A energiza��o da pista se basear� nos valores que o programa recebe pela porta serial.

- Caso a comunica��o entre o programa conectado ao MindWave e o arduino estiver comprometida, � poss�vel acionar
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
A corrida for finalizada.

- Caso nenhuma corrida esteja ativa, � poss�vel iniciar o posicionamento autom�tico dos carrinhos. Baseado em qual
pista est� selecionada (externa ou interna), ao pressionar o bot�o de posicionamento, o carrinho ir� at� o sensor,
ficar� parado por um pequeno per�odo de tempo e iniciar� o movimento novamente por um certo per�odo de tempo at�
que pare na posi��o desejada.
OBS.: Para ajustar a posi��o de parada, � necess�rio mexer na dura��o da energiza��o da pista durante o posicionamento
atrav�s da vari�vel array duracaoPosicionamentoAutomatico[] - Sendo que o primeiro valor � da posi��o externa e o
segundo � da posi��o interna. Tamb�m � poss�vel mexer na velocidade do carrinho atrav�s da vari�vel array
valorPosicionamentoAutomatico[] - seguindo as mesmas posi��es citadas acima para os valores de cada pista.


LEDs DE CONTROLE

Existem 8 LEDs que indicam o status de alguns componentes do circuito:

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
- LED Amarelo - Posicionamento Autom�tico
Quando o bot�o de posicionamento autom�tico for pressionado, o LED ir� ficar aceso, caso contr�rio, ele ficar� apagado.
- LED Laranja - Pista Externa Selecionada
Quando esse LED estiver aceso, significa que o arduino est� programado para funcionar na pista externa, isso implicar�
apenas no posicionamento autom�tico, onde os valores de dura��o da energiza��o s�o diferentes para cada pista.
- LED Amarelo - Pista Interna Selecionada
Quando esse LED estiver aceso, significa que o arduino est� programado para funcionar na pista interna, isso implicar�
apenas no posicionamento autom�tico, onde os valores de dura��o da energiza��o s�o diferentes para cada pista.

*/

#include <SimpleTimer.h>

SimpleTimer simpleTimer;

int botaoInicio = A2;
int chaveEmergencia = A1;
int chaveSelecaoPista = A0;
int botaoPosicionamento = A3;

int sensor = A4;

int pistaPositivo = 2;
int pista = 3;
int pistaNegativo = 4;
int energiaPista;

int ledOn = 12;
int ledOff = 11;
int ledConcentracao = 10;
int ledEmergencia = 9;
int ledVoltas = 8;
int ledPosicionamento = 7;
int ledPistaExterna = 6;
int ledPistaInterna = 5;

bool corridaIniciada = false;

bool emergenciaAtivado = false;

bool botaoInicioLiberado = true;
bool emergenciaLiberado = true;

bool sensorLiberado = false;
unsigned long tempoSensor;
float delaySensor = 500;

int voltas = 0;
int maxVoltas = 3;
unsigned long tempoLedVoltas;
int contadorSensorVoltas;
int contadorNecessarioSensorVoltas = 5;

int concentracao = 0;
int concentracaoAleatoria = 0;
int concentracaoIntervalo[] = {0, 100};
unsigned long tempoUltimaConcentracaoRecebida;
float delayUltimaConcentracao = 10000;
bool ledConcentracaoLigado = false;
int variacaoConcentracaoAleatoria[] = {10, 41};

int energizarIntervalo[] = {50, 84};
int energizacaoFixa = 0;

unsigned long tempoAtual;
unsigned long tempoUltimaAtualizacao;
float delayAtualizacao = 2000;

bool botaoPosicionamentoLiberado = false;
bool posicionamentoLiberado = false;
bool posicionamentoIniciado = false;
int valorPosicionamentoAutomatico[] = {65, 65};
int duracaoPosicionamentoAutomatico[] = {1500, 0};
int delayAposEtapa1 = 500;
unsigned long tempoEnergizacao;
float delayEnergizacao;

int pistaSelecionada;

int valoresRecebidos[3];
unsigned long tempoUltimoValorRecebido;
float delayUltimoValorRecebido = 100;
int quantidadeValoresRecebidos;

void AtualizarLedsStatus()
{
	digitalWrite(ledOn, corridaIniciada ? HIGH : LOW);
	digitalWrite(ledOff, corridaIniciada ? LOW : HIGH);
}

void setup()
{
	pinMode(botaoInicio, INPUT);
	pinMode(chaveEmergencia, INPUT);
	pinMode(chaveSelecaoPista, INPUT);
	pinMode(botaoPosicionamento, INPUT);

	pinMode(sensor, INPUT);

	pinMode(pistaNegativo, OUTPUT);
	pinMode(pista, OUTPUT);
	pinMode(pistaPositivo, OUTPUT);

	pinMode(ledOn, OUTPUT);
	pinMode(ledOff, OUTPUT);
	pinMode(ledConcentracao, OUTPUT);
	pinMode(ledEmergencia, OUTPUT);
	pinMode(ledVoltas, OUTPUT);
	pinMode(ledPosicionamento, OUTPUT);
	pinMode(ledPistaExterna, OUTPUT);
	pinMode(ledPistaInterna, OUTPUT);

	Serial.begin(9600);

	AtualizarLedsStatus();
	AtualizarLedPistaSelecionada();
}

void AtualizarLedEmergencia()
{
	digitalWrite(ledEmergencia, emergenciaAtivado ? HIGH : LOW);
}

void AtualizarLedVoltas(bool desligar = false)
{
	digitalWrite(ledVoltas, (!sensorLiberado && !desligar) ? HIGH : LOW);
}

void ApagarLedVoltas()
{
	AtualizarLedVoltas(true);
}

void ApagarLedConcentracao()
{
	digitalWrite(ledConcentracao, LOW);
}

void AcenderLedConcentracao()
{
	digitalWrite(ledConcentracao, HIGH);
}

void PiscarLedConcentracao()
{
	ApagarLedConcentracao();
	simpleTimer.setTimeout(100, AcenderLedConcentracao);
}

void AtualizarLedPosicionamento(bool desligar = false)
{
	digitalWrite(ledPosicionamento, posicionamentoIniciado ? HIGH : LOW);
}

void AtualizarLedPistaSelecionada()
{
	digitalWrite(ledPistaExterna, pistaSelecionada == 0 ? HIGH : LOW);
	digitalWrite(ledPistaInterna, pistaSelecionada == 1 ? HIGH : LOW);
}

void ExibirVoltas()
{
	Serial.println(voltas);
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
	simpleTimer.setTimeout(100, ApagarLedVoltas);
}

void ContarVoltas()
{
	voltas++;

	ExibirVoltas();

	if (voltas == maxVoltas)
		EncerrarCorrida();
}

void ChecarBotaoInicio()
{
	if (corridaIniciada || posicionamentoIniciado)
		return;

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

void ChecarPistaSelecionada()
{
	bool leituraSelecaoPista = digitalRead(chaveSelecaoPista);
	if (leituraSelecaoPista != pistaSelecionada)
	{
		pistaSelecionada = leituraSelecaoPista;
		AtualizarLedPistaSelecionada();
	}
}

void EnergizarPistaTempo(int energia, float tempo)
{
	energizacaoFixa = energia;
	delayEnergizacao = tempo;
	tempoEnergizacao = tempoAtual;
	EnergizarPista();
}

void EnergizarPistaTempoAutomatico()
{
	EnergizarPistaTempo(valorPosicionamentoAutomatico[pistaSelecionada], duracaoPosicionamentoAutomatico[pistaSelecionada]);
}

void ChecarSensor()
{
	if (!corridaIniciada && !posicionamentoIniciado)
		return;

	bool leituraSensor = !digitalRead(sensor);

	if (sensorLiberado && leituraSensor)
	{
		contadorSensorVoltas++;
		if (contadorSensorVoltas >= contadorNecessarioSensorVoltas)
		{
			sensorLiberado = false;
			AtualizarLedVoltas();
			if (corridaIniciada)
				ContarVoltas();
			else if (posicionamentoIniciado)
				ProcessarPosicionamento(2);
		}

		tempoSensor = tempoAtual;
	}
	else if (!sensorLiberado && !leituraSensor && tempoAtual > tempoSensor + delaySensor)
	{
		contadorSensorVoltas = 0;
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
			ApagarLedConcentracao();
			ledConcentracaoLigado = false;
		}
	}
	else if (tempoUltimaConcentracaoRecebida != 0 && tempoAtual < tempoUltimaConcentracaoRecebida + delayUltimaConcentracao && !ledConcentracaoLigado)
	{
		AcenderLedConcentracao();
		ledConcentracaoLigado = true;
	}
}

void ChecarRecebimentoSerial()
{
	if (Serial.available() > 0)
	{
		int dadoRecebido = Serial.read();

		if (dadoRecebido == 97)
			ChecarConexaoUnity(dadoRecebido);
		else
		{
			int valorRecebido = dadoRecebido - '0';
			tempoUltimoValorRecebido = tempoAtual;
			valoresRecebidos[quantidadeValoresRecebidos] = valorRecebido;
			quantidadeValoresRecebidos++;
		}
	}
}

void ValidarRecebimentoSerial()
{
	if (tempoAtual > tempoUltimoValorRecebido + delayUltimoValorRecebido && quantidadeValoresRecebidos > 0)
	{
		String valorRecebidoCompleto = "";
		for (int i = 0; i < quantidadeValoresRecebidos; i++)
		{
			int valor = valoresRecebidos[i];
			if (valor >= 0)
			{
				valorRecebidoCompleto += valoresRecebidos[i];
				valoresRecebidos[i] = -1;
			}
		}

		int valorRecebido = valorRecebidoCompleto.toInt();
		AtualizarConcentracao(valorRecebido);
		quantidadeValoresRecebidos = 0;
	}
}

void AtualizarConcentracao(int valor)
{
	concentracao = valor;
	PiscarLedConcentracao();
	tempoUltimaConcentracaoRecebida = tempoAtual;
}

void AtualizarConcentracaoAleatoria()
{
	if (tempoAtual > tempoUltimaAtualizacao + delayAtualizacao)
	{
		int modificador = random(variacaoConcentracaoAleatoria[0], variacaoConcentracaoAleatoria[1]);
		int multiplicador = concentracaoAleatoria > 30 ? random(0, 2) == 0 ? 1 : -1 : 1;

		concentracaoAleatoria = min(concentracaoIntervalo[1], max(concentracaoIntervalo[0], concentracao + modificador * multiplicador));

		tempoUltimaAtualizacao = tempoAtual;
	}
}

void EnergizarPista()
{
	int energia;
	if (energizacaoFixa > 0)
		energia = energizacaoFixa;
	else if (energizacaoFixa == -1)
	{
		energia = 0;
		energizacaoFixa = 0;
	}
	else
	{
		if (emergenciaAtivado)
			concentracao = concentracaoAleatoria;

		int energizacaoIntervaloReal = energizarIntervalo[1] - energizarIntervalo[0];
		energia = corridaIniciada ? max(energizarIntervalo[0], min(energizarIntervalo[1], energizarIntervalo[0] + concentracao * energizacaoIntervaloReal / 100)) : 0;
	}

	if (tempoEnergizacao > 0 && tempoAtual > tempoEnergizacao + delayEnergizacao)
	{
		delayEnergizacao = 0;
		tempoEnergizacao = 0;
		energizacaoFixa = 0;
	}

	if (energia != energiaPista)
	{
		digitalWrite(pistaPositivo, HIGH);
		digitalWrite(pistaNegativo, LOW);
		analogWrite(pista, energia);
		energiaPista = energia;
	}
}

void DesenergizarPista()
{
	energizacaoFixa = -1;
	EnergizarPista();
}

void IniciarPosicionamento()
{
	if (!posicionamentoIniciado)
	{
		ProcessarPosicionamento(1);
		posicionamentoIniciado = true;
		AtualizarLedPosicionamento();
	}
}

void ProcessarPosicionamento(int etapaPosicionamento)
{
	if (etapaPosicionamento == 1)
		energizacaoFixa = valorPosicionamentoAutomatico[pistaSelecionada];
	else if (etapaPosicionamento == 2)
	{
		DesenergizarPista();
		simpleTimer.setTimeout(delayAposEtapa1, EnergizarPistaTempoAutomatico);
		simpleTimer.setTimeout(duracaoPosicionamentoAutomatico[pistaSelecionada], EncerrarPosicionamento);
	}
}

void EncerrarPosicionamento()
{
	if (posicionamentoIniciado)
	{
		posicionamentoIniciado = false;
		AtualizarLedPosicionamento();
		ApagarLedVoltas();
	}
}

void ChecarPosicionamentoAutomatico()
{
	if (corridaIniciada || posicionamentoIniciado)
		return;

	bool leituraBotaoPosicionamento = digitalRead(botaoPosicionamento);
	if (botaoPosicionamentoLiberado && leituraBotaoPosicionamento)
	{
		IniciarPosicionamento();
		botaoPosicionamentoLiberado = false;
	}
	else if (!botaoPosicionamentoLiberado && !leituraBotaoPosicionamento)
		botaoPosicionamentoLiberado = true;
}

void ChecarConexaoUnity(int dadoRecebido)
{
	if (dadoRecebido == 99)
		Serial.println(dadoRecebido);
}

void loop()
{
	simpleTimer.run();

	tempoAtual = millis();

	ChecarRecebimentoSerial();
	ValidarRecebimentoSerial();

	ChecarPosicionamentoAutomatico();

	ChecarBotaoInicio();
	ChecarEmergencia();
	ChecarSensor();
	ChecarPistaSelecionada();

	ValidarConcentracao();
	AtualizarConcentracaoAleatoria();

	EnergizarPista();

	delay(1); // Apenas para controlar o tempo de execu��o do programa
}
