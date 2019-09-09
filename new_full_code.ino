/****************************************
 * Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>

#define WIFISSID "x_Bhaiii_x" // Put your WifiSSID here
#define PASSWORD "getyourownwifi" // Put your wifi password here
#define TOKEN "BBFF-9f0ooUnFzfnJ6R8aGsJzdL7Wtrg6MG" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "mohammad123" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                           //it should be a random and unique ascii string and different from all other devices
//#include <MPU6050.h>

#include<Wire.h>
#define SDA 5
#define SCL 18
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
//GSM
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#define SerialMon Serial
#define SerialAT Serial1

#define SMS_TARGET  "+918220807238"
#define CALL_TARGET "+918220807238"

#define TINY_GSM_TEST_CALL true
#define TINY_GSM_TEST_SMS true
#define TINY_GSM_TEST_GPRS true

const char apn[]  = "airtelgprs.com";
const char gprsUser[] = "";
const char gprsPass[] = "";


char serialData;

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif
int incoming;

const int MPU_addr = 0x68; // I2C address of the MPU-6050

/****************************************
 * Define Constants
 ****************************************/
#define VARIABLE_LABEL "sensor" // Assing the variable label
#define DEVICE_LABEL "esp32" // Assig the device label

#define SENSOR 12 // Set the GPIO12 as SENSOR

char mqttBroker[]  = "industrial.api.ubidots.com";

char payload[100];
char topic1[150];
char topic2[150];
char topic3[150];
// Space to store values to send
char str_sensor1[10];
char str_sensor2[10];
char str_sensor3[10];

  const char * VARIABLE_LABEL_1 = "AcX"; // Assign the variable label
  const char * VARIABLE_LABEL_2 = "AcY"; // Assign the variable label
  const char * VARIABLE_LABEL_3 = "AcZ"; // Assign the variable label
/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("M Cntd");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

/****************************************
 * Main Functions
 ****************************************/
void setup() {
  SerialMon.begin(115200);
  delay(10);

  SerialAT.begin(9600,SERIAL_8N1,4,2,false);
  delay(3000);
  //print(F ... means it is stored in program memory, without F means it is stored in RAM
  // If flash memory is low then store in ram, and vice - versa
  SerialMon.println(F("Initializing modem..."));
  modem.restart();
  SerialMon.print(F("Waiting for network..."));
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println("Network Connected");

  SerialBT.begin("ESP32 test");
  
  Wire.begin(SDA, SCL);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as INPUT 

  
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);  
}

void loop() {
    serialData = SerialBT.read();
      if(serialData == '1')
     {
         #if TINY_GSM_TEST_GPRS
          DBG("Connecting to", apn);
          if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
            delay(100);
            return;
          }
         #endif
         
         String gsmLoc = modem.getGsmLocation();
         DBG("GSM location:", gsmLoc);
         SerialMon.println(gsmLoc);
         bool res = modem.isGprsConnected();
         DBG("GPRS status:", res ? "connected" : "not connected");
            
        #if TINY_GSM_TEST_SMS && defined(SMS_TARGET)
          res = modem.sendSMS(SMS_TARGET, String("Fall has been detected at ") + gsmLoc);
          DBG("SMS:", res ? "OK" : "fail");
        #endif
        
        #if TINY_GSM_TEST_CALL && defined(CALL_TARGET)
          DBG("Calling:", CALL_TARGET);
        
          res = modem.callNumber(CALL_TARGET);
          DBG("Call:", res ? "OK" : "fail");
        #endif
      }

  if (!client.connected()) {
    reconnect();
  }

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 6, true); // request a total of 14 registers

  float AcX = (Wire.read() << 8 | Wire.read()) / 16384.0; // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  float AcY = (Wire.read() << 8 | Wire.read()) / 16384.0; // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  float AcZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  
  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(AcX, 4, 2, str_sensor1);
  dtostrf(AcY, 4, 2, str_sensor2);
  dtostrf(AcZ, 4, 2, str_sensor3);
  
  
    sprintf(topic1, "%s", ""); // Cleans the topic content
    sprintf(topic1, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload content
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL_1); // Adds the variable label   
    sprintf(payload, "%s {\"value\": %s", payload,  str_sensor1); // Adds the value
    sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
    client.publish(topic1, payload);


    sprintf(topic2, "%s", ""); // Cleans the topic content
    sprintf(topic2, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload content
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL_2); // Adds the variable label   
    sprintf(payload, "%s {\"value\": %s", payload,  str_sensor2); // Adds the value
    sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
    client.publish(topic2, payload);


    sprintf(topic3, "%s", ""); // Cleans the topic content
    sprintf(topic3, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

    sprintf(payload, "%s", ""); // Cleans the payload content
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL_3); // Adds the variable label   
    sprintf(payload, "%s {\"value\": %s", payload,  str_sensor3); // Adds the value
    sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
    client.publish(topic3, payload);


 
  Serial.println("Publishing data to Ubidots Cloud");

  client.loop();
  delay(1000);

}
