/** @file main.cpp
 *  This file contains a program that reads weather data.
 */

#include <Arduino.h>
#include "PrintStream.h"

#include <dht11.h>
#include <PubSubClient.h>

// #include "taskqueue.h"
#include "taskshare.h"
#include "task_anemometer.h"
#include "task_vane.h"
#include "task_mqtt.h"

// #include "ESP32Time.h"


/// The pin to which a DHT11 temperature/humidity sensor is connected
const uint8_t DHT11_PIN = 32;


/// A share for the measured wind speed in mph, declared extern in shares.h
Share<float> wind_speed ("Wind Speed");

/// A share for the average wind direction during a certain period
Share<float> wind_dir ("Wind Dir");


/** @brief   Task which shows useful debugging stuff on a serial port.
 */
void serial_task (void* p_params)
{
    // uint32_t time = 0;
    const TickType_t delay = 5000;
    String stringy;

    for (;;)
    {
        // for (uint16_t count = 0; count < ARRAY_SIZE; count++)
        // {
        //     float sineful = sin((float)count / 7);
        //     float cosful = cos((float)count / 7);
        //     times[count] = count;
        //     sines[count] = sineful;
        //     cosines[count] = cosful;

        //     float blah[2];
        //     blah[0] = sineful;
        //     blah[1] = cosful;
        //     plotzy.add_data(count, blah);
        //     plotzy.print_data(Serial);

        //     vTaskDelay(delay);
        // }
        // plotzy.clear();
        Serial << "Wind Speed: " << wind_speed.get () << " mph" << endl;
        vTaskDelay (60000);
    }
}


/** @brief   Task which measures temperature and humidity.
 */
void temp_humid_task (void* p_params)
{
    dht11 sensor;

    for (;;)
    {
        int chk = sensor.read(DHT11_PIN);

        Serial << "Humidity: " << (float)sensor.humidity
               << "%, Temperature: " << (float)sensor.temperature << "C" 
               << endl;

        vTaskDelay (60000);
    }
}


/** @brief   The Arduino setup function which runs when the program is started.
 *  @details This function sets up the serial port and starts the tasks.
 */
void setup ()
{
    Serial.begin (115200);
    vTaskDelay (500);
    while (!Serial) { }
    Serial << "Test of Anemometer" << endl;
    vTaskDelay (5000);

    // Initialize shared variables
    wind_speed.put (0.0);
    wind_dir.put (0.0);

    // Create the task objects; this starts each one immediately
    xTaskCreate (anemometer_task, "Anemometer", 4096, NULL, 7, NULL);
    xTaskCreate (vane_task, "Wind Vane", 2048, NULL, 5, NULL);
    xTaskCreate (mqtt_task, "MQTT/RSSI", 8192, NULL, 3, NULL);
    xTaskCreate (temp_humid_task, "Temp/Humid", 1024, NULL, 2, NULL);
    xTaskCreate (serial_task, "Serial", 4096, NULL, 1, NULL);
}


/** @brief   The Arduino loop function, which does pretty much nothing. 
 */
void loop () 
{
    vTaskDelay (10000);
}


