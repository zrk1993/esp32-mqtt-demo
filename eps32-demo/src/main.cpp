using namespace std;

#include <WiFi.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <WebServer.h>
#include<string.h>
#include <list>
#include <PubSubClient.h>


const int ledPin = 13;

unsigned long previousMillis = 0;

const char* ssid = "helloword";
const char* passphrase = "zxcvbnm8";
const char* mqtt_server = "192.168.31.246";

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

unsigned long WiFiPreviousMillis = 0;

WiFiClient espClient;
PubSubClient mqttclient(espClient);

list<String> displayLines(3);

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);

WebServer server(80);

void display (String str) {
  Serial.println(str);
  displayLines.push_back(str);
  if (displayLines.size() > 3) {
    displayLines.pop_front();
  }
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    size_t i = 0;
    auto it = displayLines.begin();
    while (it!=displayLines.end() && i < 3) {
      u8g2.setCursor(0, i * 22 + 20);
      u8g2.print(*it);
      ++it;
      ++i;
    }
  } while (u8g2.nextPage());
  delay(1000);
}

void display (int str) {
  char s[10]={"\0"};
  itoa(str, s, 10);
  display(s);
}

void initWiFi()
{
  WiFi.mode(WIFI_STA);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    display("wifi connected");
    delay(3000);
    display("IP address：");
    display(WiFi.localIP().toString());
  }, ARDUINO_EVENT_WIFI_STA_CONNECTED);

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
		display("wifi disconnect");
    }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.begin(ssid, passphrase);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display("connect...");
  }
  display("IP address：");
  display(WiFi.localIP().toString());
}

void handRoot()
{
  String ledStatus = server.arg("led");
  if (ledStatus == "on")
  {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
  display("req: led=" + ledStatus);
  server.send(200, "text/html", "hello,world");
}

void callback(char* topic, byte* payload, unsigned int length) {
  display("Message arrived [");
  display(topic);
  display("] ");
  for (int i = 0; i < length; i++) {
    display((char)payload[i]);
  }
  display("");

  // Switch on the LED if an 1 was received as first character
  // if ((char)payload[0] == '1') {
  //   digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
  //   // but actually the LED is on; this is because
  //   // it is active low on the ESP-01)
  // } else {
  //   digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  // }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    display("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttclient.connect(clientId.c_str())) {
      display("connected");
      // Once connected, publish an announcement...
      mqttclient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttclient.subscribe("inTopic");
    } else {
      display("failed, rc=");
      display(mqttclient.state());
      display(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  u8g2.begin();
  u8g2.enableUTF8Print(); // 使print支持UTF8字集

  initWiFi();

  server.on("/", handRoot);
  server.on("/ip", []() { server.send(200, "text/html", WiFi.localIP().toString()); });
  server.onNotFound([]() { server.send(404, "text/html", "Not Found"); });
  server.begin();

  mqttclient.setServer(mqtt_server, 1883);
  mqttclient.setCallback(callback);
  mqttclient.setKeepAlive(65535);
}

void loop()
{
  unsigned long currentMillis = millis();
  server.handleClient();

  if (WiFi.status() != WL_CONNECTED && currentMillis - WiFiPreviousMillis > 30000) {
    display("wifi reconnect...");
    WiFiPreviousMillis = currentMillis;
    WiFi.disconnect();
		WiFi.reconnect();
  }

  if (!mqttclient.connected()) {
    reconnect();
  }
  mqttclient.loop();

  unsigned long now = millis();
  if (now - previousMillis > 2000) {
    previousMillis = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    mqttclient.publish("outTopic", msg);
  }

  delay(300);
}