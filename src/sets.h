#pragma once

// Can easly be modified to suit your project
#define PROJECT_NAME "Weather Display"
#define LOCATION_ALTITUDE 112 // meters above sea level (112 for Vilnius)

// LCD settings
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// Pin definitions
#define LED_PIN 2

// Constants for temperature and humidity ranges
#define MAX_TEMP_C 50
#define MIN_TEMP_C 0

#define MAX_HUMIDITY 100
#define MIN_HUMIDITY 0

// Pressure range for BMP280 (atmospheric pressure in hPa)
// BMP280 typical full-scale range: 300..1100 hPa (datasheet)
#define MIN_PRESSURE 300
#define MAX_PRESSURE 1100

#define COMFORT_MIN 20.0
#define COMFORT_MAX 24.0

// Other constants
#define FETCH_INTERVAL 10000  // milliseconds (10 seconds - weather doesn't change that fast)
#define FORECAST_INTERVAL 1800000  // milliseconds (30 minutes for forecast updates)
#define SEALEVELPRESSURE_HPA (1013.25)

// Open-Meteo API configuration
#define OPENMETEO_API_URL "https://api.open-meteo.com/v1/forecast"
#define OPENMETEO_LATITUDE 54.7868   // Your location coordinates
#define OPENMETEO_LONGITUDE 25.3621  // Your location coordinates
#define OPENMETEO_TIMEZONE "Europe/Vilnius"  // Timezone for forecast times

// Dont touch below this line
#define MDNS_ADDRESS "weather-display"

#define SENSOR_STANTION_MDNS "weather-station"
#define SENSOR_STANTION_API_PORT 81
#define SENSOR_STANTION_API_PATH "/weather"

extern float tempC;
extern float humidity;
extern float pressure;

#include <GyverDBFile.h>
#include <SettingsGyver.h>

extern GyverDBFile db;
extern SettingsGyver sett;

void sett_begin();
void sett_loop();

DB_KEYS(
    kk,
    wifi_ssid,
    wifi_pass,
    close_ap,
    switch_state,
    weather_display_state,
    cloth_recommendation_state,
    hourly_forecast_state,
    daily_forecast_state
);