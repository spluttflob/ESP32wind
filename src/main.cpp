/** @file main.cpp
 *  This file contains a program that reads weather data.
 */

#include <Arduino.h>
#include "PrintStream.h"

// #include "taskqueue.h"
#include "taskshare.h"
#include "task_anemometer.h"
#include "task_vane.h"
#include "task_mqtt.h"


/// A share for the measured wind speed in mph, declared extern in shares.h
Share<float> wind_speed ("Wind Speed");

/// A share for the average wind direction during a certain period
Share<float> wind_dir ("Wind Dir");


/** @brief   Task which shows useful debugging stuff on a serial port.
 */
void serial_task (void* p_params)
{
    for (;;)
    {
        vTaskDelay (60000);
        Serial << "Wind Speed: " << wind_speed.get () << " mph" << endl;
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
    xTaskCreate (serial_task, "Serial", 1024, NULL, 1, NULL);
}


/** @brief   The Arduino loop function, which does pretty much nothing. 
 */
void loop () 
{
    vTaskDelay (10000);
}


