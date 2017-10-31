#include <SPI.h>

/*//Se for usar a placa de rede normal*/
//#include <UIPEthernet.h>
//#include <utility/logging.h>

/*Se for usar a placa Shield*/
#include <Ethernet.h>

/*Para o MQTT*/
#include <PubSubClient.h>

#include <Ultrasonic.h>

//Define o pino do Arduino a ser utilizado com o pino Trigger do sensor
#define PINO_TRIGGER 9
//Define o pino do Arduino a ser utilizado com o pino Echo do sensor
#define PINO_ECHO 8

//Inicializa o sensor ultrasonico
Ultrasonic ultrasonic(PINO_TRIGGER, PINO_ECHO);

//const int ledVermelho = 5;
//const int ledVerde = 6;
//const int ledAzul = 3;
//char* leitura;

int distancia;
char* payloadAsChar;
String hexstring;
String EstadoVaga = "vazia";



/*Atualizar ultimo valor para ID do seu Kit para evitar duplicatas*/
byte mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xF1, 0x55 };
//byte ip[] = { 192,168,3,155 };

/*Endereço do Cloud MQTT*/
char* server = "m10.cloudmqtt.com";

/*Valor da porta do servidor MQTT*/
int port = 13038 ;

EthernetClient ethClient;

/* FUNÇÃO que irá receber o retorno do servidor*/
void whenMessageReceived(char* topic, byte* payload, unsigned int length) {

  /*Converter pointer do tipo `byte` para typo `char`*/

  char* payloadAsChar = payload;

  

  Serial.flush();

  //int msgComoNumero = msg.toInt();

 
  //
  //  Serial.flush();
  }

  /*Dados do MQTT Cloud*/
  PubSubClient client(server, port, whenMessageReceived, ethClient);
  long lastReconnectAttempt = 0;

  void setup() {

    Serial.begin(9600);
    //  pinMode(ledVermelho, OUTPUT);
    //  pinMode(ledVerde, OUTPUT);
    //  pinMode(ledAzul, OUTPUT);

    Serial.begin(9600);
    Serial.println("Connecting...");


    while (!Serial) {}

    if (!Ethernet.begin(mac)) {
      Serial.println("DHCP Failed");

    } else {
      Serial.println(Ethernet.localIP());

    }

    lastReconnectAttempt = 0;

  }

  void loop() {
    /*Faz a conexão no cloud com nome do dispositivo, usuário e senha respectivamente*/
    if (!client.connected()) {
      long now = millis();
      if (now - lastReconnectAttempt > 5000) {
        lastReconnectAttempt = now;
        if (reconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      /*A biblioteca PubSubClient precisa que este método seja chamado em cada iteração de `loop()`
        para manter a conexão MQTT e processar mensagens recebidas (via a função callback)*/
      client.loop();

      int distancia = ultrasonic.distanceRead();
      delay (1000);

  Serial.print("valor lido: "); Serial.println(distancia);

  if (distancia <= 10 && EstadoVaga == "vazia") {
    delay (500);
    vagaOcupada();
    EstadoVaga = "ocupada";

  }
 if (distancia >= 10 && EstadoVaga == "ocupada")  {
   delay (500);
   vagaLivre();
   EstadoVaga = "vazia";
  }
  }
  }
  //////////////////////////////////////////////
  //FUNÇÕES
  //////////////////////////////////////////////

  // Função para (re)conectar no mqtt
  boolean reconnect() {
    if (client.connect("vagas", "rodolfo", "rodolfo")) {
      Serial.println("Connected");

      /*Envia uma mensagem para o cloud no topic portao2*/
      client.publish("vaga/1", "Online");
      Serial.println("LED sent");

      /*Conecta no topic para receber mensagens*/
      client.subscribe("vaga/1");
      Serial.println("conectado A LED");

     
  } else {
    Serial.println("Failed to connect to MQTT server");

  }
}

void vagaOcupada() {
  client.publish("vaga/1", "1");
}

void vagaLivre() {
  client.publish("vaga/1", "0");
}
