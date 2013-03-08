
// Version 1.9 

//beta to github
//sodifhskdjhflksjdhfsd

#include <EEPROM.h>



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
volatile double cost=0.0;
volatile double totalPowerForTheDay=0.0;




volatile double hourArray[25];


volatile int counter=0;
volatile int hourCounter=0;



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




void setup()

{
gotoinHouseMode();
  
  
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

attachInterrupt(0, changeStateOfBedroomLight, CHANGE);

//RTC

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
     


}



//TEST FUNCTIONS


//software interrupt service routine every 4s for smoke detector


ISR(TIMER1_COMPA_vect){
  
 readPower(); 
 
 
 
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
  
  if (currentConsumed < 20.0) 
  
  {

  counter = counter +1;
  
    //Serial.print( "c = ");    
//Serial.print( counter );  

//Serial.print( " CC = ");    
//Serial.print( currentConsumed );
  
  
  totalCurrent=totalCurrent+  currentConsumed;
  
  //Serial.print( "    tC = " );
  //Serial.println( totalCurrent );
  
 
 if (counter == 900)
 
 {
   
   hourCounter=hourCounter+1;
    averageCurrentOverOneHour = totalCurrent/900.0;
    
    // to ocnvert watts to kiliwatt multiply by 0.001
    
  averagePowerOverOneHour =     averageCurrentOverOneHour *220.0*0.001;
  
  // for expats the rate is 20 fils per kilowatt hour
  
  cost = averagePowerOverOneHour * 20/100.0;
  
  
  
    //Serial.print( "KWH = " );
  //Serial.print(   averagePowerOverOneHour );
  
      //Serial.print( "  Cost AED = " );
  //Serial.println(   cost );
 
  hourArray[hourCounter]=averagePowerOverOneHour;
  
  counter =0;
  totalCurrent=0.0;
  averageCurrentOverOneHour =0.0;
  averagePowerOverOneHour=0.0;
  
  if (hourCounter ==24)
  
  {
    
    for (int i=1; i <25; i++)
    
    
    {
          totalPowerForTheDay = totalPowerForTheDay + hourArray[i];
          
      
    }
    
EEPROM.write(0,totalPowerForTheDay);


    
    hourCounter=0;
  }
  
  
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
 
 void introScreen ()
{
  GLCD.ClearScreen();
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
 
 


void defaultScreen()

{

 

  GLCD.ClearScreen();
  
  repeat:
  
  GLCD.CursorTo(0, 0);
  readRTC();
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
    GLCD.print("C= ");
    GLCD.print( counter ); 
    
   GLCD.print( "CC = ");    
 GLCD.print( currentConsumed );
 GLCD.print( "A= ");
 GLCD.print(averagePowerOverOneHour);
    
    GLCD.CursorTo(0,5);
    GLCD.print ("Press * for Settings");
    
    
    
    GLCD.CursorTo(0,6);
    GLCD.print ("KeyPressed =");
    
    
    
    
    
    
    
      readKeypad();
    GLCD.print (customKey);
    
    if (customKey == '*')
    
    {
      settingsScreen();
    }
    
   
    goto repeat;
    

  
}


void modeScreen()

{
  
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 0);
  GLCD.println("Press A for Manual");
  GLCD.CursorTo(0, 1);
  GLCD.println("Press C for inhouse");
  GLCD.CursorTo(0, 2);
  GLCD.println("Press D for Daily ");
  GLCD.CursorTo(0, 3);
  GLCD.println("Press B for back ");
  readKeypad();
    GLCD.CursorTo(0,4);
    GLCD.print ("KeyPressed =");
    
    while (1)
    
    {
      readKeypad();
    GLCD.print (customKey);
    
    if (customKey == 'A')
    
    {
      manualScreen();
    }
    
    if (customKey == 'C')
    
    {
      inHouseScreen();
    }
    
    if (customKey == 'D')
    
    {
      dailyScreen();
    }
    
    
    if (customKey == 'B')
    
    {
      defaultScreen();
    }
    
  
    }
  

  
}

void settingsScreen()

{
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 0);
  GLCD.println("Press A for Mode");
  GLCD.CursorTo(0, 1);
  GLCD.println("Press C for Cons/Bill");
  GLCD.CursorTo(0, 2);
  GLCD.println("Press B for back ");
  readKeypad();
    GLCD.CursorTo(0,3);
    GLCD.print ("KeyPressed =");
    
    while (1)
    
    {
      readKeypad();
    GLCD.print (customKey);
    
    if (customKey == 'A')
    
    {
      modeScreen();
    }
    
    if (customKey == 'C')
    
    {
      consumptionScreen();
    }
    
    if (customKey == 'B')
    
    {
      defaultScreen();
    }
    
  
    }
}

void manualScreen()

{
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 0);
  GLCD.println("Are you sure");
  GLCD.CursorTo(0, 1);
  GLCD.println("you want");
  GLCD.CursorTo(0, 2);
  GLCD.println("manual mode? ");
  GLCD.CursorTo(0, 3);
  GLCD.println("# to accept ");
  GLCD.CursorTo(0, 4);
  GLCD.println("C to cancel");
  readKeypad();
    GLCD.CursorTo(0,5);
    GLCD.print ("KeyPressed =");
    
    while (1)
    
    {
      readKeypad();
    GLCD.print (customKey);
    
    if (customKey == '#')
    
    {
      manualFunction();
    }
    
    if (customKey == 'C')
    
    {
      defaultScreen();
    }
    
    
    
  
    }
  

  
}

void inHouseScreen()

{
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 0);
  GLCD.println("Are you Sure");
  GLCD.CursorTo(0, 1);
  GLCD.println("you want");
  GLCD.CursorTo(0, 2);
  GLCD.println("inhouse mode? ");
  GLCD.CursorTo(0, 3);
  GLCD.println(" # to accept");
  GLCD.CursorTo(0, 4);
  GLCD.println("C to cancel");
  readKeypad();
    GLCD.CursorTo(0,5);
    GLCD.print ("KeyPressed =");
    
    while (1)
    
    {
      readKeypad();
    GLCD.print (customKey);
    
    if (customKey == '#')
    
    {
      inHouseFunction();
    }
    
    if (customKey == 'C')
    
    {
      defaultScreen();
    }
    
    
    
  
    }
  
  

  
}

