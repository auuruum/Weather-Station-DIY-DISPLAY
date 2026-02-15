#pragma once

#include "types.h"

// Forecast storage (global variables)
extern HourlyForecast hourlyForecasts[12];  // Next 12 hours
extern DailyForecast dailyForecasts[7];     // Next 7 days
extern int hourlyForecastCount;
extern int dailyForecastCount;

// Start background task for fetching forecast data
void startForecastClient();
