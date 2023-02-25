#include <WiFi.h>
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>

const int EEPROM_SIZE = 512;
const int SSID_SIZE = 32;
const int PASSWORD_SIZE = 32;
const char* ACCESS_POINT_NAME = "ESP32AP";
const char* ACCESS_POINT_PASSWORD = "password";

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

  for (int i = 0; i < SSID_SIZE; i++) {
    ssid += char(EEPROM.read(i));
  }

  for (int i = 0; i < PASSWORD_SIZE; i++) {
    password += char(EEPROM.read(i + SSID_SIZE));
  }

  if (ssid != "" && password != "") {
    Serial.print("Saved WiFi credentials found. SSID: ");
    Serial.print(ssid);
    Serial.print(", Password: ");
    Serial.println(password);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Connecting to WiFi...");

    // Wait for WiFi to connect, or timeout after 30 seconds
    int wifiTimeout = 30000;
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < wifiTimeout) {
      Serial.print(".");
      delay(1000);
    }

    // If WiFi connection was successful, print out the IP address assigned to the device
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.print("WiFi connected. IP address: ");
      Serial.println(WiFi.localIP());
    }
    // If WiFi connection was not successful, start the access point
    else {
      Serial.println("");
      Serial.println("WiFi connection failed. Starting access point...");
      startAccessPoint();
    }
  }
  else {
    Serial.println("No saved WiFi credentials found. Starting access point...");
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
  String ssid = WiFi.SSID();
  String html = "<html><body>";
  html += "<h1>WiFi Credentials</h1>";
  html += "<form action=\"/save\">";
  html += "<label for=\"ssid\">SSID:</label><br>";
  html += "<input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"" + ssid + "\"><br>";
  html += "<label for=\"password\">Password:</label><br>";
  html += "<input type=\"password\" id=\"password\" name=\"password\"><br><br>";
  html += "<input type=\"submit\" value=\"Save\">";
  html += "</form>";
  html += "</body></html>";
  request->send(200, "text/html", html);
}


void handleSave(AsyncWebServerRequest *request) {
  String ssid = request->arg("ssid");
  String password = request->arg("password");
     // Add null characters to the end of the SSID and password strings
  ssid += '\0';
  password += '\0';
  Serial.println("Saving WiFi credentials...");
  for (int i = 0; i < ssid.length(); i++) {
    EEPROM.write(i, ssid[i]);
  }
  for (int i = 0; i < password.length(); i++) {
    EEPROM.write(i + SSID_SIZE, password[i]);
  }

  EEPROM.commit();
  Serial.println("Saved WiFi credentials. Restarting...");
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
