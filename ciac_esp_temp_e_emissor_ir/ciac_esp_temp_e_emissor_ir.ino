// --- Rede MESH ---

#include "painlessMesh.h" //Biblioteca responsável pela comunicação entre os ESP32
#include <ArduinoJson.h> //Biblioteca responsável pela formatação em JSON

// predefinições da Rede MESH
#define   MESH_PREFIX     "AlgoQueVocêGoste"
#define   MESH_PASSWORD   "AlgoEscondido"
#define   MESH_PORT       5555

Scheduler userScheduler; //Objeto que controla as tarefas
painlessMesh  mesh; //Objeto da rede

void sendMessage() ; //Tem que existir para funcionar

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage ); //Criação da tarefa de mandar mensagem

// --- IR ---

#include <Arduino.h> //Biblioteca que adiciona mais funções vindas do ARDUINO
#include <IRremoteESP8266.h> //Biblioteca responsável pelo IR
#include <IRsend.h> //Biblioteca responsável pelo emissor IR

const uint16_t kIrLed = 10; //Pino do sensor

IRsend irsend(kIrLed); //Objeto do IR

// --- DS18b20 ---

#include <OneWire.h> //Biblioteca com a funcionalidade de conexão
#include <DallasTemperature.h> //Biblioteca responsável pelo sensor de temperarura

const int oneWireBus = 0; //Pino do sensor

//Objeto do sensor de temperatura
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

float temperatura; //Variável da temperatura

// --- MQTT ---

bool presenca = true;
bool shutdown = false;
int graus_ar = 25;
float ar_temperatura_on = 10;
int graus_ar_mudanca = 25;

void setup()
{
  Serial.begin(115200);
    
  irsend.begin(); //Inicaliza o emissor IR

  sensors.begin(); //Inicializa o DS18b20

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION ); //Define as mensagens de eventos na rede MESH
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT ); //Inicializa a rede
  //Declara os eventos
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.setContainsRoot(true); //declara que existe um root na rede

  userScheduler.addTask( taskSendMessage ); //Adicioana a função na tarefa
  taskSendMessage.enable(); //Inicia as tarefas
}
void loop() {
  mesh.update(); //Atualiza a rede
}

//Funções de mensagem para eventos na rede MESH
void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> Comecou aqui: Nova conexao, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Conexoes trocadas\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Tempo ajustado %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

//Função de enviar mensagem
void sendMessage() {
  //Recebe a temperatura do sensor
  sensors.requestTemperatures(); 
  temperatura = sensors.getTempCByIndex(0);
  Serial.print(temperatura);   Serial.println("ºC");

  //Atribui um ID e a temperatura em JSON e o envia
  DynamicJsonDocument doc(1024);
  doc["ID"] = "Sensor TEMP";
  doc["TEMP"] = temperatura;
  String msg ;
  serializeJson(doc, msg);
  mesh.sendBroadcast( msg );
  Serial.println(msg);
  taskSendMessage.setInterval((TASK_SECOND * 10)); //intervalo de 10 segundos
}

//Função que é executada quando recebe uma mensagem
void receivedCallback( uint32_t from, String &msg ) {
  //Recebe a mensagem em JSON e o lê
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }

  //Se receber dados de determinados nodos, coleta a mensagem
  if(doc["ID"] == "AIR_TEMP") ar_temperatura_on = doc["AIR_TEMP"];
  if(doc["ID"] == "AIR_CONT") graus_ar_mudanca = doc["AIR_CONT"];
  if(doc["ID"] == "SHUT") shutdown = doc["SHUT"];
  if(doc["ID"] == "Sensor PRES") presenca = doc["PRES"];

  if(!shutdown && presenca && temperatura >= ar_temperatura_on){  
    irsend.sendNEC(0x00FFE02FUL); //Ligar ar-condicionado
    delay(50);
    
  } else{
      irsend.sendNEC(0x00FFE01FUL); //Desiga o ar-condicionado
      delay(50);
  }
  while(graus_ar_mudanca < graus_ar){
    irsend.sendNEC(0x00FFE03FUL); //aumenta a temperatura do ar
    delay(50);
    graus_ar += 1;
  }
  while(graus_ar_mudanca > graus_ar){
  irsend.sendNEC(0x00FFE04FUL); //diminui a temperatura do ar
  delay(50);
  graus_ar -= 1;
  }
}