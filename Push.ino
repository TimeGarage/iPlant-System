#include <DHT.h>
#include <SoftwareSerial.h>
#include <BlynkSimpleStream.h>
#include <MQ135.h>
#include<WidgetRTC.h>
#include <TimeLib.h>
SoftwareSerial DebugSerial(0, 1); 
char auth[] = "c3e48c1157a244bfad31615350cc7cb0";

const int pinAir = A0;
const int pinLumiSensor = A5;
const int pinDHT = 2;
const int pinWaterHeight = A1;
const int pinMoisture = A2;
const int pinLED = 3;
//LightSystem
int LightSystemOpen = 0;
int AutoLight = 0;
int SleepSlot =0;
int LightTime =0;
int LightMode = 0;
int LightOpen = 1;
int LightPower =0;
//Settings
#define DHTTYPE DHT11

DHT dht(pinDHT,DHTTYPE);
MQ135 AirSensor = MQ135(pinAir);

BlynkTimer timer;
WidgetRTC rtc;
WidgetTerminal terminal(V0);

BLYNK_WRITE(V15)
{
  LightSystemOpen = param.asInt();
}

BLYNK_WRITE(V16)
{
  AutoLight = param.asInt();
}

BLYNK_WRITE(V17)
{
  long TimeStart = param[0].asLong();
  long TimeStop = param[1].asLong();
  int hours = hour();
  int minutes = minute();
  int seconds = second();
  long TimeNow = hours*3600L+minutes*60L+seconds;
  if(TimeStart <= TimeNow && TimeStop >= TimeNow)
      LightOpen = 1;
  else
      LightOpen = 0;
}

BLYNK_CONNECTED()
{
  rtc.begin();
  Blynk.syncAll();
}

BLYNK_WRITE(V18)
{
    LightPower = param.asInt();
}

BLYNK_WRITE(V19)
{
  LightMode = param.asInt();
  if(LightMode==1)
      {
        LightPower = 30;
        Blynk.virtualWrite(V18,30);
      }
  else if(LightMode==2)
      {
        LightPower = 120;
        Blynk.virtualWrite(V18,120);
       }
  else
      {
        LightPower = 255;    
        Blynk.virtualWrite(V18,255);
       }
 }
    

void ReadSensors()
{
  int temperature = getTemperature();
  int humidity = getHumidity();
  int lumiosity = getLumiosity();
  int air = getAir();
  int water = getWaterHeight();
  int moisture = getMoisture();
  Blynk.virtualWrite(V5,temperature);
  Blynk.virtualWrite(V6,lumiosity);
  Blynk.virtualWrite(V7,humidity);
  Blynk.virtualWrite(V8,air);
  Blynk.virtualWrite(V9,moisture);
  Blynk.virtualWrite(V10,water);
}

void LightSystem()
{
  if(LightSystemOpen & LightOpen)
  {
    if(AutoLight)
    {
      int Elight = analogRead(pinLumiSensor);
      if(Elight<255)
      {
        LightPower = 255-Elight;
      }
      else
        LightPower = 0;    
    Blynk.virtualWrite(V18,LightPower);
    if(LightPower<=30)
      Blynk.virtualWrite(V19,1);
    else if(LightPower<=120)
      Blynk.virtualWrite(V19,2);
    else
      Blynk.virtualWrite(V19,3);
    }
    analogWrite(pinLED,LightPower);
  }
  else
    digitalWrite(pinLED,LOW);
}

void setup()
{
  DebugSerial.begin(9600);
  Serial.begin(9600);
  Blynk.begin(Serial, auth);
  pinMode(pinLED,OUTPUT);
  TCCR2B = TCCR2B & B11111000 | B00000001;//D3 & D11 frequency 31372.55 Hz
  setSyncInterval(10*60);
  timer.setInterval(2000L, ReadSensors);
  timer.setInterval(1000L, LightSystem);
}

int getTemperature()
{
  int temperature = dht.readTemperature();
  return temperature;
}

int getHumidity()
{
  int humidity = dht.readHumidity();
  return humidity;
}

int getLumiosity()
 {
  int val = analogRead(pinLumiSensor);
  return val;
 }

 int getAir()
 {
  int val = AirSensor.getPPM();
  return val;
 }

 int getWaterHeight()
 {
  int val = analogRead(pinWaterHeight);
  if(val>=590)
    return 100;
  else if(val>=570)
    return (val-570)*5/4+75; 
  else if(val>=530)
    return (val-530)*5/8+50;
  else if(val>=500)
    return (val-500)*5/6+25;
  else if(val>=400)
    return (val-400)/4;
  else
    return 0;
 }

 int getMoisture()
 {
  float val = analogRead(pinMoisture);
  val = int(((1010-val)/680)*100);
  if(val>=100)
    return 100;
  else if(val<=5)
    return 0;
  else
    return val;
 }
 
void loop()
{
  Blynk.run();
  timer.run();
}

