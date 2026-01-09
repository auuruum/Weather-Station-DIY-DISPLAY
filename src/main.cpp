#include <Arduino.h>

#include "sets.h"

#include <Wire.h>

#include <GTimer.h>
#include <LiquidCrystal_I2C.h>

#include <fetchWeatherData.h>
#include <fetchForecastData.h>

GTimer<millis> DisplayRotateTimer(5000, true); // Rotate display every 5 seconds

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

bool weatherDataValid = false;  // Track if weather data was successfully fetched

enum DisplayState {
    DISPLAY_WEATHER,
    DISPLAY_RECOMMENDATION,
    DISPLAY_HOURLY_FORECAST,
    DISPLAY_DAILY_FORECAST
};

DisplayState currentDisplay = DISPLAY_WEATHER;
int forecastDisplayIndex = 0;  // For cycling through forecasts

void initializeLCD() {
    lcd.init();          // initialize LCD
    lcd.backlight();     // turn on backlight
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Weather Display");

    lcd.setCursor(0, 1);
    lcd.print("By aurum");
}

void showWeatherDataError() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weather Data");
    lcd.setCursor(0, 1);
    lcd.print("Not Available");
    lcd.setCursor(0, 2);
    lcd.print("Check sensor...");
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

void showClothRecommendation() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp Comfort:");
    lcd.setCursor(0, 1);
    if (tempC < COMFORT_MIN) {
        lcd.print("It's cold! Wear");
        lcd.setCursor(0, 2);
        lcd.print("warm clothes.");
    } else if (tempC > COMFORT_MAX) {
        lcd.print("It's hot! Wear");
        lcd.setCursor(0, 2);
        lcd.print("light clothes.");
    } else {
        lcd.print("Comfortable!");
        lcd.setCursor(0, 2);
        lcd.print("Enjoy your day!");
    }
}

// Convert WMO weather code to readable text
String getWeatherDescription(int code) {
    if (code == 0) return "Clear";
    if (code == 1) return "MostClear";
    if (code == 2) return "PartCloud";
    if (code == 3) return "Overcast";
    if (code == 45 || code == 48) return "Fog";
    if (code >= 51 && code <= 55) return "Drizzle";
    if (code >= 61 && code <= 65) return "Rain";
    if (code >= 71 && code <= 75) return "Snow";
    if (code == 77) return "SnowGrain";
    if (code >= 80 && code <= 82) return "Showers";
    if (code >= 85 && code <= 86) return "SnowShow";
    if (code == 95) return "Thunder";
    if (code == 96 || code == 99) return "ThunHail";
    return "Unknown";
}

void showHourlyForecast() {
    lcd.clear();
    if (hourlyForecastCount == 0) {
        lcd.setCursor(0, 0);
        lcd.print("No forecast data");
        return;
    }
    
    // Show 3 hours at a time, cycle through them
    int startIdx = forecastDisplayIndex % hourlyForecastCount;
    
    lcd.setCursor(0, 0);
    String timeStr = hourlyForecasts[startIdx].time;
    // Extract hour from ISO timestamp (e.g., "2026-01-08T14:00")
    int hourStart = timeStr.indexOf('T') + 1;
    lcd.print(timeStr.substring(hourStart, hourStart + 5));
    lcd.print(" ");
    lcd.print(hourlyForecasts[startIdx].temperature, 1);
    lcd.print("C");
    
    lcd.setCursor(0, 1);
    lcd.print(getWeatherDescription(hourlyForecasts[startIdx].weatherCode));
}

void showDailyForecast() {
    lcd.clear();
    if (dailyForecastCount == 0) {
        lcd.setCursor(0, 0);
        lcd.print("No forecast data");
        return;
    }
    
    // Show 1 day at a time, cycle through them
    int dayIdx = forecastDisplayIndex % dailyForecastCount;
    
    lcd.setCursor(0, 0);
    String dateStr = dailyForecasts[dayIdx].date;
    lcd.print(dateStr.substring(5));  // Show MM-DD
    lcd.print(" ");
    lcd.print(dailyForecasts[dayIdx].tempMin, 0);
    lcd.print("|");
    lcd.print(dailyForecasts[dayIdx].tempMax, 0);
    lcd.print("C");
    
    lcd.setCursor(0, 1);
    lcd.print(getWeatherDescription(dailyForecasts[dayIdx].weatherCode));
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
    
    // Start async weather fetch task
    startWeatherFetchTask();
    
    // Start async forecast fetch task
    startForecastFetchTask();
}

void loop() {
    // Weather data is now fetched in background task
    // Forecast data is now fetched in background task

    // Rotate display asynchronously
    if (DisplayRotateTimer.tick()) {
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