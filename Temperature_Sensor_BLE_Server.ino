#include <BLEDevice.h>
#include <BLEServer.h>
//#include <BLEUtils.h>
//#include <BLE2902.h>
#include <M5StickCPlus.h>
#include "DHT.h"

//change to unique BLE server name
#define BLE_SERVER_TEMPERATURE "CSC2106-BLE-TEMPERATURE"

#define DHTPIN 26 // what pin we're connected to
#define DHTTYPE DHT11 // DHT 11

// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;   // update refresh every 15sec

bool deviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "01234567-0123-4567-89ab-0123456789ab"

// Battery temperature Characteristic and Descriptor
BLECharacteristic axpTemperatureCharacteristics("01234567-0123-4567-89ab-0123456789cd", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor axpTemperatureDescriptor(BLEUUID((uint16_t)0x2903));

//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("MyServerCallbacks::Connected...");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("MyServerCallbacks::Disconnected...");
  }
};

void setup() {
  // Start serial communication 
  Serial.begin(115200);

  dht.begin();

  // put your setup code here, to run once:
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("BLE Server", 0);

  // Create the BLE Device
  BLEDevice::init(BLE_SERVER_TEMPERATURE);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bleService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  bleService->addCharacteristic(&axpTemperatureCharacteristics);
  axpTemperatureDescriptor.setValue("temperature Sensor");
  axpTemperatureCharacteristics.addDescriptor(&axpTemperatureDescriptor); 
    
  // Start the service
  bleService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    if ((millis() - lastTime) > timerDelay) {
      // Buffer to hold the generated string
      char buf[32]; // Ensure this buffer is large enough to hold the resulting string
      
      float h = dht.readHumidity();

      // Read temperature as Celsius
      float t = dht.readTemperature();

      // Read temperature as Fahrenheit
      float f = dht.readTemperature(true);

      dtostrf(t, 4, 2, buf);

      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }

      // Set temperature Characteristic value and notify connected client
      axpTemperatureCharacteristics.setValue((uint8_t*)buf, strlen(buf)); // Convert string to byte array and set the length
      axpTemperatureCharacteristics.notify();
      Serial.print(" - temperature = ");
      Serial.println(buf); // Use println to move to a new line after printing

      // Update the display
      M5.Lcd.setCursor(0, 40, 2);
      M5.Lcd.print("temperature = ");
      M5.Lcd.println(buf); // Use println to move to a new line after printing

      lastTime = millis();
    }
  }
}





