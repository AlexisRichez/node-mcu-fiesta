#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <PubSubClient.h>

const char *ssid     = "";
const char *password = "";

const char* mqtt_server = "192.168.0.27";

int pinCS = D4; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;

int wait   = 90; // In milliseconds between scroll movements
int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels
String message;

WiFiClient client;
PubSubClient subClient(client);
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);


void setup() 
{
    Serial.begin(9600);
    delay(10);

    //Matrix setup
    matrix.setIntensity(5);    // Use a value between 0 and 15 for brightness
    matrix.setRotation(0, 1);  // The first display is position upside down
    matrix.setRotation(1, 1);  // The first display is position upside down
    matrix.setRotation(2, 1);  // The first display is position upside down
    matrix.setRotation(3, 1);  // The first display is position upside down
    matrix.fillScreen(LOW);
    
    //WIFI connection
    Serial.println("Connecting to ");
    Serial.println(ssid); 
    display_message(ssid);
    display_message("...");
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected"); 
    Serial.println(WiFi.localIP());
    display_message(WiFi.localIP().toString());

    //MQTT server configuration
    subClient.setServer(mqtt_server, 1883);
    //Callback for incoming messages
    subClient.setCallback(callback);
}

void loop()
{
    if (!subClient.connected()) {
        reconnect();
    }
    subClient.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  message = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message.concat((char)payload[i]);
  }
  display_message(message);
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!subClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (subClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      subClient.publish("outTopic", "hello world");
      // ... and resubscribe
      subClient.subscribe("mqttclock");
    } else {
      Serial.print("failed, rc=");
      Serial.print(subClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void display_message(String message) {
  for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background OFF, reverse these to invert the display!
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait / 2);
  }
}
