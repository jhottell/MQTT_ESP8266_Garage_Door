//########################################################################
//#  ESP8266 MQTT Garage Door Opener with Temperature                    #
//#  Blog post, videos, and code can  be found at www.electronhacks.com  #
//########################################################################


//##### Declarations and Variables #####
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
char vInp13 = 0;
int vA0 = 0;
int iA0 = 0;
int Counter = 0;
String rx;
int  rxLength = 0;
float tempaverage = 0;
float temparray[9]; 
int arraycounter = 0;
bool startup = 1;

//Configuration, enter your own values here
char ssid[]         = "Your Wifi SSID";          // your network name also called SSID
char password[]     = "Your Wifi Password";      // your network password
char server[]       = "192.168.1.165";           // MQTT Server IP or machine name
char topic[]        = "GarageDoor/";             // MQTT Topic
char mQTTID[]       = "ESP8266GarageDoor";       // MQTT ID. Make this unique for each device connecting to the MQTT Broker or they will disconnect eachother!
char mQTTUser[]     = "MQTTuser";                // MQTT UserName (See below for modification if you don't use user and password)
char mQTTPassword[] = "MQTTPass";                // MQTT Password (See below for modification if you don't use user and password)

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);


//##### Void Setup, runs once on startup #####
void setup()
{
  pinMode(5, OUTPUT);
  pinMode(13, INPUT_PULLUP);
  
  Serial.begin(115200);
  Serial.print("Attempting to connect to Network named: ");   
  Serial.println(ssid);   
  WiFi.begin(ssid, password);  
  while ( WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");     // print dots while we wait to connect
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  while (WiFi.localIP() == INADDR_NONE) 
  {
    Serial.print(".");     // print dots while we wait for an ip addresss
    delay(300);
  }
  Serial.println("\nIP Address obtained");
}




//##### Main Program #####
void loop()  
{ 
  //Reconnect if we are not subscribed to the LaunchPad/In topic
  if (!client.connected()) { 
    Serial.println("Disconnected. Reconnecting....");
    delay(30000); //Wait 30 seconds between connection attempts
    if(!client.connect(mQTTID, mQTTUser, mQTTPassword)) {   //Use this line if you have a User or Password for the MQTT Broker 
  //if(!client.connect(mQTTID)) {                           //Use this line if you don't have a User or Password for the MQTT Broker 
      Serial.println("Connection failed");
    } else {
      Serial.println("Connection success");
      if(client.subscribe(topic)) {
        Serial.println("Subscription successfull");
      }
    }
  }

  ///// Evaluate input 13 and send a message if the value changes 
  if (digitalRead(13) != vInp13)
  {
   vInp13 = digitalRead(13);
   if (vInp13 == LOW)
   {    
     client.publish(topic, "DoorOpened");
     Serial.println("TX: DoorOpened");
   }
   else
   {
     client.publish(topic, "DoorClosed");
     Serial.println("TX: DoorClosed");
   }
  } 

  //#############Evaluate the analog input###############
  if (Counter >= 20) // Counter reduces how often we update to help prevent broker spam
  {
    Counter = 0;
    if (startup == 1){                    // Load all 10 slots in the array if this is startup so we don't get wonky readings
      iA0 = analogRead(A0);               // Read the analog value
      iA0 = ((iA0/5.2)-53);               //Scale and offset the value to Degrees F
      for (int i = 0; i < 10; i++){
        temparray[i] = iA0;
      }
      startup = 0;
    }

    iA0 = analogRead(A0);                  // Read the analog value
    iA0 = ((iA0/9.76)+16);                 //Scale and offset the value to Degrees F
    if (arraycounter >= 9) {               //If the counter reaches 9 set it back to 0
      arraycounter = 0;}   
    else{
      arraycounter ++;}                    //Otherwise increment the array counter
    temparray[arraycounter] = iA0;         //Load one slot in the array, this loops through the slots on each execution                            
    
    if (((iA0 * 1.02) < vA0) or ((iA0 * 0.98) > vA0))   //(Deadband)Only Update if the raw value is +- whatever % you choose 
    {  
      vA0 = iA0;                           //Set the saved temp equal to the current temp for next time comparison
      tempaverage = ((temparray[0] + temparray[1] + temparray[2] + temparray[3] + temparray[4] + temparray[5]+ temparray[6]+ temparray[7]+ temparray[8]+ temparray[9])/10);
      String str = (String)tempaverage;     //PubSubClient requires data to be a char* (char array) so we have to convert the int
      int str_len = str.length() +1;        //Length + null terminator
      char char_array[str_len];             //Prepare the character array (buffer)
      str.toCharArray(char_array, str_len); // Copy it over 
      client.publish(GarageTemp, char_array);    // Publish to MQTT
      Serial.println("TX: Temperature: " + String(tempaverage) + "  RAW:" + String(analogRead(A0)));  // Print to Seral window  
     //iA0 = ((iA0/9.76)+16); 
    }  
  }  
  else
  {
    Counter++;
  }   
 
  client.loop(); // Check if any subscribed messages were received
  delay(10);
}
   


//##### Subroutines #####
///// VOID CALLBACK - prints to the serial monitor if we recieve a MQTT message /////
void callback(char* topic, byte* payload, unsigned int length) 
{
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
  if ((rx == "OpenGarageDoor") && (vInp13 == HIGH)){digitalWrite(5, 1);} //Turn the output on / open the door 
  delay(1000);                                     //Wait a second
  digitalWrite(5, 0);                              //Turn the output back off   
  delay(1000);                                     //Let Voltage settle before resuming.  

  if ((rx == "CloseGarageDoor") && (vInp13 == LOW)){digitalWrite(5, 1);} //Turn the output on / close the door 
  delay(1000);                                     //Wait a second
  digitalWrite(5, 0);                              //Turn the output back off   
  delay(1000);                                     //Let Voltage settle before resuming.  
  
}



