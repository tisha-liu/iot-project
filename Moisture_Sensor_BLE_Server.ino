#include <BLEDevice.h>
#include <BLEServer.h>
//#include <BLEUtils.h>
//#include <BLE2902.h>
#include <M5StickCPlus.h>

//change to unique BLE server name
#define BLE_SERVER_MOISTURE "CSC2106-BLE-MOISTURE"

const int moistureSensorPin = 36; // GPIO 36 for the moisture sensor

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;   // update refresh every 15sec

bool deviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "01234567-0123-4567-89ab-0123456789ab"

// Battery MOISTURE Characteristic and Descriptor
BLECharacteristic axpMoistureCharacteristics("01234567-0123-4567-89ab-0123456789aa", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor axpMoistureDescriptor(BLEUUID((uint16_t)0x2903));

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

  // Set the MOISTURE sensor pin as input
  pinMode(moistureSensorPin, INPUT);


  // put your setup code here, to run once:
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("BLE Server", 0);

  // Create the BLE Device
  BLEDevice::init(BLE_SERVER_MOISTURE);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bleService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  bleService->addCharacteristic(&axpMoistureCharacteristics);
  axpMoistureDescriptor.setValue("MOISTURE Sensor");
  axpMoistureCharacteristics.addDescriptor(&axpMoistureDescriptor); 
    
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
      
      int moistureLevel = analogRead(moistureSensorPin);

      // Map the analog reading (which goes from 0 - 4095) to a range (0-100%)
      int moisturePercent = map(moistureLevel, 1392, 2902, 100, 0); // Adjust these values based on your calibration
      itoa(moisturePercent, buf, 10);

      // Set MOISTURE Characteristic value and notify connected client
      axpMoistureCharacteristics.setValue((uint8_t*)buf, strlen(buf)); // Convert string to byte array and set the length
      axpMoistureCharacteristics.notify();
      Serial.print(" - Moisture = ");
      Serial.println(buf); // Use println to move to a new line after printing

      // Update the display
      M5.Lcd.setCursor(0, 40, 2);
      M5.Lcd.print("Moisture = ");
      M5.Lcd.println(buf); // Use println to move to a new line after printing

      lastTime = millis();
    }
  }
}





