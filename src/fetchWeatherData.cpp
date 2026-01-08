#include "fetchWeatherData.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "sets.h"

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
