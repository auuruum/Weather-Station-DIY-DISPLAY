#include "fetchWeatherData.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "sets.h"

// Task handle for async fetch
TaskHandle_t fetchTaskHandle = NULL;

// Task function that runs in background
void fetchWeatherTask(void * parameter) {
    while(true) {
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
            
            StaticJsonDocument<512> doc;
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                tempC = doc["temp"].as<float>();
                humidity = doc["humidity"].as<float>();
                pressure = doc["pressure"].as<float>();
                weatherDataValid = true;
                Serial.println("Weather data updated");
            } else {
                Serial.print("JSON parse error: ");
                Serial.println(error.c_str());
                weatherDataValid = false;
            }
        } else {
            Serial.print("HTTP request failed, code: ");
            Serial.println(httpCode);
            weatherDataValid = false;
        }

        http.end();
        
        // Wait before next fetch
        vTaskDelay(FETCH_INTERVAL / portTICK_PERIOD_MS);
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
