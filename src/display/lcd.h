#pragma once

#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include "../sets.h"
#include "../fetch/fetchForecastData.h"

extern Adafruit_ILI9341 tft;
extern int forecastDisplayIndex;

// Utility functions
String getWeatherDescription(int code);

// LCD display functions
void initializeLCD();
void showWeatherDataError();
void showNoDisplayEnabledError();
void showWeatherOnLCD();
void showClothRecommendation();
void showHourlyForecast();
void showDailyForecast();