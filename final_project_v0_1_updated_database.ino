//**********************************************************************
//** GENERAL GLOBAL VARIABLE
//**********************************************************************
char lcdBuffer[512];
char uidCard[64];
char macDoor[64];
#define DOOR_CONTACT_PIN 2
#define LOCK_PIN 3  

int intervalMQTT = 0;
unsigned long currMillis, prevMillis;
unsigned long accessGrantedTime = 0; 
unsigned long unlockTime = 10000; 
const int doorHoldTimeLimit = 60000;
unsigned long doorHoldStartTime = 0;
int doorHoldViolation = 0;
volatile bool isDoorClosed = true;
volatile bool prevDoorState = false;
volatile bool doorStateChange = false;
//String doorStatus = "";
int doorClosed = 1;
char doorStatus[7] = "closed";
bool doorUnlocked = false;
int isAccessGranted = 0;
int isAccessDenied = 0;
int isNoCardFound = 0;
int accessStatus = 0;

char* apiDoorStatus = "/get_door_status.php?door_status=";
char apiSent[50];
//**********************************************************************
//** SWAP TO ESP32
//**********************************************************************
#include <stdio.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
int statusWiFi = WL_IDLE_STATUS;

//#include <WiFi.h>


//**********************************************************************
//** WIFI SETUPT
//**********************************************************************
#include "time.h"
#include <WiFiClient.h>
char ssid[] = "TP-LINK_D4D4";           
char pass[] = "79936941";       
IPAddress serverIP(192, 168, 0, 122);  // the IP address of the XAMPP server
const char* host = "192.168.0.122"; // For ease of use
const int port = 80;
WiFiClient client;

//**********************************************************************
//** CARD READER SETUP
//**********************************************************************
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
//String tagID = "";

//**********************************************************************
//** MQTT SETUP
//**********************************************************************

#include <Arduino_JSON.h>
#include <MQTT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <stdio.h>

WiFiUDP ntpUDP;
MQTTClient mqttClient;
const char* ntpServer = "time.google.com";
const int  mqttPort = 1883;
unsigned long epochTime;
const char mqttBroker[63] = "public.cloud.shiftr.io";
//const char mqttBroker[63] = "192.168.0.122";
NTPClient ntpClient(ntpUDP, "time.google.com", 0, 60000);
char mqttClienName[31] = "client_team11_door";

volatile bool timerFlagMQTT = false;
String mqttStringMessage;
long nmrMqttMessages = 0;
long epochTimeNTP = 0;


char topicPub[61]  = "";

JSONVar myJsonMqtt;
String myStringMqtt;

//**********************************************************************
//**********************************************************************
//**********************************************************************

void setup() {
  Serial.begin(9600);              
  while (!Serial);   
  pinMode(DOOR_CONTACT_PIN, INPUT_PULLUP);
  pinMode(LOCK_PIN, OUTPUT); 

  // setup Wifi
  Serial.println("Board is: Arduino");
  setupWifi();
  attachInterrupt(digitalPinToInterrupt(DOOR_CONTACT_PIN), doorStateChanged, CHANGE);

//  Serial.println("Board is: ESP32");
//  setupWifiEsp32();

  Serial.println("Connected to WiFi network");

  getMacWifiShieldMacRouterSS(macDoor);
  Serial.println("MAC: " + String(macDoor));

  // setup MQTT
  sprintf(topicPub, "%s", "ece508/project/team11/door1");
  sprintf(mqttClienName, "%s", "team11_door1");

  ntpClient.begin();

  mqttClient.begin(mqttBroker, client);
  mqttClient.onMessage(messageReceived);
  
  Serial.print("Time Synchronization NTP");
  epochTimeNTP = ntpClient.getEpochTime();
  while (epochTimeNTP > 1680300000) {    
    epochTimeNTP = ntpClient.getEpochTime();    
    Serial.print(".");
    delay(2000);
  }  
  Serial.println(ntpClient.getEpochTime());

  
//  setupMQTT();

  SPI.begin();          // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  
  Serial.println("Scan PICC to see UID, SAK, type, and data blocks...");
  prevMillis = millis();
}

