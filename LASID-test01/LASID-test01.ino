//Bibliotecas ESP8266 e Firebase.
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
 
//Iniciações do banco de dados Firebase.
#define FIREBASE_HOST "lasid-test01.firebaseio.com"
#define FIREBASE_AUTH "f6vZk8KxKT5X6LLrutYjn1UrzeQnM3D0KMOhHzYL"
#define WIFI_SSID "LASID"
#define WIFI_PASSWORD "lasid@ci.ufpb.br"

//Informações do batimento
int batimentos = 0;
float batimento_medio = 0.0;
int countb = 0;
char lixo;

//utilizados para medir a velocidade 
#define sensor D8
int tempo_atual = 0;
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

//informações não utilizadas no momento
float comprimento = 0;
float raio = 4.75;                             // Colocar aqui o valor do raio em cm do cilíndro
float rRoda = 30.48;                           // Colocar aqui o valor do raio em cm da roda (aqui ta pra aro 24 que dá um comprimento de 60,96 DIV 2)
float comprimentoRoda = 0;
float rpm = 0.0;

float velocidadeInicial = 3.0;
float incrementoVeloc = 1.0;
long tempoEstagio = 60000;
long correcaoTempo = 0;
long tempoAnterior = 0;

void calcula_batimento(){
  
  if(Serial.available() > 0){
    batimentos = Serial.parseInt();
    lixo = Serial.read();
    Firebase.setInt("Batimentos", batimentos);
    
    countb = Firebase.getInt("ContadorBatimentos") + 1;
    Firebase.setInt("ContadorBatimentos", countb);
    
    batimento_medio = Firebase.getFloat("BatimentoMedio") + batimentos;
    Firebase.setFloat("BatimentoMedio", batimento_medio);
  }
}

void calcula_velocidade(){                              //alterar velocidade m/s

    int line_detector;
    int aux_tempo = 0;

    line_detector = digitalRead(sensor);
    //Serial.print("Detector de linha: ");
    //Serial.println(line_detector);
    
    if (!line_detector){ //Ativou o sensor
      flag = 1;
    }

    if(line_detector && flag){
      flag = 0;
      count += 1;
    }
    
    tempo = millis() - tempoAnterior;

    //Serial.print("Contador: ");
    //Serial.println(count);
    
    /*Serial.print("Tempo: ");
    Serial.print(tempo/1000);
    Serial.println("s");*/

    if ((tempo - correcaoTempo) >= 3000) {   // 3 segundos 
      correcaoTempo = tempo;
      //Serial.print("Correcao tempo agora eh: ");   
      //Serial.println(correcaoTempo);
      
      distancia = count * comprimento;
      distAcumulada += distancia;
      Firebase.setFloat("Distancia", distAcumulada);
      
      /*Serial.print("Distancia: ");
      Serial.println(distAcumulada);*/

      tempo_atual = tempo/1000;
      Firebase.setInt("Tempo", tempo_atual/60);
  
      velocidade_atual = (3.6 * distancia) / 3;
      Firebase.setFloat("Velocidade", velocidade_atual);

      countv = Firebase.getInt("ContadorVelocidade") + 1;
      Firebase.setInt("ContadorVelocidade", countv);
                
      velocidade_media = Firebase.getFloat("VelocidadeMedia") + velocidade_atual;
      Firebase.setFloat("VelocidadeMedia", velocidade_media);

      /*Serial.print("Velocidade: ");
      Serial.println(velocidade_atual);  
      Serial.println("");*/    
  
      //rpm = 20*(count*(comprimento/comprimentoRoda));
      distancia = 0.0;
      count = 0;
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

void encerra(){                                                         ///acrescentar os segundos no aplicativo
  tempo_atual = tempo/1000;
  tempoAnterior = tempo;
  
  Firebase.setFloat("Distancia", distAcumulada);
  Firebase.setInt("Tempo", (tempo_atual/60));
  
  countb = Firebase.getInt("ContadorBatimentos");
  countv = Firebase.getInt("ContadorVelocidade");
  velocidade_media = (float) (velocidade_media / (float) countv);
  batimento_medio = (float) (batimento_medio / (float) countb);
  
  Firebase.setFloat("VelocidadeMedia", velocidade_media);
  Firebase.setFloat("BatimentoMedio", batimento_medio);
  
  velocidade_media = batimento_medio = 0.0;
  countb = countv = distAcumulada = 0;
  /*
  Serial.print("distancia: ");
  Serial.println(distAcumulada);
  Serial.print("tempo: ");
  Serial.println(tempo_atual);
  Serial.print("velocidade media: ");
  Serial.println(velocidade_media);
  Serial.print("batimento medio: ");
  Serial.println(batimentos);
  Serial.println("");*/
}

void setup(){

  Serial.begin(115200);                                 // For Serial Monitor

  //Rotina pra conectar ao wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //Serial.print("conectando");
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    delay(500);
  }
  //Serial.println();
  //Serial.print("conectado: ");
  Serial.println(WiFi.localIP());
 
  //Iniciar Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Configure the PulseSensor object, by assigning our variables to it. 

  comprimento = (2 * PI * raio)/100;                        //calcula o comprimento em centímetros e transforma em metros (divide por 100)
  comprimentoRoda = (2 * PI * rRoda)/100;           //calcula o comprimento em centímetros e transforma em metros (divide por 100)
   
  float velocidadeInicial = 3.0;
  float incrementoVeloc = 1.0;
  long tempoEstagio = 60000;
  //correcaoTempo = millis();
}
 
void loop(){
  int vez=0;
  char iniciaArr[5];
  String iniciaStr;
  int inicia_int;
  
  iniciaStr = Firebase.getString("Inicio");

  //Serial.print("IniciaStr = ");
  //Serial.println(iniciaStr);
  
  iniciaStr.toCharArray(iniciaArr, 5);
  
  //Serial.print("IniciaArr = ");
  //Serial.println(iniciaArr);
  
  inicia_int = atoi(iniciaArr);

  //Serial.print("Inicia = ");
  //Serial.println(inicia_int);
  
  while(inicia_int){
    if(vez==0){
      Firebase.setInt("Tempo", 0);
      Firebase.setInt("Batimentos", 0);
      Firebase.setInt("ContadorBatimentos", 0);
      Firebase.setInt("ContadorVelocidade", 0);
      Firebase.setFloat("Velocidade", 0);
      Firebase.setFloat("VelocidadeMedia", 0);
      Firebase.setFloat("BatimentoMedio", 0);
      Firebase.setFloat("Distancia", 0);
      /*Serial.println("Iniciando em.... 5s");
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
      Serial.println("");*/
      delay(5000);
      vez=1;
    }
    
    iniciaStr = Firebase.getString("Inicio");
    iniciaStr.toCharArray(iniciaArr, 3);
    inicia_int = atoi(iniciaArr);
    calcula_velocidade();
    calcula_batimento();
    
    if(!inicia_int){
      void encerra();
    }
  }
}
