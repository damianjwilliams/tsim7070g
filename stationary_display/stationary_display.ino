/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/lilygo-t-sim7000g-esp32-lte-gprs-gps/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// Based on the following example: https://github.com/Xinyuan-LilyGO/LilyGO-T-SIM7000G/blob/master/examples/Arduino_TinyGSM/AllFunctions/AllFunctions.ino

#define TINY_GSM_MODEM_SIM7070
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]  = "hologram";     //SET TO YOUR APN
const char gprsUser[] = "";
const char gprsPass[] = "";

// Set phone number, if you want to test SMS
// Set a recipient phone number to test sending SMS (it must be in international format including the "+" sign)
#define SMS_TARGET  "+13012045738"

#include <TinyGsmClient.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, Serial);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60          // Time ESP32 will go to sleep (in seconds)

#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4

#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13
#define LED_PIN     12


#define I2C_SDA 21
#define I2C_SCL 22
#define OLED_RST -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
const char str[] = "temp here: ";
char valueStr[25]; 


int counter, lastIndex, numberOfPieces = 24;
String pieces[24], input;
String msg;
char message[] = "Hello WOrrrrrllllllllllllllllllllllllllllldddd";
int  x, minX;



void setup(){
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  Wire.begin(I2C_SDA,I2C_SCL);

   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setTextWrap(true);
  display.print("NO TEXTS");
  display.display();


  // Set LED OFF
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);

  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SDCard MOUNT FAIL");
  } else {
    uint32_t cardSize = SD.cardSize() / (1024 * 1024);
    String str = "SDCard Size: " + String(cardSize) + "MB";
    Serial.println(str);
  }

  Serial.println("\nWait...");

  delay(1000);

  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  if (!modem.restart()) {
    Serial.println("Failed to restart modem, attempting to continue without restarting");
  }



  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  if (!modem.init()) {
    Serial.println("Failed to restart modem, attempting to continue without restarting");
  }

  String name = modem.getModemName();
  delay(500);
  Serial.println("Modem Name: " + name);

  String modemInfo = modem.getModemInfo();
  delay(500);
  Serial.println("Modem Info: " + modemInfo);
  

  // Unlock your SIM card with a PIN if needed
  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
      modem.simUnlock(GSM_PIN);
  }
  modem.sendAT("+CFUN=0 ");
  if (modem.waitResponse(10000L) != 1) {
    DBG(" +CFUN=0  false ");
  }
  delay(200);

  /*
    2 Automatic
    13 GSM only
    38 LTE only
    51 GSM and LTE only
  * * * */
  String res;
  // CHANGE NETWORK MODE, IF NEEDED
  res = modem.setNetworkMode(2);
  if (res != "1") {
    DBG("setNetworkMode  false ");
    return ;
  }
  delay(200);

  /*
    1 CAT-M
    2 NB-Iot
    3 CAT-M and NB-IoT
  * * */
  // CHANGE PREFERRED MODE, IF NEEDED
  res = modem.setPreferredMode(1);
  if (res != "1") {
    DBG("setPreferredMode  false ");
    return ;
  }
  delay(200);

  /*AT+CBANDCFG=<mode>,<band>[,<band>…]
   * <mode> "CAT-M"   "NB-IOT"
   * <band>  The value of <band> must is in the band list of getting from  AT+CBANDCFG=?
   * For example, my SIM card carrier "NB-iot" supports B8.  I will configure +CBANDCFG= "Nb-iot ",8
   */
  /* modem.sendAT("+CBANDCFG=\"NB-IOT\",8 ");*/
  
  /* if (modem.waitResponse(10000L) != 1) {
       DBG(" +CBANDCFG=\"NB-IOT\" ");
   }*/
   delay(200);

  modem.sendAT("+CFUN=1 ");
  if (modem.waitResponse(10000L) != 1) {
    DBG(" +CFUN=1  false ");
  }
  delay(200);

  SerialAT.println("AT+CGDCONT?");
  delay(500);
  if (SerialAT.available()) {
    //Serial.print("lllllllllllllllllaaaaaaaaaaaaaaaaaaaazzzzzzzzzzzzzzzeeeeeeeeeeeeerrrrrr");
   
    input = SerialAT.readString();
    for (int i = 0; i < input.length(); i++) {
      if (input.substring(i, i + 1) == "\n") {
        pieces[counter] = input.substring(lastIndex, i);
        lastIndex = i + 1;
        counter++;
       }
        if (i == input.length() - 1) {
          pieces[counter] = input.substring(lastIndex, i);
        }
      }
      // Reset for reuse
      input = "";
      counter = 0;
      lastIndex = 0;

      for ( int y = 0; y < numberOfPieces; y++) {
        for ( int x = 0; x < pieces[y].length(); x++) {
          char c = pieces[y][x];  //gets one byte from buffer
          if (c == ',') {
            if (input.indexOf(": ") >= 0) {
              String data = input.substring((input.indexOf(": ") + 1));
              if ( data.toInt() > 0 && data.toInt() < 25) {
                modem.sendAT("+CGDCONT=" + String(data.toInt()) + ",\"IP\",\"" + String(apn) + "\",\"0.0.0.0\",0,0,0,0");
              }
              input = "";
              break;
            }
          // Reset for reuse
          input = "";
         } else {
          input += c;
         }
      }
    }
  } else {
    Serial.println("Failed to get PDP!");
  }

  Serial.println("\n\n\nWaiting for network...");
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected()) {
    Serial.println("Network connected");
  }
  
  // --------TESTING GPRS--------
  Serial.println("\n---Starting GPRS TEST---\n");
  Serial.println("Connecting to: " + String(apn));
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    delay(10000);
    return;
  }

  Serial.print("GPRS status: ");
  if (modem.isGprsConnected()) {
    Serial.println("connected");
  } else {
    Serial.println("not connected");
  }

  String ccid = modem.getSimCCID();
  Serial.println("CCID: " + ccid);

  String imei = modem.getIMEI();
  Serial.println("IMEI: " + imei);

  String cop = modem.getOperator();
  Serial.println("Operator: " + cop);

  IPAddress local = modem.localIP();
  Serial.println("Local IP: " + String(local));

  int csq = modem.getSignalQuality();
  Serial.println("Signal quality: " + String(csq));

  SerialAT.println("AT+CPSI?");     //Get connection type and band
  delay(500);
  if (SerialAT.available()) {
    String r = SerialAT.readString();
    Serial.println(r);
  }

  Serial.println("\n---End of GPRS TEST---\n");

  modem.gprsDisconnect();
  if (!modem.isGprsConnected()) {
    Serial.println("GPRS disconnected");
  } else {
    Serial.println("GPRS disconnect: Failed.");
  }
