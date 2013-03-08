
// Version 1.6 

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
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), 
rowPins, colPins, ROWS, COLS); 
char customKey;

//GLCD
#include <glcd.h>
#include <fonts/allFonts.h>

//PIR sensor
int PIRPin = 2;

//CT sensor
#include "EmonLib.h"                   
EnergyMonitor emon1;
volatile double currentConsumed=0.0;
volatile double totalCurrent=0.0;
volatile double averageCurrentOverOneHour=0.0;
volatile double averagePowerOverOneHour=0.0;
volatile double cost;



volatile int counter=0;




// this is to prevent multiple SMS's
// being sent every 4 seconds when there is a fire
//thus saving mobile credit

volatile int smsCount=0;

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
int LDRSensorThreshold = 250;

//Smoke and gas detector
int SmokedetectorPin = A5;
int SmokedetectorValue;
int SmokedetecorThreshold = 250;

//SSR
volatile int bedroomlight = 17;
volatile int entrancelamp = 47;
volatile int waterheater = 48;
volatile int fridge = 49;
volatile int TV = 14;
volatile int HiFi = 15;
volatile int Airconditionar = 16;

//STATE OF APPLIANCES - on or off 

volatile int bedroomlightState = LOW;
volatile int entrancelampState = LOW;
volatile int waterheaterState = LOW;
volatile int fridgeState = LOW;
volatile int TVState = LOW;
volatile int HiFiState = LOW;
volatile int AirconditionarState = LOW;

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
gotoinHouseMode();
  
  
 Serial.begin(115200); 
  
//Pin modes, initializing variables and communication, seeting up timers


//GLCD
  GLCD.Init();
  GLCD.SelectFont(System5x7);

//PIR
pinMode(PIRPin, INPUT);

//CT sensor
emon1.current(A1, 111.1);

delay (5000);

//temp/hum sensor
dht.begin();


//GSM

Serial1.begin(9600);


//SSR


pinMode(bedroomlight, OUTPUT);
pinMode(entrancelamp, OUTPUT);
pinMode(waterheater, OUTPUT);
pinMode(fridge, OUTPUT);
pinMode(TV, OUTPUT);
pinMode(HiFi, OUTPUT);
pinMode(Airconditionar, OUTPUT);

// Hardware interrupt setup for PIR and bedroom light)
// using interrupt 0 which is PIR PIN2

//attachInterrupt(0, changeStateOfBedroomLight, CHANGE);

//RTC

Serial.begin(14400);
    Wire.begin();
    RTC.begin();
    
 
// software interrupt to set timer1 to read smoke detector every 4 seconds  

//set timer1 interrupt at 0.25Hz = every 4 seconds
  
  noInterrupts();//stop interrupts
  
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 62499;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  interrupts();//allow interrupts  
     
     
// call intro screen

//Screen0();

//delay (3000);


          
}

//TEST FUNCTIONS


//software interrupt service routine every 4s for smoke detector


ISR(TIMER1_COMPA_vect){
  
 readPower(); 
 
 /*
 
 readTempHum(); 
  
 readSmokeDetector();

 
if  ((SmokedetectorValue > SmokedetecorThreshold)&& (smsCount <3))

{
  sendSMS();
  smsCount = smsCount+1;
  
}
 
 
  
   
 readLDR();



if (LDRSensorValue >LDRSensorThreshold)

{
  digitalWrite (entrancelamp, LOW);
}
else
{
  digitalWrite (entrancelamp, HIGH);
  
}

*/

}



//hardware interrupt service routine for PIR

 void changeStateOfBedroomLight ()
 
 {
  
   bedroomlightState = !bedroomlightState;

   digitalWrite (bedroomlight, bedroomlightState );
   
   
 }


void readPower()

