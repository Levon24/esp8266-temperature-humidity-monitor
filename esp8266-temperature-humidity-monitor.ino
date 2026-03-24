#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <AHT20.h>
#include <SSD1306Spi.h>
#include <NTPClient.h>
#include <time.h>
#include "wifi.h"

const long utcOffsetInSeconds = 4 * 60 * 60;
//char daysOfTheWeek[7][] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

AHT20 aht20;
SSD1306Spi display(D0, D4, D8, GEOMETRY_128_64);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ru.pool.ntp.org", utcOffsetInSeconds);

void setup() {
  // debug
  Serial.begin(115200);

  // WiFi
  WiFi.mode(WIFI_STA); 
  WiFi.begin(wifi_sid, wifi_password);
  WiFi.printDiag(Serial);
  
  // i2c
  Wire.begin();
  
  // sensor
  aht20.begin();

  // display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
}

void loop() {
  delay(2000);

  float temperature = aht20.getTemperature();
  float humidity = aht20.getHumidity();

  display.clear();

  if (WiFi.status() == WL_CONNECTED) {
    display.drawString(0, 0, "IP: " + WiFi.localIP().toString());

    timeClient.update();

    time_t time = timeClient.getEpochTime();

    struct tm * ti;
    ti = localtime(&time);
    
    char date_value[13];
    strftime(date_value, 13, "%d/%m/%Y", ti);

    char date_str[20] = "Date: ";

    display.drawString(0, 10, strcat(date_str, date_value));
    display.drawString(0, 20, "Time: " + timeClient.getFormattedTime());
  }

  display.drawString(0, 30, "Temperature: " + String(temperature, 3) + "°C");
  display.drawString(0, 40, "Humidity: " + String(humidity, 3) + "%");

  display.display();
}
