#include <SPI.h>
#include <UIPEthernet.h>
#include <utility/logging.h>
#include <PubSubClient.h>  

long lastReconnectAttempt = 0;

char payload = 0;
char topic = 0;

byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xF1, 0x64};  // Mudar de acordo com a rede

void callback(char* topic, byte* payload, unsigned int length); // Callback function header

EthernetClient ethClient;

PubSubClient client("m11.cloudmqtt.com", 11084, callback, ethClient); // Dados do MQTT Cloud


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

void feedback(char estadoConexao)
{
  int ledVerde = 6;
  int ledAmarelo = 7;
  int ledVermelho = 8;
  int buzzer = 5;
  
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(buzzer, OUTPUT);

  int estadoConexao = 0;

  Serial.begin(9600);
     
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
      tone (buzz, 500);
      delay (50);
      noTone(buzz);
      delay (50);
      tone (buzz, 500);
      delay (50);
      noTone(buzz);
      break;
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  char* payloadAsChar = payload;
  payloadAsChar[length] = 0;
  String msgRecebida = String(payloadAsChar);
  
  Serial.println(msgRecebida);
  Serial.println();
  Serial.println(topic);
  delay(100);
  feedback(5);  // Recebendo Mensagem


  byte* p = (byte* )malloc(length);
  memcpy(p, payload, length);
  free(p);
}

boolean reconnect() 
{
  if (client.connect("arduino", "arduino", "ard123")) 
  {
    client.publish("teste","hello world");
    client.subscribe("recebido");
  }
  feedback(3);  // Conectado
  Serial.println("Conectado MQTT");
  return client.connected();
}

void setup()
{
  lastReconnectAttempt = 0;
  
  Serial.begin(9600);
  Serial.println("Iniciando...");
  delay(50);
  feedback(2);
  Ethernet.begin(mac);  

  if (client.connect("arduino", "arduino", "ard123")) // Nome do dispositivo, usuário e senha respectivamente
  {
    client.publish("teste", "v"); // Topico e mensagem respectivamente
    client.subscribe("recebido");  
   
    Serial.println("Conectado MQTT");
    delay(50);
    feedback(3);    
  }

  else
  {
    Serial.println("Erro ao Conectar!!!"); 
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
      
      if (reconnect()) 
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  
  else 
  {
    client.loop();
  }
}
