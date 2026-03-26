#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <AHT20.h>
#include <NTPClient.h>
#include <time.h>
#include <ESP8266TimerInterrupt.h>
#include <PubSubClient.h>
#include "LGFX.hpp"
#include <SPI.h>
#include "wifi.h"

const long utcOffsetInSeconds = 4 * 60 * 60;
char daysOfweek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

AHT20 aht20;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ru.pool.ntp.org", utcOffsetInSeconds);

time_t unixTime;
long timeSync;

ESP8266Timer InterruptTimer;

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

const char *mqttHost = "mercury.home.lan";
const int mqttPort = 1883;
const char *mqttTopicTemperature = "sensors/kitchen/temperature";
const char *mqttTopicHumidity = "sensors/kitchen/humidity";
const char *mqttClientId = "kitchen";

LGFX display;

// Time
ICACHE_RAM_ATTR void TimerHandler() {
  unixTime++;
}

void setup() {
  // debug
  Serial.begin(115200);

  // WiFi
  WiFi.mode(WIFI_STA); 
  WiFi.begin(wifiSid, wifiPassword);
  WiFi.printDiag(Serial);
  
  // i2c
  Wire.begin();
  
  // sensor
  aht20.begin();

  // display
  display.init();
  //display.fillScreen(TFT_BLACK);
  
  // Timer
  InterruptTimer.attachInterruptInterval(1000000, TimerHandler);

  // mqtt
  pubSubClient.setServer(mqttHost, mqttPort);
}

void loop() {
  float temperature = aht20.getTemperature();
  float humidity = aht20.getHumidity();

  if (WiFi.status() == WL_CONNECTED) {
    if (timeSync == 0 && timeClient.update()) {
      Serial.println("Sync time...");
      unixTime = timeClient.getEpochTime();
      timeSync = 12 * 1800; // 12 hours
    } else {
      if (timeSync > 0) {
        timeSync--;
      }
    }

    if (pubSubClient.connect(mqttClientId)) {
      char payload[10];

      snprintf(payload, sizeof(payload), "%.5f", temperature);
      if (pubSubClient.publish(mqttTopicTemperature, payload)) {
        //Serial.println("MQTT Temperature send");
        //delay(1000);
      }

      snprintf(payload, sizeof(payload), "%.5f", humidity);
      if (pubSubClient.publish(mqttTopicHumidity, payload)) {
        //Serial.println("MQTT Humidity send");
        //delay(1000);
      }
      
      pubSubClient.disconnect();
    }
  }

  // time
  struct tm* ti = localtime(&unixTime);
 
  // date
  char date[13];
  strftime(date, 13, "%d/%m/%Y", ti);

  // display
  display.clearDisplay();

  if (WiFi.status() == WL_CONNECTED) {
    display.setTextColor(TFT_GREENYELLOW);
    display.setTextSize(2);
    display.drawString("IP: " + WiFi.localIP().toString(), 0, 0);
  } else {
    display.setTextColor(TFT_RED);
    display.setTextSize(2);
    display.drawString("IP: Disconnected", 0, 0);
  }

  char text[24];

  snprintf(text, sizeof(text), "Date: %s", date);
  display.setTextColor(TFT_CYAN);
  display.setTextSize(2);
  display.drawString(text, 0, 16);

  snprintf(text, sizeof(text), "Week: %s", daysOfweek[timeClient.getDay()]);
  display.setTextColor(TFT_CYAN);
  display.setTextSize(2);
  display.drawString(text, 0, 32);

  snprintf(text, sizeof(text), "Time: %s", timeClient.getFormattedTime());
  display.setTextColor(TFT_GREEN);
  display.setTextSize(2);
  display.drawString(text, 0, 48);

  snprintf(text, sizeof(text), "Temperature: %.2fC", temperature);
  display.setTextColor(TFT_YELLOW);
  display.setTextSize(2);
  display.drawString(text, 0, 64);

  snprintf(text, sizeof(text), "Humidity: %.2f%%", humidity);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(2);
  display.drawString(text, 0, 80);

  delay(2000);
}
