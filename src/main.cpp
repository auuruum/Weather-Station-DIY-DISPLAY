#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "sets.h"

#include <GTimer.h>

#include "fetch/fetchWeatherData.h"
#include "fetch/fetchForecastData.h"
#include "display/lcd.h"

GTimer<millis> DisplayRotateTimer(DISPLAY_ROTATE_INTERVAL, true); // Rotate display every 5 seconds

bool weatherDataValid = false;  // Track if weather data was successfully fetched

enum DisplayState {
    DISPLAY_WEATHER,
    DISPLAY_RECOMMENDATION,
    DISPLAY_HOURLY_FORECAST,
    DISPLAY_DAILY_FORECAST
};

DisplayState currentDisplay = DISPLAY_WEATHER;
int forecastDisplayIndex = 0;  // For cycling through forecasts

uint32_t lastNtpSyncMs = 0;

void syncTimeWithNtp(bool force = false) {
    if (!WiFi.isConnected()) return;

    uint32_t nowMs = millis();
    if (!force && lastNtpSyncMs != 0 && (nowMs - lastNtpSyncMs < NTP_SYNC_INTERVAL_MS)) return;

    configTzTime(NTP_TZ_INFO, NTP_SERVER_1, NTP_SERVER_2);

    struct tm tmInfo;
    if (getLocalTime(&tmInfo, 3000)) {
        Serial.printf("NTP sync OK: %02d:%02d:%02d\n", tmInfo.tm_hour, tmInfo.tm_min, tmInfo.tm_sec);
    } else {
        Serial.println("NTP sync failed");
    }

    lastNtpSyncMs = nowMs;
}

bool areAllDisplaysDisabled() {
    return !db[kk::weather_display_state] && 
           !db[kk::cloth_recommendation_state] && 
           !db[kk::hourly_forecast_state] && 
           !db[kk::daily_forecast_state];
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    initializeLCD();

    sett_begin();

    Serial.println(db[kk::wifi_ssid]);

    Serial.print("SETUP | LED is now ");
    Serial.println(db[kk::switch_state] ? "ON" : "OFF");

    // Start async weather fetch task
    startWeatherFetchTask();
    
    // Start async forecast fetch task
    startForecastFetchTask();

    // Initial NTP sync (if Wi-Fi is already connected)
    syncTimeWithNtp(true);
}

void loop() {
    // Re-sync NTP every 6 hours while connected
    syncTimeWithNtp();

    // Weather data is now fetched in background task
    // Forecast data is now fetched in background task

    // Rotate display asynchronously
    if (DisplayRotateTimer.tick()) {
        // Check if all displays are disabled
        if (areAllDisplaysDisabled()) {
            showNoDisplayEnabledError();
            return;
        }
        
        // Find next enabled display state
        bool stateChanged = false;
        int attempts = 0;
        
        while (!stateChanged && attempts < 10) {
            attempts++;
            
            switch (currentDisplay) {
                case DISPLAY_WEATHER:
                    if (db[kk::weather_display_state]) {
                        if (weatherDataValid) {
                            showWeatherOnLCD();
                        } else {
                            showWeatherDataError();
                        }
                        stateChanged = true;
                    }
                    currentDisplay = DISPLAY_RECOMMENDATION;
                    break;
                
                case DISPLAY_RECOMMENDATION:
                    if (db[kk::cloth_recommendation_state]) {
                        if (weatherDataValid) {
                            showClothRecommendation();
                        } else {
                            showWeatherDataError();
                        }
                        stateChanged = true;
                    }
                    currentDisplay = DISPLAY_HOURLY_FORECAST;
                    forecastDisplayIndex = 0;
                    break;
                
                case DISPLAY_HOURLY_FORECAST:
                    if (db[kk::hourly_forecast_state]) {
                        showHourlyForecast();
                        stateChanged = true;
                        forecastDisplayIndex++;
                        if (forecastDisplayIndex >= 3) {  // Show 3 hourly forecasts
                            currentDisplay = DISPLAY_DAILY_FORECAST;
                            forecastDisplayIndex = 0;
                        }
                    } else {
                        currentDisplay = DISPLAY_DAILY_FORECAST;
                        forecastDisplayIndex = 0;
                    }
                    break;
                
                case DISPLAY_DAILY_FORECAST:
                    if (db[kk::daily_forecast_state]) {
                        showDailyForecast();
                        stateChanged = true;
                        forecastDisplayIndex++;
                        if (forecastDisplayIndex >= 3) {  // Show 3 daily forecasts
                            currentDisplay = DISPLAY_WEATHER;
                            forecastDisplayIndex = 0;
                        }
                    } else {
                        currentDisplay = DISPLAY_WEATHER;
                        forecastDisplayIndex = 0;
                    }
                    break;
            }
        }
    }

    sett_loop();
}
