// --- MQTT ---
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient esp32Client; //Objeto do WiFi
PubSubClient client(esp32Client); //Objeto do MQTT usando o obejto do WiFi

// Variáveis de acesso à rede
const char* ssid     = "SuaSenha";
const char* password = "12345";

// Variáveis de acesso ao broker MQTT
const char* mqtt_Broker = "seuBroker";
const char* mqtt_ClientID = "esp32";

// Tópicos em que o ESP se increverá
const char* topico_shut = "Seu_Projeto/SHUT";
const char* topico_air_temp = "Seu_Projeto/AIR_TEMP";
const char* topico_air_cont_on = "Seu_Projeto/AIR_CONT_ON";
const char* topico_air_cont_off = "Seu_Projeto/AIR_CONT_OFF";
const char* topico_lumi = "Seu_Projeto/LUMI";

// Variáveis que serão atribuídas com as mensagens recebidas do MQTT
float temperatura;
float consumo_ener;
bool presenca;

// Tópicos em que o ESP publicará
const char* topico_temp = "Seu_Projeto/TEMP";
const char* topico_ener = "Seu_Projeto/ENER";
const char* topico_pres = "Seu_Projeto/PRES";

// --- Comunicação Serial ---

#include <SoftwareSerial.h> //Biblioteca que simplifica o Serial

//Define os pinos responsáveis pelo Serial
#define RXD 3 
#define TXD 1

SoftwareSerial MQTT(RXD, TXD); //Declara o objeto

String message = "";
bool mensagemRecebida = false;

// --- JSON ---

#include <ArduinoJson.h> //Biblioteca responsável pelas mensagens em JSON

void setup() {
  Serial.begin(115200);
  MQTT.begin(9600); //Inicializa a Serial que se comunica com o outro ESP32

  conectaWifi(); //Função que conecta o ESP32 à rede WiFi

  client.setServer(mqtt_Broker, 1883); //Definição do broker MQTT
  client.setCallback(callback); //Define a função do recebimento de recebimento de mensagens
}

//Função que se conecta ao WiFi
void conectaWifi(){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

//Função que estabelece conexão com o broker MQTT
void conectaMQTT() {
  while (!client.connected()) {
    client.connect(mqtt_ClientID);
    Serial.print("*");
    client.subscribe(topico_shut);
    client.subscribe(topico_air_temp);
    client.subscribe(topico_air_cont_on);
    client.subscribe(topico_air_cont_off);
    client.subscribe(topico_air_temp);
    client.subscribe(topico_lumi);
  }
}

//Função do recebimento de recebimento de novas mensagens nos tópicos inscritos
void callback(char* topic, byte* payload, unsigned int length) {
  String sub = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  //Forma a estrutura da mensagem JSON
  JsonDocument doc;
  for (int i=0;i<length;i++) {
    sub += (char)payload[i];
  }
  Serial.println(sub);
  //Separa de que tópico a mensagem foi recebida, atribui um ID de acordo com o tópico e realiza a conversão do tipo da mensagem
  if(strcmp(topic, topico_shut) == 0){
    bool shut;
    if(sub == "1")  shut = true;
    else if(sub == "0") shut = false;
    doc["SHUT"] = shut;
  }
  if(strcmp(topic, topico_air_cont_on) == 0){
    doc["AR_CONT_ON"] = sub.toInt();
  }
  if(strcmp(topic, topico_air_cont_off) == 0){
    doc["AR_CONT_OFF"] = sub.toInt();
  }
  if(strcmp(topic, topico_air_temp) == 0){
    doc["AR_TEMP"] = sub.toInt();
  }
  if(strcmp(topic, topico_lumi) == 0){
    bool lumi;
    if(sub == "1")  lumi = true;
    else if(sub == "0") lumi = false;
    doc["ON_LUMI"] = lumi;
  }
  serializeJson(doc, Serial);
  Serial.println();
  //Envia a mensagem formatada ao outro ESP conectado na rede via Serial
  serializeJson(doc, MQTT);
  MQTT.println(""); //Espaço crucial para que o remetente entenda corretamente
  MQTT.flush();
  delay(10);
}

void loop() {
  //caso o cliente não esteja conectado a conexão é (re)feita
  if (!client.connected()) {
    conectaMQTT();
  }
  client.loop();
  //Se tiver mensagens disponíveis
  if(MQTT.available() && !mensagemRecebida)
  {
    message = ""; //Limpa a string
    message = MQTT.readStringUntil('\n'); //Lê a Serial até um espaço
    delay(10);
    Serial.print("Received Message - ");
    Serial.println(message);
    mensagemRecebida = true;  //Mensagem pronta

  }
  //Se tiver uma mensagem pronta
  if (mensagemRecebida){
    
    //Forma a estrutura da mensagem JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      MQTT.println("a");
    } else MQTT.println("A");

    //Identifica o dado recebido e publica no MQTT
    if(doc.containsKey("TEMP")){
      temperatura = doc["TEMP"];
      Serial.println(temperatura);
      client.publish(topico_temp, String(temperatura).c_str(), true);
      Serial.println("Publicacao enviada");
    }
    if(doc.containsKey("ENER")){
      consumo_ener = doc["ENER"];
      Serial.println(consumo_ener);
      client.publish(topico_ener, String(consumo_ener).c_str(), true);
      Serial.println("Publicacao enviada");
    }
    if(doc.containsKey("PRES")){
      presenca = doc["PRES"];
      Serial.println(presenca);
      client.publish(topico_pres, String(presenca).c_str(), true);
      Serial.println("Publicacao enviada");
    }
  }
}