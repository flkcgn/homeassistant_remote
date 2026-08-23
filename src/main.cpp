#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <TFT_22_ILI9225.h>

#include "secrets.h"

// =========================
// Buttons
// =========================
constexpr int BTN_UP    = 2;
constexpr int BTN_DOWN  = 3;
constexpr int BTN_ENTER = 4;
constexpr int BTN_BACK  = 5;

// =========================
// TFT ILI9225
// bestätigte Verdrahtung
// =========================
constexpr int8_t TFT_RST = 8;
constexpr int8_t TFT_RS  = 9;
constexpr int8_t TFT_CS  = 7;
constexpr int8_t TFT_SDI = 10;
constexpr int8_t TFT_CLK = 11;
constexpr int8_t TFT_LED = 0;   // Backlight direkt versorgt

TFT_22_ILI9225 tft(
    TFT_RST,
    TFT_RS,
    TFT_CS,
    TFT_SDI,
    TFT_CLK,
    TFT_LED
);

// =========================
// Menüeinträge
// =========================
struct AreaItem {
    const char* label;
    const char* areaId;
    bool isOn;   // lokaler Prototyp-Status
};

AreaItem areas[] = {
    { "Wohnzimmer",    "wohnzimmer",    false },
    { "Arbeitszimmer", "arbeitszimmer", false }
};

constexpr int AREA_COUNT = sizeof(areas) / sizeof(areas[0]);
int selectedIndex = 0;

// =========================
// UI / Status
// =========================
char statusLine[64] = "Starte...";
bool wifiConnected = false;

// =========================
// Button-Zustände
// =========================
bool lastUp    = HIGH;
bool lastDown  = HIGH;
bool lastEnter = HIGH;
bool lastBack  = HIGH;

unsigned long lastButtonMs = 0;
const unsigned long debounceMs = 180;

// --------------------------------------------------
// Hilfsfunktionen
// --------------------------------------------------
void setStatus(const char* text) {
    snprintf(statusLine, sizeof(statusLine), "%s", text);
}

void drawBananaIcon() {
    // sehr einfache stilisierte Banane oben rechts
    // Breite ~ 20 px
    tft.fillCircle(198, 14, 8, COLOR_YELLOW);
    tft.fillCircle(194, 14, 8, COLOR_BLACK);
    tft.fillRectangle(202, 6, 205, 9, COLOR_GREEN);
    tft.fillCircle(206, 18, 2, COLOR_YELLOW);
}

void drawUI() {
    tft.clear();

    // Header
    tft.setFont(Terminal12x16);
    tft.drawText(8, 8, "ne0nbanana", COLOR_YELLOW);
    drawBananaIcon();

    tft.setFont(Terminal6x8);
    if (wifiConnected) {
        tft.drawText(8, 28, "WLAN: verbunden", COLOR_YELLOW);
    } else {
        tft.drawText(8, 28, "WLAN: offline", COLOR_YELLOW);
    }

    // Menü
    tft.setFont(Terminal12x16);

    for (int i = 0; i < AREA_COUNT; i++) {
        int y = 48 + (i * 26);

        char line[48];
        snprintf(
            line,
            sizeof(line),
            "%d. %s [%s]",
            i + 1,
            areas[i].label,
            areas[i].isOn ? "AN" : "AUS"
        );

        if (i == selectedIndex) {
            // gelber Balken für Auswahl
            tft.fillRectangle(6, y - 2, 213, y + 15, COLOR_YELLOW);
            tft.drawText(10, y, line, COLOR_BLACK);
        } else {
            tft.drawText(10, y, line, COLOR_YELLOW);
        }
    }

    // Footer
    tft.setFont(Terminal6x8);
    tft.drawText(8, 138, "UP/DOWN = Auswahl", COLOR_YELLOW);
    tft.drawText(8, 148, "ENTER = AN", COLOR_YELLOW);
    tft.drawText(8, 158, "BACK = AUS", COLOR_YELLOW);
    tft.drawText(8, 168, statusLine, COLOR_YELLOW);
}

