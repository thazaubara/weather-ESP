#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// WIFI CREDENTIALS
const char* SSID = "MartinRouterPing";
const char* PSK = "_1HaveAStream";
const char* MQTT_BROKER = "192.168.0.105";
const int PORT = 1883;

String main_topic = "homesens/ESP32/";

// LOOP DELAY
const int DELAY = 2500;

// MQTT STUFF
String mqtt_subtopic = "balkon";
WiFiClient espClient;
PubSubClient client(espClient);
String espClientNameString = "ESPClient_" + mqtt_subtopic;
char espClientName[50];

// BME STUFF
Adafruit_BME280 bme;
const float TEMP_SENS_OFFSET = -2;

void setup() {
  // INIT LED. TURN IT ON
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // INIT SERIAL
	Serial.begin(115200);
  Serial.println("\nWelcome to Weater-ESP");

  // SETUP WIFI
	Serial.println(String("Connecting to ") + SSID);
	WiFi.begin(SSID, PSK);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println(" WiFi connected");
	Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // SETUP MQTT
	client.setServer(MQTT_BROKER, PORT);
  espClientNameString.toCharArray(espClientName, sizeof(espClientName));

  // INIT BME SENSOR
  static bool connected = false;
  if (!connected) {
    while(!bme.begin(0x76)){
      Serial.println("Could not find BME280. Check Wiring!");
      delay(500);
    }
  }
  Serial.println("BME280 connected");
}

void reconnect() {
	while (!client.connected()) {
		Serial.println("Reconnecting...");
		if (!client.connect(espClientName)) {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" retrying in 5 seconds");
			delay(5000);
		}
	}
}

void loop() {
  // CONNECTION CHECK
	if (!client.connected()) {
    digitalWrite(LED_BUILTIN, LOW);
		reconnect();
	}

  digitalWrite(LED_BUILTIN, LOW);
  client.loop();

  // SEND REAL STUFF
  char msg[50];
  snprintf (msg, 50, "Alive since %ld milliseconds", millis());
  Serial.println(msg);
  String topicMsg_string = main_topic + mqtt_subtopic + "/message";
  char topicMsg [50];
  topicMsg_string.toCharArray(topicMsg, sizeof(topicMsg));
  client.publish(topicMsg, msg);

  // SEND IP ADDRESS
  char bufIp [20];
  IPAddress ipaddr = WiFi.localIP();
  sprintf(bufIp, "%d.%d.%d.%d", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
  String topicIp_string = "homesens/ESP32/" + mqtt_subtopic + "/ip";
  char topicIp [50];
  topicIp_string.toCharArray(topicIp, sizeof(topicIp));
  client.publish(topicIp, bufIp);

  // SEND MAC ADDRESS
  String mac = String(WiFi.macAddress());
  char bufMac [20];
  mac.toCharArray(bufMac, 20);
  String topicMac_string = main_topic + mqtt_subtopic + "/mac";
  char topicMac [50];
  topicMac_string.toCharArray(topicMac, sizeof(topicMac));
  client.publish(topicMac, bufMac);

  // SEND TEMPERATURE
  float temp = bme.readTemperature() + TEMP_SENS_OFFSET;
  char buftemp[20];
  sprintf (buftemp, "%f", temp);
  String topicTemperature_string = main_topic + mqtt_subtopic + "/temperature";
  char topicTemperature [50];
  topicTemperature_string.toCharArray(topicTemperature, sizeof(topicTemperature));
  client.publish(topicTemperature, buftemp);

  // SEND PRESSURE
  float pressure = bme.readPressure()/100;
  char bufpressure[20];
  sprintf (bufpressure, "%f", pressure);
  String topicPressure_string = main_topic + mqtt_subtopic + "/pressure";
  char topicPressure [50];
  topicPressure_string.toCharArray(topicPressure, sizeof(topicPressure));
  client.publish(topicPressure, buftemp);

  // SEND HUMIDITY
  float humidity = bme.readHumidity();
  char bufhumidity[20];
  sprintf (bufhumidity, "%f", humidity);
  String topicHumidity_string = main_topic + mqtt_subtopic + "/humidity";
  char topicHumidity [50];
  topicHumidity_string.toCharArray(topicHumidity, sizeof(topicHumidity));
  client.publish(topicHumidity, buftemp);

  // FINISH
  digitalWrite(LED_BUILTIN, HIGH);

  // WAIT FOR GIVEN TIME PERIOD
  delay(DELAY);
}
     