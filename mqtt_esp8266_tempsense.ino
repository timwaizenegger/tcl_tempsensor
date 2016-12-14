

/*

  TCL IOT script
  Tim Waizenegger
  ESP8266 arduino, pusub mqtt client
  created:        04.2015
  Last change: 02.12.2016
*/
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define nnnDEBUGMODE

const char* ssid = "hive_iot";
const char* password = "xxx";
const char* mq_org = "tju8ax";
const char* mq_type = "ESP8266";
const char* mq_id = "tempsense1";
//const char* mq_user = "use-token-auth";
const char* mq_user = "tempsense1";
//const char* mq_authtoken = "6W1j9xX-(bvFMH43Fy";
const char* mq_authtoken = "xxx";
//const char* mq_clientId = "d:tju8ax:ESP8266:tempsense1"; //"d:"+mq_org+":"+mq_type+":"+mq_id;
const char* mq_clientId = mq_id;
//const char* mq_serverUrl = "tju8ax.messaging.internetofthings.ibmcloud.com";
const char* mq_serverUrl = "xxx";

const int pin_a = 0;
const int pin_b = 2;
const int pin_led = 5;

#define SLEEPTIME10M 60000000*10
#define SLEEPTIME10S 10000000

const long interval = 2000; // millis
unsigned long previousMillis = 0;

DHT dht1(12, DHT22);
DHT dht2(14, DHT22);
WiFiClient espClient;
PubSubClient client(mq_serverUrl, 1883, espClient);





/////////////////

float getBattCapacity() {
  float vin = analogRead(A0);
#ifdef DEBUGMODE
  Serial.println(vin);
#endif
  vin = vin / 1024;
#ifdef DEBUGMODE
  Serial.println(vin);
#endif
  vin = vin * 8; // correct for voltage divider; this is the real voltage. Now we have a value 3.3 .. 4.2
#ifdef DEBUGMODE
  Serial.println(vin);
#endif
  return vin; // return the raw voltage;


}


void getSensorValues() {

  if (!client.connected()) {
    return;
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  float h1 = dht1.readHumidity();
  float h2 = dht2.readHumidity();
  // Read temperature as Celsius (the default)
  float t1 = dht1.readTemperature();
  float t2 = dht2.readTemperature();
  float batt = getBattCapacity();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h1) || isnan(t1)) {
#ifdef DEBUGMODE
    Serial.println("Failed to read from DHT sensor _1_ !");
#endif
    h1 = 0;
    t1 = 0;
  }
  if (isnan(h2) || isnan(t2)) {
#ifdef DEBUGMODE
    Serial.println("Failed to read from DHT sensor _2_ !");
#endif
    h2 = 0;
    t2 = 0;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic1 = dht1.computeHeatIndex(t1, h1, false);
  float hic2 = dht2.computeHeatIndex(t2, h2, false);
#ifdef DEBUGMODE
  Serial.print("batt: ");
  Serial.print(batt);
  Serial.print(" %\tHumidity 1: ");
  Serial.print(h1);
  Serial.print(" %");
  Serial.print("\tTemperature 1: ");
  Serial.print(t1);
  Serial.print(" *C ");
  Serial.print("\tHeat index 1: ");
  Serial.print(hic1);
  Serial.println(" *C ");
  Serial.print(" %\tHumidity 2: ");
  Serial.print(h2);
  Serial.print(" %");
  Serial.print("\tTemperature 2: ");
  Serial.print(t2);
  Serial.print(" *C ");
  Serial.print("\tHeat index 2: ");
  Serial.print(hic2);
  Serial.println(" *C ");
#endif
  const int BUFFER_SIZE = JSON_OBJECT_SIZE(9);// + JSON_ARRAY_SIZE(2);
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  JsonObject& root = jsonBuffer.createObject();
  root["temp1"] = t1;
  root["humid1"] = h1;
  root["heat1"] = hic1;
  root["temp2"] = t2;
  root["humid2"] = h2;
  root["heat2"] = hic2;
  root["vbatt"] = batt;

  char buffer[BUFFER_SIZE];
  root.printTo(buffer, BUFFER_SIZE);
#ifdef DEBUGMODE
  Serial.println(BUFFER_SIZE);
  Serial.println(buffer);
#endif
//  client.publish("iot-2/evt/tempsensor/fmt/json", buffer);
//  client.publish("iot-2/evt/tempsensor/fmt/text", "HALLO");
  client.publish("tempsensor", buffer);
}


/////////////////



void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
#ifdef DEBUGMODE
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
#ifdef DEBUGMODE
    Serial.print(".");
#endif
  }
  delay(100);
#ifdef DEBUGMODE
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}




void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
#ifdef DEBUGMODE
    Serial.print("Attempting MQTT connection...");
#endif
    // Attempt to connect
    if (client.connect(mq_clientId, mq_user, mq_authtoken)) {
#ifdef DEBUGMODE
      Serial.println("connected");
#endif
      // ... and resubscribe
      //client.subscribe("iot-2/cmd/switch/fmt/json");
    } else {
#ifdef DEBUGMODE
      Serial.print("failed");
      Serial.println(" try again in 1 second");
#endif
      // Wait 1 second before retrying
      delay(1000);
    }
  }
}




//////////////////////////////////////////////////////////////////////////////////

void setup() {
  pinMode(pin_led, OUTPUT);
  //pinMode(pin_b, OUTPUT);
  digitalWrite(pin_led, 1);
  //digitalWrite(pin_b,1);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");


#ifdef DEBUGMODE
  Serial.begin(57600);
  Serial.println("hi");
  Serial.println(ESP.getFreeSketchSpace());
#endif
  setup_wifi();


  dht1.begin();
  dht2.begin();
  //ArduinoOTA.begin();
  reconnect();
  getSensorValues();
}


long lastMsg = 0;

void loop() {

  if (!client.connected()) {
    reconnect();
    previousMillis = 0;
  }
  //
  //


  client.loop();





  unsigned long currentMillis = millis();
  if (0 == previousMillis) previousMillis = currentMillis;

  // loop a little to make sure transmission has occured
  if (currentMillis - previousMillis >= interval) {
    ESP.deepSleep(SLEEPTIME10M, WAKE_RF_DEFAULT);

  }




}
