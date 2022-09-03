/** @file main.cpp
 *  This file contains a program that reads weather data.
 */

#include <Arduino.h>
#include "PrintStream.h"

#include <dht11.h>

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


/** @brief   Class which stores data from which to make a Node Red plot.
 */
class NodeRedPlot
{
protected:
    uint16_t n_pts;              ///< Maximum number of points per curve
    uint8_t n_curves;            ///< Number of curves on the plot
    uint16_t n_saved;            ///< Number of points saved so far
    float* x_data;               ///< Pointer to array of X data for curves
    float** curves;              ///< Pointer to arrays of Y data for curves
    String* curve_labels;        ///< Pointer to array of curve labels

public:
    NodeRedPlot(uint16_t max_points, uint8_t num_curves,
                const char** curve_names);
    void add_data(float x, float* p_y);
    void print_data(Print& printer);
    void clear(void);
    // void mqtt_send(derpy_thing);
};


/** @brief   Constructor for a NodeRed plot object.
 */
NodeRedPlot::NodeRedPlot(uint16_t max_points, uint8_t num_curves,
                         const char** curve_names)
{
    n_pts = max_points;
    n_curves = num_curves;
    n_saved = 0;

    // Allocate memory for the maximum amount of X and Y data to be plotted
    if ((x_data = new float[n_pts]) == NULL 
        || (curves = new float*[n_curves]) == NULL)
    {
        Serial << "Memory allocation error in NodeRedPlot";
        return;
    }
    for (uint8_t index = 0; index < n_curves; index++)
    {
        if ((curves[index] = new float[n_pts]) == NULL)
        {
            Serial << "Memory allocation error in NodeRedPlot";
            return;
        }
    }

    // Save the names of the curves in Arduino Strings 'cause we're lazy
    curve_labels = new String[n_curves];
    for (uint8_t index = 0; index < n_curves; index++)
    {
        curve_labels[index] += curve_names[index];
    }
}


/** @brief   Add one set of data to the set to be transmitted to Node-RED.
 *  @details If one tries to add data when the arrays are full, nothing happens
 *           except an error message. The number of items in the array @c p_y
 *           @b must match the number of curves in the plot.
 *  @param   x One float with the data's X coordinate
 *  @param   p_y A pointer to an array of Y coordinate data
 */
void NodeRedPlot::add_data(float x, float* p_y)
{
    // Make sure we're not trying to save more points than we have room for
    if (n_saved >= n_pts)
    {
        Serial << "ERROR: Too many points in NodeRedPlot" << endl;
        return;
    }

    // Save the X data
    x_data[n_saved] = x;

    // Save the Y data for each curve
    for (uint8_t curve = 0; curve < n_curves; curve++)
    {
        (curves[curve])[n_saved] = p_y[curve];
    }

    n_saved++;
}


/** @brief   Print the data in the arrays as it will be sent to Node-RED.
 */
void NodeRedPlot::print_data(Print& printer)
{
    // Print the series labels first
    printer.print("[{\n\"series\": [");
    for (uint8_t curve = 0; curve < n_curves; curve++)
    {
        printer.print("\"");
        printer.print(curve_labels[curve]);
        printer.print("\",");
    }
    printer.println("],\ndata: [");

    // Next print the data for each series
    for (uint8_t curve = 0; curve < n_curves; curve++)
    {
        printer.print("[");
        for (uint16_t point = 0; point < n_saved; point++)
        {
            printer.print("{\"x\": ");
            printer.print(x_data[point]);
            printer.print(", \"y\": ");
            printer.print((curves[curve])[point]);
            printer.print("},");
        }
        printer.println("],");
    }
    printer.println("],\n\"labels\": [\"\"]\n}]");
}


/** @brief   Reset the plot data set so it can be refilled from an empty state.
 */
void NodeRedPlot::clear(void)
{
    n_saved = 0;
}




/** @brief   Task which shows useful debugging stuff on a serial port.
 */
void serial_task (void* p_params)
{
    // uint32_t time = 0;
    const TickType_t delay = 5000;
    String stringy;

    const uint16_t ARRAY_SIZE = 5;
    uint32_t times[ARRAY_SIZE];
    float sines[ARRAY_SIZE];
    float cosines[ARRAY_SIZE];

    const char* labels[2] = {"Sines", "Cosines"};
    NodeRedPlot plotzy(ARRAY_SIZE, 2, labels);

    for (;;)
    {
        for (uint16_t count = 0; count < ARRAY_SIZE; count++)
        {
            float sineful = sin((float)count / 7);
            float cosful = cos((float)count / 7);
            times[count] = count;
            sines[count] = sineful;
            cosines[count] = cosful;

            float blah[2];
            blah[0] = sineful;
            blah[1] = cosful;
            plotzy.add_data(count, blah);
            plotzy.print_data(Serial);

            vTaskDelay(delay);
        }
        plotzy.clear();
        // Serial << "Wind Speed: " << wind_speed.get () << " mph" << endl;
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
    // xTaskCreate (mqtt_task, "MQTT/RSSI", 8192, NULL, 3, NULL);
    xTaskCreate (temp_humid_task, "Temp/Humid", 1024, NULL, 2, NULL);
    xTaskCreate (serial_task, "Serial", 1024, NULL, 1, NULL);
}


/** @brief   The Arduino loop function, which does pretty much nothing. 
 */
void loop () 
{
    vTaskDelay (10000);
}


