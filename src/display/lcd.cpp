#include "lcd.h"
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>

Adafruit_ILI9341 tft(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_RST);

static void drawHeader(const String& title, uint16_t color = ILI9341_CYAN) {
    tft.fillScreen(ILI9341_BLACK);
    tft.fillRect(0, 0, TFT_WIDTH, 30, ILI9341_DARKCYAN);
    tft.setTextColor(ILI9341_WHITE, ILI9341_DARKCYAN);
    tft.setTextSize(2);
    tft.setCursor(8, 8);
    tft.print(title);
    tft.drawFastHLine(0, 30, TFT_WIDTH, color);
}

static void drawBodyText(int16_t x, int16_t y, const String& text, uint16_t color = ILI9341_WHITE, uint8_t size = 2) {
    tft.setTextSize(size);
    tft.setTextColor(color, ILI9341_BLACK);
    tft.setCursor(x, y);
    tft.print(text);
}

// Convert WMO weather code to readable text
String getWeatherDescription(int code) {
    if (code == 0) return "Clear";
    if (code == 1) return "MostClear";
    if (code == 2) return "PartCloud";
    if (code == 3) return "Overcast";
    if (code == 45 || code == 48) return "Fog";
    if (code >= 51 && code <= 55) return "Drizzle";
    if (code >= 61 && code <= 65) return "Rain";
    if (code >= 71 && code <= 75) return "Snow";
    if (code == 77) return "SnowGrain";
    if (code >= 80 && code <= 82) return "Showers";
    if (code >= 85 && code <= 86) return "SnowShow";
    if (code == 95) return "Thunder";
    if (code == 96 || code == 99) return "ThunHail";
    return "Unknown";
}

void initializeLCD() {
    SPI.begin(TFT_PIN_SCK, TFT_PIN_MISO, TFT_PIN_MOSI, TFT_PIN_CS);
    tft.begin();
    tft.setRotation(TFT_ROTATION);

    pinMode(TFT_PIN_BL, OUTPUT);
    digitalWrite(TFT_PIN_BL, HIGH);

    drawHeader("Weather Display", ILI9341_GREEN);
    drawBodyText(12, 56, "By A/N", ILI9341_YELLOW, 2);
    drawBodyText(12, 90, "TFT 240x320", ILI9341_LIGHTGREY, 2);
}

void showWeatherDataError() {
    drawHeader("Weather", ILI9341_RED);
    drawBodyText(12, 60, "Weather Data", ILI9341_WHITE, 2);
    drawBodyText(12, 90, "Not Available", ILI9341_RED, 2);
}

void showNoDisplayEnabledError() {
    drawHeader("Display", ILI9341_ORANGE);
    drawBodyText(12, 60, "Enable displays", ILI9341_WHITE, 2);
    drawBodyText(12, 90, "in settings!", ILI9341_ORANGE, 2);
}

void showWeatherOnLCD() {
    drawHeader("Current Weather", ILI9341_CYAN);
    drawBodyText(12, 48, "Temp: " + String(tempC, 1) + " C", ILI9341_WHITE, 3);
    drawBodyText(12, 92, "Humidity: " + String(humidity, 0) + " %", ILI9341_YELLOW, 2);
    drawBodyText(12, 124, "Pressure: " + String(pressure, 0) + " hPa", ILI9341_GREEN, 2);
    drawBodyText(12, 156, "Cast: " + String(cast, 1) + " / 10", ILI9341_MAGENTA, 2);
}

