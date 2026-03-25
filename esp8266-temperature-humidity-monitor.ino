#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <AHT20.h>
#include <SSD1306Spi.h>
#include <NTPClient.h>
#include <time.h>
#include <ESP8266TimerInterrupt.h>
#include <PubSubClient.h>
#include "wifi.h"

const long utcOffsetInSeconds = 4 * 60 * 60;
char daysOfweek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

AHT20 aht20;
SSD1306Spi display(D0, D4, D8, GEOMETRY_128_64);

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
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  // Timer
  InterruptTimer.attachInterruptInterval(1000000, TimerHandler);

  // mqtt
  pubSubClient.setServer(mqttHost, mqttPort);
}

void loop() {
  float temperature = aht20.getTemperature();
  float humidity = aht20.getHumidity();

  display.clear();

  if (WiFi.status() == WL_CONNECTED) {
    display.drawString(0, 0, "IP: " + WiFi.localIP().toString());

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
        delay(1000);
      }

      snprintf(payload, sizeof(payload), "%.5f", humidity);
      if (pubSubClient.publish(mqttTopicHumidity, payload)) {
        //Serial.println("MQTT Humidity send");
        delay(1000);
      }
      
      pubSubClient.disconnect();
    }
  } else {
    display.drawString(0, 0, "IP: Disconnected");
  }

  // time
  struct tm* ti = localtime(&unixTime);
  char buffer[20];

  // date
  char date[13];
  strftime(date, 13, "%d/%m/%Y", ti);

  display.drawStringf(0, 10, buffer, "Date: %s", date);
  display.drawStringf(0, 20, buffer, "Week: %s", daysOfweek[timeClient.getDay()]);
  display.drawString(0, 30, "Time: " + timeClient.getFormattedTime());
  display.drawString(0, 40, "Temperature: " + String(temperature, 3) + "°C");
  display.drawString(0, 50, "Humidity: " + String(humidity, 3) + "%");

  display.display();

  delay(2000);
}
