#ifndef FETCH_WEATHER_DATA_H
#define FETCH_WEATHER_DATA_H

#include <Arduino.h>

extern bool weatherDataValid;
extern bool weatherApiReachable;
extern uint32_t lastWeatherUpdateMs;

// Start background task for fetching weather data
void startWeatherFetchTask();

#endif