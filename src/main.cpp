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

#define STASSID "***REMOVED***"
#define STAPSK  "***REMOVED***"

#define HTTPPORT 80

const IPAddress IP(192, 168, 1, 139);
const IPAddress GATEWAY(192, 168, 1, 1);
const IPAddress SUBNET(255, 255, 255, 0);

AsyncWebServer SERVER(HTTPPORT);

bool LEDS_ON = true;
bool RESPONSIVE_MODE = true;

long STATIC_R_VALUE = 0;
long STATIC_G_VALUE = 0;
long STATIC_B_VALUE = 0;

long RESPONSIVE_R_VALUE = 0;
long RESPONSIVE_G_VALUE = 0;
long RESPONSIVE_B_VALUE = 0;

void turn_off_leds()
{
  analogWrite(R_LED_PIN, 0);
  analogWrite(G_LED_PIN, 0);
  analogWrite(B_LED_PIN, 0);
}

void display_color()
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

void set_led_state(String state)
{
  LEDS_ON = state == "true";
  if (!LEDS_ON)
  {
    turn_off_leds();
  }
}

String get_led_state(){
  return LEDS_ON ? "true" : "false";
}

void set_led_mode(String mode){
    RESPONSIVE_MODE = mode == "RESPONSIVE";
}

String get_led_mode(){
  return RESPONSIVE_MODE ? "RESPONSIVE" : "STATIC";
}

void set_color(long r, long g, long b)
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

void get_color(long* r, long* g, long* b){
  if (RESPONSIVE_MODE){
    *r = RESPONSIVE_R_VALUE;
    *g = RESPONSIVE_G_VALUE;
    *b = RESPONSIVE_B_VALUE;
  }
  else{
    *r = STATIC_R_VALUE;
    *g = STATIC_G_VALUE;
    *b = STATIC_B_VALUE;
  }
}

void handleSettingLEDState(AsyncWebServerRequest *request)
{
  if (!request->hasParam("state"))
  {
    request->send(400);
  }
  else
  {
    String stateValue = request->getParam("state")->value();
    set_led_state(stateValue);
    request->send(204);
  }
}

void handleGettingLEDState(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", get_led_state());
}

void handleColorSetting(AsyncWebServerRequest *request)
{
  unsigned long start = micros();
  set_color(255, 255, 255);
  unsigned long end = micros();
  unsigned long delta = end - start;
  Serial.println(delta);
  request->send(204);
}

void handleColorGetting(AsyncWebServerRequest *request)
{
  long r, g, b;
  get_color(&r ,&g, &b);

  String message = "R" + String(r) + "G" + String(g) + "B" + String(b);
  request->send(200, "text/plain", message);
}

void handleSettingLEDMode(AsyncWebServerRequest *request)
{
  if (!request->hasParam("mode"))
  {
    request->send(400);
  }
  else
  {
    String modeValue = request->getParam("mode")->value();
    set_led_mode(modeValue);
    request->send(204);
  }
}

void handleGettingLEDMode(AsyncWebServerRequest *request)
{
  request->send(200, "text/plain", get_led_mode());
}

void setup_pins()
{
  pinMode(R_LED_PIN, OUTPUT);
  pinMode(G_LED_PIN, OUTPUT);
  pinMode(B_LED_PIN, OUTPUT);
}

void connect_to_wifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.config(IP, GATEWAY, SUBNET);
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}

void setup_ArduinoOTA(){
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("***REMOVED***");

  // Password can be set with it's md5 value as well
  // MD5(***REMOVED***) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
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
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  
  ArduinoOTA.begin();
}

void setup_server()
{

  SERVER.on("/api/setState", HTTP_GET, handleSettingLEDState);
  SERVER.on("/api/getState", handleGettingLEDState);

  SERVER.on("/api/setColor", handleColorSetting);
  SERVER.on("/api/static/getColor", handleColorGetting);

  SERVER.on("/api/setMode", handleSettingLEDMode);
  SERVER.on("/api/getMode", handleGettingLEDMode);

  SERVER.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  SERVER.serveStatic("/CSS/", LittleFS, "/CSS/");
  SERVER.serveStatic("/JS/", LittleFS, "/JS/");

  SERVER.begin();
}

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Booting");

  LittleFS.begin();

  setup_pins();

  connect_to_wifi();

  //setup_ArduinoOTA();

  setup_server();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}
void loop(void)
{
  //ArduinoOTA.handle();

  if (RESPONSIVE_MODE)
  {
    delay(0);
  }
  if (LEDS_ON)
  {
    display_color();
  }
}
