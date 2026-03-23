#include <IskakINO_LiquidCrystal_I2C.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <AHT20.h>

#define LCD_ADDRESS   0x27
#define LCD_CMD_LIGHT 0x10

const char* ssid = "Eris";
const char* password = "***";

LiquidCrystal_I2C lcd(16, 2);
AHT20 aht20;

void setup() {
  lcd.begin();
  lcd.backlight();

  lcd.setCursor(0, 0); 
  lcd.print("Just for fun !!!");

  lcd.setCursor(0, 1);
  lcd.print("Second line");

  Serial.begin(115200);

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  lcd.setCursor(0, 1);
  lcd.print("IP " + WiFi.localIP().toString());

  Wire.begin();
  aht20.begin();
}

void loop() {
  delay(2000);

  float temperature = aht20.getTemperature();
  float humidity = aht20.getHumidity();

  lcd.setCursor(0, 0);
  lcd.print("Temper: " + String(temperature, 3) + " C");

  lcd.setCursor(0, 1);
  lcd.print("Humidy: " + String(humidity, 3) + " %");
}
