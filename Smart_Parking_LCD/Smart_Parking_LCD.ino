#include <SPI.h>
#include <UIPEthernet.h>
#include <utility/logging.h>
#include <PubSubClient.h>

#include <LiquidCrystal.h>
LiquidCrystal lcd(9, 8, 5, 4, 3, 6);

long lastReconnectAttempt = 0;

int vagas = 10;

byte vagas1[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

char payload = 0;
char topic = 0;

char payloadAsChar;

int ledVerde = A0;
int ledAmarelo = A1;
int ledVermelho = A2;
int buzzer = 7;

byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xF1, 0x64};  // Mudar de acordo com a rede

void callback(char* topic, byte* payload, unsigned int length); // Callback function header

EthernetClient ethClient;

PubSubClient client("m10.cloudmqtt.com", 13038 , callback, ethClient); // Dados do MQTT Cloud


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

void feedback(char msg)
{
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(buzzer, OUTPUT);
  char estadoConexao;

  estadoConexao = msg;

  //Serial.begin(9600);

  switch (estadoConexao)
  {
    case '1': // Desconectado
      {
        acendeLed(ledVermelho, ledVerde, ledAmarelo);
        delay(100);
        break;
      }
    case '2': // Conectando
      {
        acendeLed(ledAmarelo, ledVermelho, ledVerde);
        break;
      }
    case '3': // Conectado
      {
        acendeLed(ledVerde, ledAmarelo, ledVermelho);
        break;
      }
    case '4': // Enviando Mensagem
      {
        acendeLed(ledVerde, ledAmarelo, ledVermelho);
        tone(buzzer, 500);
        delay(500);
        noTone(buzzer);
        break;
      }
    case '5': // Recebendo Mensagem
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

void displayLcd()
{
  lcd.setCursor(8, 1);
  lcd.print("        ");
  lcd.setCursor(8, 1);
  lcd.println(vagas);

  Serial.print("Ocupadas: ");
  Serial.println(10 - vagas);
}

void callback(char* topic, byte* payload, unsigned int length)
{
  int msgRecebida = payload[0] - '0';
  //  //payloadAsChar[length] = 0;
  //  String payloadAsChar = payload;
  //
  //  int msgRecebida = payloadAsChar.toInt();

  //int topic1 = (int)topic;


  vagas1[topic[5] - '0' - 1] = msgRecebida;

  //  Serial.print("Vaga: ");
  //  Serial.println(topic[5]);
  //  Serial.println(topic[5]-1);
  //  Serial.print("Mensagem: ");
  //  Serial.println(msgRecebida);
  //  Serial.println(vagas1[0]);

  //  Serial.print("Código Vagas ");
  //  Serial.println("2 ");
  //  Serial.print("codigo msg ");
  //  Serial.println(vagas1[1]);
  //
  //  Serial.print("Código Vagas ");
  //  Serial.println("3 ");
  //  Serial.print("codigo msg ");
  //  Serial.println(vagas1[2]);
  //
  //  Serial.print("Código Vagas ");
  //  Serial.println("4 ");
  //  Serial.print("codigo msg ");
  //  Serial.println(vagas1[3]);


  for (int i = 0; i < 10 ; i++)
  {
    Serial.print("Código Vagas ");
    Serial.println(i);
    Serial.print("msg ");
    Serial.println(vagas1[i]);

  }


  delay(100);
  feedback(5);  // Recebendo Mensagem


  if (msgRecebida == 1)
  {
    //Serial.println("OCUPADO");
    vagas--;
    displayLcd();
  }

  if (msgRecebida == 0)
  {
    //Serial.println("VAZIO");
    vagas++;
    displayLcd();
  }

  Serial.flush();

  byte* p = (byte* )malloc(length);
  memcpy(p, payload, length);
  free(p);
}

boolean reconnect()
{
  if (client.connect("arduino", "rodolfo", "rodolfo"))
  {
    client.subscribe("vaga/#");
    //client.publish("vaga/1", "conectado");
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

//  callback('vaga/1', 0, 10);
//  delay(500);
//  callback('vaga/2', 1, 10);
//  delay(500);
//  callback('vaga/3', 1, 10);
//  delay(500);

  lcd.begin(16, 2); //Inicializa LCD
  lcd.clear();     //Limpa o LCD

  lcd.setCursor(0, 0);
  lcd.print("VAGAS:  10");
  lcd.setCursor(0, 1);
  lcd.print("LIVRES: ");
  lcd.setCursor(8, 1);
  lcd.print(10);

  delay(50);
  feedback(2);
  Ethernet.begin(mac);

  if (!Ethernet.begin(mac))
  {
    Serial.println("DHCP Failed");
  }

  else
  {
    Serial.println(Ethernet.localIP());
  }

  if (client.connect("arduino", "rodolfo", "rodolfo")) // Nome do dispositivo, usuário e senha respectivamente
  {
    client.subscribe("vaga/#");
    //client.publish("vaga/1", "conectado");

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
