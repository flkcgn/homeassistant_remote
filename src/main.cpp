#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <TFT_22_ILI9225.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

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
    // Von der realen Home-Assistant-Instanz bestätigte area_id-Werte.
    { "Wohnzimmer",    "wohnzimmer",    false },
    { "Arbeitszimmer", "arbeitszimmer", false }
};

constexpr int AREA_COUNT = sizeof(areas) / sizeof(areas[0]);
constexpr int ROOT_ITEM_COUNT = 2;
constexpr int ACTION_ITEM_COUNT = 2;

enum class MenuScreen {
    Root,
    Rooms,
    RoomActions
};

MenuScreen currentScreen = MenuScreen::Root;
int selectedIndex = 0;
int selectedRoomIndex = 0;

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

    int itemCount = ROOT_ITEM_COUNT;
    if (currentScreen == MenuScreen::Rooms) {
        itemCount = AREA_COUNT;
    } else if (currentScreen == MenuScreen::RoomActions) {
        itemCount = ACTION_ITEM_COUNT;
    }

    for (int i = 0; i < itemCount; i++) {
        int y = 48 + (i * 26);

        char line[48];
        if (currentScreen == MenuScreen::Root) {
            snprintf(
                line,
                sizeof(line),
                "%d. %s",
                i + 1,
                i == 0 ? "Licht" : "Ausschalten"
            );
        } else if (currentScreen == MenuScreen::Rooms) {
            snprintf(
                line,
                sizeof(line),
                "1.%d %s [%s]",
                i + 1,
                areas[i].label,
                areas[i].isOn ? "AN" : "AUS"
            );
        } else {
            snprintf(
                line,
                sizeof(line),
                "1.%d.%d %s",
                selectedRoomIndex + 1,
                i + 1,
                i == 0 ? "An" : "Aus"
            );
        }

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
    tft.drawText(8, 148, "ENTER = Auswaehlen", COLOR_YELLOW);
    if (currentScreen == MenuScreen::Root) {
        tft.drawText(8, 158, "BACK = ohne Funktion", COLOR_YELLOW);
    } else {
        tft.drawText(8, 158, "BACK = Zurueck", COLOR_YELLOW);
    }
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

    String url = String(HA_BASE_URL) + "/api/services/light/";
    url += turnOn ? "turn_on" : "turn_off";

    http.begin(url);
    http.addHeader("Authorization", String("Bearer ") + HA_TOKEN);
    http.addHeader("Content-Type", "application/json");

    String payload = String("{\"area_id\":\"") + areaId + "\"}";

    int httpCode = http.POST(payload);
    http.end();

    Serial.print("HTTP ");
    Serial.println(httpCode);

    return (httpCode >= 200 && httpCode < 300);
}

void switchSelectedArea(bool turnOn) {
    char msg[64];

    snprintf(
        msg,
        sizeof(msg),
        "%s %s...",
        areas[selectedRoomIndex].label,
        turnOn ? "AN" : "AUS"
    );
    setStatus(msg);
    drawUI();

    bool ok = callHomeAssistantAreaService(areas[selectedRoomIndex].areaId, turnOn);

    if (ok) {
        areas[selectedRoomIndex].isOn = turnOn;

        snprintf(
            msg,
            sizeof(msg),
            "%s %s",
            areas[selectedRoomIndex].label,
            turnOn ? "AN" : "AUS"
        );
        setStatus(msg);
    } else {
        snprintf(
            msg,
            sizeof(msg),
            "Fehler bei %s",
            areas[selectedRoomIndex].label
        );
        setStatus(msg);
    }

    drawUI();
}

int currentItemCount() {
    if (currentScreen == MenuScreen::Rooms) {
        return AREA_COUNT;
    }
    if (currentScreen == MenuScreen::RoomActions) {
        return ACTION_ITEM_COUNT;
    }
    return ROOT_ITEM_COUNT;
}

