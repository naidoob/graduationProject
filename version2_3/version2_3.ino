
// Version 2_2



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
volatile double lastHourCost=0.0;
volatile double Hour24Cost=0.0;
volatile double totalPowerForTheDay=0.0;
volatile double oneDayPower=0.0;
volatile double kwNow=0.0;
volatile double apooh;
volatile int bedroomDelay=0;

byte hoursBack=0;
volatile int timeToSwitchOnWaterHeater=0;
volatile int hoursToSwitchOnWaterHeater=0;



//mode =0 inhouse default mode
//mode =1 manual mode
//mode =2 daily mode

int mode=0;




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



//RTC
#include <Wire.h>
#include "RTClib.h"
 RTC_DS1307 RTC;
 DateTime now;




void setup()

{
  
  mode=0;
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
        RTC.adjust(DateTime(__DATE__, __TIME__));
    
    
 
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
  sendSMSsmoke();
  smsCount = smsCount+1;
  
}
 
 
  
   
 readLDR();



if ((LDRSensorValue >LDRSensorThreshold)&&((mode==0)||(mode==2)))

{
  digitalWrite (entrancelamp, LOW);
}
else
{
  digitalWrite (entrancelamp, HIGH);
  
}

if ((mode==2) && ((now.hour()==timeToSwitchOnWaterHeater)))

{
  digitalWrite (waterheater, HIGH);
}


}



//hardware interrupt service routine for PIR

 void changeStateOfBedroomLight ()
 
 {
  
   
   
if (mode==0)
{
   digitalWrite (bedroomlight, HIGH );
   bedroomDelay=0;
}  
   
 }


void readPower()

