// Include the libraries 
#include "BLEDevice.h"
#include <M5StickCPlus.h>
#include <ArduinoJson.h>

// Define names for the BLE servers for different sensors
#define BLE_SERVER_LIGHT "CSC2106-BLE-LIGHT"
#define BLE_SERVER_TEMPERATURE "CSC2106-BLE-TEMPERATURE"
#define BLE_SERVER_MOISTURE "CSC2106-BLE-MOISTURE"

// Define UUIDs for the service and characteristics for light, temperature, and moisture
static BLEUUID bleServiceUUID("01234567-0123-4567-89ab-0123456789ab");
static BLEUUID lightCharacteristicUUID("01234567-0123-4567-89ab-0123456789ef");
static BLEUUID temperatureCharacteristicUUID("01234567-0123-4567-89ab-0123456789cd");
static BLEUUID moistureCharacteristicUUID("01234567-0123-4567-89ab-0123456789aa");

// Flags for managing connection states
static boolean connectToLight = false, connectToTemperature = false, connectToMoisture = false;
static boolean connectedToLight = false, connectedToTemperature = false, connectedToMoisture = false;

// Addresses for the BLE servers
static BLEAddress *LightServerAddress = nullptr, *TemperatureServerAddress = nullptr, *MoistureServerAddress = nullptr;

// Pointers to the remote characteristics for light, temperature, and moisture
static BLERemoteCharacteristic *LightCharacteristic = nullptr, *TemperatureCharacteristic = nullptr, *MoistureCharacteristic = nullptr;

// Notification values for enabling and disabling notifications
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

#define MAX_DATA_LENGTH 64 // Maximum length of data to be received
char LightStr[MAX_DATA_LENGTH] = {0}; // Buffer for light data
char TemperatureStr[MAX_DATA_LENGTH] = {0}; // Buffer for temperature data
char MoistureStr[MAX_DATA_LENGTH] = {0}; // Buffer for moisture data

// Flags for new sensor readings
boolean newLight = false;
boolean newTemperature = false;
boolean newMoisture = false;

// Callback function for receiving light notifications
static void LightNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  size_t copyLength = length < MAX_DATA_LENGTH ? length : MAX_DATA_LENGTH - 1;
  memcpy(LightStr, pData, copyLength);
  LightStr[copyLength] = '\0'; // Ensure null-termination
  newLight = true;
}

// Callback function for receiving temperature notifications
static void TemperatureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  size_t copyLength = length < MAX_DATA_LENGTH ? length : MAX_DATA_LENGTH - 1;
  memcpy(TemperatureStr, pData, copyLength);
  TemperatureStr[copyLength] = '\0'; // Ensure null-termination
  newTemperature = true;
}

// Callback function for receiving moisture notifications
static void MoistureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  size_t copyLength = length < MAX_DATA_LENGTH ? length : MAX_DATA_LENGTH - 1;
  memcpy(MoistureStr, pData, copyLength);
  MoistureStr[copyLength] = '\0'; // Ensure null-termination
  newMoisture = true;
}

// Function to connect to a specified BLE server
bool connectToServer(BLEAddress pAddress) {
   BLEClient* pClient = BLEDevice::createClient();
 
  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");
 
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(bleServiceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(bleServiceUUID.toString().c_str());
    return (false);
  }
 
  // Obtain a reference to the characteristics in the service of the remote BLE server.
  LightCharacteristic = pRemoteService->getCharacteristic(lightCharacteristicUUID);
  TemperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharacteristicUUID);
  MoistureCharacteristic = pRemoteService->getCharacteristic(moistureCharacteristicUUID);

  if (LightCharacteristic == nullptr && TemperatureCharacteristic == nullptr && MoistureCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID");
    return false;
  }

  Serial.println(" - Found our characteristics");
 
  //Assign callback functions for the Characteristics
  if (LightCharacteristic != nullptr){
    LightCharacteristic->registerForNotify(LightNotifyCallback);
  }
  if (TemperatureCharacteristic != nullptr){
    TemperatureCharacteristic->registerForNotify(TemperatureNotifyCallback);
  }
  if (MoistureCharacteristic != nullptr){
    MoistureCharacteristic->registerForNotify(MoistureNotifyCallback);
  }

  return true;
}

// Function to disconnect from a BLE server
void disconnectFromServer(BLEClient* pClient) {
  if (pClient->isConnected()) {
    pClient->disconnect();
    Serial.println("Disconnected from the BLE Server.");
  }
}

