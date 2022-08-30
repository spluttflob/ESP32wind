/** @file task_anemometer.cpp
 *  This file contains a task that reads a surplus Second Wind C3 anemometer.
 */

#include <Arduino.h>
#include "PrintStream.h"
#include "task_anemometer.h"

#include "shares.h"


const uint8_t RecordTime = 10;      ///< Pulse counting interval (Seconds)
const int SensorPin = 23;           ///< Pin to which C3 anemometer output goes
int InterruptCounter;               ///< Global used to count anemometer pulses


/** @brief   Convert the number of hacked C3 pulses to wind speed in mph.
 *  @details It is assumed that a Hall effect sensor has been placed in the C3
 *           anemometer and produces two pulses per revolution, and that the
 *           pulses have been counted over a 10 second period.
 *           Calibration: m/s = Hz * 0.766 + 0.324
 *                        mph = Hz * 1.714 + 0.725
 */
inline float sec_to_mph_C3 (uint16_t counts)
{
    return (float)counts * 0.1714 + 0.0725;
}


/** @brief   Absurdly simple interrupt service routine that adds up pulses from
 *           the anemometer.
 */
void countup () 
{
    InterruptCounter++;
}


/** @brief   Task function which controls the anemometer.
 */
void anemometer_task (void* p_params)
{
    float WindSpeed;                      // Local to this task function
    pinMode (SensorPin, INPUT_PULLUP);    // Must use pullup for Hall sensor

    for (;;)
    {
        InterruptCounter = 0;
        attachInterrupt (digitalPinToInterrupt (SensorPin), countup, RISING);
        vTaskDelay (1000 * RecordTime);
        detachInterrupt (digitalPinToInterrupt (SensorPin));

        WindSpeed = sec_to_mph_C3 (InterruptCounter);
        wind_speed.put (WindSpeed);

        vTaskDelay (1000);
    }
}
