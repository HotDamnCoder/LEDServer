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
AsyncWebSocket SERVER_WEBSOCKET("/");
WiFiUDP UDPSERVER;

IPAddress CONNECTED_AUDIO_IP;

bool LEDS_ON = true;
bool RESPONSIVE_MODE = true;

struct colors
{
  int r = 0;
  int g = 0;
  int b = 0;

  String getColorCode()
  {
    return "R" + String(r) + "G" + String(g) + "B" + String(b) + "E";
  }

  void setColors(int r_value, int g_value, int b_value)
  {
    r = r_value;
    g = g_value;
    b = b_value;
  }

  void setColorsFromCode(String code)
  {
    int code_starts = code.indexOf('R');
    int red_ends = code.indexOf('G');
    int green_ends = code.indexOf('B');
    int code_ends = code.indexOf('E');

    r = code.substring(code_starts + 1, red_ends).toInt();
    g = code.substring(red_ends + 1, green_ends).toInt();
    b = code.substring(green_ends + 1, code_ends).toInt();
  }
} RESPONSIVE_COLOR_VALUES, STATIC_COLOR_VALUES;

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
    analogWrite(R_LED_PIN, RESPONSIVE_COLOR_VALUES.r);
    analogWrite(G_LED_PIN, RESPONSIVE_COLOR_VALUES.g);
    analogWrite(B_LED_PIN, RESPONSIVE_COLOR_VALUES.b);
  }
  else
  {
    analogWrite(R_LED_PIN, STATIC_COLOR_VALUES.r);
    analogWrite(G_LED_PIN, STATIC_COLOR_VALUES.g);
    analogWrite(B_LED_PIN, STATIC_COLOR_VALUES.b);
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

void setColor(int r, int g, int b)
{
  if (RESPONSIVE_MODE)
  {
    RESPONSIVE_COLOR_VALUES.setColors(r, g, b);
  }
  else
  {
    STATIC_COLOR_VALUES.setColors(r, g, b);
  }
}

void setColor(String code)
{
  if (RESPONSIVE_MODE)
  {
    RESPONSIVE_COLOR_VALUES.setColorsFromCode(code);
  }
  else
  {
    STATIC_COLOR_VALUES.setColorsFromCode(code);
  }
}

void setAudioIP(String ip){
  IPAddress new_ip;
  new_ip.fromString(ip);
  CONNECTED_AUDIO_IP = new_ip;
}
String getAudioIP(){
  return CONNECTED_AUDIO_IP.toString() == "(IP unset)" ? "none" : CONNECTED_AUDIO_IP.toString();
}
colors getColors()
{
  if (RESPONSIVE_MODE)
  {
    return RESPONSIVE_COLOR_VALUES;
  }
  else
  {
    return STATIC_COLOR_VALUES;
  }
}

void textAllExceptClient(AsyncWebSocketClient *client, String text)
{
  for (const auto &c : SERVER_WEBSOCKET.getClients())
  {
    if (c->status() == WS_CONNECTED && c->id() != client->id())
    {
      SERVER_WEBSOCKET.text(c->id(), text);
    }
  }
}

void handleIncomingData(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    String data_string = String((char *)data);

    int splitting_index = data_string.indexOf('=');
    String parameter = data_string.substring(0, splitting_index);
    String value = data_string.substring(splitting_index + 1);

    if (parameter == "COLOR")
    {
      setColor(value);
      textAllExceptClient(client, "COLOR=" + getColors().getColorCode());
    }
    else if (parameter == "MODE")
    {
      setMode(value);
      textAllExceptClient(client, "MODE=" + getMode());
      textAllExceptClient(client, "COLOR=" + getColors().getColorCode());
    }
    else if (parameter == "STATE")
    {
      setState(value);
      textAllExceptClient(client, "STATE=" + getState());
    }
    else if (parameter == "AUDIO_SOURCE"){
      setAudioIP(value);
      textAllExceptClient(client, "AUDIO_SOURCE=" + getAudioIP());
    }
  }
}

void handleWebsocket(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                     void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_DATA:
    handleIncomingData(client, arg, data, len);
    break;
  case WS_EVT_CONNECT:
    Serial.printf("Client %s connected with an ID of %u!\n", client->remoteIP().toString().c_str(), client->id());
    client->text("STATE=" + getState());
    client->text("MODE=" + getMode());
    client->text("COLOR=" + getColors().getColorCode());
    client->text("AUDIO_SOURCE=" + getAudioIP());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("Client %u disconnected!\n", client->id());
    break;
  case WS_EVT_PONG:
    Serial.printf("Pong! ID:%u!\n", client->id());
    break;
  case WS_EVT_ERROR:
    Serial.printf("Error %u %s! Client ID:%u!\n", *((uint16_t *)arg), (char *)data, client->id());
    break;
  }
}

void setupPins()
{
  Serial.println("Setting up the pins...");
  pinMode(R_LED_PIN, OUTPUT);
  pinMode(G_LED_PIN, OUTPUT);
  pinMode(B_LED_PIN, OUTPUT);
}

void connectToWiFi(const char *stassid, const char *stapsk,
                   const char *ip_str, const char *gateway_str, const char *subnet_str)
{
  Serial.println("Connecting to WiFi...");

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
  Serial.println("Setting up the LittleFS filesystem...");

  if (!LittleFS.begin())
  {
    Serial.println("LittleFS mount failed");
    restart();
  }
}

void setupArduinoOTA(const int port, const char *pass, const char *hostname)
{
  Serial.println("Setting up ArduinoOTA...");

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
  Serial.println("Setting up the HTTP and WebSocket server...");

  SERVER_WEBSOCKET.onEvent(handleWebsocket);

  SERVER.addHandler(&SERVER_WEBSOCKET);

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

  Serial.print("Ready!");
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());
}

void loop(void)
{
  SERVER_WEBSOCKET.cleanupClients();
  ArduinoOTA.handle();
  if (RESPONSIVE_MODE)
  {
    int packetSize = UDPSERVER.parsePacket();
    if (packetSize && UDPSERVER.remoteIP() == CONNECTED_AUDIO_IP)
    {
      char packet[packetSize];
      UDPSERVER.read(packet, packetSize);
      setColor(String(packet));
      SERVER_WEBSOCKET.textAll("COLOR=" + getColors().getColorCode());
    }
  }
  if (LEDS_ON)
  {
    displayColor();
  }
}
