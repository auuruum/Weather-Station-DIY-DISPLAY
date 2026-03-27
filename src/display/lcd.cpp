#include "lcd.h"
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>

#include "../fetch/fetchWeatherData.h"

Adafruit_ILI9341 tft(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_RST);

namespace {

struct Palette {
    uint16_t bg;
    uint16_t panel;
    uint16_t text;
    uint16_t accent;
};

int16_t sw() {
    return tft.width();
}

int16_t sh() {
    return tft.height();
}

Palette paletteFromCode(int code) {
    if (code == 0 || code == 1) return {ILI9341_NAVY, ILI9341_BLUE, ILI9341_WHITE, ILI9341_YELLOW};
    if (code >= 71 && code <= 77) return {ILI9341_BLACK, ILI9341_CYAN, ILI9341_WHITE, ILI9341_LIGHTGREY};
    if (code == 95 || code == 96 || code == 99) return {ILI9341_BLACK, ILI9341_MAROON, ILI9341_WHITE, ILI9341_ORANGE};
    if (code >= 51) return {ILI9341_NAVY, ILI9341_BLUE, ILI9341_WHITE, ILI9341_CYAN};
    return {ILI9341_DARKGREY, ILI9341_LIGHTGREY, ILI9341_WHITE, ILI9341_CYAN};
}

bool isRainCode(int code) {
    return (code >= 51 && code <= 67) || (code >= 80 && code <= 82) || code == 95 || code == 96 || code == 99;
}

String ageText(uint32_t lastMs) {
    if (lastMs == 0) return "--";
    uint32_t sec = (millis() - lastMs) / 1000;
    if (sec < 60) return String(sec) + "s";
    return String(sec / 60) + "m";
}

void fillBody(const Palette& p) {
    tft.fillRect(0, 46, sw(), sh() - 46, p.bg);
}

void drawText(int16_t x, int16_t y, const String& text, const Palette& p, uint16_t color, uint8_t size = 2) {
    tft.setCursor(x, y);
    tft.setTextSize(size);
    tft.setTextColor(color, p.bg);
    tft.print(text);
}

void drawSun(int16_t cx, int16_t cy, uint16_t c) {
    tft.fillCircle(cx, cy, 12, c);
    for (int i = 0; i < 8; i++) {
        float a = i * 0.785398f;
        int16_t x1 = cx + (int16_t)(16 * cosf(a));
        int16_t y1 = cy + (int16_t)(16 * sinf(a));
        int16_t x2 = cx + (int16_t)(22 * cosf(a));
        int16_t y2 = cy + (int16_t)(22 * sinf(a));
        tft.drawLine(x1, y1, x2, y2, c);
    }
}

void drawCloud(int16_t cx, int16_t cy, uint16_t c) {
    tft.fillCircle(cx - 12, cy + 2, 9, c);
    tft.fillCircle(cx, cy - 4, 11, c);
    tft.fillCircle(cx + 12, cy + 2, 9, c);
    tft.fillRoundRect(cx - 24, cy + 6, 48, 14, 6, c);
}

void drawRain(int16_t cx, int16_t cy, uint16_t cloudC, uint16_t rainC) {
    drawCloud(cx, cy, cloudC);
    for (int i = -14; i <= 14; i += 7) {
        tft.drawLine(cx + i, cy + 22, cx + i - 2, cy + 30, rainC);
    }
}

void drawSnow(int16_t cx, int16_t cy, uint16_t cloudC, uint16_t snowC) {
    drawCloud(cx, cy, cloudC);
    for (int i = -10; i <= 10; i += 10) {
        int16_t sx = cx + i;
        int16_t sy = cy + 27;
        tft.drawLine(sx - 3, sy, sx + 3, sy, snowC);
        tft.drawLine(sx, sy - 3, sx, sy + 3, snowC);
        tft.drawLine(sx - 2, sy - 2, sx + 2, sy + 2, snowC);
        tft.drawLine(sx - 2, sy + 2, sx + 2, sy - 2, snowC);
    }
}

void drawWeatherIcon(int16_t x, int16_t y, int code, const Palette& p) {
    if (code == 0 || code == 1) {
        drawSun(x, y, p.accent);
        return;
    }
    if (code >= 71 && code <= 77) {
        drawSnow(x, y, p.text, p.accent);
        return;
    }
    if (code >= 51) {
        drawRain(x, y, p.text, p.accent);
        return;
    }
    drawCloud(x, y, p.text);
}

void drawStatusAndTitle(const String& title, const Palette& p) {
    tft.fillRect(0, 0, sw(), 18, p.panel);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE, p.panel);

    String wifi = WiFi.isConnected() ? "W:OK" : "W:--";
    String upd = ageText(lastWeatherUpdateMs) + "/" + ageText(lastForecastUpdateMs);
    String api = (weatherApiReachable && forecastApiReachable) ? "API:OK" : "API:ER";

    tft.setCursor(2, 6);
    tft.print(wifi);
    tft.setCursor(sw() / 2 - 20, 6);
    tft.print(upd);
    tft.setCursor(sw() - 38, 6);
    tft.print(api);

