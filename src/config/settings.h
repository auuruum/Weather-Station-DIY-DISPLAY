#pragma once

#include "constants.h"
#include <GyverDBFile.h>
#include <SettingsGyver.h>

// Global weather data variables
extern float tempC;
extern float humidity;
extern float pressure;
extern float cast;
extern bool weatherDataValid;

// Database and settings objects
extern GyverDBFile db;
extern SettingsGyver sett;

// Database keys
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

// Functions
void settings_begin();
void settings_loop();
