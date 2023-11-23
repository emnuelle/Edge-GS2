#include <WiFi.h>
#include <PubSubClient.h>
#include <Keypad.h>

const byte LINHAS = 4;
const byte COLUNAS = 4;
char teclas[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte pinosLinhas[LINHAS] = {2, 16, 17, 18};
byte pinosColunas[COLUNAS] = {25, 26, 27, 14};

int numeros[] = {1, 2, 3, 4, 5};
const char *informacoes[] = {"Paciente com bronquite", "Paciente com ansiedade e TDAH", "Paciente com Osteoporose", "Paciente com Diabetes", "Paciente com arteriosclerose"};
int tamanhoLista = sizeof(numeros) / sizeof(numeros[0]);

// WIFI
const char* SSID = "Wokwi-GUEST"; // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = ""; // Senha da rede WI-FI que deseja se conectar

// MQTT
const char* BROKER_MQTT = "46.17.108.113"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT
//definindo id mqtt
#define ID_MQTT  "HT300"

//#define TOPICO_SUBSCRIBE    "/TEF/HT300/cmd"
#define TOPICO_PUBLISH   "/TEF/HT300/attrs/s"

Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  initWiFi();
  initMQTT();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10); // this speeds up the simulation
  VerificaConexoesWiFIEMQTT();
  char tecla = teclado.getKey();
  
  if (tecla) {
    int numeroDigitado = tecla - '0';
    bool encontrado = false;
    
    for (int i = 0; i < tamanhoLista; ++i) {
      if (numeros[i] == numeroDigitado) {
        Serial.print("Numero encontrado: ");
        Serial.println(numeroDigitado);
        Serial.print("Informacao correspondente: ");
        Serial.println(informacoes[i]);
        encontrado = true;
        MQTT.publish(TOPICO_PUBLISH,informacoes[i]);
        break;
      }
    }
    
    if (!encontrado) {
      Serial.println("Numero nao encontrado na lista.");
      MQTT.publish(TOPICO_PUBLISH,"não encontrado");
    }
  }
  MQTT.loop();
}

void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconectWiFi();
}

void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD, 6); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            //MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
     
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
    
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);
}

void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}