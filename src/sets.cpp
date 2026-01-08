#include "sets.h"

#include <LittleFS.h>
#include <WiFiConnector.h>

#include "getColorByTemp.h"

float tempC = 0.0;
float humidity = 0.0;
float pressure = 0.0;

GyverDBFile db(&LittleFS, "/data.db");
SettingsGyver sett(PROJECT_NAME, &db);

// ========== build ==========
static void build(sets::Builder& b) {
    if (b.Switch(kk::switch_state, "Switch State")) {
        digitalWrite(LED_PIN, db[kk::switch_state] ? HIGH : LOW);
        
        Serial.print("LED is now ");
        Serial.println(db[kk::switch_state] ? "ON" : "OFF");
        
        db.update();
    }

    b.LinearGauge(101, "Temperature", MIN_TEMP_C, MAX_TEMP_C, "°C", tempC, getColorByTemp(tempC));
    b.LinearGauge(102, "Humidity", MIN_HUMIDITY, MAX_HUMIDITY, "%", humidity, sets::Colors::Blue);
    b.LinearGauge(103, "Pressure", MIN_PRESSURE, MAX_PRESSURE, "hPa", pressure, sets::Colors::Aqua);

    if (b.beginMenu("WiFi")) {
        b.Input(kk::wifi_ssid, "SSID");
        b.Pass(kk::wifi_pass, "Pass", "");
        if (b.Switch(kk::close_ap, "Close AP")) {
            WiFiConnector.closeAP(db[kk::close_ap]);
        }
        if (b.Button("Connect")) {
            WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);
        }
        
        b.Label("Status", WiFi.isConnected() ? "Connected" : "Disconnected", 
                WiFi.isConnected() ? sets::Colors::Green : sets::Colors::Red);
        
        if (WiFi.isConnected()) {
            b.Label("IP Address", WiFi.localIP().toString());
        }
        
        b.endMenu();
    }
}

// ========== update ==========
static void update(sets::Updater& u) {
    digitalWrite(LED_PIN, db[kk::switch_state] ? HIGH : LOW);

    u.update(101, tempC);
    u.updateColor(101, getColorByTemp(tempC));
    
    u.update(102, humidity);

    u.update(103, pressure);
}

// ========== begin ==========
void sett_begin() {
    // fs
#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif

    // database
    db.begin();
    db.init(kk::wifi_ssid, "");
    db.init(kk::wifi_pass, "");
    db.init(kk::switch_state, false);
    db.init(kk::close_ap, true);

    // wifi
    WiFiConnector.onConnect([]() {
        Serial.print("Connected: ");
        Serial.println(WiFi.localIP());

        Serial.print("API Started on: ");
        Serial.println(WiFi.localIP().toString() + ":" + String(API_PORT));
    });
    WiFiConnector.onError([]() {
        Serial.print("Error. Start AP: ");
        Serial.println(WiFi.softAPIP());

        Serial.print("API Started on: ");
        Serial.println(WiFi.softAPIP().toString() + ":" + String(API_PORT));
    });

    WiFiConnector.setName(PROJECT_NAME);
    WiFiConnector.closeAP(db[kk::close_ap]);
    WiFiConnector.connect(db[kk::wifi_ssid], db[kk::wifi_pass]);

    // settings
    sett.begin();
    sett.onBuild(build);
    sett.onUpdate(update);

    // configure LED pin and set initial state from saved setting
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, db[kk::switch_state] ? HIGH : LOW);
}

// ========== loop ==========
void sett_loop() {
    WiFiConnector.tick();
    sett.tick();
}