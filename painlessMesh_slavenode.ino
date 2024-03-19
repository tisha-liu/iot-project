#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <M5StickCPlus.h>
#include <iostream>
#include <string>

// MESH Details
#define   MESH_PREFIX     "T21MESH" //name for your MESH
#define   MESH_PASSWORD   "keemstar" //password for your MESH
#define   MESH_PORT       5555 //default port

//Number for this node
int nodeNumber;
int masterNode;
// char temperature[64] = {0};
// char light[64] = {0};
// char moisture[64] = {0};
// double moisture;
// double temperature;

//String to send to other nodes with sensor readings
String readings;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

//User stubs
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings

// //Create tasks: to send messages and get readings;
// Task taskSendMessage(TASK_SECOND * 5 , TASK_FOREVER, &sendMessage);

// String getReadings () {
//   // JSONVar jsonReadings;
//   DynamicJsonDocument jsonReadings(256); // Adjust the size according to your needs
//   jsonReadings["node"] = nodeNumber;
//   jsonReadings["temp"] = temp;
//   jsonReadings["hum"] = humidity;
//   serializeJson(jsonReadings, readings);
//   return readings;
// }

void sendMessage () {
  Serial.println("Sending message");

  // Allocate the JSON document
    // This one must be bigger than the sender's because it must store the strings
    StaticJsonDocument<256> doc;

    // Read the JSON document from the "link" serial port
    DeserializationError err = deserializeJson(doc, Serial2);

    if (err == DeserializationError::Ok) 
    {
      // Print the values
      // (we must use as<T>() to resolve the ambiguity)
      // Serial.print("temperature = ");
      // Serial.println(doc["temperature"].as<double>());
      // Serial.print("moisture = ");
      // Serial.println(doc["moisture"].as<double>());
      const char* light = doc["light"].as<const char*>();
      const char* temperature = doc["temperature"].as<const char*>();
      const char* moisture = doc["moisture"].as<const char*>();

      Serial.print("Light: ");
      Serial.println(light);
      Serial.print("Temperature: ");
      Serial.println(temperature);
      Serial.print("Moisture: ");
      Serial.println(moisture);

      String msg;

      DynamicJsonDocument jsonReadings(256); // Adjust the size according to your needs
      jsonReadings["node"] = nodeNumber;
      jsonReadings["light"] = light;
      jsonReadings["temperature"] = temperature;
      jsonReadings["moisture"] = moisture;

      serializeJson(jsonReadings, msg);

      mesh.sendSingle(masterNode, msg);

    } 
    else 
    {
      // Print error to the "debug" serial port
      Serial.print("deserializeJson() returned ");
      Serial.println(err.c_str());
  
      // Flush all bytes in the "link" serial port buffer
      while (Serial2.available() > 0)
        Serial2.read();
    }
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());

  if (msg.indexOf("master") != -1)
  // Allocate memory for the JSON document (adjust capacity as needed)
  {
    DynamicJsonDocument jsonReadings(256); // Example capacity of 256 bytes

  // Parse the received message into the JSON document
  auto error = deserializeJson(jsonReadings, msg.c_str());

  // Check for parsing errors
  if (error) {
    Serial.print("Error parsing JSON: ");
    Serial.println(error.c_str());
    return; // Handle error if parsing fails
  }

  // Access data using member functions (assuming correct key names)
  int node = jsonReadings["node"];
  double temp = jsonReadings["temp"];
  double hum = jsonReadings["hum"];

  Serial.print("Node: ");
  Serial.println(node);
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println(" %");

  M5.Lcd.setCursor(0, 40, 2);
  M5.Lcd.printf("Node %d", nodeNumber);
  }

  else{
    masterNode = static_cast<int>(from);
    Serial.println("The Master node is " + String(masterNode));
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void getNodeId(){
  nodeNumber = static_cast<int>(mesh.getNodeId());
}


void setup() {
  Serial.begin(115200);
  
  Serial2.begin(115200, SERIAL_8N1, 33,
                  32);

  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // getting its own id
  getNodeId();

  // adding task to scheduler to send messages to master node
  // userScheduler.addTask(taskSendMessage);
  // taskSendMessage.enable();

  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("Node %d", nodeNumber);

}

void loop() {
  mesh.update();

  if (Serial2.available()) 
  {
    sendMessage();
  }
}