void showClothRecommendation() {
    drawHeader("Clothing", ILI9341_PINK);
    
    // Calculate heat index (feels like temperature with humidity)
    float feelsLike = tempC;
    if (tempC >= 27) {
        // Simplified heat index for hot weather
        feelsLike = tempC + (0.5 * (humidity - 40) / 100 * tempC);
    } else if (tempC <= 10) {
        // Wind chill approximation (assuming moderate wind)
        feelsLike = tempC - (humidity < 50 ? 2 : 0);
    }
    
    // Check for precipitation in next 3 hours
    bool rainExpected = false;
    bool snowExpected = false;
    float totalPrecip = 0;
    if (hourlyForecastCount > 0) {
        for (int i = 0; i < min(3, hourlyForecastCount); i++) {
            totalPrecip += hourlyForecasts[i].precipitation;
            int code = hourlyForecasts[i].weatherCode;
            if (code >= 71 && code <= 77) {  // Snow codes
                snowExpected = true;
            } else if (code >= 51) {  // Rain/drizzle codes
                rainExpected = true;
            }
        }
    }
    
    String condition;
    
    if (snowExpected) {
        condition = "Snow";
    } else if (rainExpected || totalPrecip > 0.5) {
        condition = "Rain";
    } else if (cast <= 2) {
        condition = "Good";
    } else if (cast <= 5) {
        condition = "Fair";
    } else if (cast <= 8) {
        condition = "Poor";
    } else {
        condition = "Storm";
    }

    drawBodyText(12, 48, "Feels like: " + String(feelsLike, 1) + " C", ILI9341_WHITE, 2);
    drawBodyText(12, 80, "Condition: " + condition, ILI9341_CYAN, 2);
    
    String recommendation;
    if (snowExpected) {
        recommendation = "Warm + waterproof";
    } else if (rainExpected || totalPrecip > 0.5) {
        recommendation = "Take umbrella";
    } else if (feelsLike < -10) {
        recommendation = "Heavy winter clothes";
    } else if (feelsLike < 0) {
        recommendation = "Winter coat";
    } else if (feelsLike < 10) {
        recommendation = "Jacket needed";
    } else if (feelsLike < 18) {
        recommendation = "Light layer";
    } else if (feelsLike < 25) {
        recommendation = "T-shirt is OK";
    } else if (feelsLike < 30) {
        recommendation = "Light clothes";
    } else {
        recommendation = "Stay cool";
    }

    drawBodyText(12, 112, "Recommendation:", ILI9341_YELLOW, 2);
    drawBodyText(12, 144, recommendation, ILI9341_WHITE, 2);
}

void showHourlyForecast() {
    drawHeader("Hourly Forecast", ILI9341_BLUE);
    if (hourlyForecastCount == 0) {
        drawBodyText(12, 60, "No forecast data", ILI9341_RED, 2);
        return;
    }
    
    // Show 3 hours at a time, cycle through them
    int startIdx = forecastDisplayIndex % hourlyForecastCount;
    
    String timeStr = hourlyForecasts[startIdx].time;
    // Extract hour from ISO timestamp (e.g., "2026-01-08T14:00")
    int hourStart = timeStr.indexOf('T') + 1;
    String timeLabel = timeStr.substring(hourStart, hourStart + 5);

    drawBodyText(12, 52, "Time: " + timeLabel, ILI9341_WHITE, 2);
    drawBodyText(12, 84, "Temp: " + String(hourlyForecasts[startIdx].temperature, 1) + " C", ILI9341_YELLOW, 2);
    drawBodyText(12, 116, "Rain: " + String(hourlyForecasts[startIdx].precipitation, 1) + " mm", ILI9341_CYAN, 2);
    drawBodyText(12, 148, getWeatherDescription(hourlyForecasts[startIdx].weatherCode), ILI9341_GREEN, 2);
}

void showDailyForecast() {
    drawHeader("Daily Forecast", ILI9341_NAVY);
    if (dailyForecastCount <= 1) {
        drawBodyText(12, 60, "No forecast data", ILI9341_RED, 2);
        return;
    }
    
    // Skip today (index 0), show next 3 days (indices 1, 2, 3)
    int dayIdx = (forecastDisplayIndex % min(3, dailyForecastCount - 1)) + 1;
    
    String dateStr = dailyForecasts[dayIdx].date;
    String shortDate = dateStr.substring(5);
    Serial.println(dateStr.substring(5)); 
    Serial.println(dailyForecasts[dayIdx].tempMin, 0);
    Serial.println(dailyForecasts[dayIdx].tempMax, 0);
    Serial.println(getWeatherDescription(dailyForecasts[dayIdx].weatherCode));

    drawBodyText(12, 52, "Date: " + shortDate, ILI9341_WHITE, 2);
    drawBodyText(12, 84, "Min/Max: " + String(dailyForecasts[dayIdx].tempMin, 0) + "|" + String(dailyForecasts[dayIdx].tempMax, 0) + " C", ILI9341_YELLOW, 2);
    drawBodyText(12, 116, "Rain: " + String(dailyForecasts[dayIdx].precipitationSum, 1) + " mm", ILI9341_CYAN, 2);
    drawBodyText(12, 148, getWeatherDescription(dailyForecasts[dayIdx].weatherCode), ILI9341_GREEN, 2);
}