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
    
    // Line 1: Show feels-like temperature (16 chars max)
    lcd.setCursor(0, 0);
    lcd.print("Feel:");
    lcd.print(feelsLike, 1);
    lcd.print("C ");
    
    // Add weather condition based on cast (0-10 forecast scale)
    if (snowExpected) {
        lcd.print("Snow");
    } else if (rainExpected || totalPrecip > 0.5) {
        lcd.print("Rain");
    } else if (cast <= 2) {
        lcd.print("Good");
    } else if (cast <= 5) {
        lcd.print("Fair");
    } else if (cast <= 8) {
        lcd.print("Poor");
    } else {
        lcd.print("Storm");
    }
    
    // Line 2: Clothing recommendation (16 chars max)
    lcd.setCursor(0, 1);
    if (snowExpected) {
        lcd.print("Warm+Waterproof!");
    } else if (rainExpected || totalPrecip > 0.5) {
        lcd.print("Take umbrella!  ");
    } else if (feelsLike < -10) {
        lcd.print("Heavy winter!  ");
    } else if (feelsLike < 0) {
        lcd.print("Winter coat!   ");
    } else if (feelsLike < 10) {
        lcd.print("Jacket needed  ");
    } else if (feelsLike < 18) {
        lcd.print("Light layer    ");
    } else if (feelsLike < 25) {
        lcd.print("T-shirt is OK  ");
    } else if (feelsLike < 30) {
        lcd.print("Light clothes  ");
    } else {
        lcd.print("Stay cool!     ");
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