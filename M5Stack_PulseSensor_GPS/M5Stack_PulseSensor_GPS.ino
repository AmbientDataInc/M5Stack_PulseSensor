#ifdef ARDUINO_M5Stack_Core_ESP32
#include <M5Stack.h>
#endif
#include <TinyGPS++.h>
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>
#include "Ambient.h"

const int OUTPUT_TYPE = SERIAL_PLOTTER;

HardwareSerial GPS_s(2);
TinyGPSPlus gps;

const int PIN_INPUT = 36;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

PulseSensorPlayground pulseSensor;

WiFiClient client;
Ambient ambient;

const char* ssid = "ssid";
const char* password = "password";

unsigned int channelId = 100; // AmbientのチャネルID
const char* writeKey = "writeKey"; // ライトキー

void setup() {
#ifdef ARDUINO_M5Stack_Core_ESP32
    M5.begin();
    dacWrite(25, 0); // Speaker OFF
#endif

    GPS_s.begin(9600);

    WiFi.begin(ssid, password);  //  Wi-Fi APに接続
    while (WiFi.status() != WL_CONNECTED) {  //  Wi-Fi AP接続待ち
        delay(100);
    }

    Serial.print("WiFi connected\r\nIP address: ");
    Serial.println(WiFi.localIP());

    pulseSensor.analogInput(PIN_INPUT);
    pulseSensor.setSerial(Serial);
    pulseSensor.setOutputType(OUTPUT_TYPE);
    pulseSensor.setThreshold(THRESHOLD);

    if (!pulseSensor.begin()) {
        Serial.println("PulseSensor.begin: failed");
        for(;;) {
            delay(0);
        }
    }

    ambient.begin(channelId, writeKey, &client); // チャネルIDとライトキーを指定してAmbientの初期化
}

const int LCD_WIDTH = 320;
const int LCD_HEIGHT = 240;
const int DOTS_DIV = 30;
#define GREY 0x7BEF

void DrawGrid() {
    for (int x = 0; x <= LCD_WIDTH; x += 2) { // Horizontal Line
        for (int y = 0; y <= LCD_HEIGHT; y += DOTS_DIV) {
            M5.Lcd.drawPixel(x, y, GREY);
        }
        if (LCD_HEIGHT == 240) {
            M5.Lcd.drawPixel(x, LCD_HEIGHT - 1, GREY);
        }
    }
    for (int x = 0; x <= LCD_WIDTH; x += DOTS_DIV) { // Vertical Line
        for (int y = 0; y <= LCD_HEIGHT; y += 2) {
            M5.Lcd.drawPixel(x, y, GREY);
        }
    }
}

#define REDRAW 20 // msec
#define PERIOD 30 // sec
short lastMin = 0, lastMax = 4096;
short minS = 4096, maxS = 0;
int lastY = 0;
int x = 0;
int loopcount = 0;

void loop() {
    delay(REDRAW);

    int y = pulseSensor.getLatestSample();
    if (y < minS) minS = y;
    if (maxS < y) maxS = y;
    if (x > 0) {
        y = (int)(LCD_HEIGHT - (float)(y - lastMin) / (lastMax - lastMin) * LCD_HEIGHT);
        M5.Lcd.drawLine(x - 1, lastY, x, y, WHITE);
        lastY = y;
    }
    if (++x > LCD_WIDTH) {
        x = 0;
        M5.Lcd.fillScreen(BLACK);
        DrawGrid();
        lastMin = minS - 20;
        lastMax = maxS + 20;
        minS = 4096;
        maxS = 0;
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(4);
        M5.Lcd.printf("BPM: %d", pulseSensor.getBeatsPerMinute());
    }
    if (++loopcount > PERIOD * 1000 / REDRAW) {
        char buf[16];

        loopcount = 0;

        while (!gps.location.isUpdated()) {
            while (GPS_s.available() > 0) {
                if (gps.encode(GPS_s.read())) {
                    break;
                }
                delay(0);
            }
        }

        Serial.println("send to ambient");
        Serial.printf("lat: %f, lng: %f\r\n", gps.location.lat(), gps.location.lng());
        ambient.set(1, pulseSensor.getBeatsPerMinute());
        dtostrf(gps.location.lat(), 12, 8, buf);
        ambient.set(9, buf);
        dtostrf(gps.location.lng(), 12, 8, buf);
        ambient.set(10, buf);
        ambient.send();
    }
}