/*
  // --------TESTING GPS--------
  
  Serial.println("\n---Starting GPS TEST---\n");
  // Set SIM7000G GPIO4 HIGH ,turn on GPS power
  // CMD:AT+SGPIO=0,4,1,1
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,1");
  if (modem.waitResponse(10000L) != 1) {
    DBG(" SGPIO=0,4,1,1 false ");
  }
  modem.enableGPS();
  float lat,  lon;
  while (1) {
    if (modem.getGPS(&lat, &lon)) {
      Serial.printf("lat:%f lon:%f\n", lat, lon);
      break;
    } else {
      Serial.print("getGPS ");
      Serial.println(millis());
    }
    delay(2000);
  }

  */
  modem.disableGPS();

  // Set SIM7000G GPIO4 LOW ,turn off GPS power
  // CMD:AT+SGPIO=0,4,1,0
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,0");
  if (modem.waitResponse(10000L) != 1) {
    DBG(" SGPIO=0,4,1,0 false ");
  }
  Serial.println("\n---End of GPRS TEST---\n");


  // --------TESTING SENDING SMS--------
  //res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
  //DBG("SMS:", res ? "OK" : "fail");


  modem.sendAT("+CMGF=1");                                                       // Set the message format to text mode
  modem.waitResponse(1000L);
  modem.sendAT("+CNMI=2,2,0,0,0\r");
  delay(100);


}

void Read_SMS() { 

  
      
 
      modem.sendAT("+CMGL=\"ALL\"");  
      modem.waitResponse(5000L);
      

      String data = SerialAT.readString();
      Serial.println(data);

 
      
      if(data.indexOf("+CMT:") > 0)

      {

      parseSMS(data);

      
       
      }     
      
      modem.sendAT("+cmgd=,4");
      modem.waitResponse(1000L);
}


void parseSMS(String data) {


data.replace(",,", ",");
data.replace("\r", ",");
data.replace("\"", "");
Serial.println(data);

String for_mess = data;

char delimiter = ',';
String date_str =  parse_SMS_by_delim(for_mess, delimiter,2);
String time_str =  parse_SMS_by_delim(for_mess, delimiter,3);
String message_str =  parse_SMS_by_delim(for_mess, delimiter,4);

Serial.println("************************");
Serial.println(date_str);
Serial.println(time_str);
Serial.println(message_str);
Serial.println("************************");

display_sms(message_str,time_str,date_str);


}

String parse_SMS_by_delim(String sms, char delimiter, int targetIndex) {
  // Tokenize the SMS content using the specified delimiter
  int delimiterIndex = sms.indexOf(delimiter);
  int currentIndex = 0;

  while (delimiterIndex != -1) {
    if (currentIndex == targetIndex) {
    
    String targetToken = sms.substring(0, delimiterIndex);
    targetToken.replace("\"", "");
    targetToken.replace("\r", "");
    targetToken.replace("\n", "");
    return targetToken;
    }

    // Move to the next token
    sms = sms.substring(delimiterIndex + 1);
    delimiterIndex = sms.indexOf(delimiter);
    currentIndex++;
  }

  // If the target token is not found, return an empty string
  return "";
}

void display_sms(String message_str,String time_str,String date_str){

//display.stopscroll();
   display.clearDisplay();
   display.setCursor(0,0);
   display.setTextSize(1);   
   display.print(message_str);
   display.setCursor(0,15);
   display.setTextSize(1);   
   display.print(time_str);
   display.setCursor(0,30);
   display.setTextSize(1);   
   display.print(date_str);
   display.display();
   
}



void loop()
{
Read_SMS();


}






  
