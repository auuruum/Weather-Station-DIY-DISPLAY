#include <Arduino.h>

#include "sets.h"

#include <Wire.h>

#include <GTimer.h>
#include <LiquidCrystal_I2C.h>

#include <fetchWeatherData.h>

GTimer<millis> FetchTimer(FETCH_INTERVAL, true);

void setup() {
    Serial.begin(115200);
    Serial.println();

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
    }

    sett_loop();
}