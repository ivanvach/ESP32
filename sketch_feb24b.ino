
// this code is working
// if WiFi is not connected or if there are no WiFi credentials in the EEPROM memory
// than the code starts AP with name ESP32AP and password: password
// at IP address 192.168.4.1 one can enter WiFi credentials
// Also it allows for entering MQTT credentials from the web form 
// it is public onGitHub under the name of ESP32



#include <WiFi.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>

const int EEPROM_SIZE = 512;
const int SSID_SIZE = 32;
const int PASSWORD_SIZE = 32;
const char* ACCESS_POINT_NAME = "ESP32AP";
const char* ACCESS_POINT_PASSWORD = "password";
const int MQTT_SERVER_SIZE = 64;
const int MQTT_PORT_SIZE = 6; // Max value is 65535, which is 5 characters
const int MQTT_USERNAME_SIZE = 32;
const int MQTT_PASSWORD_SIZE = 32;
#define DEFAULT_MQTT_PORT 1883



AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  checkWiFiCredentials();
  server.on("/", handleRoot);
  server.on("/save", handleSave);
  server.begin();
}

void checkWiFiCredentials() {
  String ssid = "";
  String password = "";
  String mqttServer = "";
  String mqttPort = "";
  String mqttUsername = "";
  String mqttPassword = "";
  // Read WiFi credentials from EEPROM
  for (int i = 0; i < SSID_SIZE; i++) {
    ssid += char(EEPROM.read(i));
  }

  for (int i = 0; i < PASSWORD_SIZE; i++) {
    password += char(EEPROM.read(i + SSID_SIZE));
  }

  // Read MQTT credentials from EEPROM
  for (int i = 0; i < MQTT_SERVER_SIZE; i++) {
    mqttServer += char(EEPROM.read(i + SSID_SIZE + PASSWORD_SIZE));
  }

  for (int i = 0; i < MQTT_PORT_SIZE; i++) {
    mqttPort += char(EEPROM.read(i + SSID_SIZE + PASSWORD_SIZE + MQTT_SERVER_SIZE));
  }

  for (int i = 0; i < MQTT_USERNAME_SIZE; i++) {
    mqttUsername += char(EEPROM.read(i + SSID_SIZE + PASSWORD_SIZE + MQTT_SERVER_SIZE + MQTT_PORT_SIZE));
  }

  for (int i = 0; i < MQTT_PASSWORD_SIZE; i++) {
    mqttPassword += char(EEPROM.read(i + SSID_SIZE + PASSWORD_SIZE + MQTT_SERVER_SIZE + MQTT_PORT_SIZE + MQTT_USERNAME_SIZE));
  }

  if (ssid != "" && password != "") {
    Serial.println("Saved WiFi credentials found. Connecting to WiFi...");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("WiFi Password: ");
    Serial.println(password);
    WiFi.begin(ssid.c_str(), password.c_str());
    delay(40000); // Wait 40 seconds for connection to be established
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected.");
    } else {
      Serial.println("Failed to connect to saved WiFi network. Clearing saved credentials...");
      clearCredentials();
    }
  } else {
    Serial.println("No saved WiFi credentials found.");
  }

Serial.print("SSID: ");
Serial.println(ssid);
Serial.print("WiFi Password: ");
Serial.println(password);

Serial.print("mqttServer1: ");
Serial.println(mqttServer);
Serial.print("mqttPort1: ");
Serial.println(mqttPort);
Serial.print("mqttUsername1: ");
Serial.println(mqttUsername);
Serial.print("mqttPassword1: ");
Serial.println(mqttPassword);
  
  // Check if MQTT credentials are present
  if (mqttServer != "" && mqttPort != "" && mqttUsername != "" && mqttPassword != "") {
    Serial.println("Saved MQTT credentials found.");
    // Connect to MQTT broker
  } else {
    Serial.println("No saved MQTT credentials found.");
  }

  if (ssid == "" || password == "") {
    Serial.println("Starting access point...");
    startAccessPoint();
  }
}





void startAccessPoint() {
  WiFi.softAP(ACCESS_POINT_NAME, ACCESS_POINT_PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access point IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_GET, handleSave);
  server.begin();

  Serial.println("Access point started.");
}

