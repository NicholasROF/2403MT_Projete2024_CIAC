/*Créditos:

Inspiração para receptor IR - https://lobodarobotica.com/blog/controle-remoto-com-arduino-ir/ ;
https://github.com/crankyoldgit/IRremoteESP8266/blob/master/examples/IRrecvDemo/IRrecvDemo.ino

Inspiração para display OLED - https://www.makerhero.com/blog/controlando-um-display-oled-com-a-biblioteca-ssd1306/

*/
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
bool mudanca = true; //Se houve uma alteração de informação para o Display atualzar

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
  if(mudanca){
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
  mudanca = false; //Fecha a atualização
  }
  //Se receber algum sinal, o atribuirá a uma variável
  if (irrecv.decode(&results)) {
    mudanca = true; //Abre a atualização
    Serial.print("Valor: ");
    Serial.println(results.value, HEX);
    armazenavalor = (results.value);
    //Identifa o sinal e executa certas funções dependendo desses
    if(armazenavalor == 0xFFE44F || armazenavalor == 0xC8072A411 || armazenavalor == 0xC80722411) graus = graus - 1; //diminui o texto da temperatura em 1
    else if(armazenavalor == 0xFFE33F || armazenavalor == 0xC80722410 || armazenavalor == 0xC8072A410)  graus++; //aumnenta o texto da temperatura em 1
    else if(armazenavalor == 0xFFE22F || armazenavalor == 0xC8072241E || armazenavalor == 0xC8072A41E)  ligado = true; //"Liga" o lcd
    else if(armazenavalor == 0xFFE11F || armazenavalor == 0xC8072A41F || armazenavalor == 0xC8072241F)  ligado = false; //"Desliga" o lcd
    delay(50);
    irrecv.resume(); //Retoma a recepção de novos sinais
  }
}