void loop() {
//**********************************************************************

  mqttClient.loop();

  if (!mqttClient.connected()) {
    connectMqtt(mqttClienName);
  }
  currMillis = millis();
  // publish a message roughly every second.
  if (currMillis - prevMillis > 1000) {
    ntpClient.update();
    prevMillis = currMillis;
   
    intervalMQTT++;
    if (intervalMQTT >= 1) {   
      intervalMQTT = 0;    
      publishMQTT();
    }    
  }
//**********************************************************************
//  this is to reduce https requestfunction in the interrupt

//  if (digitalRead(2) != prevDoorState){
//    isDoorClosed = digitalRead(2) == LOW;
//    prevDoorState = digitalRead(2);
//    updateDoorStatus();
////    Serial.print("Door state changed to ");
////    Serial.println(isDoorClosed ? "closed" : "open");
//  }

  if (doorStateChange){
    doorStateChange = false;
    isDoorClosed = digitalRead(DOOR_CONTACT_PIN) == LOW;
    if (isDoorClosed){
      doorClosed = 1;
      strcpy(doorStatus, "Closed");    
    }else{
      doorClosed = 0;
      strcpy(doorStatus, "Open");
    }
    //updateDoorStatus();
  }
  
  checkForDoorHoldViolation();
  // Check if door is unlocked
  if (accessGrantedTime != 0 && millis() - accessGrantedTime > unlockTime) {
    lockDoor();
  }

  if (getID(uidCard)) {
    if (isDoorClosed){
      checkUIDExists(uidCard,macDoor);
      switch(accessStatus){
        case 1: 
          Serial.println("Access granted");
          unlockDoor();
        break;
        case 10:
          Serial.println("Access Denied");
        break;
        case 100:
          Serial.println("Access Denied::No_Card_Found");
        break;
        default:
          Serial.println("Unexpected Response!!!");
      }
    }else{
      if (!isDoorClosed){
        Serial.println("Door is opened. Close door before scan your card!!");
      }
    }
    delay(500);
  }


  
//  else{
//    Serial.print(".");
//  }
//**********************************************************************
  //delay(10000);
}

//**********************************************************************
//** When do we check Door Hold violation?
//** Card is swiped with access granted
//** door is open
//** no current doorHoldViolation
//**********************************************************************
void checkForDoorHoldViolation() {
  if (!isDoorClosed)// && !doorHoldViolation) {
  {
    if (doorHoldStartTime == 0) {
      doorHoldStartTime = millis();
    }
//    Serial.println("Timer = " + String(millis() - doorHoldStartTime));
    if (millis() - doorHoldStartTime >= doorHoldTimeLimit) {
      doorHoldViolation = 1;
      //Serial.println("Door hold violation.");
    }
  }
  // If the door contact is closed, reset the door hold start time and the door hold violation flag.
  else if (isDoorClosed) {
    doorHoldStartTime = 0;
    doorHoldViolation = 0;
  }
}

//**********************************************************************
//** @input: char [] 
//** @ouput: bool 
//** description: check if new card is present ang get the card's UID
//**********************************************************************

boolean getID(char *UID) 
{
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
  return false;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
  return false;
  }
  byte tagIdChar[4];
  for ( uint8_t i = 0; i < 4; i++) { 
    tagIdChar[i] = mfrc522.uid.uidByte[i];
  }
  sprintf(uidCard, "%02X%02X%02X%02X", tagIdChar[0], tagIdChar[1], tagIdChar[2],tagIdChar[3]);
  Serial.println("Card Scanned UID : " + String(uidCard));
  mfrc522.PICC_HaltA(); // Stop reading
  mfrc522.PCD_StopCrypto1(); // stop crypto
  return true;
}

//**********************************************************************
//** @input: String, String, char *
//** @ouput: N/A
//** description: 
//** Pass card's UID, MAC of door, get String of reponse from server
//** send infor to Apache's server (XAMPP) with api /access_door.php
//**********************************************************************
void checkUIDExists(String UID, String macDoor) {

  if (client.connect(serverIP, port)){
    Serial.println("Connected to server");
    client.print("GET /access_door.php?uid=" + UID + "&mac=" + macDoor + " HTTP/1.1\r\n"); // send the GET request
    client.print("Host: 192.168.0.122\r\n");
    client.print("Connection: close\r\n");
    client.print("\r\n");

    while (client.connected()) {
      int i = 0;
      while (client.available() && i < 1023) {
        lcdBuffer[i] = client.read();
        i++;
      }
      lcdBuffer[i] = '\0';
    }
    Serial.println();
    Serial.println(lcdBuffer);
    Serial.println("Connection closed after response");

    // extract the result from the response
    if (strstr(lcdBuffer, "Access_Granted")) {
      accessStatus = 1;
//      isAccessGranted = 1;
//      isAccessDenied = 0;
//      isNoCardFound = 0;
    }
    else if (strstr(lcdBuffer, "Access_Denied")) {
      accessStatus = 10;
//      isAccessGranted = 0;
//      isAccessDenied = 1;
//      isNoCardFound = 0;
    }
    else if (strstr(lcdBuffer, "No_Card_Found")) {
      accessStatus = 100;
//      isAccessGranted = 0;
//      isAccessDenied = 0;
//      isNoCardFound = 1;
    }
    
//    if (lcdBuffer.indexOf("Access_Granted") != -1) {
//      strcpy(res,"Access_Granted");
//    }
//    else if (lcdBuffer.indexOf("Access_Denied") != -1) {
//      strcpy(res,"Access_Denied");
//    }
//    else if (lcdBuffer.indexOf("No_Card_Found") != -1) {
//      strcpy(res,"No_Card_Found");
//    }  

    client.stop();
    delay(1000);   
       
  }else{
    Serial.println("Connection Failed - Scan card !!!");
  }
}

