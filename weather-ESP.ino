#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

const char* SSID = "";
const char* PSK = "";
const char* MQTT_BROKER = "";
const int PORT = 1883;

const float TEMP_SENS_OFFSET = -2;

const int DELAY = 2500;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);
	Serial.begin(115200);
	setup_wifi();
	client.setServer(MQTT_BROKER, PORT);
  connect_bme();
}

bool connect_bme(){
  static bool connected = false;
  if (!connected) {
    connected = bme.begin(0x76);
  }
  if (!connected) {
    snprintf (msg, 50, "Could not find BME280 sensor, check wiring!");
    //Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("homesens/wohnzimmer/message", msg);
  }
  return connected;
}

void setup_wifi() {
	delay(10);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(SSID);

	WiFi.begin(SSID, PSK);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void reconnect() {
	while (!client.connected()) {
		Serial.print("Reconnecting...");
		if (!client.connect("ESP8266Client_wohnzimmer")) {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" retrying in 5 seconds");
			delay(5000);
		}
	}
}
void loop() {
	if (!client.connected()) {
    digitalWrite(LED_BUILTIN, LOW);
		reconnect();
	}

  if (connect_bme()){
    digitalWrite(LED_BUILTIN, LOW);
    client.loop();

    // SEND REAL STUFF
    snprintf (msg, 50, "Alive since %ld milliseconds", millis());
    //Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("homesens/wohnzimmer/message", msg);

    // SEND IP ADDRESS
    char bufIp [20];
    IPAddress ipaddr = WiFi.localIP();
    sprintf(bufIp, "%d.%d.%d.%d", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
    client.publish("homesens/wohnzimmer/ip", bufIp);

    // SEND MAC ADDRESS
    String mac = String(WiFi.macAddress());
    char bufMac [20];
    mac.toCharArray(bufMac, 20);
    client.publish("homesens/wohnzimmer/mac", bufMac);

    //GET VALUES
    float temp = bme.readTemperature() + TEMP_SENS_OFFSET;
    float pressure = bme.readPressure()/100;
    float humidity = bme.readHumidity();
    char buftemp[20];
    char bufpressure[20];
    char bufhumidity[20];

    sprintf (buftemp, "%f", temp);
    sprintf (bufpressure, "%f", pressure);
    sprintf (bufhumidity, "%f", humidity);

    //Serial.println(buftemp);
    //Serial.println(bufpressure);
    //Serial.println(bufhumidity);
    client.publish("homesens/wohnzimmer/temperature", buftemp);
    client.publish("homesens/wohnzimmer/pressure", bufpressure);
    client.publish("homesens/wohnzimmer/humidity", bufhumidity);

    // FINISH
    digitalWrite(LED_BUILTIN, HIGH);

  }
  delay(DELAY);
  
}
     