void dailyScreen()

{
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 0);
  GLCD.println("Enter hour you");
  GLCD.CursorTo(0, 1);
  GLCD.println("will be back");
  GLCD.CursorTo(0, 2);
  GLCD.println("24 hour format ");
  GLCD.CursorTo(0, 3);
  GLCD.println("only 2 digits ");
  GLCD.CursorTo(0, 4);
  GLCD.println("# to accept");
  GLCD.CursorTo(0, 5);
  GLCD.println("C to cancel");
  readKeypad();
    GLCD.CursorTo(0,6);
    GLCD.print ("KeyPressed =");
    
    while (1)
    
    {
      readKeypad();
    GLCD.print (customKey);
    
    if (customKey == '#')
    
    {
      dailyFunction();
    }
    
    if (customKey == 'C')
    
    {
      defaultScreen();
    }
    
    
    
  
    }
  
  
}


void consumptionScreen()

{
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 0);
  GLCD.println("You consumption");
  GLCD.CursorTo(0, 1);
  GLCD.println("and bill for");
  GLCD.CursorTo(0, 2);
  GLCD.println("last hour ");
  GLCD.CursorTo(0, 3);
  GLCD.println("consumption=");
  GLCD.CursorTo(0, 4);
  GLCD.println("bill AED");
  GLCD.CursorTo(0, 5);
  GLCD.println("B for back");
  readKeypad();
    GLCD.CursorTo(0,6);
    GLCD.print ("KeyPressed =");
    
    
    while (1)
    
    {
      readKeypad();
    GLCD.print (customKey);
    
    
    
    if (customKey == 'B')
    
    {
      defaultScreen();
    }
    
    
    
  
    }
  
  
  
}

void manualFunction()

{
  
  defaultScreen();
}

void inHouseFunction()

{
  defaultScreen();
}

void dailyFunction()

{
  defaultScreen();
}




void loop ()

{
  

introScreen();

delay(3000);
 
 defaultScreen(); 
          


 
 
 

 
  
}


