#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "../sets.h"
#include "../fetch/fetchForecastData.h"

extern LiquidCrystal_I2C lcd;
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