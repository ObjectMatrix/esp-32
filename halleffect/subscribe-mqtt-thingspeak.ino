#include "certs.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

const char* ssid = "XXXXXX";
const char* password = "XXXXXXXXXX";

// The name of the device. This MUST match up with the name defined in the AWS console
#define DEVICE_NAME "esp-32-thingy"

// The MQTTT endpoint for the device (unique for each AWS account but shared amongst devices within the account)
#define AWS_IOT_ENDPOINT "a3amyzmzkjgky3-ats.iot.us-east-1.amazonaws.com"

// The MQTT topic that this device should publish to
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"

#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
// How many times we should attempt to connect to AWS
#define AWS_MAX_RECONNECT_TRIES 50

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(512);

const byte led_gpio = 32;

// hall effect variables
int hallVal = 0;
const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "XXXXXXXXX";
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

// lamp control (LED)
void messageHandler(String &topic, String &payload) {
  Serial.println("Receiving message from topic sub");
  Serial.println("incoming: " + topic + " - " + payload);

  //  deserialize json
    StaticJsonDocument<200> doc;
    deserializeJson(doc, payload);
    // String message = doc["message"];
    Serial.println(payload);
    if(payload == "1") {
    digitalWrite(led_gpio, HIGH);
    } else {
      digitalWrite(led_gpio, LOW);
    }
}

void setup()
{
   pinMode(led_gpio, OUTPUT);
   Serial.begin(115200);
    
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    // connect to AWS
    connectToAWS();
    
}

void sendJsonToAWS()
{
  StaticJsonDocument<128> jsonDoc;
  JsonObject stateObj = jsonDoc.createNestedObject("state");
  JsonObject reportedObj = stateObj.createNestedObject("reported");
  
  // Write the temperature & humidity. Here you can use any C++ type (and you can refer to variables)
  hallVal = hallRead();
  reportedObj["hall"] = hallVal;
  Serial.println(hallVal);
  reportedObj["wifi_strength"] = WiFi.RSSI();
  
  // Create a nested object "location"
  JsonObject locationObj = reportedObj.createNestedObject("location");
  locationObj["name"] = "Blackhole";

  Serial.println("Publishing message to AWS...");
  //serializeJson(doc, Serial);
  char jsonBuffer[512];
  serializeJson(jsonDoc, jsonBuffer);

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void sendHallEffectToThingSpeak() {
      hallVal = hallRead();
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "api_key=" + apiKey + "&field1=" + String(hallRead());           
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Hall Effect: ");
      Serial.println(hallVal);
      // Free resources
      http.end();   
}

void connectToAWS()
{
  // Configure WiFiClientSecure to use the AWS certificates we generated
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
   client.onMessage(messageHandler);

  // Try to connect to AWS and count how many times we retried.
  int retries = 0;
  Serial.print("Connecting to AWS IOT");

  while (!client.connect(DEVICE_NAME) && retries < AWS_MAX_RECONNECT_TRIES) {
    Serial.print(".");
    delay(100);
    retries++;
  }

  // Make sure that we did indeed successfully connect to the MQTT broker
  // If not we just end the function and wait for the next loop.
  if(!client.connected()){
    Serial.println(" Timeout!");
    return;
  }
  // client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  // If we land here, we have successfully connected to AWS!
  // And we can subscribe to topics and send messages.
  Serial.println(" > Connected to AWS");
  
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
}


void loop(){
  client.loop();
  delay(500);
  // send hollo effect
  // sendHallEffectToThingSpeak();
  // sendJsonToAWS();
  // Serial.println(AWS_IOT_PUBLISH_TOPIC);
  delay(500);
  
}
