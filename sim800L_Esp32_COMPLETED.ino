/**************************************************************
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *   Limited functions link is 
 *    https://gist.github.com/copercini/4a47ba2375a96320918c47ef6b7d3bbd#file-esp32_sim800l-ino
 *   All functions link is 
 *   https://github.com/vshymanskyy/TinyGSM/blob/master/examples/AllFunctions/AllFunctions.ino
 **************************************************************/
//BT to ESP32
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
  
  SerialBT.begin("ESP32 test"); //Bluetooth device name  
}

void loop() {
     
     //Insert the If loop here to read the error from python file then go to the loop else do nothing

//if(SerialBT.available())
  //{
   // serialData = SerialBT.read();
   //   if(serialData == '1')
    //  {
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
     // }
  
     // else 
     // {
        Serial.println("Bye");
     // }
//}
//delay(2000);
}
