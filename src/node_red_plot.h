/** @file node_red_plot.h
 *  This template sends a whole plot full of data to an MQTT broker in a format
 *  which allows the Node-RED dashboard to make a plot in one fell swoop.
 */

#include <Arduino.h>
#include <PubSubClient.h>


/** @brief   Class which stores data from which to make a Node Red plot.
 */
template<uint8_t num_curves, uint16_t max_points>
class NodeRedPlot
{
protected:
    uint16_t n_saved;            ///< Number of points saved so far
    float x_data[max_points];
    float curves[num_curves][max_points];
    String curve_labels[num_curves];
    char* topic_name;            ///< Name of MQTT topic to which to broadcast

public:
    NodeRedPlot<num_curves, max_points>(const char* topic, 
                                        const char** curve_names);
    void add_data(float x, float* p_y);
    void print_data(Print& printer);
    void clear(void);
    void mqtt_send(PubSubClient& client);
};


/** @brief   Constructor for a NodeRed plot object.
 *  @param   topic The MQTT topic to which we'll send the data
 *  @param   curve_names A pointer to character strings holding the names of
 *           each of the plotted curves
 */
template<uint8_t num_curves, uint16_t max_points>
NodeRedPlot<num_curves, max_points>::NodeRedPlot(const char* topic, 
                                                 const char** curve_names)
{
    n_saved = 0;

    // Save the name of the topic to which to publish the data
    topic_name = new char[strlen(topic)];
    strcpy(topic_name, topic);

    // Save the names of the curves in Arduino Strings 'cause we're lazy
    // curve_labels = new String[n_curves];
    for (uint8_t index = 0; index < num_curves; index++)
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
template<uint8_t num_curves, uint16_t max_points>
void NodeRedPlot<num_curves, max_points>::add_data(float x, float* p_y)
{
    // Make sure we're not trying to save more points than we have room for
    if (n_saved >= max_points)
    {
        Serial << "ERROR: Too many points saved in NodeRedPlot" << endl;
        return;
    }

    // Save the X data
    x_data[n_saved] = x;

    // Save the Y data for each curve
    for (uint8_t curve = 0; curve < num_curves; curve++)
    {
        curves[curve][n_saved] = p_y[curve];
    }

    n_saved++;
}


/** @brief   Print the data in the arrays as it will be sent to Node-RED.
 */
template<uint8_t num_curves, uint16_t max_points>
void NodeRedPlot<num_curves, max_points>::print_data(Print& printer)
{
    // Print the series labels first
    printer.print("[{\n\"series\": [");
    for (uint8_t curve = 0; curve < num_curves; curve++)
    {
        printer.print("\"");
        printer.print(curve_labels[curve]);
        printer.print("\",");
    }
    printer.println("],\ndata: [");

    // Next print the data for each series
    for (uint8_t curve = 0; curve < num_curves; curve++)
    {
        printer.print("  [");
        for (uint16_t point = 0; point < n_saved; point++)
        {
            printer.print("{\"x\": ");
            printer.print(x_data[point]);
            printer.print(", \"y\": ");
            printer.print(curves[curve][point]);
            printer.print("},");
        }
        printer.println("],");
    }
    printer.println("],\n\"labels\": [\"\"]\n}]");
}


/** @brief   Send the data in the arrays to Node-RED via an MQTT broker.
 */
template<uint8_t num_curves, uint16_t max_points>
void NodeRedPlot<num_curves, max_points>::mqtt_send(PubSubClient& client)
{
    // This thing may eat a whole lotta memory. Begin with a header
    String sendstring("[{\"series\":[");

    // Print the series labels first
    for (uint8_t curve = 0; curve < num_curves; curve++)
    {
        if (curve)
        {
            sendstring += ",";
        }
        sendstring += "\"";
        sendstring += curve_labels[curve];
        sendstring += "\"";
    }
    sendstring += "],\"data\":[";

    // Next print the data for each series
    for (uint8_t curve = 0; curve < num_curves; curve++)
    {
        if (curve)
        {
            sendstring += ",";
        }
        sendstring += "[";
        for (uint16_t point = 0; point < n_saved; point++)
        {
            if (point)
            {
                sendstring += ",";
            }
            sendstring += "{\"x\":";
            sendstring += x_data[point];
            sendstring += ",\"y\":";
            sendstring += curves[curve][point];
            sendstring += "}";
        }
        sendstring += "]";
    }
    sendstring += "],\"labels\":[\"\"]}]\n";

    // Send this big mess to the MQTT broker
    unsigned int howbig = sendstring.length();
    char char_array[howbig];
    // Serial.println(sendstring);         ///////////////////////////////////////
    sendstring.toCharArray(char_array, howbig);
    Serial << "Sending " << howbig << " byte MQTT message" << endl;
    if (!client.publish(topic_name, char_array))
    {
        Serial << " MQTT Problem! Buffer too small." << endl;
    }
}


/** @brief   Reset the plot data set so it can be refilled from an empty state.
 */
template<uint8_t num_curves, uint16_t max_points>
void NodeRedPlot<num_curves, max_points>::clear(void)
{
    n_saved = 0;
}
