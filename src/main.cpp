#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>

#define R_LED_PIN D5
#define G_LED_PIN D6
#define B_LED_PIN D8

#define HOSTNAME "esp8266"
#define STASSID "***REMOVED***"
#define STAPSK "***REMOVED***"
#define OTAPASS "***REMOVED***"

#define IP "192.168.1.139"
#define GATEWAY "192.168.1.1"
#define SUBNET "255.255.255.0"

#define HTTPPORT 80
#define OTAPORT 8069
#define UDPPORT 8888

AsyncWebServer SERVER(HTTPPORT);
WiFiUDP UDPSERVER;

bool LEDS_ON = true;
bool RESPONSIVE_MODE = true;

long STATIC_R_VALUE = 0;
long STATIC_G_VALUE = 0;
long STATIC_B_VALUE = 0;

long RESPONSIVE_R_VALUE = 0;
long RESPONSIVE_G_VALUE = 0;
long RESPONSIVE_B_VALUE = 0;

void restart()
{
  Serial.println("Rebooting...");
  delay(5000);
  ESP.restart();
}

void turnOffLeds()
{
  analogWrite(R_LED_PIN, 0);
  analogWrite(G_LED_PIN, 0);
  analogWrite(B_LED_PIN, 0);
}

void displayColor()
{
  if (RESPONSIVE_MODE)
  {
    analogWrite(R_LED_PIN, RESPONSIVE_R_VALUE);
    analogWrite(G_LED_PIN, RESPONSIVE_G_VALUE);
    analogWrite(B_LED_PIN, RESPONSIVE_B_VALUE);
  }
  else
  {
    analogWrite(R_LED_PIN, STATIC_R_VALUE);
    analogWrite(G_LED_PIN, STATIC_G_VALUE);
    analogWrite(B_LED_PIN, STATIC_B_VALUE);
  }
}

void setState(String state)
{
  LEDS_ON = state == "true";
  if (!LEDS_ON)
  {
    turnOffLeds();
  }
}

String getState()
{
  return LEDS_ON ? "true" : "false";
}

void setMode(String mode)
{
  RESPONSIVE_MODE = mode == "RESPONSIVE";
}

String getMode()
{
  return RESPONSIVE_MODE ? "RESPONSIVE" : "STATIC";
}

void setColor(long r, long g, long b)
{
  if (RESPONSIVE_MODE)
  {
    RESPONSIVE_R_VALUE = r;
    RESPONSIVE_G_VALUE = g;
    RESPONSIVE_B_VALUE = b;
  }
  else
  {
    STATIC_R_VALUE = r;
    STATIC_G_VALUE = g;
    STATIC_B_VALUE = b;
  }
}

void getColor(long *r, long *g, long *b)
{
  if (RESPONSIVE_MODE)
  {
    *r = RESPONSIVE_R_VALUE;
    *g = RESPONSIVE_G_VALUE;
    *b = RESPONSIVE_B_VALUE;
  }
  else
  {
    *r = STATIC_R_VALUE;
    *g = STATIC_G_VALUE;
    *b = STATIC_B_VALUE;
  }
}

void setColorFromBuffer(char* packet)
{

  long r, g, b;
  String packet_string = String(packet);

  int packet_starts = packet_string.indexOf('R');
  int red_ends = packet_string.indexOf('G');
  int green_ends = packet_string.indexOf('B');
  int packet_ends = packet_string.indexOf('E');

  r = packet_string.substring(packet_starts + 1, red_ends).toInt();
  g = packet_string.substring(red_ends + 1, green_ends).toInt();
  b = packet_string.substring(green_ends + 1, packet_ends).toInt();

  setColor(r, g, b);
}

void setupPins()
{
  pinMode(R_LED_PIN, OUTPUT);
  pinMode(G_LED_PIN, OUTPUT);
  pinMode(B_LED_PIN, OUTPUT);
}

void connectToWiFi(const char *stassid, const char *stapsk,
                   const char *ip_str, const char *gateway_str, const char *subnet_str)
{
  WiFi.mode(WIFI_STA);

  IPAddress ip, gateway, subnet;

  ip.fromString(ip_str);
  gateway.fromString(gateway_str);
  subnet.fromString(subnet_str);

  WiFi.config(ip, gateway, subnet);
  WiFi.begin(stassid, stapsk);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed!");
    restart();
  }
}

void setupLittleFS()
{
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS mount failed");
    restart();
  }
}

void setupArduinoOTA(const int port, const char* pass, const char* hostname)
{
  ArduinoOTA.setPort(port);
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPassword(pass);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
}

void setupServer()
{

  SERVER.on("/api/setState", [](AsyncWebServerRequest *request) {
    if (request->hasParam("state"))
    {
      String stateValue = request->getParam("state")->value();
      setState(stateValue);
      request->send(204);
    }
    else
    {
      request->send(400);
    }
  });

  SERVER.on("/api/getState", [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getState());
  });

  SERVER.on("/api/setColor", [](AsyncWebServerRequest *request) {
    if (request->hasParam("R") && request->hasParam("G") && request->hasParam("B"))
    {
      long r, g, b;
      getColor(&r, &g, &b);

      r = request->getParam("R")->value().toInt();
      g = request->getParam("G")->value().toInt();
      b = request->getParam("B")->value().toInt();

      setColor(r, g, b);

      request->send(204);
    }
    else
    {
      request->send(400);
    }
  });

  SERVER.on("/api/static/getColor", [](AsyncWebServerRequest *request) {
    long r, g, b;
    getColor(&r, &g, &b);

    String message = "R" + String(r) + "G" + String(g) + "B" + String(b);
    request->send(200, "text/plain", message);
  });

  SERVER.on("/api/setMode", [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode"))
    {
      String modeValue = request->getParam("mode")->value();
      setMode(modeValue);
      request->send(204);

    }
    else
    {
      request->send(400);
    }
  });

  SERVER.on("/api/getMode", [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", getMode());
  });

  SERVER.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Booting...");

  setupPins();
  setupLittleFS();

  connectToWiFi(STASSID, STAPSK, IP, GATEWAY, SUBNET);

  setupArduinoOTA(OTAPORT, OTAPASS, HOSTNAME);
  setupServer();

  ArduinoOTA.begin();
  UDPSERVER.begin(UDPPORT);
  SERVER.begin();

  Serial.println("Ready!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
void loop(void)
{
  ArduinoOTA.handle();

  if (RESPONSIVE_MODE)
  {
    int packetSize = UDPSERVER.parsePacket();
    if (packetSize)
    {
      char packet[packetSize];
      UDPSERVER.read(packet, packetSize);
      setColorFromBuffer(packet);
    }
  }
  if (LEDS_ON)
  {
    displayColor();
  }
}