void handleRoot(AsyncWebServerRequest *request) {
  String html = "<form action=\"/save\">";
  html += "<label>WiFi SSID:</label><input type=\"text\" name=\"ssid\"><br>";
  html += "<label>WiFi Password:</label><input type=\"password\" name=\"password\"><br>";
  html += "<label>MQTT Broker:</label><input type=\"text\" name=\"mqttServer\"><br>";
  html += "<label>MQTT Port:</label><input type=\"text\" name=\"mqttPort\"><br>";
  html += "<label>MQTT Username:</label><input type=\"text\" name=\"mqttUsername\"><br>";
  html += "<label>MQTT Password:</label><input type=\"password\" name=\"mqttPassword\"><br>";
  html += "<input type=\"submit\" value=\"Save\">";
  html += "</form>";
  request->send(200, "text/html", html);
}



void handleSave(AsyncWebServerRequest *request) {
 
  Serial.print("Saving started....");
  
  String ssid = request->getParam("ssid")->value();
  String password = request->getParam("password")->value();
  String mqttServer = request->getParam("mqttServer")->value();
  String mqttPort = request->getParam("mqttPort")->value();
  String mqttUsername = request->getParam("mqttUsername")->value();
  String mqttPassword = request->getParam("mqttPassword")->value();


  request->send(303, "text/plain", "Credentials saved.");
  
Serial.print("mqttServer2: ");
Serial.println(mqttServer);
Serial.print("mqttPort2: ");
Serial.println(mqttPort);
Serial.print("mqttUsername2: ");
Serial.println(mqttUsername);
Serial.print("mqttPassword2: ");
Serial.println(mqttPassword);

  if (ssid.length() == 0 || password.length() == 0) {
    request->send(400, "text/plain", "SSID and password are required");
    return;
  }

  Serial.println("Saving WiFi credentials...");

// Clear previous wifi credentials

clearCredentials();
  
  for (int i = 0; i < ssid.length(); i++) {
    EEPROM.write(i, ssid[i]);
  }

  for (int i = 0; i < password.length(); i++) {
    EEPROM.write(i + SSID_SIZE, password[i]);
  }

// clear previous mqtt values

for (int i = SSID_SIZE + PASSWORD_SIZE; i < SSID_SIZE + PASSWORD_SIZE + MQTT_SERVER_SIZE + MQTT_PORT_SIZE + MQTT_USERNAME_SIZE + MQTT_PASSWORD_SIZE ; i++) {
    EEPROM.write(i, 0);}
 
  
  
  for (int i = 0; i < mqttServer.length(); i++) {
    EEPROM.write(i + SSID_SIZE + PASSWORD_SIZE, mqttServer[i]);
  }

  String mqttPortString = String(DEFAULT_MQTT_PORT);
  if (mqttPort.length() > 0) {
    mqttPortString = mqttPort;
  }

  int portSize = mqttPortString.length();
  for (int i = 0; i < portSize; i++) {
    EEPROM.write(i + SSID_SIZE + PASSWORD_SIZE + MQTT_SERVER_SIZE, mqttPortString[i]);
  }

  for (int i = 0; i < mqttUsername.length(); i++) {
    EEPROM.write(i + SSID_SIZE + PASSWORD_SIZE + MQTT_SERVER_SIZE + MQTT_PORT_SIZE, mqttUsername[i]);
  }

  for (int i = 0; i < mqttPassword.length(); i++) {
    EEPROM.write(i + SSID_SIZE + PASSWORD_SIZE + MQTT_SERVER_SIZE + MQTT_PORT_SIZE + MQTT_USERNAME_SIZE, mqttPassword[i]);
  }

  EEPROM.commit();

  Serial.println("Saved WiFi and MQTT credentials. Restarting...");
  delay(2000);
  ESP.restart();
}




void clearCredentials() {
  for (int i = 0; i < SSID_SIZE; i++) {
    EEPROM.write(i, 0);
  }

  for (int i = 0; i < PASSWORD_SIZE; i++) {
    EEPROM.write(i + SSID_SIZE, 0);
  }

  EEPROM.commit();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Starting access point...");
    startAccessPoint();
  }
  delay(1000);
}