// Callback class for handling BLE advertisements
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == BLE_SERVER_LIGHT) { //Check if the name of the advertiser matches
      LightServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      connectToLight = true; //Set indicator, stating that we are ready to connect
      Serial.println("Device found for light sensor. Connecting!");
    }
    if (advertisedDevice.getName() == BLE_SERVER_TEMPERATURE) { //Check if the name of the advertiser matches
      TemperatureServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      connectToTemperature = true; //Set indicator, stating that we are ready to connect
      Serial.println("Device found for temperature sensor. Connecting!");
    }
    if (advertisedDevice.getName() == BLE_SERVER_MOISTURE) { //Check if the name of the advertiser matches
      MoistureServerAddress = new BLEAddress(advertisedDevice.getAddress()); //Address of advertiser is the one we need
      connectToMoisture = true; //Set indicator, stating that we are ready to connect
      Serial.println("Device found for moisture sensor. Connecting!");
    }
    else
      Serial.print(".");
  }
};

// Function to display sensor readings on the OLED display
void printReadings(){
  StaticJsonDocument<256> doc;

  // Store light, temperature, and moisture readings in the document
  doc["light"] = LightStr;
  doc["temperature"] = TemperatureStr;
  doc["moisture"] = MoistureStr;

  // Serialize the JSON document to a string and print it
  String output;
  serializeJson(doc, Serial2);
  // Serial.println(output);

  // // Deserialize the JSON string
  // DeserializationError error = deserializeJson(doc, output);

  // // Test if parsing succeeds.
  // if (error) {
  //   Serial.print(F("deserializeJson() failed: "));
  //   Serial.println(error.f_str());
  //   return;
  // }

  // // Extract values
  // // Use asString() if the values are represented as strings in JSON
  // // Or directly as int, float, etc., based on the actual data type
  // const char* light = doc["light"].as<const char*>();
  // const char* temperature = doc["temperature"].as<const char*>();

  // // Print values
  // Serial.print("Light: ");
  // Serial.println(light);
  // Serial.print("Temperature: ");
  // Serial.println(temperature);

  M5.Lcd.setCursor(0, 20, 2);

  // Display Light reading
  M5.Lcd.print("Light Detected = ");
  M5.Lcd.println(LightStr);

  Serial.print(" - Light Detected = ");
  Serial.println(LightStr); 

  // Display Temperature reading
  M5.Lcd.setCursor(0, 40, 2);
  M5.Lcd.print("Temperature = ");
  M5.Lcd.println(TemperatureStr);

  Serial.print(" - Temperature = ");
  Serial.println(TemperatureStr);

  // Display Soil Moisture reading
  M5.Lcd.setCursor(0, 60, 2);
  M5.Lcd.print("Soil Moisture = ");
  M5.Lcd.println(MoistureStr);

  Serial.print(" - Soil Moisture = ");
  Serial.println(MoistureStr);
}

void setup() {
  //Start serial communication
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 32,
                  33);
  Serial.println("Starting BLE Client application...");

  // put your setup code here, to run once:
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("BLE Client", 0);

  // Init BLE device
  BLEDevice::init("");
 
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device. Specify that we want active scanning and start the
  // scan to run for 30 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30, false);
}

void loop() {
  // Check connection flags and connect to servers accordingly
  if (connectToLight == true) {
    if (connectToServer(*LightServerAddress)) {
      Serial.println("Connected to the BLE Server.");
      
      // Activate the Notify property of each Characteristic
      LightCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2903))->writeValue((uint8_t*)notificationOn, 2, true);
      connectedToLight = true;

    } else {
      Serial.println("Failed to connect to the server; Restart device to scan for nearby BLE server again.");
    }
    connectToLight = false;
  }

  if (connectToTemperature == true) {
    if (connectToServer(*TemperatureServerAddress)) {
      Serial.println("Connected to the BLE Server.");
      
      // Activate the Notify property of each Characteristic
      TemperatureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2903))->writeValue((uint8_t*)notificationOn, 2, true);
      connectedToTemperature = true;

    } else {
      Serial.println("Failed to connect to the server; Restart device to scan for nearby BLE server again.");
    }
    connectToTemperature = false;
  }

  if (connectToMoisture == true) {
    if (connectToServer(*MoistureServerAddress)) {
      Serial.println("Connected to the BLE Server.");
      
      // Activate the Notify property of each Characteristic
      MoistureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2903))->writeValue((uint8_t*)notificationOn, 2, true);
      connectedToMoisture = true;

    } else {
      Serial.println("Failed to connect to the server; Restart device to scan for nearby BLE server again.");
    }
    connectToMoisture = false;
  }

  // Print sensor readings if new data is available
  if (newLight || newTemperature || newMoisture){
    newLight = false;
    newTemperature = false;
    newMoisture = false;
    printReadings();
  }
  delay(1000); // Delay one second between loops.
}
