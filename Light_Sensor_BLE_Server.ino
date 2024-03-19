#include <BLEDevice.h>
#include <BLEServer.h>
#include <M5StickCPlus.h>

#define BLE_SERVER_LIGHT "CSC2106-BLE-LIGHT"

const int lightSensorPin = 26;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;   // update refresh every 1 sec

bool deviceConnected = false;

#define SERVICE_UUID "01234567-0123-4567-89ab-0123456789ab"

BLECharacteristic axpLightCharacteristics("01234567-0123-4567-89ab-0123456789ef", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor axpLightDescriptor(BLEUUID((uint16_t)0x2903));

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

  // Set the light sensor pin as input
  pinMode(lightSensorPin, INPUT);

  // put your setup code here, to run once:
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("BLE Server", 0);

  // Create the BLE Device
  BLEDevice::init(BLE_SERVER_LIGHT);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bleService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and Create a BLE Descriptor
  bleService->addCharacteristic(&axpLightCharacteristics);
  axpLightDescriptor.setValue("Light Sensor");
  axpLightCharacteristics.addDescriptor(&axpLightDescriptor); 
    
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
      // Read the digital input
      int sensorValue = digitalRead(lightSensorPin);
      
      // Invert the logic here
      bool isLightDetected = !sensorValue;
      
      //Set light Characteristic value and notify connected client
      axpLightCharacteristics.setValue(isLightDetected ? "Yes" : "No ");
      axpLightCharacteristics.notify();   
      Serial.print(" - light = ");
      Serial.println(isLightDetected ? "Yes" : "No ");

      M5.Lcd.setCursor(0, 40, 2);
      M5.Lcd.print("Light = ");
      M5.Lcd.println(isLightDetected ? "Yes" : "No ");
      
      lastTime = millis();
    }
  }
}