{
 
  
  //current sensor over reading by 0.25 amps
  currentConsumed =(emon1.calcIrms(1480))-0.25;
  
  if (currentConsumed < 10.0) 
  
  {


    
    

  
  
  counter = counter +1;
  
    Serial.print( "c = ");    
Serial.print( counter );  

Serial.print( " CC = ");    
Serial.print( currentConsumed );
  
  
  totalCurrent=totalCurrent+  currentConsumed;
  
  Serial.print( "    tC = " );
  Serial.println( totalCurrent );
  
  
  
  
 

  
 
 if (counter == 900)
 
 {
    averageCurrentOverOneHour = totalCurrent/900.0;
    
    // to ocnvert watts to kiliwatt multiply by 0.001
    
  averagePowerOverOneHour =     averageCurrentOverOneHour *220.0*0.001;
  
  // for expats the rate is 20 fils per kilowatt hour
  
  cost = averagePowerOverOneHour * 20/100.0;
  
  
  
    Serial.print( "KWH = " );
  Serial.print(   averagePowerOverOneHour );
  
      Serial.print( "  Cost AED = " );
  Serial.println(   cost );
 
  
  counter =0;
  totalCurrent=0.0;
  averageCurrentOverOneHour =0.0;
  averagePowerOverOneHour=0.0;
  
  
 }
  
  
  
 
 
 
  
  
  }
  
  
  
  
  

  
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
      Serial1.print("Smoke/Gas detected  here http://tinyurl.com/cqnezgq"); //Print the message
      delay(100);
      Serial1.print("\x1A"); //Send it ascii SUB
     
 }
 
  void gotoinHouseMode()
  
  {
    
digitalWrite(bedroomlight, HIGH);
digitalWrite(entrancelamp, HIGH);
digitalWrite(waterheater, HIGH);
digitalWrite(fridge, HIGH);
digitalWrite(TV, HIGH);
digitalWrite(HiFi, HIGH);
digitalWrite(Airconditionar, HIGH);
    
    
  }
  
 void gotoManualMode()
 
 {
  
  digitalWrite(bedroomlight, HIGH);
digitalWrite(entrancelamp, HIGH);
digitalWrite(waterheater, HIGH);
digitalWrite(fridge, HIGH);
digitalWrite(TV, HIGH);
digitalWrite(HiFi, HIGH);
digitalWrite(Airconditionar, HIGH);
    
   
 }
 void gotoDailyMode()
 
 {
digitalWrite(bedroomlight, LOW);
digitalWrite(entrancelamp, LOW);
digitalWrite(waterheater, LOW);
digitalWrite(fridge, HIGH);
digitalWrite(TV, LOW);
digitalWrite(HiFi, LOW);
digitalWrite(Airconditionar, LOW);
   
   
 }
 
 void Screen0 ()
{
  GLCD.CursorTo(0, 0);
  GLCD.println("Welcome To WafferBill");
  GLCD.CursorTo(0, 1);
  GLCD.println("Home Energy Saver");
  GLCD.CursorTo(0, 2);
  GLCD.println("Final EE Project");
  GLCD.CursorTo(0, 3);
  GLCD.println("Fatima, Maha ");
  GLCD.CursorTo(0, 4);
  GLCD.println("Ahmed");
  GLCD.CursorTo(0, 5);
  GLCD.println("Supervisor Barry");
  GLCD.CursorTo(0, 6);
  GLCD.println("Sponsor HCT");
  GLCD.CursorTo(0, 7);
  GLCD.println("Completed June 2013");

  
}
 
 


void Screen1()

{

  
  GLCD.CursorTo(0, 0);
  GLCD.print("Temp celsius= ");
  GLCD.print(t);
  GLCD.CursorTo(0, 1);
  GLCD.print("Humidity % = ");
  GLCD.print(h);
  GLCD.CursorTo(0, 2);
  GLCD.print("Date = ");
  
  

   
    GLCD.print(now.year(), DEC);
    GLCD.print('/');
    GLCD.print(now.month(), DEC);
    GLCD.print('/');
    GLCD.print(now.day(), DEC);
    GLCD.print(' ');
    GLCD.CursorTo(0, 3);
    GLCD.print("Time = ");
    GLCD.print(now.hour(), DEC);
    GLCD.print(':');
    GLCD.print(now.minute(), DEC);
    GLCD.print(':');
    GLCD.print(now.second(), DEC);
    GLCD.print(' ');
    GLCD.CursorTo(0, 4);
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
  
//readRTC();  
 
 //Screen1();
 
 //readKeypad();
 

 
  
}


