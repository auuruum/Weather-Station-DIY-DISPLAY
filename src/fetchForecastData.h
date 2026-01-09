#ifndef FETCH_FORECAST_DATA_H
#define FETCH_FORECAST_DATA_H

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

// Forecast storage (global variables)
extern HourlyForecast hourlyForecasts[12];  // Next 12 hours
extern DailyForecast dailyForecasts[7];     // Next 7 days
extern int hourlyForecastCount;
extern int dailyForecastCount;

// Start background task for fetching forecast data
void startForecastFetchTask();

#endif
