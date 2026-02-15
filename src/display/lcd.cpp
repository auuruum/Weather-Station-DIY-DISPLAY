#include "lcd.h"
#include <Arduino.h>

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

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
    lcd.init();          // initialize LCD
    lcd.backlight();     // turn on backlight
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Weather Display");

    lcd.setCursor(0, 1);
    lcd.print("By A/N");
}

void showWeatherDataError() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weather Data");
    lcd.setCursor(0, 1);
    lcd.print("Not Available");
    lcd.setCursor(0, 2);
    lcd.print("Check sensor...");
}

void showNoDisplayEnabledError() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enable displays");
    lcd.setCursor(0, 1);
    lcd.print("in settings!");
}

void showWeatherOnLCD() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(tempC);
    lcd.print("C H:");
    lcd.print(humidity);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.print("P:");
    lcd.print(pressure);

    lcd.print(" C:");
    lcd.print(cast);
}

void showClothRecommendation() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp Comfort:");
    lcd.setCursor(0, 1);
    if (tempC < COMFORT_MIN) {
        lcd.print("It's cold! Wear");
        lcd.setCursor(0, 2);
        lcd.print("warm clothes.");
    } else if (tempC > COMFORT_MAX) {
        lcd.print("It's hot! Wear");
        lcd.setCursor(0, 2);
        lcd.print("light clothes.");
    } else {
        lcd.print("Comfortable!");
        lcd.setCursor(0, 2);
        lcd.print("Enjoy your day!");
    }
}

void showHourlyForecast() {
    lcd.clear();
    if (hourlyForecastCount == 0) {
        lcd.setCursor(0, 0);
        lcd.print("No forecast data");
        return;
    }
    
    // Show 3 hours at a time, cycle through them
    int startIdx = forecastDisplayIndex % hourlyForecastCount;
    
    lcd.setCursor(0, 0);
    String timeStr = hourlyForecasts[startIdx].time;
    // Extract hour from ISO timestamp (e.g., "2026-01-08T14:00")
    int hourStart = timeStr.indexOf('T') + 1;
    lcd.print(timeStr.substring(hourStart, hourStart + 5));
    lcd.print(" ");
    lcd.print(hourlyForecasts[startIdx].temperature, 1);
    lcd.print("C");
    
    lcd.setCursor(0, 1);
    lcd.print(getWeatherDescription(hourlyForecasts[startIdx].weatherCode));
}

void showDailyForecast() {
    lcd.clear();
    if (dailyForecastCount <= 1) {
        lcd.setCursor(0, 0);
        lcd.print("No forecast data");
        return;
    }
    
    // Skip today (index 0), show next 3 days (indices 1, 2, 3)
    int dayIdx = (forecastDisplayIndex % min(3, dailyForecastCount - 1)) + 1;
    
    lcd.setCursor(0, 0);
    String dateStr = dailyForecasts[dayIdx].date;
    lcd.print(dateStr.substring(5));  // Show MM-DD
    Serial.println(dateStr.substring(5)); 
    lcd.print(" ");
    lcd.print(dailyForecasts[dayIdx].tempMin, 0);
    Serial.println(dailyForecasts[dayIdx].tempMin, 0);
    lcd.print("|");
    lcd.print(dailyForecasts[dayIdx].tempMax, 0);
    Serial.println(dailyForecasts[dayIdx].tempMax, 0);
    lcd.print("C");
    
    lcd.setCursor(0, 1);
    lcd.print(getWeatherDescription(dailyForecasts[dayIdx].weatherCode));
    Serial.println(getWeatherDescription(dailyForecasts[dayIdx].weatherCode));
}