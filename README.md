# Yet Another ESP32 Weather Station
Mostly being developed so that I can get some practice with PlatformIO and C++
prior to teaching these things...it has been a few months and I'm rusty.

The system is designed to publish data to a Mosquitto server on a local WLAN. 
Node-RED is used to pick up the data and make a pretty plot on a dashboard.
One could also use a Python program to make plots with Matplotlib and send 'em
to a web server using SCP; this produces bigger and better plots but takes a
bit more work. 
