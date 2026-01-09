#include "fetchForecastData.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "sets.h"

// Global forecast storage
HourlyForecast hourlyForecasts[12];
DailyForecast dailyForecasts[7];
int hourlyForecastCount = 0;
int dailyForecastCount = 0;

// Task handle for async fetch
TaskHandle_t forecastTaskHandle = NULL;

// Task function that runs in background
void fetchForecastTask(void * parameter) {
    while(true) {
        HTTPClient http;
        
        String url = String(OPENMETEO_API_URL) + 
                     "?latitude=" + String(OPENMETEO_LATITUDE, 4) +
                     "&longitude=" + String(OPENMETEO_LONGITUDE, 4) +
                     "&current=temperature_2m" +
                     "&hourly=temperature_2m,precipitation,weather_code" +
                     "&daily=weather_code,temperature_2m_max,temperature_2m_min,precipitation_sum" +
                     "&timezone=" + String(OPENMETEO_TIMEZONE) +
                     "&forecast_days=4";  // Request 4 days to get 3 future days (skipping today)
        
        Serial.print("Fetching forecast from: ");
        Serial.println(url);
        
        http.begin(url);
        http.setTimeout(10000);
        int httpCode = http.GET();
        
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                String currentTime = doc["current"]["time"] | "";
                
                JsonArray hourlyTime = doc["hourly"]["time"];
                JsonArray hourlyTemp = doc["hourly"]["temperature_2m"];
                JsonArray hourlyPrecip = doc["hourly"]["precipitation"];
                JsonArray hourlyWeather = doc["hourly"]["weather_code"];
                
                int startIndex = 0;
                if (currentTime.length() > 0) {
                    for (int i = 0; i < hourlyTime.size(); i++) {
                        String timeStr = hourlyTime[i].as<String>();
                        if (timeStr >= currentTime) {
                            startIndex = i;
                            break;
                        }
                    }
                }
                
                Serial.print("Current time: ");
                Serial.println(currentTime);
                Serial.print("Starting from index: ");
                Serial.println(startIndex);
                
                hourlyForecastCount = 0;
                for (int i = startIndex; i < hourlyTime.size() && hourlyForecastCount < 12; i++) {
                    hourlyForecasts[hourlyForecastCount].time = hourlyTime[i].as<String>();
                    hourlyForecasts[hourlyForecastCount].temperature = hourlyTemp[i].as<float>();
                    hourlyForecasts[hourlyForecastCount].precipitation = hourlyPrecip[i].as<float>();
                    hourlyForecasts[hourlyForecastCount].weatherCode = hourlyWeather[i].as<int>();
                    hourlyForecastCount++;
                }
                
                JsonArray dailyDate = doc["daily"]["time"];
                JsonArray dailyTempMax = doc["daily"]["temperature_2m_max"];
                JsonArray dailyTempMin = doc["daily"]["temperature_2m_min"];
                JsonArray dailyPrecipSum = doc["daily"]["precipitation_sum"];
                JsonArray dailyWeather = doc["daily"]["weather_code"];
                
                dailyForecastCount = min(7, (int)dailyDate.size());
                for (int i = 0; i < dailyForecastCount; i++) {
                    dailyForecasts[i].date = dailyDate[i].as<String>();
                    dailyForecasts[i].tempMax = dailyTempMax[i].as<float>();
                    dailyForecasts[i].tempMin = dailyTempMin[i].as<float>();
                    dailyForecasts[i].precipitationSum = dailyPrecipSum[i].as<float>();
                    dailyForecasts[i].weatherCode = dailyWeather[i].as<int>();
                }
                
                Serial.println("Forecast data parsed successfully");
                Serial.print("Hourly forecasts: ");
                Serial.println(hourlyForecastCount);
                Serial.print("Daily forecasts: ");
                Serial.println(dailyForecastCount);
            } else {
                Serial.print("JSON parse error: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.print("Forecast HTTP request failed, code: ");
            Serial.println(httpCode);
        }
        
        http.end();
        
        // Wait before next fetch (30 minutes)
        vTaskDelay(FORECAST_INTERVAL / portTICK_PERIOD_MS);
    }
}

// Start the background fetch task
void startForecastFetchTask() {
    xTaskCreatePinnedToCore(
        fetchForecastTask,   // Task function
        "ForecastFetch",     // Name
        16384,               // Stack size (larger for JSON parsing)
        NULL,                // Parameters
        1,                   // Priority
        &forecastTaskHandle, // Task handle
        0                    // Core (0 = background core)
    );
}
