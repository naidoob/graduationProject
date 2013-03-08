//KEYPAD
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
double currentconsumed;
double powerconsumed;


//temp/hum sensor
#include "DHT.h"
#define temphumPin 11
#define DHTTYPE DHT22
DHT dht(temphumPin, DHTTYPE);
float h;
float t;


//LDR light sensor
int LDRPin = A7;
int LDRSensorValue;


//Smoke and gas detector
int SmokedetectorPin = A5;
int SmokedetectorValue;


//SSR
int bedroomlight = 17;
int entrancelamp = 47;
int waterheater = 48;
int fridge = 49;
int TV = 14;
int HiFi = 15;
int Airconditionar = 16;

//RTC
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;
DateTime now;






//USER DATA

// 'A' for manual mode and 'D' for daily mode
// 'B' for inhouse mode which is default

char mode;
byte hourReturning;





void setup()

{

 Serial.begin(9600); 
  
//Pin modes, initializing variables and communication, seeting up timers


//GLCD
  GLCD.Init();
  GLCD.SelectFont(System5x7);



//PIR
pinMode(PIRPin, INPUT);


//CT sensor
emon1.current(A1, 111.1);


//temp/hum sensor
dht.begin();


//GSM

Serial1.begin(9600);
//sendSMS();

//SSR


pinMode(bedroomlight, OUTPUT);
pinMode(entrancelamp, OUTPUT);
pinMode(waterheater, OUTPUT);
pinMode(fridge, OUTPUT);
pinMode(TV, OUTPUT);
pinMode(HiFi, OUTPUT);
pinMode(Airconditionar, OUTPUT);


//RTC

Serial.begin(9600);
    Wire.begin();
    RTC.begin();


//TEST FUNCTIONS

 


}

void setTimerA()

{
  
}

 void setTimerB()
 
 {
   
   
 }
 
 
 void setTimerC()
 
 {
   
   
 }
 
 
 void setPirInterrupt()
 
 {
   
 }



void readPower()

{
 
  currentconsumed =emon1.calcIrms(1480);
  
  currentconsumed = currentconsumed - 0.08 ;//Current sensor overreading by 0.08 amps
  
  powerconsumed=currentconsumed*220.0;
  
}



void readTempHum() 

{
  
  h = dht.readHumidity();
  t = dht.readTemperature();
  
}

void readRTC()
{
   now = RTC.now();
   
   
   
  
}


void readKeypad()

{
  
  // read modes, numberOfHoursOut, save
  
 customKey = customKeypad.getKey(); 
}


 void readLDR()
 
 {
  
 
 LDRSensorValue =analogRead (LDRPin);
   
 }

void readSmokeDetector()

{
 
  
SmokedetectorValue = analogRead (SmokedetectorPin);
  
  
}

 void sendSMS()
 
 {
   
   Serial1.print("AT+CMGF=1\r"); //Set text mode.
   delay (100);
   Serial1.print("AT+CMGS=0507323554\r"); //AT command to send SMS
      delay(100);
      Serial1.print("Smoke/Gas has been detected"); //Print the message
      delay(100);
      Serial1.print("\x1A"); //Send it ascii SUB
   
   
 }
 
  void gotoinHouseMode()
  
  {
    
digitalWrite(bedroomlight, HIGH);
digitalWrite(entrancelamp, LOW);
digitalWrite(waterheater, LOW);
digitalWrite(fridge, LOW);
digitalWrite(TV, LOW);
digitalWrite(HiFi, LOW);
digitalWrite(Airconditionar, LOW);
    
    
    
  }
 void gotoManualMode()
 
 {
   digitalWrite(bedroomlight, HIGH);
digitalWrite(entrancelamp, LOW);
digitalWrite(waterheater, LOW);
digitalWrite(fridge, LOW);
digitalWrite(TV, LOW);
digitalWrite(HiFi, LOW);
digitalWrite(Airconditionar, LOW);
   
   
   
 }
 void gotoDailyMode()
 
 {
digitalWrite(bedroomlight, LOW);
digitalWrite(entrancelamp, LOW);
digitalWrite(waterheater, LOW);
digitalWrite(fridge, LOW);
digitalWrite(TV, LOW);
digitalWrite(HiFi, LOW);
digitalWrite(Airconditionar, LOW);
   
   
 }
 
 




void Screen1()

{
  GLCD.CursorTo(0, 0);
  GLCD.print("Welcome to WafferBill ");
  GLCD.CursorTo(0, 1);
  GLCD.print("Ahmed,Maha,Fatma,Bary");
  GLCD.CursorTo(0, 2);
  GLCD.print("Power in Watts= ");
  GLCD.print(powerconsumed);
  GLCD.CursorTo(0, 3);
  GLCD.print("Temp celsius= ");
  GLCD.print(t);
  GLCD.CursorTo(0, 4);
  GLCD.print("Humidity % = ");
  GLCD.print(h);
  GLCD.CursorTo(0, 5);
  GLCD.print("Date = ");
  
  

   
    GLCD.print(now.year(), DEC);
    GLCD.print('/');
    GLCD.print(now.month(), DEC);
    GLCD.print('/');
    GLCD.print(now.day(), DEC);
    GLCD.print(' ');
    GLCD.CursorTo(0, 6);
    GLCD.print("Time = ");
    GLCD.print(now.hour(), DEC);
    GLCD.print(':');
    GLCD.print(now.minute(), DEC);
    GLCD.print(':');
    GLCD.print(now.second(), DEC);
    GLCD.print(' ');
    GLCD.CursorTo(0, 7);
    GLCD.print("KeyPressed =");
    GLCD.print(customKey);
  
}


