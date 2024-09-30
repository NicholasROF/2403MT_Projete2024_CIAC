/*Créditos:

Inspiração para rede Mesh - https://github.com/techiesms/Internet-to-Mesh-Networking-/tree/master

Inspiração para emissor IR - https://github.com/crankyoldgit/IRremoteESP8266/blob/master/examples/IRsendDemo/IRsendDemo.ino

Inspiração para sensor de temperatura - https://randomnerdtutorials.com/esp32-ds18b20-temperature-arduino-ide/

*/
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
float ar_temperatura_on = 23;
float ar_temperatura_off = 20;
int graus_ar_mudanca = 25;
bool ligado = false;

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

  //Atribui a temperatura em JSON e o envia
  JsonDocument doc;
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
  JsonDocument doc;
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }

  //Se receber dados de determinados nodos, coleta a mensagem
  if(doc.containsKey("AR_TEMP")) graus_ar_mudanca = doc["AR_TEMP"];
  if(doc.containsKey("AR_CONT_ON")) ar_temperatura_on = doc["AR_CONT_ON"];
  if(doc.containsKey("AR_CONT_OFF")) ar_temperatura_off = doc["AR_CONT_OFF"];
  if(doc.containsKey("SHUT")) shutdown = doc["SHUT"];
  if(doc.containsKey("PRES")) presenca = doc["PRES"];

  if(ar_temperatura_on > ar_temperatura_off){
    //Se não tiver um desligamento forçado, tiver presença e a temperatura dor maior do que o configurado
    if(!shutdown && presenca && temperatura >= ar_temperatura_on && !ligado){  
      Serial.println("Condição 1");
      Serial.println(ar_temperatura_on);
      Serial.println(ar_temperatura_off);
      for(int i = 0; i < 5; i++){
        irsend.sendNEC(0x00FFE22FUL); //Ligar ar-condicionado
      }
      delay(250);
      ligado = true;
    } else if(!shutdown && presenca && temperatura < ar_temperatura_off && ligado){ //Se a temperatura for menor do que o configurado
      Serial.println("Condição 2");
      Serial.println(ar_temperatura_on);
      Serial.println(ar_temperatura_off);
      for(int i = 0; i < 5; i++){
        irsend.sendNEC(0x00FFE11FUL); //Desliga o ar-condicionado
      }
      delay(250);
      ligado = false;
    } else if(shutdown || !presenca){ //Se a temperatura for menor do que o configurado
      Serial.println("Condição 3");
      for(int i = 0; i < 5; i++){
        irsend.sendNEC(0x00FFE11FUL); //Desliga o ar-condicionado
      }
      delay(250);
      ligado = false;
    }
  }
  while(graus_ar_mudanca > graus_ar){ //Se tiver mudança na temperatura do ar-condicionado
    Serial.println("Valor emitido!");
    irsend.sendNEC(0x00FFE33FUL); //aumenta a temperatura do ar
    graus_ar += 1;
    delay(250);
  }
  while(graus_ar_mudanca < graus_ar){
    Serial.println("Valor emitido!");
    irsend.sendNEC(0x00FFE44FUL); //diminui a temperatura do ar
    graus_ar -= 1;
    delay(250);
  }
}