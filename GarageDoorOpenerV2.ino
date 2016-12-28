/*
//########################################################################
//#  ESP8266 MQTT Garage Door Opener with Temperature                    #
//#  Code refactored 12/27/2016 to work with the new Arduinio IDE 1.6.13 #
//#  Blog post, videos, and code can  be found at www.electronhacks.com  #
//########################################################################
*/


#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Variables
//Update these with values suitable for your network.
const char* ssid = "shp";
const char* password = "fef105ec00";
const char* mqtt_server = "10.100.100.181";
const char* mqtt_topic = "GarageDoor/";
const char* mqtt_user = "guest";
const char* mqtt_password = "12345";

char vInp13 = 0;
String rx;
int  rxLength = 0;
//Analog input variables
int vA0 = 0;
int iA0 = 0;
int Counter = 0;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(5, OUTPUT);
  pinMode(13, INPUT_PULLUP);
  
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {\
  Serial.print("RX: ");
  //Convert and clean up the MQTT payload messsage into a String
  rx = String((char *)payload);                    //Payload is a Byte Array, convert to char to load it into the "String" object 
  rxLength = rx.length();                          //Figure out how long the resulting String object is 
  for (int i = length; i <= rxLength; i++)         //Loop through setting extra characters to null as garbage may be there
  {
    rx.setCharAt(i, ' ');
  }
  rx.trim();                                       //Use the Trim function to finish cleaning up the string   
  Serial.print(rx);                                //Print the recieved message to serial
  Serial.println();

  //Evaulate the recieved message to do stuff
  if ((rx == "OpenGarageDoor") && (vInp13 == HIGH))
  {digitalWrite(5, 1);} //Turn the output on / open the door 
  delay(1000);                                     //Wait a second
  digitalWrite(5, 0);                              //Turn the output back off   
  delay(1000);                                     //Let Voltage settle before resuming.  

  if ((rx == "CloseGarageDoor") && (vInp13 == LOW))
  {digitalWrite(5, 1);} //Turn the output on / close the door 
  delay(1000);                                     //Wait a second
  digitalWrite(5, 0);                              //Turn the output back off   
  delay(1000);                                     //Let Voltage settle before resuming.  
  
  /*Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }*/

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_topic,mqtt_user,mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic, "hello world");
      // ... and resubscribe
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }

  ///// Evaluate input 13 and send a message if the value changes 
  if (digitalRead(13) != vInp13)
  {
   vInp13 = digitalRead(13);
   if (vInp13 == LOW)
   {    
     client.publish(mqtt_topic, "DoorOpened");
     Serial.println("TX: DoorOpened");
   }
   else
   {
     client.publish(mqtt_topic, "DoorClosed");
     Serial.println("TX: DoorClosed");
   }
  }
 //Evaluate ADC
 /* iA0 = analogRead(A0); // Read the analog value
  if (Counter >= 150)   // Counter reduces how often we update to help prevent broker spam
  {
    Counter = 0;
    //if (1==1)   //For testing and scaling,
    if (((iA0 * 1.02) < vA0) or ((iA0 * 0.98) > vA0))   //(Deadband)Only Update if the raw value is +- 10% 
    {
     vA0 = iA0;                               //Set the variables equal to eachother for next time comparison
     iA0 = ((iA0/9.76)+12);                  //Scale and offset the value to Degrees F https://www.youtube.com/watch?v=ub20R9WnH-g
     String str = (String)iA0;                //PubSubClient requires data to be a char* (char array) so we have to convert the int
     int str_len = str.length() +1;           //Length + null terminator
     char char_array[str_len];                //Prepare the character array (buffer)
     str.toCharArray(char_array, str_len);    // Copy it over 
     client.publish(mqtt_topic, char_array); // Publish to MQTT
     Serial.print("TX: RAW: ");                    // Print to Seral window  
     Serial.print(vA0);    
     Serial.print("   Scaled: ");                    // Print to Seral window  
     Serial.println(iA0);        
    }  
  }  
  else
  {
    Counter++;
  }  */

  
  client.loop();

  //long now = millis();
  //if (now - lastMsg > 2000) {
  //  lastMsg = now;
  //  ++value;
  //  snprintf (msg, 75, "hello world #%ld", value);
  //  Serial.print("Publish message: ");
  //  Serial.println(msg);
  //  client.publish(mqtt_topic, msg);
  //}
  delay(10);  
}