void doorStateChanged() {
//  isDoorClosed = digitalRead(DOOR_CONTACT_PIN) == LOW;
  doorStateChange = true;
//  updateDoorStatus();
}

void updateDoorStatus() {
  String doorStatus = isDoorClosed ? "Closed" : "Open";
  String postValue = "door_status=" + doorStatus;
  Serial.println("Door state changed to " + String(doorStatus));
  sprintf(apiSent, "%s%s",apiDoorStatus,doorStatus);
  Serial.println(String(apiSent));
  size_t apiSentLength = strlen(apiSent);
//  Serial.println("String length: " + String(apiSentLength));
  Serial.println("String length: " + String(postValue.length()));
  if (client.connect(serverIP, port)){
    // Send the POST request with the value
    client.println("POST /get_door_status.php HTTP/1.1");
    client.println("Host: 192.168.0.122");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(postValue.length());
//    client.println(apiSentLength);  
    client.println();
    client.println(postValue);
//    client.println( "door_status="+String(doorStatus));
 
    // wait for the server to respond
    while (client.connected()) {
      int i = 0;
      while (client.available() && i < 1023) {
        lcdBuffer[i] = client.read();
        i++;
      }
      lcdBuffer[i] = '\0';
    }
  
    Serial.println();
    Serial.println(lcdBuffer);
    Serial.println("Connection closed after response");
  
    client.stop();
    delay(1000);
  }else{
    Serial.println("Connection Failed - Door updated");
  }
}

void getMacWifiShieldMacRouterSS(char *macCombined) 
{
    byte macWifiShield[6];
    WiFi.macAddress(macWifiShield);
    sprintf(macCombined, "%02X%02X%02X%02X%02X%02X", macWifiShield[5], macWifiShield[4], macWifiShield[3],macWifiShield[2],macWifiShield[1], macWifiShield[0] );
};

void unlockDoor() {
  digitalWrite(LOCK_PIN, HIGH);  
  accessGrantedTime = millis(); 
  doorUnlocked = true; 
}

void lockDoor() {
  digitalWrite(LOCK_PIN, LOW);  
  accessGrantedTime = 0;  
  doorUnlocked = false;
}

void setupMQTT(){
  printf(topicPub, "%s", "ece508/project/door1");

  // setUP MQTT client 
  mqttClient.begin(mqttBroker, client);
  mqttClient.onMessage(messageReceived);
  // setup NTP
  ntpClient.begin();
  Serial.print("Time Synchronization NTP");
  epochTimeNTP = ntpClient.getEpochTime();
  while (epochTimeNTP > 1680300000) {    
    epochTimeNTP = ntpClient.getEpochTime();    
    Serial.print(".");
    delay(2000);
  }  
  Serial.println(ntpClient.getEpochTime()); 
}

void publishMQTT()
{
  nmrMqttMessages++;
  epochTimeNTP = ntpClient.getEpochTime();
  myJsonMqtt["doorStatus"] = doorStatus;
  myJsonMqtt["doorClosed"] = doorClosed;
  myJsonMqtt["board"] = String(macDoor);
  myJsonMqtt["rssi"] = WiFi.RSSI();
  myJsonMqtt["DoorHold"] = doorHoldViolation;
  myJsonMqtt["epoch"] = epochTimeNTP;
  
  mqttStringMessage = JSON.stringify(myJsonMqtt);
    
  mqttClient.publish(topicPub, mqttStringMessage);

  Serial.println("Publishing in " + String(topicPub) + " " + String(mqttStringMessage));
}

void connectMqtt(char *mqttClienName) 
{
  Serial.println("connectMqtt: Checking WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(1000);
  }
  Serial.println("connectMqtt: WiFi Ok...");
 
  Serial.println("connectMqtt: Checking MQTT...");
  while (!mqttClient.connect(mqttClienName, "public", "public")) {
    Serial.print("."); delay(1000);
  }
  Serial.println("connectMqtt: MQTT Ok...");
 
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}



//void setupWifiEsp32()
//{
//  WiFi.begin(ssid, pass);
//  while (WiFi.status() != WL_CONNECTED) {    
//    Serial.println("Attempting to connect to WiFi: ");   
//    delay(250);
//  }
//  // you're connected now, so print out the status:
//  printWifiStatus();
//}

void setupWifi()
{
    // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  String fv = WiFi.firmwareVersion();

  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {

    Serial.println("Please upgrade the firmware");

  }
  // attempt to connect to Wifi network:
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) { 
    Serial.println("Attempting to connect to SSID: " + String(ssid));      
    status = WiFi.begin(ssid, pass); delay(100);
  }
  // you're connected now, so print out the status:
  printWifiStatus();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
