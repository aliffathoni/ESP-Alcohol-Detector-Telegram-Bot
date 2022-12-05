#include <TaskScheduler.h>

#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

LiquidCrystal_PCF8574 lcd(0x3F);  // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

static const int RXPin = 12, TXPin = 13;
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

#include <PCF8574.h>

PCF8574 io(0x27); 

#define engine_pin    10
#define emergency_pin 14

#include <MQUnifiedsensor.h>
/************************Hardware Related Macros************************************/
#define         Board                   ("ESP8266")
#define         Pin                     (A0)
/***********************Software Related Macros************************************/
#define         Type                    ("MQ-3")
#define         Voltage_Resolution      (3.3)
#define         ADC_Bit_Resolution      (10)
#define         RatioMQ3CleanAir        (60) //RS / R0 = 60 ppm 
/*****************************Globals***********************************************/
//Declare Sensor
MQUnifiedsensor MQ3(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Wifi network station credentials
#define WIFI_SSID "3DPractical"
#define WIFI_PASSWORD "embeddedelectronics"
#define BOT_TOKEN "5948278068:AAFoK4XdIh68_sZILypkfP9ydIJc8l23Vk4"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

String user_id = "1925023039";
String police_id = "1276693290";
String emergency_id = "";

unsigned long engineTresshold = 2000;
unsigned long starterTresshold = 100;
unsigned long pressHold;

bool engineState = false;
bool starterState = false;
bool ledState = false;
bool starting = false;
bool alcoholState = false;

double alcohol;
double lat = -7.953998, 
       lng = 112.613559;
double toleransi_alcohol = 0.30;

void getPosition(){
  if(gps.encode(ss.read())){
    if (gps.location.isValid()){
      lat = gps.location.lat();
      Serial.println(lat);
      lng = gps.location.lng();
      Serial.println(lng);
    }  
  }
}

void getAlcohol(){
  int adc = analogRead(A0);
  double mq3val = adc/310.0;
  // double mq3val = map(adc, 0.0, 1023.0, 0.05, 10.0);
  // MQ3.update(); // Update data, the arduino will read the voltage from the analog pin
  // double mq3val = MQ3.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  double vOut = adc/1023.0 * 3.3;
  
  if(mq3val > toleransi_alcohol){
    digitalWrite(emergency_pin, HIGH);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alcohol ");
    lcd.print(mq3val);
    lcd.print("mg/L");
    lcd.setCursor(0, 1);
    lcd.print("SEGERA MENEPI!!!");

    String msg = "PERINGATAN!!!\n\nKadar alkohol terdeteksi "+String(mq3val)+"mg/L\n"
                +"Lokasi saat ini https://www.google.com/maps/search/?api=1&query="+String(lat, 6)+"%2C"+String(lng, 6);

    if(bot.sendMessage(user_id, msg, "")){
      Serial.println(msg);
      Serial.println("Message sent!");
      delay(250);
    } else{
      Serial.println("Sent Failed.");
    }

    if(bot.sendMessage(police_id, msg, "")){
      Serial.println(msg);
      Serial.println("Message sent!");
      delay(250);
    } else{
      Serial.println("Sent Failed.");
    }

    // if(bot.sendMessage(emergency_id, msg, "")){
    //   Serial.println(msg);
    //   Serial.println("Message sent!");
    //   delay(250);
    // } else{
    //   Serial.println("Sent Failed.");
    // }
  
    alcoholState = true;
  
  } else{
    digitalWrite(emergency_pin, LOW);
  }

  alcohol = mq3val;
}

void buttonControl(){
  if(!io.read(4)){
    digitalWrite(engine_pin, HIGH);
    delay(250);
    if(!io.read(4)){
      delay(2500);
      if(!io.read(4)){
        engineState = true;
        starterState = true;
        Serial.println("Long pressed!");
        return;
      } else{
        engineState = false;
        starterState = true;
        Serial.println("Pressed!");
        return;
      } 
    } else{
      engineState = false;
      starterState = false;
      Serial.println("Not Pressed!");
    }
  }
}

void engineControl(){
  if(engineState==true && starterState==true){
    Serial.println("Engine Staerted");
    digitalWrite(engine_pin, HIGH);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Mesin Menyala!");
    lcd.setCursor(0, 1);
    lcd.print("Perhatikan Jalan");
    delay(2000);

    starting = true;
  } else if(engineState==false && starterState==true){
    ledState=!ledState;
    digitalWrite(engine_pin, ledState);
    Serial.println("Need more power!");
    delay(300);
  } else{
    digitalWrite(engine_pin, LOW);
  }
}

void printALC(){
  getPosition();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alcohol ");
  lcd.print(alcohol);
  lcd.print("mg/L");
  lcd.setCursor(0, 1);
  if(engineState==true && starterState==true){
    lcd.print("Mesin Menyala");
  } else if(engineState==false && starterState==true){
    lcd.print("Nyalakan Mesin");
  } else{
    lcd.print("Nyalakan Mesin");
  }
}

void printGPS(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lat:");
  lcd.print(String(lat, 6));
  lcd.setCursor(0, 1);
  lcd.print("Lng:");
  lcd.print(String(lng, 6));
}

void taskControl();

Task get_alcohol(20, TASK_FOREVER, &getAlcohol);
Task task_control(20, TASK_FOREVER, &taskControl);
Task button_control(20, TASK_FOREVER, &buttonControl);
Task engine_control(20, TASK_FOREVER, &engineControl);
Task print_alcohol(2500, TASK_FOREVER, &printALC);
Task get_position(50, TASK_FOREVER, &getPosition);
Task print_gps(5000, TASK_FOREVER, &printGPS);

Scheduler sch;

void taskControl(){
  if(starting == true){
    button_control.disable();
    engine_control.disable();
  } else{
    sch.addTask(button_control);
    button_control.enable();
    sch.addTask(engine_control);
    engine_control.enable();
  }

  if(alcoholState == true){
    button_control.disable();
    engine_control.disable();
    print_alcohol.disable();
    print_gps.disable();
    get_alcohol.disable();
    get_position.disable();
  }
}

void setup()
{
  Serial.begin(115200);
  ss.begin(GPSBaud);
  Wire.begin();
  io.begin();
  pinMode(engine_pin, OUTPUT);
  pinMode(emergency_pin, OUTPUT);

  delay(10);
  digitalWrite(engine_pin, LOW);
  digitalWrite(emergency_pin, LOW);

  MQ3.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ3.setA(0.3934); MQ3.setB(-1.504); // Configure the equation to to calculate Benzene concentration

  MQ3.init(); 

  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ3.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ3.calibrate(RatioMQ3CleanAir);
    Serial.print(".");
  }
  MQ3.setR0(calcR0/10);
  

  lcd.begin(16, 2);  // initialize the lcd
  lcd.setBacklight(255);

  // attempt to connect to Wifi network:
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  lcd.setCursor(0, 1);
  lcd.print("Connected.");
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
  delay(1000);

  sch.init();

  sch.addTask(task_control);
  task_control.enable();
  sch.addTask(print_alcohol);
  print_alcohol.enable();
  sch.addTask(print_gps);
  print_gps.enable();
  sch.addTask(get_alcohol);
  get_alcohol.enable();
  sch.addTask(get_position);
  get_position.enable();
}

void loop(){sch.execute();}