{
 
  
  //current sensor over reading by 0.25 amps
  currentConsumed =(emon1.calcIrms(1480))-0.21;
  
  kwNow =     currentConsumed *220.0*0.001;
  
  if (currentConsumed < 20.0) 
  
  {

  counter = counter +1;
  
  //Serial.print("counter=");
  //Serial.print(counter);  
  //Serial.println();
  
  
  totalCurrent=totalCurrent+  currentConsumed;
  
  
  
  // after one minute i.e bedroomDelay =15 turn off bedroom lamp
  
  
  
  bedroomDelay = bedroomDelay+1;
  
  if (bedroomDelay==15)
    
    {
      if (mode==0)
      
      {
       digitalWrite(bedroomlight, LOW); 
      }
      bedroomDelay=0;
    }
  
  //Serial.print (bedroomDelay);
 
 if (counter == 900)
 
 {
   
   hourCounter=hourCounter+1;
    averageCurrentOverOneHour = totalCurrent/900.0;
    
    //Serial.print(" hourCounter=");   
    //Serial.print(hourCounter);
    //Serial.println();
    
    // to ocnvert watts to kiliwatt multiply by 0.001
    
  averagePowerOverOneHour =     averageCurrentOverOneHour *220.0*0.001;
  
          
   

  
  
  
  
   
  hourArray[hourCounter]=averagePowerOverOneHour;
  
  apooh = averagePowerOverOneHour;
  lastHourCost=   apooh *0.2;
  
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
    

oneDayPower =totalPowerForTheDay;

// for expats the cost is 20 fils per Kwh
    

Hour24Cost = oneDayPower*.20;

    totalPowerForTheDay=0.0;
    hourCounter=0;
    
    // for expats the cost is 20 fils per Kwh
    
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

 void sendSMSsmoke()
 
 {
   
   Serial1.print("AT+CMGF=1\r"); //Set text mode.
   delay (100);
   Serial1.print("AT+CMGS=0507323554\r"); //AT command to send SMS
      delay(100);
      Serial1.print("Smoke/Gas detected  here http://tinyurl.com/cqnezgq"); //Print the message
      delay(100);
      Serial1.print("\x1A"); //Send it ascii SUB
     
 }
 
 void sendSMSrobber()
 
 {
   
   Serial1.print("AT+CMGF=1\r"); //Set text mode.
   delay (100);
   Serial1.print("AT+CMGS=0507323554\r"); //AT command to send SMS
      delay(100);
      Serial1.print("Robber in your home"); //Print the message
      delay(100);
      Serial1.print("\x1A"); //Send it ascii SUB
     
 }
 
 void sendSMSpowerFailure()
 
 {
   
   Serial1.print("AT+CMGF=1\r"); //Set text mode.
   delay (100);
   Serial1.print("AT+CMGS=0507323554\r"); //AT command to send SMS
      delay(100);
      Serial1.print("Power Failure in your home"); //Print the message
      delay(100);
      Serial1.print("\x1A"); //Send it ascii SUB
     
 }
 
 void sendSMSemergency()
 
 {
   
   Serial1.print("AT+CMGF=1\r"); //Set text mode.
   delay (100);
   Serial1.print("AT+CMGS=0507323554\r"); //AT command to send SMS
      delay(100);
      Serial1.print("Your child needs you now  stop shopping"); //Print the message
      delay(100);
      Serial1.print("\x1A"); //Send it ascii SUB
     
 }
 
 
 
 
  void gotoinHouseMode()
  
  {
    
digitalWrite(bedroomlight, LOW);
digitalWrite(entrancelamp, LOW);
digitalWrite(waterheater, HIGH);
digitalWrite(fridge, HIGH);
digitalWrite(TV, HIGH);
digitalWrite(HiFi, HIGH);
digitalWrite(Airconditionar, LOW);

    
    
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
    GLCD.print("kwNow=");
    GLCD.print( kwNow ); 
    
   
 GLCD.print( " KLH=");
 GLCD.print(apooh);
    
    GLCD.CursorTo(0, 5);
    GLCD.print( "CostLastHour AED=");
 GLCD.print(lastHourCost);
    
    GLCD.CursorTo(0,6);
    
    if (mode==0)
    {
    GLCD.print ("Mode= inHouse");
    }
    
    if (mode==1)
    {
    GLCD.print ("Mode= manual");
    }
    
    if (mode==2)
    {
    GLCD.print ("Mode= Daily");
    }
    
    
    
    
    
    if ((t<=24.00)&&(mode==0))
    
    {
      digitalWrite (Airconditionar,LOW);
    }
    
    if ((t>24.00)&&(mode==0))
    
    {
      digitalWrite (Airconditionar,HIGH);
    }
    
    
    
    
    GLCD.CursorTo(0,7);
    GLCD.print ("Press * for Settings");
    
    
    
    
    
    
    
    
    
    
    
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
  
  
  GLCD.CursorTo(0,0);
    GLCD.print ("Mode Screen");
    
  
  GLCD.CursorTo(0, 2);
  GLCD.println("Press C for Manual");
  GLCD.CursorTo(0, 3);
  GLCD.println("Press A for inhouse");
  GLCD.CursorTo(0, 4);
  GLCD.println("Press D for Daily ");
  GLCD.CursorTo(0, 5);
  GLCD.println("Press B for back ");
  
    
    
    while (1)
    
    {
      readKeypad();
    GLCD.print (customKey);
    
    if (customKey == 'C')
    
    {
      manualScreen();
    }
    
    if (customKey == 'A')
    
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
  
  GLCD.CursorTo(0,0);
    GLCD.print ("Settings Screen");
    
  
  GLCD.CursorTo(0, 2);
  GLCD.println("Press A for Mode");
  GLCD.CursorTo(0, 3);
  GLCD.println("Press C for Cons/Bill");
  GLCD.CursorTo(0, 4);
  GLCD.println("Press B for back ");
  
    
    
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
  
  GLCD.CursorTo(0,0);
    GLCD.print ("ManualMode Screen");
    
  
  GLCD.CursorTo(0, 2);
  GLCD.println("Are you sure");
  GLCD.CursorTo(0, 3);
  GLCD.println("you want");
  GLCD.CursorTo(0, 4);
  GLCD.println("manual mode? ");
  GLCD.CursorTo(0, 5);
  GLCD.println("# to accept ");
  GLCD.CursorTo(0, 6);
  GLCD.println("Press C to cancel");
  
    
    
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
  GLCD.CursorTo(0,0);
    GLCD.print ("inHouse Screen");
    
  
  GLCD.CursorTo(0, 2);
  GLCD.println("Are you Sure");
  GLCD.CursorTo(0, 3);
  GLCD.println("you want");
  GLCD.CursorTo(0, 4);
  GLCD.println("inhouse mode? ");
  GLCD.CursorTo(0, 5);
  GLCD.println(" # to accept");
  GLCD.CursorTo(0, 6);
  GLCD.println("Press C to cancel");
  
   
    
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
  GLCD.CursorTo(0,0);
    GLCD.print ("daily Screen");
    
  
  GLCD.CursorTo(0, 2);
  GLCD.println("Press # to");
  GLCD.CursorTo(0, 3);
  GLCD.println("enter the number");
  GLCD.CursorTo(0, 4);
  GLCD.println("of hours you will ");

  GLCD.CursorTo(0, 6);
  GLCD.println(" be gone for");
  GLCD.CursorTo(0, 7);
  GLCD.println("Press C to cancel");
  
  
  
    
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
  GLCD.CursorTo(0,0);
    GLCD.print ("Consumption Screen");
    
    
    
  
  GLCD.CursorTo(0, 2);
  GLCD.println("Consumption for");
  GLCD.CursorTo(0, 3);
  GLCD.print("24h KW= ");
  GLCD.print(oneDayPower);
  GLCD.CursorTo(0, 4);
  GLCD.println("Cost for ");
  GLCD.CursorTo(0, 5);
  GLCD.print("24h AED= ");
  GLCD.print(Hour24Cost);
  GLCD.CursorTo(0, 7);
  GLCD.println("Press B for back");
  
    
    
    
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
  mode=1;
  gotoManualMode();
  defaultScreen();
 
}

void inHouseFunction()

{
  mode=0;
  gotoinHouseMode();
  defaultScreen();
  
}

void dailyFunction()

{
  mode=2;
  gotoDailyMode();
  
  setHoursScreen();
  
  defaultScreen();
  
}

void setHoursScreen()

{
  GLCD.ClearScreen();
  GLCD.CursorTo(0,0);
    GLCD.print ("set hours screen");
    
  
  GLCD.CursorTo(0, 2);
  GLCD.println("Enter a number ");
  GLCD.CursorTo(0, 3);
  GLCD.println("from 1 to 9");
  

  
  GLCD.CursorTo(0, 7);
  GLCD.println("Press C to cancel");
  
  
  
    
    while (1)
    
    {
      readKeypad();
      GLCD.print(customKey);
  
  hoursBack = customKey -'0';
    
    if ((hoursBack >0)&&(hoursBack<10))
    
    {
      Serial.print("hoursBack = ");
      Serial.print(hoursBack);
            Serial.println();
      Serial.print("hour Now= ");
      Serial.print(now.hour());
                  Serial.println();
      hoursToSwitchOnWaterHeater = hoursBack-1;
      Serial.print("HTWOWH= ");
      Serial.print(hoursToSwitchOnWaterHeater);
                  Serial.println();
      timeToSwitchOnWaterHeater = now.hour()+hoursToSwitchOnWaterHeater;
      Serial.print("TTWOWH= ");
      Serial.print(timeToSwitchOnWaterHeater);
                  Serial.println();
      
      defaultScreen();
      
    }
    
    if (customKey == 'C')
    
    {
      defaultScreen();
    }
    
    
    
  
    }
  
  
}


  
  


  



void loop ()

{
  

introScreen();

delay(3000);
 
 defaultScreen(); 
          


 
 
 

 
  
}


