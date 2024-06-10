/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/
// WEMOS D1 MINI



#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "Anu";
const char* password = "anu12345";
const char* mqtt_server = "broker.hivemq.com";
const char* relayTopic = "org.ua4ever.electric_smart_controller/001/relay";
const char* limitTopic = "org.ua4ever.electric_smart_controller/001/relay/limit";
const char* dataTopic = "org.ua4ever.electric_smart_controller/001/data";

WiFiClient espClient;
PubSubClient client(espClient);
// unsigned long lastMsg = 0;
// #define MSG_BUFFER_SIZE	(50)
// char msg[MSG_BUFFER_SIZE];
// int value = 0;



double limit = 20.0;


// kode parsing data berdasarkan delimeter
// String getValue(String data, char separator, int index) {
//   int found = 0;
//   int strIndex[] = {0, -1};
//   int maxIndex = data.length() - 1;
//   for (int i = 0; i <= maxIndex && found ;  index ? data.substring(strIndex[0], strIndex[1]) : "";
// }
String getValue(String data, char separator, int index) {
  if (index < 0 || data.length() == 0) {
    // Handle invalid index or empty data gracefully (return empty string)
    return "";
  }

  int found = 0;
  int strIndex[2] = {0, -1}; // Corrected array size for clarity

  for (int i = 0; i <= data.length() - 1 && found <= index; ++i) {
    if (data.charAt(i) == separator || i == data.length() - 1) {
      found++;
      strIndex[0] = strIndex[1] + 1; // Start of next value
      strIndex[1] = (i == data.length() - 1) ? i : i; // End of current value (consider last element)
    }
  }

  return found > index ? "" : data.substring(strIndex[0], strIndex[1] + 1); // Include the separator for clarity
}


// kode setup wifi
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  payload[length] = 0;
  String data = String((char*)payload);
  // for (int i = 0; i < length; i++) {
  //   Serial.print((char)payload[i]);
  // }
  Serial.println(data);
  Serial.println();


  // jika ini adalah topic untuk relay
  if(strcmp(relayTopic, topic) == 0) {
    if ((char)payload[0] == '1') {
      digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
      // but actually the LED is on; this is because
      // it is active low on the ESP-01)
    } else {
      digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    }

  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");



      // Once connected, publish an announcement...
      // tempat pengiriman data
      client.publish(limitTopic, String(limit).c_str());



      // ... and resubscribe
      // tempat penerimaan data
      client.subscribe(relayTopic);
      client.subscribe(limitTopic);



    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {


  if(Serial.available()) {
    String command = Serial.readStringUntil('\n');

    if (command.startsWith("sensor#")) {
      // parsing nilai sensor
      String v = getValue(command, '#', 1);
      String a = getValue(command, '#', 2);
      String p = getValue(command, '#', 3);
      String e = getValue(command, '#', 4);
      String f = getValue(command, '#', 5);
      String pf = getValue(command, '#', 6);

      client.publish(dataTopic, command.c_str());

      double power = p.toDouble();

      if (power >= limit) {
        digitalWrite(BUILTIN_LED, HIGH); // matikan relay
        client.publish(relayTopic, "0"); // relay mati
      }
    }
  }


  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // unsigned long now = millis();
  // if (now - lastMsg > 2000) {
  //   lastMsg = now;
  //   ++value;
  //   snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
  //   Serial.print("Publish message: ");
  //   Serial.println(msg);
  //   client.publish("outTopic", msg);
  // }
}
