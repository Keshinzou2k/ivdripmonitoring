#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "USFI Plant Station"
#define WIFI_PASSWORD "11AhHkh0oD"
//#define WIFI_SSID "GlobeAtHome_1A76A"
//#define WIFI_PASSWORD "4ABA4AD1"
#define API_KEY "AIzaSyAwcRAd0RkyNvtAUNjDsRFPgVOgd4QhjUM"
#define DATABASE_URL "https://iv-drip-monitoring-default-rtdb.asia-southeast1.firebasedatabase.app"
#define USER_EMAIL "keshinzou.thesis.2025@gmail.com"
#define USER_PASSWORD "thesis@project1001"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long previousMillis = 0;
const unsigned long interval = 500;
float ivLevel = 100.0;
float dropRate = 0.0;

void setupWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
    }
}

void setupFirebase() {
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096, 1024);
    Firebase.begin(&config, &auth);
    Firebase.setDoubleDigits(5);
}

void writeToFirebase(float ivLevel, float dropRate) {
    if (Firebase.ready()) { 
        Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/iv-level"), ivLevel) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/iv-drop-rate"), dropRate) ? "ok" : fbdo.errorReason().c_str());
    } else {
        Serial.println("Firebase not ready. Skipping update.");
    }
}

void sendPatientNotification(int value) {
    if (Firebase.ready()) {
        Serial.printf("Set integer... %s\n", Firebase.setInt(fbdo, F("/patientNotification"), value) ? "ok" : fbdo.errorReason().c_str());
    } else {
        Serial.println("Firebase not ready. Skipping notification.");
    }
}

void setup() {
    Serial.begin(115200);
    setupWiFi();
    setupFirebase();
}

void loop() {
    unsigned long currentMillis = millis();

    // Handle periodic Firebase updates
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // Decrease ivLevel over time
        if (ivLevel > 0) {
            ivLevel -= 1;  // Decrease by 0.1 every second
        }

        // Reset ivLevel back to 100 when it reaches 0
        if (ivLevel <= 0) {
            ivLevel = 100.0;
        }

        // Simulate a random drop rate between 5 and 10
        dropRate = random(5, 10);

        writeToFirebase(ivLevel, dropRate);
    }

    // Handle serial input for patient notifications
    if (Serial.available() > 0) {
        int input = Serial.read();
        if (input == '1') {
            sendPatientNotification(input);
        }
    }
}