void Screen2()

{
  GLCD.CursorTo(0, 0);
  GLCD.println("Screen2");
  GLCD.CursorTo(0, 1);
  GLCD.println("");
  GLCD.CursorTo(0, 2);
  GLCD.println(" ");
  GLCD.CursorTo(0, 3);
  GLCD.println(" ");
  GLCD.CursorTo(0, 4);
  GLCD.println("");
  GLCD.CursorTo(0, 5);
  GLCD.println("");
  GLCD.CursorTo(0, 6);
  GLCD.println(" ");
  GLCD.CursorTo(0, 7);

  
}

void Screen3()

{
  GLCD.CursorTo(0, 0);
  GLCD.println("Screen3");
  GLCD.CursorTo(0, 1);
  GLCD.println("");
  GLCD.CursorTo(0, 2);
  GLCD.println(" ");
  GLCD.CursorTo(0, 3);
  GLCD.println(" ");
  GLCD.CursorTo(0, 4);
  GLCD.println("");
  GLCD.CursorTo(0, 5);
  GLCD.println("");
  GLCD.CursorTo(0, 6);
  GLCD.println(" ");
  GLCD.CursorTo(0, 7);

  
}

void Screen4()

{
  GLCD.CursorTo(0, 0);
  GLCD.println("Screen4");
  GLCD.CursorTo(0, 1);
  GLCD.println("");
  GLCD.CursorTo(0, 2);
  GLCD.println(" ");
  GLCD.CursorTo(0, 3);
  GLCD.println(" ");
  GLCD.CursorTo(0, 4);
  GLCD.println("");
  GLCD.CursorTo(0, 5);
  GLCD.println("");
  GLCD.CursorTo(0, 6);
  GLCD.println(" ");
  GLCD.CursorTo(0, 7);

  
}

void Screen5()

{
  GLCD.CursorTo(0, 0);
  GLCD.println("Screen5");
  GLCD.CursorTo(0, 1);
  GLCD.println("");
  GLCD.CursorTo(0, 2);
  GLCD.println(" ");
  GLCD.CursorTo(0, 3);
  GLCD.println(" ");
  GLCD.CursorTo(0, 4);
  GLCD.println("");
  GLCD.CursorTo(0, 5);
  GLCD.println("");
  GLCD.CursorTo(0, 6);
  GLCD.println(" ");
  GLCD.CursorTo(0, 7);

  
}

void Screen6()

{
  GLCD.CursorTo(0, 0);
  GLCD.println("Screen6");
  GLCD.CursorTo(0, 1);
  GLCD.println("");
  GLCD.CursorTo(0, 2);
  GLCD.println(" ");
  GLCD.CursorTo(0, 3);
  GLCD.println(" ");
  GLCD.CursorTo(0, 4);
  GLCD.println("");
  GLCD.CursorTo(0, 5);
  GLCD.println("");
  GLCD.CursorTo(0, 6);
  GLCD.println(" ");
  GLCD.CursorTo(0, 7);

  
}


void Screen7()

{
  GLCD.CursorTo(0, 0);
  GLCD.println("Screen7");
  GLCD.CursorTo(0, 1);
  GLCD.println("");
  GLCD.CursorTo(0, 2);
  GLCD.println(" ");
  GLCD.CursorTo(0, 3);
  GLCD.println(" ");
  GLCD.CursorTo(0, 4);
  GLCD.println("");
  GLCD.CursorTo(0, 5);
  GLCD.println("");
  GLCD.CursorTo(0, 6);
  GLCD.println(" ");
  GLCD.CursorTo(0, 7);

  
}

void Screen8()

{
  GLCD.CursorTo(0, 0);
  GLCD.println("Screen8");
  GLCD.CursorTo(0, 1);
  GLCD.println("");
  GLCD.CursorTo(0, 2);
  GLCD.println(" ");
  GLCD.CursorTo(0, 3);
  GLCD.println(" ");
  GLCD.CursorTo(0, 4);
  GLCD.println("");
  GLCD.CursorTo(0, 5);
  GLCD.println("");
  GLCD.CursorTo(0, 6);
  GLCD.println(" ");
  GLCD.CursorTo(0, 7);

  
}




void loop ()

{

  
  
  //gotoinHouseMode();
  
  
  
  
 
  //readSmokeDetector();
  
    //Serial.print("SmokedetectorValue =");
  //Serial.println(SmokedetectorValue);
  
 //readLDR(); 
 
 //Serial.print("LDRSensorValue = ");
 //Serial.println(LDRSensorValue);
   
 
 readRTC(); 
 readPower(); 
 readTempHum(); 
 Screen1(); 
 //Screen2();
 //Screen3();
 //Screen4();
 //Screen5();
 //Screen6();
 //Screen7();
 //Screen8();
 
 readKeypad();
 //sendSMS();
 
 //gotoManualMode();
 //gotoDailyMode();
 //setTimerA();
 //setTimerB();
 //setTimerC();
 //setPirInterrupt();
  
  
 
 
  
}


