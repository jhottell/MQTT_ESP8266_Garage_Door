# MQTT_ESP8266_Garage_Door
Sample code shows how to use an MSP8266 for a MQTT connected garage door opener.
The ESP8277 publishes door status (opened or closed) and listens for commands to open or close the door. There also is a loop to read a thermistor and send back tmeperature.

Youtube video for the garage door opener here https://youtu.be/X4qXy2JFg_w
My Blog with more projects http://ElectronHacks.com
Youtube MQTT Playlist https://www.youtube.com/playlist?list=PLkIA6OiRuCWa-H9IqUinyZlpg3te4qTuU

Unfortunately this code only works with arduino IDE version 1.6.5 and older, after this there was a bug in the ide that they don't plan to fix. A version of this code for the latest IDE is needed where the function prototype is created before it's referred to in the client object instantiation. Information here: http://forum.arduino.cc/index.php?topic=370284.0 
I will eventually make a version for the latest IDE when I have time.
