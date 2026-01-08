#include <Arduino.h>

#include "sets.h"

#include <Wire.h>

#include <GTimer.h>
#include <LiquidCrystal_I2C.h>

#include <fetchWeatherData.h>

GTimer<millis> FetchTimer(FETCH_INTERVAL, true);

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

void initializeLCD() {
    lcd.init();          // initialize LCD
    lcd.backlight();     // turn on backlight
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Weather Display");

    lcd.setCursor(0, 1);
    lcd.print("By aurum");
}

void showWeatherOnLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(tempC);
    lcd.print("C H:");
    lcd.print(humidity);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("P:");
    lcd.print(pressure);
    lcd.print("hPa");
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Wire.begin(21, 22);

    initializeLCD();

    sett_begin();

    Serial.println(db[kk::wifi_ssid]);

    Serial.print("SETUP | LED is now ");
    Serial.println(db[kk::switch_state] ? "ON" : "OFF");

    if (!MDNS.begin(MDNS_ADDRESS)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
        delay(1000);
        }
    }
    Serial.println("mDNS responder started");

    // Fetch initial weather data from HTTP endpoint
    if (fetchWeatherData()) {
        Serial.println("initial weather data fetch successful");
    } else {
        Serial.println("initial weather data fetch failed, skipping forecast");
    }
}

void loop() {
    if (FetchTimer.tick()) {
        fetchWeatherData();
        showWeatherOnLCD();
    }


    sett_loop();
}