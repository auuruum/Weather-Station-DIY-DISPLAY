#pragma once

#include <Arduino.h>

// Forecast data structures
struct HourlyForecast {
    String time;           // Hour timestamp
    float temperature;     // Temperature in °C
    float precipitation;   // Precipitation in mm
    int weatherCode;       // WMO weather code
};

struct DailyForecast {
    String date;           // Date
    float tempMax;         // Max temperature in °C
    float tempMin;         // Min temperature in °C
    float precipitationSum;// Total precipitation in mm
    int weatherCode;       // WMO weather code
};
