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
#define W_LED_PIN D2

#define HOSTNAME "esp8266"

#define ROOM "240"
#define NEXT_ROOM "241"
#define NEXT_ROOM_URL ""

#define STASSID "***REMOVED***"
#define STAPASS "***REMOVED***"

#define OTAPASS "***REMOVED***"       // ! Change it to more secure
#define HTTP_USERNAME "***REMOVED***" // ! Change it to more secure
#define HTTP_PASSWORD "***REMOVED***"  // ! Change it to more secure

#define APIENDPOINT "/api"

#define HTTPPORT 80 // * HTTPS PORT 443
#define OTAPORT 8069
#define UDPPORT 8888

AsyncWebServer SERVER(HTTPPORT);
AsyncWebSocket SERVER_WEBSOCKET(APIENDPOINT);
WiFiUDP UDPSERVER;

IPAddress CONNECTED_AUDIO_IP;

bool LEDS_ON = true;
bool RESPONSIVE_MODE = true;

int validateNumber(int number)
{
  if (number > 255)
  {
    return 255;
  }
  else if (number < 0)
  {
    return 0;
  }
  return number;
}

struct colors
{
  int r = 0;
  int g = 0;
  int b = 0;
  int w = 0;

  String getColorCode()
  {
    return "R" + String(r) + "G" + String(g) + "B" + String(b) + "W" + String(w) + "E";
  }

  void setColors(int r_value, int g_value, int b_value, int w_value)
  {
    r = r_value;
    g = g_value;
    b = b_value;
    w = w_value;
  }

  void setColorsFromCode(String code)
  {
    int code_starts = code.indexOf('R');
    int red_ends = code.indexOf('G');
    int green_ends = code.indexOf('B');
    int blue_ends = code.indexOf('W');
    int code_ends = code.indexOf('E');

    int r_value = code.substring(code_starts + 1, red_ends).toInt();
    int g_value = code.substring(red_ends + 1, green_ends).toInt();
    int b_value = code.substring(green_ends + 1, blue_ends).toInt();
    int w_value = code.substring(blue_ends + 1, code_ends).toInt();

    setColors(r_value, g_value, b_value, w_value);
  }
} RESPONSIVE_SKEWING_VALUES, RESPONSIVE_COLOR_VALUES, STATIC_COLOR_VALUES;

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
  analogWrite(W_LED_PIN, 0);
}

