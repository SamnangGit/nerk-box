#include <Arduino.h>
// #include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#include "wifimanager.h"

#define PIN 15
#define NUMPIXELS 8

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 25

#define USER_EMAIL ""
#define USER_PASSWORD ""
#define API_KEY ""
#define DATABASE_URL ""

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int touchPin = 4;
int touchPinTwo = 12;
int touchValue = 0;
int touchValueTwo = 0;
int count = 0;

// Kalman filter variables
float q = 0.125;
float r = 32;
float p = 1023;
float x = 512;
float k;

void connectWifi();
void displayNeoPixel(int colorCode);
void clearNeoPixel();
void writeValuetoRTDB(String path, String pathCount);
void readValueFromRTDB(String path, String pathColor);

float applyKalmanFilter(float rawValue);

void setup()
{
    btStop();

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(touchPin, INPUT);
    pinMode(touchPinTwo, INPUT);
    pinMode(PIN, OUTPUT);
    Serial.begin(115200);

    // connectWifi();

    setupWifi();

    delay(5000);

    Serial.println("Setup Firebase");

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    config.api_key = API_KEY;

    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    config.database_url = DATABASE_URL;

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    String base_path = "/UsersData/";

    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
}

void loop()
{

    if (millis() - dataMillis > 300 && Firebase.ready())
    {
        dataMillis = millis();
        // String path = "/UsersData/";
        // String pathColor = "/UsersData/";
        String path = "/users/";
        String pathCount = "/users/";
        String pathColor = "/users/";
        readValueFromRTDB(path, pathColor);
        writeValuetoRTDB(path, pathCount);
        delay(100);
    }
}

void writeValuetoRTDB(String path, String pathCount)
{
    // path += "/userone/int";
    pathCount += "HdzXXsZCuMYsMs66zvzL13n2naw2/123456/touch/count";
    path += "HdzXXsZCuMYsMs66zvzL13n2naw2/123456/touch/active";
    delay(100);
    touchValue = touchRead(touchPin);
    delay(50);
    touchValueTwo = touchRead(touchPinTwo);

    float filteredValue = applyKalmanFilter(touchValue);
    float filteredValueTwo = applyKalmanFilter(touchValueTwo);

    // Serial.print("D2: ");
    // Serial.println(filteredValue);
    // delay(50);
    // Serial.print("D12: ");
    // Serial.println(filteredValueTwo);
    delay(100);

    Firebase.RTDB.getInt(&fbdo, pathCount);

    if (filteredValue < 55.0 && filteredValueTwo < 55.0)
    {
        if (Firebase.RTDB.getInt(&fbdo, pathCount))
            count = fbdo.intData();
        Serial.println("Touch");
        Serial.println(filteredValue);
        Serial.println(filteredValueTwo);
        Firebase.RTDB.setInt(&fbdo, path, 1);
        Firebase.RTDB.setInt(&fbdo, pathCount, ++count);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
    }
    else
    {
        Firebase.RTDB.setInt(&fbdo, path, 0);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
}

void readValueFromRTDB(String path, String pathColor)
{
    // path += "/usertwo/int";
    // pathColor += "/usertwo/string";

    path += "KexuveflI8bCQzKeN3zqnE7YjTU2/123456/touch/active";
    pathColor += "KexuveflI8bCQzKeN3zqnE7YjTU2/123456/touch/color";

    if (Firebase.RTDB.getInt(&fbdo, path))
    {
        int intValue = 0;
        intValue = fbdo.intData();
        // Serial.println(intValue);
        if (Firebase.RTDB.getString(&fbdo, pathColor))
        {
            String colorCode = fbdo.stringData();
            // log the color code
            Serial.print("Color Code: ");
            Serial.println(colorCode);
            delay(100);
            // cast from this hexadecimal data to integer
            int colorCodeInt = (int)strtol(colorCode.c_str(), NULL, 16);
            // log the integer value
            Serial.print("Color Code Int: ");
            Serial.println(colorCodeInt);
            if (intValue == 1)
            {
                displayNeoPixel(colorCodeInt);
            }
            else if (intValue == 0)
            {
                clearNeoPixel();
            }
        }
    }
}

void displayNeoPixel(int colorCode)
{
    pixels.clear();
    for (int i = 0; i < NUMPIXELS; i++)
    {
        // pixels.setPixelColor(i, pixels.Color(10, 10, 255));
        // use hexadecimal code instead of rgb
        pixels.setPixelColor(i, colorCode);
        pixels.show();

        delay(DELAYVAL);
    }
}

void clearNeoPixel()
{
    pixels.clear();
    pixels.show();
}

float applyKalmanFilter(float rawValue)
{
    p = p + q;
    k = p / (p + r);
    x = x + k * (rawValue - x);
    p = (1 - k) * p;
    return x;
}

// void connectWifi()
// {
//     WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//     Serial.print("Connecting to Wi-Fi");
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         Serial.print(".");
//         delay(300);
//     }
//     Serial.println();
//     Serial.print("Connected with IP: ");
//     Serial.println(WiFi.localIP());
//     Serial.println();
// }
