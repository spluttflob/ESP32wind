/** @file task_mqtt.h
 *  This file contains a task which communicates measured data to an MQTT
 *  broker out there somewhere.
 */

#include <Arduino.h>
#include "PrintStream.h"


void mqtt_task (void* p_params);
