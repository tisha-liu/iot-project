#include "M5StickCPlus.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <base64.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid        = "Keemstar";
const char* password    = "12345678";
const char* mqtt_server = "192.168.239.224";

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE]; 

// Define the topics to publish to
const char* soilTopic = "soil";
const char* lightTopic = "light";
const char* temperatureTopic = "temp";

void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();

void setup() {
    M5.begin();
    M5.Lcd.setRotation(3);
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 33, 32);
    setupWifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reConnect();
    }
    client.loop();

    // Check for available data on Serial2
    if (Serial2.available()) {
        StaticJsonDocument<256> doc;

        // Read the JSON document from the "link" serial port
        DeserializationError err = deserializeJson(doc, Serial2);

        if (err == DeserializationError::Ok) {
            // Print the values
            // (we must use as<T>() to resolve the ambiguity)
            int nodeNumber = doc["node"];

            // Extract the values
            const char* soilValue = doc["moisture"].as<const char*>();
            const char* tempValue = doc["temperature"].as<const char*>();
            const char* lightValue = doc["light"].as<const char*>();

            // Publish the extracted values to MQTT topics
            client.publish(soilTopic, soilValue);
            client.publish(temperatureTopic, tempValue);
            client.publish(lightTopic, lightValue);

            Serial.print("Soil Moisture: ");
            Serial.println(soilValue);
            Serial.print("Temperature: ");
            Serial.println(tempValue);
            Serial.print("Light: ");
            Serial.println(lightValue);

        } else {
            // Print error to the "debug" serial port
            Serial.print("deserializeJson() returned ");
            Serial.println(err.c_str());

            // Flush all bytes in the "link" serial port buffer
            while (Serial2.available() > 0)
                Serial2.read();
        }
    }

    if (M5.BtnA.wasPressed()) {
    sendEmail("nurhakeemazman@gmail.com", "Low Soil Moisture Detection", "This is a test email from MQTT Publisher Node.");
    Serial.println("Email sent!!");
    }
    M5.update();
}

void setupWifi() {
    delay(10);
    M5.Lcd.printf("Connecting to %s", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
    }
    M5.Lcd.printf("\nHurray WiFi! :)\n");
}

void callback(char* topic, byte* payload, unsigned int length) {
    // This function is not needed for publishing dummy data
}

void reConnect() {
    while (!client.connected()) {
        M5.Lcd.print("Attempting MQTT connection...");
        String clientId = "M5Stack-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            M5.Lcd.printf("\nSuccess\n");
        } else {
            M5.Lcd.print("failed, rc=");
            M5.Lcd.print(client.state());
            M5.Lcd.println("try again in 5 seconds");
            delay(5000);
        }
    }
}

void sendEmail(const char* to, const char* subject, const char* message) {
 // Prepare the email data
String emailData = "from=nurhakeemazman@sandbox99a0c938a15943ffad090b76117dc3a0.mailgun.org&to=nurhakeemazman@gmail.com&subject=" + String(subject) + "&text=" + String(message);

Serial.println(emailData);

 // Prepare the HTTP request
 HTTPClient http;
 http.begin("https://api.mailgun.net/v3/sandbox99a0c938a15943ffad090b76117dc3a0.mailgun.org/messages");
 http.addHeader("Authorization", "Basic " + String(base64::encode("api:4358a1bd1474ed758a156b7e472388e5-309b0ef4-cc08a894")));
 http.addHeader("Content-Type", "application/x-www-form-urlencoded");

 // Send the HTTP request
 int httpResponseCode = http.POST(emailData);

 // Check the response
 if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
 } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
 }

 // Free resources
 http.end();
}
