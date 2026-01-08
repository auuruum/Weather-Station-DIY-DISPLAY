#include <Arduino.h>

#include "sets.h"

#include <Wire.h>

#include <GTimer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

GTimer<millis> FetchTimer(FETCH_INTERVAL, true);

// Function to fetch weather data from HTTP endpoint
bool fetchWeatherData() {
    HTTPClient http;
    
    String url =
        String("http://") +
        SENSOR_STANTION_MDNS +
        ".local:" +
        String(SENSOR_STANTION_API_PORT) +
        SENSOR_STANTION_API_PATH;

    http.begin(url);

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            tempC = doc["temp"].as<float>();
            humidity = doc["humidity"].as<float>();
            pressure = doc["pressure"].as<float>();
            
            http.end();
            return true;
        } else {
            Serial.print("JSON parse error: ");
            Serial.println(error.c_str());
        }
    } else {
        Serial.print("HTTP request failed, code: ");
        Serial.println(httpCode);
    }
    
    http.end();
    return false;
}

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