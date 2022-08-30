/** @file task_mqtt.cpp
 *  This file contains a task which communicates measured data to an MQTT
 *  broker out there somewhere.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "task_mqtt.h"
#include "mycerts.h"
#include "shares.h"
#include "ESP32Ping.h"


/// The IP address (or possibly URL) of your MQTT broker
const char* mqtt_server = "192.168.2.87";

/// The TCP/IP port on the broker machine to which we connect, usually 1883
const uint16_t mqtt_port = 1883;

// The WiFi client, something which is part of the PubSubClient framework
WiFiClient espClient;

// The MQTT client which publishes data and subscribes to topics
PubSubClient client(espClient);


/** @brief   Get the WiFi running so we can talk to the MQTT broker
 */
void setup_wifi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial << "Already connected at " << WiFi.localIP() << endl;
    }
    else
    {
        Serial << "Connecting to " << ssid;

        // The SSID and password should be kept secret in @c mycerts.h
        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED) 
        {
            Serial.print(".");
            vTaskDelay (1000);
        }

        Serial << "connected at IP address " << WiFi.localIP() << endl;
    }
}


/** @brief   Find the WiFi received signal strength.
 *  @details Take a bunch of measurements of the Wi-Fi strength and return the
 *           average result.
 *  @param   num_checks The number of measurements to take and average
 */
int32_t check_RSSI (uint8_t num_checks)
{
    int32_t rssi = 0;
    int32_t average = 0;
    
    for (uint8_t count = 0; count < num_checks; count++)
    {
        rssi += WiFi.RSSI ();
        vTaskDelay (100);
    }

    average = rssi / num_checks;
    return average;
}


/** @brief   Callback which is actived when a message is received
 *  @details The message must have come to a topic to which we've subscribed.
 *  @param   topic The MQTT topic to which the message applies
 *  @param   message The message itself
 *  @param   length The length of the message in bytes
 */
void callback (char* topic, byte* message, uint16_t length) 
{
    Serial.print ("Message arrived on topic: ");
    Serial.print (topic);
    Serial.print (". Message: ");
    String messageTemp;
    
    for (int i = 0; i < length; i++) 
    {
        Serial.print ((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();
}


/** @brief   Re-connect to an MQTT broker.
 *  @param   client The MQTT client object used on this device
 */
void reconnect(PubSubClient& client) 
{
    // Try pinging the server; if it fails, reconnect WiFi
    Serial << "Pinging server " << mqtt_server << "...";
    bool success = Ping.ping(mqtt_server, 3);
    if(!success)
    {
        Serial << "failed. ";
    }
    else
    {
        Serial << "succesful. ";
    }

    // Loop until we're reconnected
    while (!client.connected()) 
    {
        Serial.print("Connect to MQTT...");

        if (client.connect("ESP32_Wx"))           // Attempt to connect
        {
            Serial.println("connected");
            client.subscribe("test/to_ardo");
        }
        else
        {
            // Complain, then try restarting the network connection
            Serial << "failed, rc=" << client.state() << endl;
            if (!Ping.ping(mqtt_server, 3))
            {
                setup_wifi();
            }
            vTaskDelay(5000);
        }
    }
}


/** @brief   Task which publishes data to an MQTT server.
 */
void mqtt_task (void* p_params)
{
    uint8_t time_counter = 0;       // Counts seconds between publishing runs
    char a_string[64];              // Assemble an MQTT message here

    Serial << "Setting up MQTT server and callback...";
    client.setServer (mqtt_server, 1883);
    client.setCallback (callback);
    Serial << "done." << endl;

    vTaskDelay (1000);

    for (;;)
    {
        // Make sure we're on the network
        if (WiFi.status () != WL_CONNECTED)
        {
            setup_wifi ();
        }

        // Make sure this MQTT client is connected to the MQTT broker
        if (!client.loop ())
        {
            reconnect (client);
        }

        // Send that data...once a minute?
        if (++time_counter > 60)
        {
            time_counter = 0;

            sprintf (a_string, "%.1f,%.1f", wind_speed.get(), wind_dir.get());
            client.publish("travisty/weather/test", a_string);

            sprintf (a_string, "%d", check_RSSI (8));
            client.publish("travisty/network/rssi", a_string);
        }

        vTaskDelay (1000);
    }
}


