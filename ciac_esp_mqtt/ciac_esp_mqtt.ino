// --- MQTT ---
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient esp32Client; //Objeto do WiFi
PubSubClient client(esp32Client); //Objeto do MQTT usando o obejto do WiFi

// Variáveis de acesso à rede
const char* ssid     = "SuaSenha";
const char* password = "12345";

// Variáveis de acesso ao broker MQTT
const char* mqtt_Broker = "broker.emqx.io";
const char* mqtt_ClientID = "esp_nick";

// Tópicos em que o ESP se increverá
const char* topico_shut = "CIAC/SHUT";
const char* topico_air_temp = "CIAC/AIR_TEMP";
const char* topico_air_cont = "CIAC/AIR_CONT";
const char* topico_lumi = "CIAC/LUMI";

// Variáveis que serão atribuídas com as mensagens recebidas do MQTT
float temperatura;
float consumo_ener;
bool presenca;

// Tópicos em que o ESP publicará
const char* topico_temp = "CIAC/TEMP";
const char* topico_ener = "CIAC/ENER";
const char* topico_pres = "CIAC/PRES";

// --- Comunicação Serial ---

#include <SoftwareSerial.h> //Biblioteca que simplifica o Serial

//Define os pinos responsáveis pelo Serial
#define RXD 3 
#define TXD 1

SoftwareSerial MQTT(RXD, TXD); //Declara o objeto

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
    client.subscribe(topico_air_cont);
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
  for (int i=0;i<length;i++) {
    sub = (char)payload[i];
    Serial.print(sub);
    //Forma a estrutura da mensagem JSON
    String json;
    DynamicJsonDocument doc(1024);
    //Separa de que tópico a mensagem foi recebida, atribui um ID de acordo com o tópico e realiza a conversão do tipo da mensagem
    if(topic == topico_shut){
    doc["REC"] = topico_shut;
    if(sub == "1")  sub = true;
    else if(sub == "0") sub = false;
    doc["MQTT/SHUT"] = sub;
    }
    if(topic == topico_air_cont){
    doc["REC"] = topico_air_cont;
    doc["MQTT/AIR_CONT"] = sub.toInt();
    }
    if(topic == topico_air_temp){
    doc["REC"] = topico_air_temp;
    doc["MQTT/AIR_TEMP"] = sub.toInt();
    }
    if(topic == topico_lumi){
    doc["REC"] = topico_lumi;
    if(sub == "1")  sub = true;
    else if(sub == "0") sub = false;
    doc["MQTT/LUMI"] = sub;
    }
    //Envia a mensagem formatada ao outro ESP conectado na rede via Serial
    String msg ;
    serializeJson(doc, msg); 
    MQTT.println( msg );
    MQTT.flush();
  }
  Serial.println();
}

void loop() {
  // caso o cliente não esteja conectado a conexão é (re)feita
  if (!client.connected()) {
    conectaMQTT();
  }
  client.loop();
  //Se tiver mensagens disponíveis
  if(MQTT.available())
  {
    String message = "";
    message = MQTT.read(); //Lê a Serial
    delay(10);
    Serial.print("Received Message - ");
    Serial.println(message);
    //Forma a estrutura da mensagem JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    //Identifica o dado recebido e publica no MQTT
    if(doc["TOPIC"] == "TEMP"){
      temperatura = doc["MQTT/TEMP"];
      Serial.println(temperatura);
      client.publish(topico_temp, String(temperatura).c_str(), true);
      Serial.println("Publicacao enviada");
    }
    if(doc["TOPIC"] == "ENER"){
      consumo_ener = doc["MQTT/ENER"];
      Serial.println(consumo_ener);
      client.publish(topico_ener, String(consumo_ener).c_str(), true);
      Serial.println("Publicacao enviada");
    }
    if(doc["TOPIC"] == "PRES"){
      presenca = doc["MQTT/PRES"];
      Serial.println(presenca);
      client.publish(topico_pres, String(presenca).c_str(), true);
      Serial.println("Publicacao enviada");
    }
  }
}