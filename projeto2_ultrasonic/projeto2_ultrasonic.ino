#include <SPI.h>
#include <UIPEthernet.h>
#include <utility/logging.h>
#include <PubSubClient.h>
#include <Ultrasonic.h>

long lastReconnectAttempt = 0;
//char payload = 0;
//char topic = 0;
int distancia;
String EstadoVaga = "vazia";

int ledVerde = 6;
int ledAmarelo = 7;
int ledVermelho = 5;
int buzzer = 4;

int estadoConexao = 0;

int ledLivre = 3;
int ledOcupada = 4;

//Define o pino do Arduino a ser utilizado com o pino Trigger do sensor
#define PINO_TRIGGER 9
//Define o pino do Arduino a ser utilizado com o pino Echo do sensor
#define PINO_ECHO 8
//Inicializa o sensor ultrasonico
Ultrasonic ultrasonic(PINO_TRIGGER, PINO_ECHO);

byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xF1, 0x64};  // Mudar de acordo com a rede
byte ip[] = {192, 168, 3, 48};

void callback(char* topic, byte* payload, unsigned int length); // Callback function header

EthernetClient ethClient;
PubSubClient client("192.168.3.186", 1883, callback, ethClient); // Dados do MQTT Cloud


void callback(char* topic, byte* payload, unsigned int length)
{
  feedback(5);  // Recebendo Mensagem
  byte* p = (byte* )malloc(length);
  memcpy(p, payload, length);
  free(p);
}

boolean reconnect()
{
  if (client.connect( "", "", ""))
  {
    //client.publish("senai-vagas/21", "10");
    //client.subscribe("vagas/#");
  }
  feedback(3);  // Conectado
  Serial.println("Conectado MQTT");
  Serial.flush ();
  return client.connected();
}

void setup()
{
  lastReconnectAttempt = 0;
  Serial.begin(9600);
  Serial.println("Iniciando...");
  Serial.flush ();
  delay(50);
  feedback(2);
    
  pinMode(ledLivre, OUTPUT);
  pinMode(ledOcupada, OUTPUT);
  
  //  Ethernet.begin(mac);
  if (!Ethernet.begin(ip)) 
  {
    Serial.println("DHCP Failed");
    Serial.flush ();
  }
  else 
  {
    Serial.println(Ethernet.localIP());
    Serial.flush ();
  }
  
  if (client.connect( "", "", "")) // Nome do dispositivo, usuário e senha respectivamente
  {
    //client.publish("vagas/21", "10"); // Topico e mensagem respectivamente
    //client.subscribe("vagas/21");
    Serial.println("Conectado MQTT");
    Serial.flush ();
    delay(50);
    feedback(3);
  }
  else
  {
    Serial.println("Erro ao Conectar!!!");
    Serial.flush ();
    delay(50);
    feedback(1);
  }
}
void loop()
{
  if (!client.connected())
  {
    long now = millis();
    feedback(1);
    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;
      feedback(2);
      Serial.println("Reconectando...");
      Serial.flush ();
      
      if (reconnect())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    client.loop();
    
    int distancia = ultrasonic.distanceRead();
    delay (1000);
    Serial.print("valor lido: "); 
    Serial.println(distancia);
    Serial.flush ();
    
    if (distancia <= 10 && EstadoVaga == "vazia")
    {
      delay (500);
      vagaOcupada();
      //client.publish("senai-vagas/21", "0");
      EstadoVaga = "ocupada";
    }
    
    if (distancia >= 10 && EstadoVaga == "ocupada")
    {
      delay (500);
      vagaLivre();
      //client.publish("senai-vagas/21", "1");
      EstadoVaga = "vazia";
    }
  }
}

void vagaOcupada() 
{
  client.publish("vagas/21", "0");
  Serial.println("ocupada");
  Serial.flush();
  digitalWrite(ledOcupada, HIGH);
  digitalWrite(ledLivre, LOW);
}

void vagaLivre() 
{
  client.publish("vagas/21", "1");
  Serial.println("vazia");
  Serial.flush();
  digitalWrite(ledLivre, HIGH);
  digitalWrite(ledOcupada, LOW);
}

void feedback(char estadoConexao)
{
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  switch (estadoConexao)
  {
    case 1: // Desconectado
    {
      acendeLed(ledVermelho, ledVerde, ledAmarelo);
      delay(100);
      break;
    }
    case 2: // Conectando
    {
      acendeLed(ledAmarelo, ledVermelho, ledVerde);
      break;
    }
    case 3: // Conectado
    {
      acendeLed(ledVerde, ledAmarelo, ledVermelho);
      break;
    }
    case 4: // Enviando Mensagem
    {
      acendeLed(ledVerde, ledAmarelo, ledVermelho);
      tone(buzzer, 500);
      delay(500);
      noTone(buzzer);
      break;
    }
    case 5: // Recebendo Mensagem
    {
      acendeLed(ledVerde, ledAmarelo, ledVermelho);
      tone (buzzer, 500);
      delay (50);
      noTone(buzzer);
      delay (50);
      tone (buzzer, 500);
      delay (50);
      noTone(buzzer);
      break;
    }
  }
}

void acendeLed(int acende, int apaga1, int apaga2)
{
  digitalWrite(apaga2, LOW);
  digitalWrite(apaga1, LOW);
  digitalWrite(acende, HIGH);
}

void apagaLeds()
{
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarelo, LOW);
  digitalWrite(ledVermelho, LOW);
}
