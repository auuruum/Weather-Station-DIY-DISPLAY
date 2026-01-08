#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#include "sets.h"
#include <GTimer.h>

GTimer<millis> FetchTimer(FETCH_INTERVAL, true);

// Variables to store weather data
float tempC = 0;
float humidity = 0;
float pressure = 0;

void fetchWeatherData() {
    HTTPClient http;
    
    // Use mDNS address or IP
    http.begin("http://weather-station.local:81/weather");
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Received: " + payload);
        
        // Parse JSON response
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            tempC = doc["temperature"];
            humidity = doc["humidity"];
            pressure = doc["pressure"];
            
            Serial.printf("Temp: %.1f°C, Humidity: %.1f%%, Pressure: %.1f hPa\n", 
                         tempC, humidity, pressure);
        } else {
            Serial.println("JSON parse failed!");
        }
    } else {
        Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    sett_begin();

    // Connect to WiFi
    WiFi.begin(db[kk::wifi_ssid].c_str(), db[kk::wifi_pass].c_str());
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    if (!MDNS.begin(MDNS_ADDRESS)) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
    Serial.println("mDNS responder started");

    // Initial fetch
    fetchWeatherData();
}

void loop() {
    if (FetchTimer.tick()) {
        fetchWeatherData();
    }

    sett_loop();
}