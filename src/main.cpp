/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP8266 chip.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right ESP8266 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>                //https://github.com/PaulStoffregen/Time
#include <WidgetRTC.h>

BlynkTimer timer;

int value1,analogVal,button,Ref1,Ref2;
int phantram,oldSecond, nowSecond;
bool timeOnOff = false;
bool oldtimeOnOff;
WidgetRTC rtc;

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
WidgetLED appledr(V15);
WidgetLED appledg(V16);
WidgetLED appledy(V17);

unsigned int TimeStart, TimeStop;
byte dayStartSelect = 0;
byte dayStopSelect = 0;
char auth[] = "EkIFNzrP6stGvakmwxG7OEi7UFyRdh3n";

char ssid[] = "WIFI Phong 4";
char pass[] = "hoil@mchi!";
char server[] = "tantien.myvnc.com";
uint16_t port = 8080;


void setup()
{
  // Debug console
  Serial.begin(9600);  
  Blynk.begin(auth, ssid, pass, server, port);
  bool status;
  status = bme.begin(0x76); 
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  digitalWrite(D5,!timeOnOff);
  pinMode(A0,INPUT);
  pinMode(D5,OUTPUT);
}

BLYNK_WRITE(V3) { // điều chỉnh chế độ
  value1 = param.asInt();
}

void checkTime(){
  Blynk.sendInternal("rtc","sync");
}

BLYNK_WRITE(V8){
  TimeInputParam t(param); 
  if (t.hasStartTime())
  {
    TimeStart = t.getStartHour()*60 + t.getStartMinute();     
  }
  else TimeStart = 0;
  if (t.hasStopTime())
  {
    TimeStop = t.getStopHour()*60 + t.getStopMinute();
  }
  else TimeStop = 0;
  dayStartSelect = 0;
  dayStopSelect  = 0;
  for (int i = 1; i <= 7; i++)
    if (t.isWeekdaySelected(i)){
      if (i == 7)
      {
        bitWrite(dayStartSelect, 0,   1);
        bitWrite(dayStopSelect,  1,   1);
      }
      else
      {
        bitWrite(dayStartSelect, i, 1);
        bitWrite(dayStopSelect,  i+1, 1);
      }
    }   
}
BLYNK_WRITE(V9){
   TimeInputParam t(param); 
  if (t.hasStartTime()){
    TimeStart = t.getStartHour()*60 + t.getStartMinute();     
  }else TimeStart = 0;

  if (t.hasStopTime()){
    TimeStop = t.getStopHour()*60 + t.getStopMinute();
  }else TimeStop = 0;
  dayStartSelect = 0;
  dayStopSelect  = 0;
  for (int i = 1; i <= 7; i++)
    if (t.isWeekdaySelected(i)){
      if (i == 7){
        bitWrite(dayStartSelect, 0,   1);
        bitWrite(dayStopSelect,  1,   1);
      }else{
        bitWrite(dayStartSelect, i, 1);
        bitWrite(dayStopSelect,  i+1, 1);
      }
    }   
}

BLYNK_CONNECTED(){
  Blynk.syncAll();
  rtc.begin();
}

void TimeAuto()
{
  unsigned int times = hour()*60 + minute();
  byte today = weekday(); //the weekday now (Sunday is day 1, Monday is day 2) 

  if (TimeStart == TimeStop){
    timeOnOff = false;
  }else if (TimeStart < TimeStop){
    if (bitRead(dayStartSelect, today - 1))
    {
      if ((TimeStart <=  times) && (times < TimeStop)) timeOnOff = true; 
      else                                             timeOnOff = false; 
    }else                                              timeOnOff = false;
  }else if (TimeStart > TimeStop){
    if ((TimeStop <= times) && (times < TimeStart))                       timeOnOff = false;
    else if((TimeStart <= times) && bitRead(dayStartSelect, today - 1))   timeOnOff = true;
    else if((TimeStop > times) && bitRead(dayStartSelect, today))         timeOnOff = true; 
  }
}

void showTime(){
  appledr.off();
  appledg.off();
  appledy.off();
  nowSecond = second();
  if (oldSecond != nowSecond)
  {
    oldSecond = nowSecond;
    if (oldtimeOnOff != timeOnOff){
      if (timeOnOff){
        digitalWrite(D5,LOW);
        Serial.println("Time schedule is ON");
      }
      else{
        digitalWrite(D5,HIGH);
        Serial.println("Time schedule is OFF");
      }
      oldtimeOnOff = timeOnOff;
    }
  }
}

void setLOW(){
  appledr.on();
  appledg.off();
  appledy.off();
}

void setNORMAL(){
  appledr.off();
  appledg.on();
  appledy.off();
  digitalWrite(D5,LOW);
}

void setHIGH(){
  appledr.off();
  appledg.off();
  appledy.on();
  digitalWrite(D5,LOW);
}

void setLED(){
  analogVal = analogRead(A0); // 0 --> 1023S
  phantram = map(analogVal, 0, 1023, 100, 0); // chuyen sang phantram
if (bme.readTemperature() >= 30){
  if(phantram >= 45 && phantram <= 75){  //check nor  
    setNORMAL();
    Blynk.virtualWrite(V6,"Độ ẩm ổn định !!!");
  }else if(phantram < 35 ){
    setLOW();
    Blynk.notify("Độ ẩm thấp, tiến hành tưới cây !!!");
    digitalWrite(D5,HIGH);
    if (phantram >= 50){
      Blynk.notify("Đã tưới cây hoàn tất !!!");
      Blynk.virtualWrite(V6,"Đã tưới hoàn tất !!!");  
      digitalWrite(D5,LOW);
    }
  }else if(phantram >= 95){
    setHIGH();
    Blynk.notify("Độ ẩm cao !!!");
    Blynk.virtualWrite(V6,"Độ ẩm cao !!!");
  }
}else if (bme.readTemperature() < 30){
  if(phantram >= 45 && phantram <= 75){  //check nor  
    setNORMAL();
    Blynk.virtualWrite(V6,"Độ ẩm ổn định !!!");
  }else if(phantram < 25 ){
    setLOW();
    Blynk.notify("Độ ẩm thấp, tiến hành tưới cây !!!");
    digitalWrite(D5,HIGH);
    if (phantram >= 45){
      Blynk.notify("Đã tưới cây hoàn tất !!!");
      Blynk.virtualWrite(V6,"Đã tưới hoàn tất !!!"); 
      digitalWrite(D5,LOW); 
    }
  }else if(phantram >= 95){
    setHIGH();
    Blynk.notify("Độ ẩm cao !!!");
    Blynk.virtualWrite(V6,"Độ ẩm cao !!!");
  }
}
}

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");

  Blynk.virtualWrite(V2, bme.readTemperature());
}

void loop()
{
  Blynk.run();
  analogVal = analogRead(A0); // 0 --> 1023S
  phantram = map(analogVal, 0, 1023, 100, 0); // chuyen sang phantram
  Blynk.virtualWrite(V20,phantram);
  if (value1 == 1){
    setLED();
  }else{
    Blynk.virtualWrite(V6,"Đang chế độ Munual !!!");
    TimeAuto();
    showTime();
  }

  Blynk.virtualWrite(V1,phantram); //hiển thị trên app độ ẩm
  printValues();

  Blynk.virtualWrite(V21,bme.readTemperature());
  delay(500);
}

