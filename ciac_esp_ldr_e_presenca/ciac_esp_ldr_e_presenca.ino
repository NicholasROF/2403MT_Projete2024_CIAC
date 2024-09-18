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

// --- LDR ---

const int LDR = 0;
int valorsensor;
bool luminosidade;

// --- HLK-LD2410b ---

const int sensor_de_presenca = 10;
bool presenca;

// --- MQTT ---

bool lumi_on = false;

void setup(){
  pinMode(LDR, INPUT); //Declaração do pino do LDR
  pinMode(sensor_de_presenca, INPUT); //Declaração do sensor de presença
  
  Serial.begin(115200); 

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

//Função que envia mensagem
void sendMessage() {
  //Recebe o valor da presença
  presenca = digitalRead(sensor_de_presenca);
 
  //Atribui um ID e o parametro da presença em JSON
  DynamicJsonDocument doc(1024);
  doc["ID"] = "Sensor PRES";
  doc["PRES"] = !presenca;

  //Se luminosidade for relevante. Configurado pelo cliente
  if(lumi_on){
    valorsensor = analogRead(LDR);
    if(valorsensor >= 2000)  luminosidade = true;
    else luminosidade = false;

  } else luminosidade = false;

  //Adiciona um ID e o parametro da luminosidade em JSON e o envia
  doc["LUMI"] = luminosidade;
  doc["ID2"] = "Sensor LUMI";
  
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

  //Se receber dados a partir do MQTT, coleta o comando que deixa a luminosidade relevante ou não
  if(doc["ID"] == "ON_LUMI")  lumi_on = doc["ON_LUMI"];
}