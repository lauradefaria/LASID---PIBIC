//Bibliotecas ESP8266 e Firebase.
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
 
//Iniciações do banco de dados Firebase.
#define FIREBASE_HOST "lasid-test01.firebaseio.com"
#define FIREBASE_AUTH "f6vZk8KxKT5X6LLrutYjn1UrzeQnM3D0KMOhHzYL"
#define WIFI_SSID "LASID"
#define WIFI_PASSWORD "lasid@ci.ufpb.br"

//Biblioteca do sensor de batimento cardiaco
#define USE_ARDUINO_INTERRUPTS true                            // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>                                

// utilizadas para medir bpm
#define sensor_bat A0              // PulseSensor conectado no ANALOG PIN 0
#define led 6                      // Led conectado no PIN 13
int Threshold = 550;                // Determina qual sinal é contabilizado como batimento e qual é ignorado
                                    // Foi-se utilizado o valor "550"(default)                              
PulseSensorPlayground pulseSensor;  // cria o objeto "pulseSensor"

//utilizados para medir a velocidade 
#define sensor_vel 8
int count = 0;
boolean flag = 0;
float velocidade = 0.0;
float distancia = 0.0;
long tempo = 0;
float distAcumulada = 0.0;
float raio = 4.75;                                  // <=== COLOCAR AQUI O VALOR DO RAIO EM CENTÍMETROS DO CILINDRO
float comprimento = (2 * PI * raio)/100;            //calcula o comprimento em centímetros e transforma em metros (divide por 100)
long correcaoTempo = 0;

void calcula_batimento(){

  int myBPM = pulseSensor.getBeatsPerMinute();       // Calls function on our pulseSensor object that returns BPM as an "int".
                                                     // "myBPM" hold this BPM value now. 
  if (pulseSensor.sawStartOfBeat()) {                // Constantly test to see if "a beat happened".    
    if(myBPM >= 40 && myBPM <= 180){                   // Print the value inside of myBPM. 
      Firebase.pushInt("Batimentos", myBPM);
      digitalWrite(led, HIGH);
      delay(500); 
      digitalWrite(led, LOW);    
   }
  }
  delay(20);                                         // considered best practice in a simple sketch.
}

void calcula_velocidade(){                             

    int line_detector;

    line_detector = digitalRead(sensor_vel);
    
    if (line_detector){ //Ativou o sensor
      flag = 1;
    }

    if(!line_detector && flag){
      flag = 0;
      count += 1;
    }
    
    tempo = millis() - correcaoTempo;

    if (tempo%2000 == 0) {   // 2 segundos 
      
      distancia = count * comprimento;
      distAcumulada += distancia;
      Firebase.pushFloat("Distancia", distAcumulada);
  
      velocidade = (3.6 * distancia) / 3;
      Firebase.pushFloat("Velocidade", velocidade);
  
      distancia = 0.0;
      count = 0;
  }
}

void encerra(){                                                         
  correcaoTempo = tempo = 0;
  
  Firebase.pushFloat("Distancia", distAcumulada);
  
  distAcumulada = 0.0;
  count = 0;
}

void setup(){

  Serial.begin(115200);                                 // For Serial Monitor

  // Configure the PulseSensor object, by assigning our variables to it. 
  pulseSensor.analogInput(sensor_bat);   
  pulseSensor.blinkOnPulse(led);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);  

  //Rotina pra conectar ao wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //Serial.print("conectando");
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    delay(500);
  }
  Serial.println(WiFi.localIP());
 
  //Iniciar Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
 
void loop(){

  int vez=0;
  char iniciaArr[5];
  String iniciaStr;
  int inicia_int;
  
  iniciaStr = Firebase.getString("Inicio");
  
  iniciaStr.toCharArray(iniciaArr, 5);
  
  inicia_int = atoi(iniciaArr);
  
  while(inicia_int){
    if(vez==0){
      Firebase.pushInt("Batimentos", 0);
      Firebase.pushFloat("Velocidade", 0);
      Firebase.pushFloat("Distancia", 0);
      delay(5000);
      
      correcaoTempo = millis();
      vez=1;
    }
    
    iniciaStr = Firebase.getString("Inicio");
    iniciaStr.toCharArray(iniciaArr, 3);
    inicia_int = atoi(iniciaArr);
    calcula_velocidade();
    calcula_batimento();
    
    if(!inicia_int && vez){
      void encerra();
    }
  }
}
