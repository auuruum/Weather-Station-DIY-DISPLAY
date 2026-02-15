#include <Arduino.h>
#include <Wire.h>
#include <GTimer.h>

// Project modules
#include "config/settings.h"
#include "display/lcd.h"
#include "weather/client.h"
#include "weather/forecast.h"

GTimer<millis> DisplayRotateTimer(5000, true); // Rotate display every 5 seconds

bool weatherDataValid = false;  // Track if weather data was successfully fetched

enum DisplayState {
    DISPLAY_WEATHER,
    DISPLAY_RECOMMENDATION,
    DISPLAY_HOURLY_FORECAST,
    DISPLAY_DAILY_FORECAST
};

DisplayState currentDisplay = DISPLAY_WEATHER;
int forecastDisplayIndex = 0;  // For cycling through forecasts

void setup() {
    Serial.begin(115200);
    Serial.println();
    Wire.begin(21, 22);

    initLCD();

    settings_begin();

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
    
    // Start async weather clients
    startWeatherClient();
    startForecastClient();
}

void loop() {
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
                        showHourlyForecast(forecastDisplayIndex);
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
                        showDailyForecast(forecastDisplayIndex);
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

    settings_loop();
}