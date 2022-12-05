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

