#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <AHT20.h>
#include <NTPClient.h>
#include <time.h>
#include <ESP8266TimerInterrupt.h>
#include <PubSubClient.h>
#include <SSD1306Spi.h>
#include <SPI.h>
#include "wifi.h"

#define BATTERY_MAX   4.2
#define BATTERY_MIN   3.0
#define MESURES       15  // 30 sec / 2 sec between mesures

//const long utcOffsetInSeconds = 4 * 60 * 60;
char daysOfweek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

AHT20 aht20;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ru.pool.ntp.org", 0);

time_t unixTime;
long timeSync;
int mesure;
float temperatures[MESURES];
float humidities[MESURES];

ESP8266Timer InterruptTimer;

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

const char *mqttHost = "192.168.4.13";
const int mqttPort = 1883;
const char *mqttTopic = "sensors/kitchen";
const char *mqttClientId = "esp8266-kitchen";

SSD1306Spi display(D0, D3, D8, GEOMETRY_128_64);

// Time
ICACHE_RAM_ATTR void TimerHandler() {
  unixTime++;
}

/**
 * Initialization
 */
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
  display.setBrightness(0x3F);
  display.flipScreenVertically();
  
  // Timer
  InterruptTimer.attachInterruptInterval(1000000, TimerHandler);

  // mqtt
  pubSubClient.setServer(mqttHost, mqttPort);
}

/**
 * Forewer loop
 */
void loop() {
  float temperature = aht20.getTemperature();
  float humidity = aht20.getHumidity();

  int batteryValue = analogRead(A0);
  float batteryVoltage = ((float) batteryValue) * (BATTERY_MAX / 1023.0);
  float batteryPercent = (batteryVoltage - BATTERY_MIN) * (100.0 / (BATTERY_MAX - BATTERY_MIN));

  display.clear();
  display.setFont(ArialMT_Plain_10);

  if (WiFi.status() == WL_CONNECTED) {
    display.drawString(0, 0, "IP: " + WiFi.localIP().toString());

    if (timeSync == 0 && timeClient.update()) {
      Serial.println("Sync time.");
      unixTime = timeClient.getEpochTime();
      timeSync = 12 * 1800; // 24 hours
    } else {
      if (timeSync > 0) {
        timeSync--;
      }
    }

    if (mesure < MESURES) {
      temperatures[mesure] = temperature;
      humidities[mesure] = humidity;
      
      mesure++;
    } else {
      if (pubSubClient.connect(mqttClientId)) {
        char payload[128];
        float t, h;

        for (int i = 0; i < MESURES; i++) {
          t += temperatures[i];
          h += humidities[i];
        }
        t = t / MESURES;
        h = h / MESURES;

        snprintf(payload, sizeof(payload), "{\"timestamp\": %ld, \"temperature\": %.6f, \"humidity\": %.6f, \"battery\": %.6f}", (long) unixTime, t, h, batteryVoltage);
        Serial.print("Payload: ");
        Serial.println(payload);
        if (pubSubClient.publish(mqttTopic, payload)) {
          Serial.println("MQTT Send");
          //delay(1000);
        }

        pubSubClient.disconnect();
        mesure = 0;
      }
    }
  } else {
    display.drawString(0, 0, "IP: Disconnected");
  }

  char text[30];
  display.setFont(ArialMT_Plain_24);
  
  snprintf(text, sizeof(text), "%.4f °C", temperature);
  display.drawString(0, 10, text);

  snprintf(text, sizeof(text), "%.4f %%", humidity);
  display.drawString(0, 30, text);

  display.setFont(ArialMT_Plain_10);

  snprintf(text, sizeof(text), "Battery: %.2f V > %.2f %%", batteryVoltage, batteryPercent);
  display.drawString(0, 52, text);

  display.display();

  delay(2000);
}
