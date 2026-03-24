#include <IskakINO_LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <AHT20.h>
#include "wifi.h"

AHT20 aht20;
LiquidCrystal_I2C lcd(16, 2);

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
  lcd.begin();
  lcd.backlight();

  lcd.setCursor(0, 0); 
  lcd.print("Just for fun !!!");

  lcd.setCursor(0, 1);
  lcd.print("Second line");
}

void loop() {
  delay(2000);

  float temperature = aht20.getTemperature();
  float humidity = aht20.getHumidity();

  lcd.setCursor(0, 0);
  lcd.print("Temper: " + String(temperature, 3) + " C");

  lcd.setCursor(0, 1);
  lcd.print("Humidy: " + String(humidity, 3) + " %");

  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 1);
    lcd.print("IP " + WiFi.localIP().toString());
  }
}