    tft.fillRect(0, 18, sw(), 28, p.panel);
    tft.drawFastHLine(0, 46, sw(), p.accent);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE, p.panel);
    tft.setCursor(8, 26);
    tft.print(title);
}

int nextRainHours() {
    for (int i = 0; i < hourlyForecastCount; i++) {
        if (hourlyForecasts[i].precipitation >= 0.2f || isRainCode(hourlyForecasts[i].weatherCode)) {
            return i;
        }
    }
    return -1;
}

void lowHigh12h(float& low, float& high) {
    int c = min(12, hourlyForecastCount);
    if (c == 0) {
        low = 0;
        high = 0;
        return;
    }

    low = 999;
    high = -999;
    for (int i = 0; i < c; i++) {
        low = min(low, hourlyForecasts[i].temperature);
        high = max(high, hourlyForecasts[i].temperature);
    }
}

int umbrellaConfidencePct() {
    int c = min(6, hourlyForecastCount);
    if (c == 0) return 0;

    float score = 0;
    for (int i = 0; i < c; i++) {
        score += min(18.0f, hourlyForecasts[i].precipitation * 18.0f);
        if (isRainCode(hourlyForecasts[i].weatherCode)) score += 8.0f;
    }
    return (int)min(100.0f, score);
}

int heatRiskPct(float feelsLike) {
    if (feelsLike < 22) return 5;
    if (feelsLike < 27) return 20;
    if (feelsLike < 31) return 45;
    if (feelsLike < 35) return 70;
    return 90;
}

void drawMinMaxBar(int16_t x, int16_t y, int16_t w, int16_t h, float minT, float maxT, const Palette& p) {
    float minBound = -20.0f;
    float maxBound = 45.0f;

    float nMin = max(minBound, min(maxBound, minT));
    float nMax = max(minBound, min(maxBound, maxT));

    int16_t left = (int16_t)(((nMin - minBound) / (maxBound - minBound)) * w);
    int16_t right = (int16_t)(((nMax - minBound) / (maxBound - minBound)) * w);

    tft.drawRect(x, y, w, h, p.text);
    tft.fillRect(x + left, y + 2, max(2, right - left), h - 4, p.accent);
}

void drawBasePage(const String& title, const Palette& p) {
    tft.fillScreen(p.bg);
    drawStatusAndTitle(title, p);
}

}  // namespace

// Convert WMO weather code to readable text
String getWeatherDescription(int code) {
    if (code == 0) return "Clear";
    if (code == 1) return "Mostly Clear";
    if (code == 2) return "Partly Cloudy";
    if (code == 3) return "Overcast";
    if (code == 45 || code == 48) return "Fog";
    if (code >= 51 && code <= 55) return "Drizzle";
    if (code >= 61 && code <= 65) return "Rain";
    if (code >= 71 && code <= 75) return "Snow";
    if (code == 77) return "Snow Grain";
    if (code >= 80 && code <= 82) return "Showers";
    if (code >= 85 && code <= 86) return "Snow Showers";
    if (code == 95) return "Thunder";
    if (code == 96 || code == 99) return "Thunder Hail";
    return "Unknown";
}

void initializeLCD() {
    SPI.begin(TFT_PIN_SCK, TFT_PIN_MISO, TFT_PIN_MOSI, TFT_PIN_CS);
    tft.begin();
    tft.setRotation(TFT_ROTATION);

    pinMode(TFT_PIN_BL, OUTPUT);
    digitalWrite(TFT_PIN_BL, HIGH);

    Palette p = {ILI9341_BLACK, ILI9341_DARKCYAN, ILI9341_WHITE, ILI9341_GREEN};
    drawBasePage("Weather Display", p);
    drawText(10, 66, "Made by A/N", p, p.text, 2);
    drawText(10, 92, "Ready", p, p.accent, 2);
}

void showWeatherDataError() {
    Palette p = {ILI9341_BLACK, ILI9341_MAROON, ILI9341_WHITE, ILI9341_RED};
    drawBasePage("Weather Error", p);
    drawText(10, 70, "Weather data", p, p.text, 2);
    drawText(10, 98, "not available", p, p.accent, 2);
}

void showNoDisplayEnabledError() {
    Palette p = {ILI9341_BLACK, ILI9341_DARKCYAN, ILI9341_WHITE, ILI9341_ORANGE};
    drawBasePage("Display Setup", p);
    drawText(10, 70, "Enable pages", p, p.text, 2);
    drawText(10, 98, "in settings", p, p.accent, 2);
}

void showWeatherOnLCD() {
    int code = (hourlyForecastCount > 0) ? hourlyForecasts[0].weatherCode : 0;
    Palette p = paletteFromCode(code);
    drawBasePage("Current Weather", p);

    int iconCx = 36;
    int iconCy = 88;
    int textX = 72;
    if (sw() < 280) {
        iconCx = 30;
        textX = 64;
    }

    drawWeatherIcon(iconCx, iconCy, code, p);
    drawText(textX, 62, String(tempC, 1) + " C", p, p.text, 3);
    drawText(textX, 98, "Humidity " + String(humidity, 0) + "%", p, p.text, 2);
    drawText(textX, 126, "Pressure " + String(pressure, 0) + " hPa", p, p.text, 2);
    drawText(textX, 154, getWeatherDescription(code), p, p.accent, 2);
}

