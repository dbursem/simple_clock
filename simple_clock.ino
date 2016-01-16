//include nescessary libraries
#include <FrequencyTimer2.h> //http://www.arduino.cc/playground/Code/FrequencyTimer2
#include <Wire.h> //http://www.arduino.cc/en/Reference/Wire
#include <RTClib.h> //https://github.com/adafruit/RTClib

#define COMMON_ANODE true //set to false for common cathode displays!

int dailyDrift = 0; // daily drift to be compensated, in seconds. Positive if your clock is running fast, negative if it's running slow. 

//LED arrays
byte segments[8] = {
  4, 2, 6, 8, 9, 3, 5, 7}; //pins for segments A, B, C, D, E, F, G, DP
byte digits[4] = {
  11, 12, A0, A1}; //pins for digits 1, 2, 3, 4
byte button[2] = {
  A2, A3 }; // pins for buttons 1, 2

byte seven_seg_digits[10][7] = { 
  { 1,1,1,1,1,1,0 },  // = 0  
  { 0,1,1,0,0,0,0 },  // = 1
  { 1,1,0,1,1,0,1 },  // = 2
  { 1,1,1,1,0,0,1 },  // = 3
  { 0,1,1,0,0,1,1 },  // = 4
  { 1,0,1,1,0,1,1 },  // = 5
  { 1,0,1,1,1,1,1 },  // = 6
  { 1,1,1,0,0,0,0 },  // = 7
  { 1,1,1,1,1,1,1 },  // = 8
  { 1,1,1,1,0,1,1 }   // = 9
};

//init libraries
RTC_Millis RTC;

// some empty variables for later use
byte hours = 0, minutes = 0, t = 0, oldHour = 0;
byte number[4];
DateTime now;
unsigned long oldmillis;

int hourlyDrift = dailyDrift / 24;
int driftRemainder = dailyDrift % 24; 

//setup function
void setup()
{
  Serial.begin(9600);
  RTC.begin(DateTime(2000,1,1,0,0,0));  //use the line below insted to automatically set the time on upload
  //RTC.begin(DateTime(F(__DATE__), F(__TIME__)));
  
  //set pinmodes
  pinMode(FREQUENCYTIMER2_PIN, OUTPUT); //this one is defined in the frequencytimer lib
  for (int i = 0; i < 8; i++) {
    //Segments (cathode)
    pinMode(segments[i], OUTPUT); 
    digitalWrite(segments[i],COMMON_ANODE); // off
  }
  for (int i = 0; i < 4; i++) {
    //Digits (anode-side)
    pinMode(digits[i], OUTPUT);
    digitalWrite(digits[i],!COMMON_ANODE); // off
  }
  
  pinMode(button[0],INPUT_PULLUP);
  pinMode(button[1], INPUT_PULLUP);
  
  // Turn off toggling of pin 11
  FrequencyTimer2::disable();
  // Set refresh rate (interrupt timeout period)
  FrequencyTimer2::setPeriod(400);
  // Set interrupt routine to be called
  FrequencyTimer2::setOnOverflow(Display);

  //print some instructions
  Serial.println("Enter ? to request the current date and time.");
  Serial.println("Enter D[yymmdd] to set the date. Example: D121302 will set 13-02-2012.");
  Serial.println("Enter T[hhmm] to set the time. Example: T1539 will set 15:39 (always use 24hr notation)." );
}

void loop(){
  if (millis() < oldmillis )
  {
    //millis function has overflown. This will happen every ~50 days.
    //Set time to last known correct time, so the clock keeps running (with a few µs delay). 
    RTC.adjust(now);
  }
  oldmillis = millis();
  now = RTC.now();
  hours = now.hour();
  minutes = now.minute();
  number[0] = floor(hours/10);
  number[1] = hours % 10;
  number[2] = floor(minutes/10);
  number[3] = minutes % 10;
  
  processSyncMessage();
  checkButtons();
  driftCorrection();
}

void Display()
{
  digitalWrite(digits[t], !COMMON_ANODE); //turn previous row off
  t++;
  if (t == 4){
    t = 0;
  }
  for (byte i = 0; i < 7; i++)
  {
    if (COMMON_ANODE)
      digitalWrite(segments[i],!seven_seg_digits[number[t]][i]);
    else
      digitalWrite(segments[i],seven_seg_digits[number[t]][i]);
  }
  digitalWrite(digits[t], COMMON_ANODE); // Turn this entire row on at once 
}

void checkButtons()
{
  if (!digitalRead(button[0]))
  {
    delay(100);
    if (!digitalRead(button[0]))
    {
      RTC.adjust(DateTime(now.year(),now.month(),now.day(),now.hour()+1,now.minute(),00));
    }
  }
  else if (!digitalRead(button[1]))
  {
    delay(100);
    if (!digitalRead(button[1]))
    {
      RTC.adjust(DateTime(now.year(),now.month(),now.day(),now.hour(),now.minute()+1,00));
    }
  }
}

void processSyncMessage() {
  // if time sync available from serial port, update time
  if(Serial.available() > 0) { 
    delay(10);
    char c = Serial.read() ; 
    if( c == 'T' || c == 't' ) {
      //read hours      
      c = Serial.read();      
      byte newHour = 10 * (c - 48);
      c = Serial.read();
      newHour += (c - 48) ;
      //read minutes
      c = Serial.read();      
      byte newMinute = 10 * (c - 48);
      c = Serial.read();
      newMinute += (c - 48) ;
      if (newHour >= 0 && newHour < 24 
        && newMinute >= 0 && newMinute < 60)
      {
        DateTime now = RTC.now();
        RTC.adjust(DateTime(now.year(),now.month(),now.day(),newHour,newMinute,00));
        printDateTime();
      }
      else {
        Serial.println("illegal time!");
      }
    }
    else if (c == 'D' || c == 'd'){
      //read year      
      c = Serial.read();      
      byte newYear= 10 * (c - 48);
      c = Serial.read();
      newYear +=  (c - 48) ;
      //read month
      c = Serial.read();      
      byte newMonth = 10 * (c - 48);
      c = Serial.read();
      newMonth += (c - 48) ;
      //read day
      c = Serial.read();      
      byte newDay = 10 * (c - 48);
      c = Serial.read();
      newDay += (c - 48) ;
      if (newYear > 0 && newYear < 60 
        && newMonth > 0 && newMonth <= 12
        && newDay > 0 && newDay <= 31)
      {
        DateTime now = RTC.now();
        RTC.adjust(DateTime(newYear,newMonth,newDay,now.hour(),now.minute(),now.second()));
        printDateTime();
      }
      else {
        Serial.println("illegal date!");
      }
    }  
    else if (c == '?'){
      printDateTime();
    }  
  }
}
void printDateTime() {
  DateTime now = RTC.now();
  Serial.print("Current date/time:  ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
}

void driftCorrection() {
  if (hours > oldHour && driftCorrection != 0)
  {
    //one hour has passed, time to compensate for drift
    oldHour = hours;
    
    RTC.adjust(RTC.now() - hourlyDrift);
    
    if (hours = 0) //one day has passed, compensate the reminder of drift
    {
      RTC.adjust(RTC.now() - driftRemainder); 
    }
  }
}