void enterDeepSleep(bool showMessage = true) {
    if (showMessage) {
        setStatus("Ausschalten...");
        drawUI();
        delay(500);
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    while (digitalRead(BTN_ENTER) == LOW) {
        delay(10);
    }

    if (showMessage) {
        tft.clear();
    }

    const gpio_num_t wakeupPin = static_cast<gpio_num_t>(BTN_ENTER);
    rtc_gpio_init(wakeupPin);
    rtc_gpio_set_direction(wakeupPin, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_en(wakeupPin);
    rtc_gpio_pulldown_dis(wakeupPin);

    esp_err_t result = esp_sleep_enable_ext0_wakeup(wakeupPin, 0);
    if (result != ESP_OK) {
        rtc_gpio_deinit(wakeupPin);
        pinMode(BTN_ENTER, INPUT_PULLUP);
        if (showMessage) {
            setStatus("Sleep-Konfiguration Fehler");
            drawUI();
        } else {
            Serial.println("Sleep-Konfiguration fehlgeschlagen");
        }
        return;
    }

    Serial.println("Remote im Tiefschlaf");
    Serial.flush();
    esp_deep_sleep_start();
}

bool enterHeldForPowerOn() {
    constexpr unsigned long POWER_ON_HOLD_MS = 3000;
    unsigned long start = millis();

    while (millis() - start < POWER_ON_HOLD_MS) {
        if (digitalRead(BTN_ENTER) != LOW) {
            return false;
        }
        delay(10);
    }

    return true;
}

void handleEnter() {
    if (currentScreen == MenuScreen::Root) {
        if (selectedIndex == 0) {
            currentScreen = MenuScreen::Rooms;
            selectedIndex = 0;
            setStatus("Raum auswaehlen");
            drawUI();
        } else {
            enterDeepSleep();
        }
        return;
    }

    if (currentScreen == MenuScreen::Rooms) {
        selectedRoomIndex = selectedIndex;
        currentScreen = MenuScreen::RoomActions;
        selectedIndex = 0;
        setStatus("Aktion auswaehlen");
        drawUI();
        return;
    }

    switchSelectedArea(selectedIndex == 0);
}

void handleBack() {
    if (currentScreen == MenuScreen::RoomActions) {
        currentScreen = MenuScreen::Rooms;
        selectedIndex = selectedRoomIndex;
        setStatus("Raum auswaehlen");
    } else if (currentScreen == MenuScreen::Rooms) {
        currentScreen = MenuScreen::Root;
        selectedIndex = 0;
        setStatus("Hauptmenue");
    } else {
        setStatus("Bereits im Hauptmenue");
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
                selectedIndex = currentItemCount() - 1;
            }
            setStatus("Auswahl geaendert");
            drawUI();
            lastButtonMs = now;
        }

        if (down == LOW && lastDown == HIGH) {
            selectedIndex++;
            if (selectedIndex >= currentItemCount()) {
                selectedIndex = 0;
            }
            setStatus("Auswahl geaendert");
            drawUI();
            lastButtonMs = now;
        }

        if (enter == LOW && lastEnter == HIGH) {
            handleEnter();
            lastButtonMs = now;
        }

        if (back == LOW && lastBack == HIGH) {
            handleBack();
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

    esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();
    if (wakeupCause == ESP_SLEEP_WAKEUP_EXT0) {
        rtc_gpio_deinit(static_cast<gpio_num_t>(BTN_ENTER));
    }

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_ENTER, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);

    if (wakeupCause == ESP_SLEEP_WAKEUP_EXT0 && !enterHeldForPowerOn()) {
        enterDeepSleep(false);
    }

    lastUp = digitalRead(BTN_UP);
    lastDown = digitalRead(BTN_DOWN);
    lastEnter = digitalRead(BTN_ENTER);
    lastBack = digitalRead(BTN_BACK);

    delay(1500);

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
