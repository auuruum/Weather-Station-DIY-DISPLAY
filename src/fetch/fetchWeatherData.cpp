#include "fetchWeatherData.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include "sets.h"

bool weatherApiReachable = false;
uint32_t lastWeatherUpdateMs = 0;

// Task handle for async fetch
TaskHandle_t fetchTaskHandle = NULL;

// Task function that runs in background
void fetchWeatherTask(void * parameter) {
    while(true) {
        if (!WiFi.isConnected()) {
            weatherApiReachable = false;
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }

        uint32_t nextDelayMs = FETCH_INTERVAL;
        HTTPClient http;
        
        String url =
            String("http://") +
            SENSOR_STANTION_MDNS +
            ".local:" +
            String(SENSOR_STANTION_API_PORT) +
            SENSOR_STANTION_API_PATH;

        http.begin(url);
        http.setTimeout(5000);

        int httpCode = http.GET();
        
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                tempC = doc["temp"].as<float>();
                humidity = doc["humidity"].as<float>();
                pressure = doc["pressure"].as<float>();
                cast = doc["cast"].as<float>();
                weatherDataValid = true;
                weatherApiReachable = true;
                lastWeatherUpdateMs = millis();
                Serial.println("Weather data updated");
            } else {
                Serial.print("JSON parse error: ");
                Serial.println(error.c_str());
                weatherDataValid = lastWeatherUpdateMs != 0;
                weatherApiReachable = false;
                nextDelayMs = 2000;
            }
        } else {
            Serial.print("HTTP request failed, code: ");
            Serial.println(httpCode);
            weatherDataValid = lastWeatherUpdateMs != 0;
            weatherApiReachable = false;
            nextDelayMs = 2000;
        }

        http.end();
        
        // Retry quickly after failures, otherwise use normal refresh interval.
        vTaskDelay(nextDelayMs / portTICK_PERIOD_MS);
    }
}

// Start the background fetch task
void startWeatherFetchTask() {
    xTaskCreatePinnedToCore(
        fetchWeatherTask,   // Task function
        "WeatherFetch",     // Name
        8192,               // Stack size
        NULL,               // Parameters
        1,                  // Priority
        &fetchTaskHandle,   // Task handle
        0                   // Core (0 = background core)
    );
}
