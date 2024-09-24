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

String message = "";  //String que recebe o dado Serial
bool mensagemRecebida = false;  //Se a mensagem do Serial esta pronta para ser lida

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
  //Se tiver mensagens disponíveis e não tiver trânsito de dados
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
  if(mensagemRecebida){

      //Forma a estrutura da mensagem JSON
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, message);

      if (error)
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }

      /*//Identifica o dado recebido e atribui em uma mensagem
      if (doc.containsKey("MQTT/SHUT")){ 
        shutdown = doc["MQTT/SHUT"];
        doc["SHUT"] = shutdown;
      }
      if (doc.containsKey("MQTT/AIR_TEMP")){ 
        temperatura_ar_cond = doc["MQTT/AIR_TEMP"];
        doc["AIR_TEMP"] = temperatura_ar_cond;
      }
      if (doc.containsKey("MQTT/AIR_CONT_ON")){ 
        controle_ar_cond = doc["MQTT/AIR_CONT_ON"];
        doc["AIR_CONT_ON"] = controle_ar_cond;
      }
      if (doc.containsKey("MQTT/AIR_CONT_OFF")){ 
        controle_ar_cond = doc["MQTT/AIR_CONT_OFF"];
        doc["AIR_CONT_OFF"] = controle_ar_cond;
      }
      if (doc.containsKey("MQTT/LUMI")){ 
        ativar_lumi = doc["MQTT/LUMI"];
        doc["ON_LUMI"] = ativar_lumi;
      }*/

      //Envia para a rede mesh
      String msg ;
      serializeJson(doc, msg);
      //Envia na rede MESH
      mesh.sendBroadcast( msg );
      Serial.println(msg);
      //conclui a filtragem da mensagem, podendo assim processar outra mensagem
      mensagemRecebida = false;
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
  JsonDocument doc;
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }

  /*//Recebe os dados da rede
  if(doc.containsKey("LUMI")) luminosidade = doc["LUMI"];

  if(doc.containsKey("TEMP")) {
    temperatura = doc["TEMP"];
    //Forma uma mensagem repassando o dado para o ESP32 MQTT via Serial
    JsonDocument doc;
    doc["MQTT/TEMP"] = temperatura;
    serializeJson(doc, MQTT);
    MQTT.println("");
    MQTT.flush();
    delay(10);
  }
  if(doc.containsKey("PRES")) {
    presenca = doc["PRES"];
    //Forma uma mensagem repassando o dado para o ESP32 MQTT via Serial
    JsonDocument doc;
    doc["MQTT/PRES"] = presenca;
    serializeJson(doc, MQTT);
    MQTT.println("");
    MQTT.flush();
    delay(10);
  }
  if(doc.containsKey("ENER")) {
    consumo_ener = doc["ENER"];
    //Forma uma mensagem repassando os dados para o ESP32 MQTT via Serial
    JsonDocument doc;
    doc["MQTT/ENER"] = consumo_ener;
    //Envia para o outro ESP32
    serializeJson(doc, MQTT);
    MQTT.println("");
    MQTT.flush();
    delay(10);
  }*/

  //se não estiver com o desligamento ativo, tiver presença e não tiver luminosidade, então o relé atracará
  if(!shutdown && presenca && !luminosidade)  digitalWrite(Driver, 1);
  else digitalWrite(Driver, 0);

  //Envia os dados recebidos para o outro ESP32
  serializeJson(doc, MQTT);
  MQTT.println(""); //Espaço crucial para que o remetente entenda corretamente
  MQTT.flush();
  delay(10);
}

//Função que envia mensagem
void sendMessage() {
}