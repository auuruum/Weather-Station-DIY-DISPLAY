#pragma once

// Project Information
#define PROJECT_NAME "Weather Display"
#define LOCATION_ALTITUDE 112 // meters above sea level (112 for Vilnius)

// LCD Configuration
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// Pin Definitions
#define LED_PIN 2

// Temperature and Humidity Ranges
#define MAX_TEMP_C 50
#define MIN_TEMP_C 0
#define MAX_HUMIDITY 100
#define MIN_HUMIDITY 0

// Pressure Range (atmospheric pressure in hPa)
// BMP280 typical full-scale range: 300..1100 hPa (datasheet)
#define MIN_PRESSURE 300
#define MAX_PRESSURE 1100

// Comfort Zone
#define COMFORT_MIN 20.0
#define COMFORT_MAX 24.0

// Cast Range
#define MIN_CAST 0
#define MAX_CAST 10

// Update Intervals
#define FETCH_INTERVAL 10000        // milliseconds (10 seconds)
#define FORECAST_INTERVAL 1800000   // milliseconds (30 minutes)
#define SEALEVELPRESSURE_HPA (1013.25)

// Open-Meteo API Configuration
#define OPENMETEO_API_URL "https://api.open-meteo.com/v1/forecast"
#define OPENMETEO_LATITUDE 54.7868   // Your location coordinates
#define OPENMETEO_LONGITUDE 25.3621  // Your location coordinates
#define OPENMETEO_TIMEZONE "Europe/Vilnius"

// Network Configuration
#define MDNS_ADDRESS "weather-display"
#define SENSOR_STANTION_MDNS "weather-station"
#define SENSOR_STANTION_API_PORT 81
#define SENSOR_STANTION_API_PATH "/weather"
