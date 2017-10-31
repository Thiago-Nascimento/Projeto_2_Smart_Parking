#include <SPI.h>

/*//Se for usar a placa de rede normal*/
//#include <UIPEthernet.h>
//#include <utility/logging.h>

/*Se for usar a placa Shield*/
#include <Ethernet.h>

/*Para o MQTT*/
#include <PubSubClient.h>

//Carrega a biblioteca LCD
#include <LiquidCrystal.h>

//Define os pinos que serão ligados ao LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

char* payloadAsChar;
String hexstring;
//String EstadoVaga = "vazia";
int Ocupadas = 0;
int Vazia = 3;



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

  String payloadAsChar = payload;
  int msgComoNumero = payloadAsChar.toInt();
     Serial.println(msgComoNumero);
  if (msgComoNumero == 1) {
    Serial.println("OCUPADO");
    lcd_ocupada();

  }

  if (msgComoNumero == 0) {
    Serial.println("VAZIO");
    lcd_vazia();

  }

  Serial.flush();

  //;


  //
  //  Serial.flush();
}

/*Dados do MQTT Cloud*/
PubSubClient client(server, port, whenMessageReceived, ethClient);
long lastReconnectAttempt = 0;

void setup() {

  Serial.begin(9600); //Inicializa a serial
  lcd.begin(16, 2); //Inicializa LCD
  lcd.clear();     //Limpa o LCD

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

void lcd_ocupada() {
  Ocupadas = Ocupadas + 1;
  Vazia = Vazia - 1;
  lcd.setCursor(0, 0);
  lcd.print("VAZIA : ");
  lcd.print("        ");
  lcd.setCursor(8, 0);
  lcd.print(Vazia);
}

void lcd_vazia() {
  Ocupadas = Ocupadas - 1 ;
  Vazia = Vazia + 1 ;
  lcd.setCursor(0, 1);
  lcd.print("OCUPADAS : ");
  lcd.print("        ");
  lcd.setCursor(11, 1);
  lcd.print(Ocupadas);
}