void displayColor()
{
  if (RESPONSIVE_MODE)
  {
    analogWrite(R_LED_PIN, validateNumber(RESPONSIVE_COLOR_VALUES.r + RESPONSIVE_SKEWING_VALUES.r));
    analogWrite(G_LED_PIN, validateNumber(RESPONSIVE_COLOR_VALUES.g + RESPONSIVE_SKEWING_VALUES.g));
    analogWrite(B_LED_PIN, validateNumber(RESPONSIVE_COLOR_VALUES.b + RESPONSIVE_SKEWING_VALUES.b));
    analogWrite(W_LED_PIN, validateNumber(RESPONSIVE_COLOR_VALUES.w + RESPONSIVE_SKEWING_VALUES.w));
  }
  else
  {
    analogWrite(R_LED_PIN, STATIC_COLOR_VALUES.r);
    analogWrite(G_LED_PIN, STATIC_COLOR_VALUES.g);
    analogWrite(B_LED_PIN, STATIC_COLOR_VALUES.b);
    analogWrite(W_LED_PIN, STATIC_COLOR_VALUES.w);
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

void setColor(String code)
{
  if (RESPONSIVE_MODE)
  {
    RESPONSIVE_SKEWING_VALUES.setColorsFromCode(code);
  }
  else
  {
    STATIC_COLOR_VALUES.setColorsFromCode(code);
  }
}

colors getColors()
{
  if (RESPONSIVE_MODE)
  {
    return RESPONSIVE_SKEWING_VALUES;
  }
  else
  {
    return STATIC_COLOR_VALUES;
  }
}

void setAudioIP(String ip)
{
  IPAddress new_ip;
  new_ip.fromString(ip);
  CONNECTED_AUDIO_IP = new_ip;
}

String getAudioIP()
{
  String ip = CONNECTED_AUDIO_IP.toString();
  if (ip == "(IP unset)" || ip == "")
  {
    return "none";
  }
  else
  {
    return ip;
  }
}

String requestProcessor(const String &var)
{
  if (var == "ROOM")
  {
    return F(ROOM);
  }
  else if (var == "NEXT_ROOM")
  {
    return F(NEXT_ROOM);
  }
  else if (var == "NEXT_ROOM_URL")
  {
    return F(NEXT_ROOM_URL);
  }

  return String();
}

void APItextAllExceptClient(AsyncWebSocketClient *client, String text)
{
  for (const auto &c : SERVER_WEBSOCKET.getClients())
  {
    if (c.status() == WS_CONNECTED && c.id() != client->id())
    {
      SERVER_WEBSOCKET.text(c.id(), text);
    }
  }
}

void APIonDataReceived(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = '\0';
    String data_string = String((char *)data);

    int splitting_index = data_string.indexOf('=');
    String parameter = data_string.substring(0, splitting_index);
    String value = data_string.substring(splitting_index + 1);

    if (parameter == "COLOR")
    {
      setColor(value);
      APItextAllExceptClient(client, "COLOR=" + getColors().getColorCode());
    }
    else if (parameter == "MODE")
    {
      setMode(value);
      APItextAllExceptClient(client, "MODE=" + getMode());
      SERVER_WEBSOCKET.textAll("COLOR=" + getColors().getColorCode());
    }
    else if (parameter == "STATE")
    {
      setState(value);
      APItextAllExceptClient(client, "STATE=" + getState());
    }
    else if (parameter == "AUDIO_SOURCE")
    {
      setAudioIP(value);
      APItextAllExceptClient(client, "AUDIO_SOURCE=" + getAudioIP());
    }
  }
}

void APIonNewConnection(AsyncWebSocketClient *client)
{
  client->text("STATE=" + getState());
  client->text("MODE=" + getMode());
  client->text("COLOR=" + getColors().getColorCode());
  client->text("AUDIO_SOURCE=" + getAudioIP());
}

void handleAPI(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
               void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_DATA:
    APIonDataReceived(client, arg, data, len);
    break;
  case WS_EVT_CONNECT:
    Serial.printf("Client %s connected to the API with an ID of #%u!\n", client->remoteIP().toString().c_str(), client->id());
    APIonNewConnection(client);
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("Client #%u disconnected!\n", client->id());
    break;
  case WS_EVT_PONG:
    Serial.printf("Pong! ID:#%u!\n", client->id());
    break;
  case WS_EVT_ERROR:
    Serial.printf("Error %u %s! Client ID:#%u!\n", *((uint16_t *)arg), (char *)data, client->id());
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
                   const char *ip_str = NULL, const char *gateway_str = NULL, const char *subnet_str = NULL)
{
  Serial.print("Connecting to WiFi");

  WiFi.mode(WIFI_STA);
  if (ip_str && gateway_str && subnet_str)
  {;
    IPAddress ip, gateway, subnet;
    ip.fromString(ip_str);
    gateway.fromString(gateway_str);
    subnet.fromString(subnet_str);

    WiFi.config(ip, gateway, subnet);
  }

  WiFi.begin(stassid, stapsk);

  esp8266::polledTimeout::oneShot timeout(30000); // number of milliseconds to wait before returning timeout error
  while (!timeout)
  {
    Serial.print(".");
    yield();
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println();
      break;
    }
    delay(500);
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Timeout! Connection Failed!");
    restart();
  }
  else
  {
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

void setupArduinoOTA(const int port = 0, const char *pass = NULL, const char *hostname = NULL)
{
  Serial.println("Setting up ArduinoOTA...");

  // * Port defaults to 8266, pass to null and hostname to esp8266-xxxxxx
  if (port)
  {
    ArduinoOTA.setPort(port);
  }
  if (hostname)
  {
    ArduinoOTA.setHostname(hostname);
  }
  if (pass)
  {
    ArduinoOTA.setPassword(pass);
  }

  ArduinoOTA.onStart([]() {
    String type = ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem";

    LittleFS.end();
    SERVER_WEBSOCKET.textAll("OTA update incoming! Shutting server down!");
    SERVER_WEBSOCKET.enable(false);
    SERVER_WEBSOCKET.closeAll();

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
  Serial.println("Setting up the HTTP and API endpoint...");

  SERVER_WEBSOCKET.onEvent(handleAPI);

  SERVER_WEBSOCKET.setAuthentication(HTTP_USERNAME, HTTP_PASSWORD);

  SERVER.addHandler(&SERVER_WEBSOCKET);

  SERVER.on("/", [](AsyncWebServerRequest *request) {
    if (!request->authenticate(HTTP_USERNAME, HTTP_PASSWORD))
      return request->requestAuthentication();
    else
    {
      request->send(LittleFS, "/room.html", String(), false, requestProcessor);
    }
  });

  SERVER.serveStatic("/", LittleFS, "/");

}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Booting...");

  setupPins();
  setupLittleFS();

  connectToWiFi(STASSID, STAPASS);

  setupArduinoOTA(OTAPORT, OTAPASS, HOSTNAME);
  setupServer();

  ArduinoOTA.begin();
  UDPSERVER.begin(UDPPORT);
  SERVER.begin();

  Serial.print("Ready!");
  Serial.print(" IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print(" Hostname: ");
  Serial.println(ArduinoOTA.getHostname());
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
      packet[packetSize] = '\0';
      UDPSERVER.read(packet, packetSize);
      RESPONSIVE_COLOR_VALUES.setColorsFromCode(String(packet));
    }
  }
  if (LEDS_ON)
  {
    displayColor();
  }
}
