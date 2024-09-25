/*Créditos:

Inspiração para rede Mesh - https://github.com/techiesms/Internet-to-Mesh-Networking-/tree/master

Inspiração para ACS - https://www.youtube.com/watch?v=ofqvW-8_H2M&pp=ygUMYWNzNzEyIGVzcDMy

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

// --- ACS ---

const int pino_asc = 0; //Pino que o ACS está conectado

// Variáveis responsáveis pela lógica do ACS
int medicao = 0;
double medida_media = 0;
double tensao_obtida = 0.0;
double calibracao = 0.0;
const float sense = 0.185;
double tensao_delta = 0.0;  
const float tensao = 5;
float potencia = 0.0;
float gasto_energetico = 0.0;
float corrente = 0.0;

void setup()
{
  pinMode(pino_asc, INPUT); //Declara o pino do ACS
  Serial.begin(115200);

  //realiza as medidas
  for(int i = 0; i < 1000; i++){
 	  medicao = analogRead(pino_asc);
    medida_media = medida_media + ((double)medicao);
    delayMicroseconds(100);
  }
  //Tira a média para que o valor não fique flutuando
  medida_media=medida_media/1000;
  Serial.print("Valor à vazio: ");
  //Calcula a tensão mensurada pelo ESP sem que haja corrente para servir de base para as futuras medidas
  calibracao = medida_media * ((double)3.3/4095.0); //Valor sem corrente
  Serial.println(calibracao);

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

  medida_media = 0;
  //realiza as medidas
  for(int i = 0; i < 1000; i++){
 	  medicao = analogRead(pino_asc);
    medida_media = medida_media + ((double)medicao);
    delayMicroseconds(100);
  }
  //Tira a média para que o valor não fique flutuando
  medida_media=medida_media/1000;
  Serial.print("Valor bruto sensor: ");
  Serial.println(medida_media);
  //Calcula a tensão mensurada pelo ESP
  tensao_obtida = medida_media * ((double)3.3/4095.0);
  tensao_delta = tensao_obtida - calibracao;
  //Calcula a corrente baseada na tensão
  corrente = (tensao_delta / sense);
  corrente = sqrt(pow(corrente, 2));
  Serial.print("Valor bruto tensao: ");
  Serial.println(tensao_obtida);
  Serial.print("Valor variacao tensao: ");
  Serial.println(tensao_delta);
  Serial.print("corrente: ");
  Serial.println(corrente);
  //Calcula o consumo
  potencia = corrente * tensao; //Watts/segundo
  gasto_energetico = potencia * 3600; //Watts/hora

  //Atribui o valor do consumo em JSON e o envia
  JsonDocument doc;
  doc["ENER"] = (int)gasto_energetico;
  String msg ;
  serializeJson(doc, msg);
  mesh.sendBroadcast( msg );
  Serial.println(msg);
  taskSendMessage.setInterval((TASK_SECOND * 10)); //intervalo de 10 segundos
}

//Função que é executada quando recebe uma mensagem
void receivedCallback( uint32_t from, String &msg ) {
}