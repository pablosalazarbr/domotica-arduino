/*
Universidad Mariano Galvez de Guatemala
Programa de control para una casa domotica (house)
Autor: autor
Fecha de creacion: 23-sep-2022
*/

//Declaracion de librerias
#include <LiquidCrystal.h> //Modulo controlador LCD
#include <Servo.h> //Modulo controlador de servos
#include <EasyBuzzer.h> //Modulo para controlar el buzzer
#include <DHT.h> //Modulo de control DHT
#include <SoftwareSerial.h>// Incluimos la librería  SoftwareSerial. 

//Creamos un objeto servo y uno lcd, bluethoot
Servo puerta_1; //Creamos una puerta 1, interior
Servo ventana_1; //Creamos una ventana 1, interior
LiquidCrystal pantalla(8, 9, 10, 11, 12, 13); //Pines de la pantalla lcd.
SoftwareSerial BT(17,18);    // Definimos los pines RX y TX del Arduino conectados al Bluetooth.

//Declaracion de variables y constantes
#define DHTPIN 2     // Pin donde está conectado el DHT11
#define DHTTYPE DHT11   //Indicar si se usa el DHT 11
#define fotoresistencia A0 //Pin de la fotoresistencia
const int luces_interior = 15; //Pin (A1) al que se conectan todas las luces del interior
const int buzzer = 16; //Pin del buzzer (A2)
const int Trigger = 4;   //Pin digital  para el Trigger del sensor
const int Echo = 6;   //Pin digital 5 para el Echo del sensor
const int PIRPin = 7; // pin de entrada (for PIR sensor)
int dato_bluethoot; //Lectura del bluethoot
boolean llave = true; //Llave de alarma bloqueada

//Creamos el objeto DHT
DHT dht(DHTPIN, DHTTYPE); //Pasamos como parametros los define de arriba

//Funcion de configuracion inicial del arduino
void setup() {
  Serial.begin(9600); //Inicializamos el monitor serie
  BT.begin(9600); // Inicializamos el puerto serie BT (Para Modo AT 2)
  dht.begin(); //Inicializamos el pin dht
  puerta_1.attach(3); //Conectamos el servo 1 al pin 3
  ventana_1.attach(5); //Conectamos el servo 2 al pin 5
  pantalla.begin(16, 2); //Inicializamos la pantalla lcd
  EasyBuzzer.setPin(buzzer); //Declaro el zumbador
  pinMode(Trigger, OUTPUT); //pin como salida
  pinMode(Echo, INPUT);  //pin como entrada
  digitalWrite(Trigger, LOW);//Inicializamos el pin con 0
  pinMode(luces_interior, OUTPUT);//Luces del interior como salida.
  digitalWrite(luces_interior, LOW); //Apagamos luces
  puerta_1.write(90); //Cerramos la puerta
}

//Funcion principal del arduino
void loop() {
  //LLamamos a la libreria del buzzer
  EasyBuzzer.update();
  house(dht, pantalla); //Funcion normal de la casa
  ldr_turn_on(fotoresistencia, luces_interior); //Evaluamos oscuridad.
  //Si el bluethoot esta disponible
  if(Serial.available()){
    dato_bluethoot = Serial.read();
    if(dato_bluethoot == 'A'){
      //abrimos la puerta principal
      puerta_1.write(90); //Abrimos la puerta 1
      //Esperamos 5 segundos
      delay(5000);
      //Cerramos puerta
      puerta_1.write(0);
    }
    if(dato_bluethoot == 'B'){
      //Abrimos la ventana
      ventana_1.write(90); //Abrimos la puerta 1
      //Esperamos 5 segundos
      delay(5000);
      //Cerramos puerta
      ventana_1.write(0);
    }
    if(dato_bluethoot == 'E'){
      //Encendemos luces
      digitalWrite(luces_interior, LOW);
    }
    if(dato_bluethoot == 'F'){
      //Apagamos luces
      digitalWrite(luces_interior, HIGH);
    }
    if(dato_bluethoot == 'C'){
      //Activamos la alarma
      pantalla.setCursor(3, 0); //Colocamos el cursos en la columna 4.
      pantalla.print("Sistema seguro"); //Mostramos mensaje en pantalla.
      pantalla.setCursor(2, 1); //Colocamos el cursor en la fila de abajo.
      pantalla.print("Alarma activa"); //Motramos el resto del mensaje.
      int lectura_ble;
      while(llave == true){
        lectura_ble = Serial.read();
        if(lectura_ble == 'D'){
          llave == false;
          break;
          }else{
            alarma_activada(PIRPin, Trigger, Echo, buzzer); //Funcion de alarma
            }
        
        }
    }
  }

}

//Funcion "normal de la casa sin alarma activa"
void house(DHT dht1, LiquidCrystal lcd){
  float t = dht1.readTemperature(); //Temperatura en grados Celsius 
  lcd.setCursor(2, 0); //Colocamos el cursos en la columna 4.
  lcd.print("Temperatura"); //Mostramos mensaje en pantalla.
  lcd.setCursor(3, 1); //Colocamos el cursor en la fila de abajo.
  lcd.print(t);//Imprimimos temperatura
  lcd.print(" grados"); //Motramos el resto del mensaje.
  }

//Funcion cuando la alarma esta activada
void alarma_activada(int sensorPIR, int trigger_ultrasonico, int echo_ultrasonico, int buzzerPin){
  //Cuando esta funcion esta activa entonces se mantendra escuchando la alarma
  //Declaramos variables a utilizar
  long t; //timepo que demora en llegar el eco
  long d; //distancia en centimetros
  digitalWrite(trigger_ultrasonico, HIGH); //Encendemos el ultrasonico
  delayMicroseconds(10); //Enviamos un pulso de 10us
  digitalWrite(trigger_ultrasonico, LOW); //Apagamos el ultrasonico
  t = pulseIn(echo_ultrasonico, HIGH); //obtenemos el ancho del pulso
  d = t/59; //escalamos el tiempo a una distancia en cm
  //Consultamos al sensor pir para saber si hay movimiento
  int lectura_pir = digitalRead(sensorPIR);
  //Si la lectura es un HIGH entonces hay movimiento, activamos el buzzer
  if(lectura_pir == HIGH or d < 5){
    //Activamos el buzzer
    EasyBuzzer.beep(250);//Beep con 150 de frecuencia
    }else if(lectura_pir == LOW or d > 5){
      EasyBuzzer.stopBeep();//Detenemos la alarma
          }
  }

//Funcion para activar una luz utilizando una fotoresistencia
void ldr_turn_on(int ldr_pin, int light_pin){
  int sensorValue = analogRead(ldr_pin); //Lectura realizada en el pin analogo del LDR
  if(sensorValue < 80){ //Si el valor es menor a 100 (oscuridad)
    digitalWrite(light_pin, LOW); //Encendemos la luz
    }else if(sensorValue > 80){ //Si el valor es mayor a 100 (dia)
      digitalWrite(light_pin, HIGH); //Apagamos las luces
      }
  }
