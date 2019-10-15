//Bibliotecas ESP8266 e Firebase.
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
 
//Iniciações do banco de dados Firebase.
#define FIREBASE_HOST "lasid-test01.firebaseio.com"
#define FIREBASE_AUTH "f6vZk8KxKT5X6LLrutYjn1UrzeQnM3D0KMOhHzYL"
#define WIFI_SSID "LASID"
#define WIFI_PASSWORD "lasid@ci.ufpb.br"

//Controla as threads.
#include <Thread.h>
#include <ThreadController.h>

ThreadController cpu;                         //Criando um objeto da classe thread
Thread calcula_velocidade;                    //A cada tempo, verifica essa funções
Thread calcula_batimento;

//Utilizados para medir o batimento cardíaco 
#define USE_ARDUINO_INTERRUPTS true             // Configura interrupções de baixo nível para obter a matemática mais precisa de BPM.
#include <PulseSensorPlayground.h>              // Inclui a biblioteca PulseSensorPlayground .   

//Variaveis
int myBPM;
float batimento_medio;
int countb=0;
const int PulseWire = 0;                                    // PulseSensor fio amarelo conectado no pino A0
const int LED13 = 13;                                       // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;                                        // Determine qual sinal será "contado como batimentos" e qual será ignorado.
                                                            // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                                                            // Otherwise leave the default "550" value. 
                               
PulseSensorPlayground pulseSensor;              // Instancio o objeto PulseSensorPlayground chamado "pulseSensor"

//utilizados para medir a velocidade 
#define sensor 8
int tempo_atual;
int count = 0;
int countv = 0;
int flatminuto = 0;
boolean flag = 0;
boolean flag2 = 1;
float velocidade_atual = 0.0;
float velocidade_media = 0.0;
float distancia = 0.0;
long tempo = 0;
float distAcumulada = 0.0;
float comprimento = 0;
float raio = 4.75;                             // Colocar aqui o valor do raio em cm do cilíndro

float rRoda = 30.48;                           // Colocar aqui o valor do raio em cm da roda (aqui ta pra aro 24 que dá um comprimento de 60,96 DIV 2)
float comprimentoRoda = 0;
float rpm = 0.0;

float velocidadeInicial = 3.0;
float incrementoVeloc = 1.0;
long tempoEstagio = 60000;
long correcaoTempo = 0;

void calcula_batimentos(){
  myBPM = pulseSensor.getBeatsPerMinute();                // chama a função no objeto pulseSensor que retorna BPM como "int".
  
  if (pulseSensor.sawStartOfBeat()) {                     // Teste constante para observar se houve um batimento.
    batimento_medio += myBPM;
    Serial.print("batimento: ");
    Serial.println(myBPM);
    Serial.println("");
    Firebase.setFloat("Batimentos", myBPM);               // Print valor de myBPM. 
    countb++;
  }
    delay(20);            
}

void calcula_velocidades(){
  
    if (digitalRead(sensor)){
      flag=1;
    }
    
    if (!digitalRead(sensor) && flag) {
      flag = 0;
      count+=1;
    }
    tempo = millis() - correcaoTempo;

    if (tempo%3000 == 0) {   // 3 segundos     
      distancia = count * comprimento;
      distAcumulada = distAcumulada + distancia;
      Firebase.setFloat("Distancia", distAcumulada);
      Serial.print("distancia: ");
      Serial.println(distAcumulada);

      tempo_atual = tempo_atual/1000;
      Firebase.setInt("Tempo", tempo_atual/60);
      Serial.print("tempo: ");
      Serial.println(tempo_atual);
  
      velocidade_atual = (3.6 * distancia)/3;
      Firebase.setFloat("Velocidade", velocidade_atual);
      velocidade_media += velocidade_atual; 
      Serial.print("velocidade: ");
      Serial.println(velocidade_atual);  
      Serial.println("");    
  
      //rpm = 20*(count*(comprimento/comprimentoRoda));
         
      distancia = 0.0;
      count = 0;
      countv++;
  }
}

/*
char * TimeToString(unsigned long t)
{
  static char str[12];
  long h = t / 3600;
  t = t % 3600;
  int m = t / 60;
  int s = t % 60;
  sprintf(str, "%02ld:%02d:%02d", h, m, s);
  return str;
}*/

void encerra(){
  tempo_atual = tempo_atual/1000;
  Firebase.setFloat("Distancia", distAcumulada);
  Firebase.setInt("Tempo", (tempo_atual/60));
  Firebase.setFloat("VelocidadeMedia", (velocidade_media/countv));
  Firebase.setFloat("BatimentoMedio", (batimento_medio/countb));
  
  velocidade_media = batimento_medio = 0.0;
  countb = countv = distAcumulada = 0;
  
  Serial.print("distancia: ");
  Serial.println(distAcumulada);
  Serial.print("tempo: ");
  Serial.println(tempo_atual);
  Serial.print("velocidade media: ");
  Serial.println(velocidade_media);
  Serial.print("batimento medio: ");
  Serial.println(batimento_medio);
  Serial.println("");
}

void setup(){

  Serial.begin(115200);                                 // For Serial Monitor

  //Rotina pra conectar ao wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("conectando");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("conectado: ");
  Serial.println(WiFi.localIP());
 
  //Iniciar Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  calcula_velocidade.setInterval(10);
  calcula_velocidade.onRun(calcula_velocidades);
  calcula_batimento.setInterval(5000);
  calcula_batimento.onRun(calcula_batimentos);
  
  cpu.add(&calcula_velocidade);
  cpu.add(&calcula_batimento);

  // Configure the PulseSensor object, by assigning our variables to it. 
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);                      //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);  
  if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }

  comprimento = (2 * PI * raio)/100;                        //calcula o comprimento em centímetros e transforma em metros (divide por 100)
  comprimentoRoda = (2 * PI * rRoda)/100;           //calcula o comprimento em centímetros e transforma em metros (divide por 100)
   
  float velocidadeInicial = 3.0;
  float incrementoVeloc = 1.0;
  long tempoEstagio = 60000;
  correcaoTempo = millis();
}
 
void loop(){
  int vez=0;
  
  String inicia =  Firebase.getInt("Inicio");
  int inicia_int = inicia.toInt();
  
  Serial.println(inicia_int);
  
  while(inicia_int){
    if(vez==0){
      Serial.println("Iniciando em.... 5s");
      delay(1000);
      Serial.println("Iniciando em.... 4s");
      delay(1000);
      Serial.println("Iniciando em.... 3s");
      delay(1000);
      Serial.println("Iniciando em.... 2s");
      delay(1000);
      Serial.println("Iniciando em.... 1s");
      delay(1000);
      Serial.println("INICIAR!!! ");
      Serial.println("");
      vez=1;
      tempo_atual = millis();
    }
    
    inicia = Firebase.getInt("Inicio");
    inicia_int = inicia.toInt();
    cpu.run();
    
    if(!inicia){
      void encerra();
    }
  }
}