void showClothRecommendation() {
    int code = (hourlyForecastCount > 0) ? hourlyForecasts[0].weatherCode : 0;
    Palette p = paletteFromCode(code);
    drawBasePage("Clothing Advice", p);

    float feelsLike = tempC;
    if (tempC >= 27) {
        feelsLike = tempC + (0.5f * (humidity - 40.0f) / 100.0f * tempC);
    } else if (tempC <= 10) {
        feelsLike = tempC - (humidity < 50 ? 2 : 0);
    }

    int umbrella = umbrellaConfidencePct();
    int heat = heatRiskPct(feelsLike);

    String recommendation = "T-shirt is OK";
    if (feelsLike < 0) recommendation = "Winter coat";
    else if (feelsLike < 10) recommendation = "Jacket needed";
    else if (feelsLike > 30) recommendation = "Light clothes";
    if (umbrella > 70) recommendation += " + Umbrella";

    int iconCx = (sw() < 280) ? 30 : 36;
    int textX = (sw() < 280) ? 64 : 72;
    drawWeatherIcon(iconCx, 88, code, p);

    drawText(textX, 62, "Feels " + String(feelsLike, 1) + " C", p, p.text, 2);
    drawText(textX, 90, "Umbrella " + String(umbrella) + "%", p, p.accent, 2);
    drawText(textX, 118, "Heat risk " + String(heat) + "%", p, p.text, 2);
    drawText(10, 154, recommendation, p, p.accent, 2);
}

void showHourlyForecast() {
    if (hourlyForecastCount == 0) {
        Palette p = {ILI9341_BLACK, ILI9341_DARKCYAN, ILI9341_WHITE, ILI9341_RED};
        drawBasePage("Hourly Forecast", p);
        drawText(10, 70, "No forecast data", p, p.accent, 2);
        return;
    }

    int idx = forecastDisplayIndex % hourlyForecastCount;
    int code = hourlyForecasts[idx].weatherCode;
    Palette p = paletteFromCode(code);
    drawBasePage("Hourly Forecast", p);

    String timeStr = hourlyForecasts[idx].time;
    int hourStart = timeStr.indexOf('T') + 1;
    String hhmm = timeStr.substring(hourStart, hourStart + 5);

    int iconCx = (sw() < 280) ? 30 : 36;
    int textX = (sw() < 280) ? 64 : 72;
    drawWeatherIcon(iconCx, 88, code, p);

    drawText(textX, 62, "Time " + hhmm, p, p.text, 2);
    drawText(textX, 90, "Temp " + String(hourlyForecasts[idx].temperature, 1) + " C", p, p.text, 2);
    drawText(textX, 118, "Rain " + String(hourlyForecasts[idx].precipitation, 1) + " mm", p, p.accent, 2);

    int rain = nextRainHours();
    String summary = (rain >= 0) ? ("Next rain in " + String(rain) + "h") : "No rain in 12h";
    drawText(10, 154, summary, p, p.accent, 2);
}

void showDailyForecast() {
    if (dailyForecastCount <= 1) {
        Palette p = {ILI9341_BLACK, ILI9341_DARKCYAN, ILI9341_WHITE, ILI9341_RED};
        drawBasePage("Daily Forecast", p);
        drawText(10, 70, "No forecast data", p, p.accent, 2);
        return;
    }

    int dayIdx = (forecastDisplayIndex % min(3, dailyForecastCount - 1)) + 1;
    int code = dailyForecasts[dayIdx].weatherCode;
    Palette p = paletteFromCode(code);
    drawBasePage("Daily Forecast", p);

    int iconCx = (sw() < 280) ? 30 : 36;
    int textX = (sw() < 280) ? 64 : 72;
    drawWeatherIcon(iconCx, 88, code, p);

    String date = dailyForecasts[dayIdx].date.substring(5);
    drawText(textX, 62, "Date " + date, p, p.text, 2);
    drawText(textX, 90, "Min " + String(dailyForecasts[dayIdx].tempMin, 0) + " C", p, p.text, 2);
    drawText(textX, 118, "Max " + String(dailyForecasts[dayIdx].tempMax, 0) + " C", p, p.text, 2);
    drawText(textX, 146, "Rain " + String(dailyForecasts[dayIdx].precipitationSum, 1) + " mm", p, p.accent, 2);

    drawMinMaxBar(10, 178, sw() - 20, 16, dailyForecasts[dayIdx].tempMin, dailyForecasts[dayIdx].tempMax, p);

    float low, high;
    lowHigh12h(low, high);
    drawText(10, 202, "12h low/high " + String(low, 1) + "/" + String(high, 1) + " C", p, p.text, 1);
}