// --- IR ---

//Bibliotecas que adicionam novas funcionalidades
#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>

#include <IRremoteESP8266.h> //Biblioteca responsável pelo IR

const uint16_t kRecvPin = 0; //Pino do receptor IR

IRrecv irrecv(kRecvPin); //Objeto do receptor IR
decode_results results; //Objeto em que será atribuido os sinais captados

// --- DISPLAY OLED SSD1306 ---

#include <Wire.h> //Biblioteca com a funcionalidade de conexão
//Bibliotecas do Display Oled
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

float armazenavalor; //Variável em que os sinais são atribuídos
int graus = 25; //Valor inicial de temperaruta do Display
bool ligado = true; //Se o Display deve exibir a temperatura, inicialmente ligado

Adafruit_SSD1306 display = Adafruit_SSD1306(); //bjeto do display

void setup() {
  Serial.begin(115200);

  Wire.begin(); //Inicializa o objeto Wire

  irrecv.enableIRIn();  //Inicializa o receptor IR

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Inicializa o Display

  display.setTextColor(WHITE); //Define a cor do texto
  display.clearDisplay(); //Limpa o display
}

void loop() {
  //Se o display estiver "ligado" então ele exibirá os dados
  if(ligado){
    display.clearDisplay(); //Limpa o display
    //Define o tamanho e posição do texto "Temperatura: "
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Temperatura: ");
    //Define o tamanho e posição do texto da variável da temperatura
    display.setTextSize(2);
    display.setCursor(0,10);
    display.print(graus);
    display.write(248); //Adiciona o ícone de graus
    display.print("C"); //Adiciona o "C" de Celsius
    display.display(); //Exibe os textos prescritos
  }
  else{
    display.clearDisplay(); //Limpa a tela
    display.display(); //Exibe o que foi predefinido, nesse caso, nada
  }
  //Se receber algum sinal, o atribuirá a uma variável
  if (irrecv.decode(&results)) {
    Serial.print("Valor: ");
    Serial.println(results.value, HEX);
    armazenavalor = (results.value);
    //Identifa o sinal e executa certas funções dependendo desses
    if(armazenavalor == 0x00FFE04F) graus = graus - 1; //diminui o texto da temperatura em 1
    else if(armazenavalor == 0xFFE03F)  graus++; //aumnenta o texto da temperatura em 1
    else if(armazenavalor == 0xFFE02F)  ligado = true; //"Liga" o display
    else if(armazenavalor == 0xFFE01F)  ligado = false; //"Desliga" o display
    delay(50);
    irrecv.resume(); //Retoma a recepção de novos sinais
  }
}