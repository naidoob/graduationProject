//keypad
#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 4; 
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {3, 4, 5, 6}; 
byte colPins[COLS] = {7, 8, 9, 10}; 


Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
char customKey;


//GLCD
#include <glcd.h>
#include <fonts/allFonts.h>


//PIR sensor
int PIRPin = 2;
int PIRVal = 0;


//CT sensor
#include "EmonLib.h"                   
EnergyMonitor emon1;                   
int CTPin=A1;


//temp/hum sensor
#include "DHT.h"
#define temphumPin 8
#define DHTTYPE DHT22
DHT dht(temphumPin, DHTTYPE);
float h;
float t;


//LDR light sensor
int LDRPin = A0;
int LDRSensorValue;


//Smoke and gas detector
int SmokedetectorPin = A5;
int SmokedetectorValue;


//SSR
int bedroomlight = 46;
int entrancelamp = 49;
int waterheater = 48;
int fridge = 49;
int TV = 14;
int HiFi = 15;
int Airconditionar = 16;


void setup()

{

//Pin modes, initializing variables and communication, seeting up timers


//GLCD
  GLCD.Init();
  GLCD.SelectFont(System5x7);
firstScreen();


//PIR
pinMode(PIRPin, INPUT);


//CT sensor
emon1.current(CTPin, 111.1);


//temp/hum sensor
dht.begin();


//SSR
pinMode(bedroomlight, OUTPUT);
pinMode(entrancelamp, OUTPUT);
pinMode(waterheater, OUTPUT);
pinMode(fridge, OUTPUT);
pinMode(TV, OUTPUT);
pinMode(HiFi, OUTPUT);
pinMode(Airconditionar, OUTPUT);



}


void firstScreen()

{
  
}


void loop ()

{
  
  
}


