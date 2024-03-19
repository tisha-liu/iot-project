#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <M5StickCPlus.h>


// MESH Details
#define   MESH_PREFIX     "T21MESH" //name for your MESH
#define   MESH_PASSWORD   "keemstar" //password for your MESH
#define   MESH_PORT       5555 //default port

//Number for this node
int nodeNumber;

//String to send to other nodes with sensor readings
String readings;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


// Needed for painless library
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());

  // Allocate memory for the JSON document (adjust capacity as needed)
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
  // int node = jsonReadings["node"];
  // double temp = jsonReadings["temp"];
  // double hum = jsonReadings["hum"];

  int node = jsonReadings["node"];
  const char* light = jsonReadings["light"].as<const char*>();
  const char* temperature = jsonReadings["temperature"].as<const char*>();
  const char* moisture = jsonReadings["moisture"].as<const char*>();

  M5.Lcd.setCursor(0, 20, 2);
  M5.Lcd.printf("Node %d", node);
  Serial.print("Node: ");
  Serial.println(node);

  M5.Lcd.setCursor(0, 40, 2);
  M5.Lcd.printf("Light: ");
  M5.Lcd.printf(light);
  Serial.print("Light: ");
  Serial.println(light);

  M5.Lcd.setCursor(0, 60, 2);
  M5.Lcd.printf("Temperature: ");
  M5.Lcd.printf(temperature);
  M5.Lcd.printf(" C");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  M5.Lcd.setCursor(0, 80, 2);
  M5.Lcd.printf("Moisture");
  M5.Lcd.printf(moisture);
  Serial.print("Moisture: ");
  Serial.print(moisture);
  Serial.println(" %");

  StaticJsonDocument<256> doc;
  doc["node"] = node;
  doc["light"] = light;
  doc["temperature"] = temperature;
  doc["moisture"] = moisture;

  // Send the JSON document over the "link" serial port
  serializeJson(doc, Serial2);
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
  declareMaster();
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  std::list<uint32_t> nodeList = mesh.getNodeList();
  Serial.println("Connected Nodes:");
  for (const auto& connectedNode : nodeList) {
    Serial.println(connectedNode);
  }
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void declareMaster(){
  Serial.print("Declaring its the master node");
  mesh.sendBroadcast("MASTER");
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 32,
                  33);


  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.printf("Node %d", nodeNumber);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  nodeNumber = mesh.getNodeId();
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  declareMaster();
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}
