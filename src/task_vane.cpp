/** @file task_vane.cpp
 *  This file contains a task which measures wind angle, averaging it over a
 *  given time period.
 */

#include <Arduino.h>
#include "PrintStream.h"
#include "AS5600.h"
#include "shares.h"
#include "task_vane.h"


/** @brief   Task which reads a wind vane, filters readings (well, averages 
 *           them over a period if time), and puts 'em into a shared variable.
 */
void vane_task (void* p_params)
{
    float sine_sum = 0.0, cosine_sum = 0.0;       // To find average wind angle
    float num_points;
    uint8_t count = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    Wire.begin (21, 22);
    AS5600 angler (Wire);

    for (;;)
    {
        // Find the angle now and add its trig functions into averaging sums
        float angle = angler.getAngle () * 360.0 / 4095.0;

        sine_sum += sin (angle * PI / 180.0);
        cosine_sum += cos (angle * PI / 180.0);

        // Periodically report the moving average by serial port for debugging 
        if (++count % 25 == 0)
        {
            float avg_angle = atan2 (sine_sum, cosine_sum);
            avg_angle = (avg_angle > 0.0) ? avg_angle : avg_angle + (2 * PI);

            Serial << "Angle: " << avg_angle * 180.0 / PI << endl;
        }

        // Periodically send the averaged value to the MQTT task for publishing
        if (count % 600)    // 600 --> 2 min.
        {
            count = 0;

            float avg_angle = atan2 (sine_sum, cosine_sum);
            avg_angle = (avg_angle > 0.0) ? avg_angle : avg_angle + (2 * PI);

            sine_sum = 0.0;
            cosine_sum = 0.0;

            wind_dir.put (avg_angle * 180.0 / PI );
        }

        vTaskDelayUntil (&xLastWakeTime, 200);      // Five readings per second
    }
}


