#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

//WIFI Network
const char *ssid = "xxxxxxxxxxxxxxxx";
const char *password = "xxxxxxxxxxxxxxxx";

//MQTT
const char *mqtt_server = "192.168.0.28";
String mqtt_client_id = "sensor_living_room";

//DHT Sensor
#define DHTTYPE DHT22
uint8_t DHTPin = D4;

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

WiFiClient client;
PubSubClient subClient(client);

void setup()
{
    pinMode(DHTPin, INPUT);
    dht.begin();

    Serial.begin(9600);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.print(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("IP address : ");
    Serial.print(WiFi.localIP());
    Serial.println("");

    //MQTT server configuration
    Serial.print("Connecting to MQTT broker ");
    Serial.print(mqtt_server);
    Serial.print("...");
    Serial.println("");
    subClient.setServer(mqtt_server, 1883);
    //Callback for incoming messages
    subClient.setCallback(mqtt_callback);
}

void loop()
{
    if (!subClient.connected())
    {
        reconnect();
    }
    subClient.loop();
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
}

void reconnect()
{
    while (!subClient.connected())
    {
        if (subClient.connect(mqtt_client_id.c_str()))
        {
            Serial.print("Connected as ");
            Serial.print(mqtt_client_id);

            //MQTT publish info message
            StaticJsonDocument<100> info_message;
            info_message["mqtt_client_id"] = mqtt_client_id;
            info_message["ip_address"] = WiFi.localIP().toString();
            char buffer[100];
            serializeJson(info_message, buffer);
            subClient.publish("sensors/info", buffer);

            //MQTT publish sensor message
            float dht_temperature;
            float dht_humidity;
            dht_temperature = dht.readTemperature();
            dht_humidity = dht.readHumidity();

            Serial.println("");
            Serial.print("Temperature : ");
            Serial.print(dht_temperature);
            Serial.println("");
            Serial.print("Humidity : ");
            Serial.println(dht_humidity);

            StaticJsonDocument<200> sensor_message;
            sensor_message["mqtt_client_id"] = mqtt_client_id;
            sensor_message["ip_address"] = WiFi.localIP().toString();
            sensor_message["temperature"] = dht_temperature;
            sensor_message["humidity"] = dht_humidity;
            char sensor_buffer[200];
            serializeJson(sensor_message, sensor_buffer);
            subClient.publish("sensors/capture", sensor_buffer);

            deepSleep();
        }
        else
        {
            Serial.print("MQTT connection failed, rc=");
            Serial.println(subClient.state());
            deepSleep();
        }
    }
}

void deepSleep()
{
    delay(5000);
    Serial.println("Going to deep sleep for 300 seconds...");
    ESP.deepSleep(300e6);
}
