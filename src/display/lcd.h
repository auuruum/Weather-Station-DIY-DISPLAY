#pragma once

#include <LiquidCrystal_I2C.h>

// LCD display functions
void initLCD();
void showWeatherDataError();
void showNoDisplayEnabledError();
void showWeatherOnLCD();
void showClothRecommendation();
void showHourlyForecast(int displayIndex);
void showDailyForecast(int displayIndex);
bool areAllDisplaysDisabled();
String getWeatherDescription(int code);

// External LCD object
extern LiquidCrystal_I2C lcd;