bool connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    setStatus("Verbinde WLAN...");
    drawUI();

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(300);
    }

    wifiConnected = (WiFi.status() == WL_CONNECTED);

    if (wifiConnected) {
        setStatus("WLAN verbunden");
    } else {
        setStatus("WLAN Fehler");
    }

    drawUI();
    return wifiConnected;
}

bool callHomeAssistantAreaService(const char* areaId, bool turnOn) {
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
        if (!connectWiFi()) {
            return false;
        }
    }

    HTTPClient http;

    String url = String(HA_BASE_URL) + "/api/services/homeassistant/";
    url += turnOn ? "turn_on" : "turn_off";

    http.begin(url);
    http.addHeader("Authorization", String("Bearer ") + HA_TOKEN);
    http.addHeader("Content-Type", "application/json");

    String payload = String("{\"target\":{\"area_id\":[\"") + areaId + "\"]}}";

    int httpCode = http.POST(payload);
    String response = http.getString();
    http.end();

    Serial.print("HTTP ");
    Serial.println(httpCode);
    Serial.println(response);

    return (httpCode >= 200 && httpCode < 300);
}

void switchSelectedArea(bool turnOn) {
    char msg[64];

    snprintf(
        msg,
        sizeof(msg),
        "%s %s...",
        areas[selectedIndex].label,
        turnOn ? "AN" : "AUS"
    );
    setStatus(msg);
    drawUI();

    bool ok = callHomeAssistantAreaService(areas[selectedIndex].areaId, turnOn);

    if (ok) {
        areas[selectedIndex].isOn = turnOn;

        snprintf(
            msg,
            sizeof(msg),
            "%s %s",
            areas[selectedIndex].label,
            turnOn ? "AN" : "AUS"
        );
        setStatus(msg);
    } else {
        snprintf(
            msg,
            sizeof(msg),
            "Fehler bei %s",
            areas[selectedIndex].label
        );
        setStatus(msg);
    }

    drawUI();
}

void handleButtons() {
    bool up    = digitalRead(BTN_UP);
    bool down  = digitalRead(BTN_DOWN);
    bool enter = digitalRead(BTN_ENTER);
    bool back  = digitalRead(BTN_BACK);

    unsigned long now = millis();

    if (now - lastButtonMs > debounceMs) {
        if (up == LOW && lastUp == HIGH) {
            selectedIndex--;
            if (selectedIndex < 0) {
                selectedIndex = AREA_COUNT - 1;
            }
            setStatus("Auswahl geaendert");
            drawUI();
            lastButtonMs = now;
        }

        if (down == LOW && lastDown == HIGH) {
            selectedIndex++;
            if (selectedIndex >= AREA_COUNT) {
                selectedIndex = 0;
            }
            setStatus("Auswahl geaendert");
            drawUI();
            lastButtonMs = now;
        }

        if (enter == LOW && lastEnter == HIGH) {
            switchSelectedArea(true);
            lastButtonMs = now;
        }

        if (back == LOW && lastBack == HIGH) {
            switchSelectedArea(false);
            lastButtonMs = now;
        }
    }

    lastUp    = up;
    lastDown  = down;
    lastEnter = enter;
    lastBack  = back;
}

// --------------------------------------------------
// Setup / Loop
// --------------------------------------------------
void setup() {
    Serial.begin(115200);
    delay(1500);

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_ENTER, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);

    tft.begin();
    tft.setOrientation(1);
    tft.setBackgroundColor(COLOR_BLACK);
    tft.clear();

    setStatus("Display bereit");
    drawUI();

    connectWiFi();

    Serial.println("Remote gestartet");
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        wifiConnected = false;
    } else {
        wifiConnected = true;
    }

    handleButtons();
    delay(10);
}