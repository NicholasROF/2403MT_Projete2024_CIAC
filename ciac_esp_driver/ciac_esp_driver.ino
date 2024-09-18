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

bool luminosidade;
float temperatura;
float consumo_ener;
bool presenca;

// --- Comunicação SERIAL ---

#include <SoftwareSerial.h> //Biblioteca que simplifica o Serial

//Define os pinos responsáveis pelo Serial
#define RXD 3 
#define TXD 1

SoftwareSerial MQTT(RXD, TXD); //Declara o objeto

// --- Driver Relé ---

const int Driver = 10; //Pino conectado na base do transistor

// --- MQTT ---

//Dados recebidos do MQTT
bool shutdown = false;
bool ativar_lumi = false;
int temperatura_ar_cond = 30;
float controle_ar_cond = 25;

void setup()
{
  pinMode(Driver, OUTPUT); //Definição do pino do driver
  digitalWrite(Driver, 0); //Começa desligado

  Serial.begin(115200);
  MQTT.begin(9600); //Inicializa a Serial que se comunica com o outro ESP32

  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION ); //Define as mensagens de eventos na rede MESH
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT ); //Inicializa a rede
  //Declara os eventos
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.setContainsRoot(true); //Declara que existe um root na rede
  mesh.setRoot(true); //Declara que esse nodo é o root

  userScheduler.addTask( taskSendMessage ); //Adicioana a função na tarefa
  taskSendMessage.enable(); //Inicia as tarefas
}

void loop() {
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
    //Identifica o dado recebido e atribui em uma mensagem
    if (doc["TOPIC"] == "CIAC/SHUT"){ 
      shutdown = doc["MQTT/SHUT"];
      doc["ID"] = "SHUT";
      doc["SHUT"] = shutdown;
    }
    if (doc["TOPIC"] == "CIAC/AIR_TEMP"){ 
      temperatura_ar_cond = doc["MQTT/AIR_TEMP"];
      doc["ID"] = "AIR_TEMP";
      doc["AIR_TEMP"] = temperatura_ar_cond;
    }
    if (doc["TOPIC"] == "CIAC/AIR_CONT"){ 
      controle_ar_cond = doc["MQTT/AIR_CONT"];
      doc["ID"] = "AIR_CONT";
      doc["AIR_CONT"] = controle_ar_cond;
    }
    if (doc["TOPIC"] == "CIAC/LUMI"){ 
      ativar_lumi = doc["MQTT/LUMI"];
      doc["ID"] = "ON_LUMI";
      doc["ON_LUMI"] = ativar_lumi;
    }
    String msg ;
    serializeJson(doc, msg);
    //Envia na rede MESH
    mesh.sendBroadcast( msg );
    Serial.println(msg);
    }
  mesh.update(); ///Atualiza a rede
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

//Função que é executada quando recebe uma mensagem
void receivedCallback( const uint32_t &from, const String &msg ) {
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

  //Recebe os dados da rede
  if(doc["ID2"] == "Sensor LUMI") luminosidade = doc["LUMI"];
  if(doc["ID"] == "Sensor TEMP") {
    temperatura = doc["TEMP"];
    //Forma uma mensagem repassando o dado para o ESP32 MQTT via Serial
    DynamicJsonDocument doc(1024);
    doc["TOPIC"] = "TEMP";
    doc["MQTT/TEMP"] = temperatura;
    String msg ;
    serializeJson(doc, msg);
    MQTT.println( msg );
    MQTT.flush();
  }
  if(doc["ID"] == "Sensor PRES") {
    presenca = doc["PRES"];
    //Forma uma mensagem repassando o dado para o ESP32 MQTT via Serial
    DynamicJsonDocument doc(1024);
    doc["TOPIC"] = "PRES";
    doc["MQTT/PRES"] = presenca;
    String msg ;
    serializeJson(doc, msg);
    MQTT.println( msg );
    MQTT.flush();
  }
  if(doc["ID"] == "Sensor ENER") {
    consumo_ener = doc["ENER"];
    //Forma uma mensagem repassando o dado para o ESP32 MQTT via Serial
    DynamicJsonDocument doc(1024);
    doc["TOPIC"] = "ENER";
    doc["MQTT/ENER"] = consumo_ener;
    String msg ;
    serializeJson(doc, msg);
    MQTT.println( msg );
    MQTT.flush();
  }

  //se não estiver com o desligamento ativo, tiver presença e não tiver luminosidade, então o relé atracará
  if(!shutdown && presenca && !luminosidade)  digitalWrite(Driver, 1);
  else digitalWrite(Driver, 0);
}

//Função que envia mensagem
void sendMessage() {
 
}