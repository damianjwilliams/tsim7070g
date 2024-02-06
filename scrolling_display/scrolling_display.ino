

// Based on the following example: https://github.com/Xinyuan-LilyGO/LilyGO-T-SIM7000G/blob/master/examples/Arduino_TinyGSM/AllFunctions/AllFunctions.ino



#define TINY_GSM_MODEM_SIM7070
#define TINY_GSM_RX_BUFFER 1024 // 
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
#include <Wire.h>
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




char valueStr[25]; 


int counter, lastIndex, x, minX,numberOfPieces = 24,incomingByte = 0;
String pieces[24], input;



#define I2C_SDA 21
#define I2C_SCL 22
#define OLED_RST -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

#define MAX_MESSAGE_LENGTH 160  // Adjust this based on your maximum expected message length

String currentMessage = "Nothing yet";
String currentTime = "";
String currentDate = "";// Default message

void setup(){
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  Wire.begin(I2C_SDA,I2C_SCL);

   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

 
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  //display.print("NO TEXTS");
  display.display();


  // Set LED OFF
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  String lampState = "off";

  
pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);

  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  

  Serial.println("\nWait...");

  delay(1000);

  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

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
  if (modem.waitResponse(10000L) != 1)
  {
    DBG(" +CFUN=1  false ");
  }
  delay(200);

  SerialAT.println("AT+CGDCONT?");
  delay(500);
  if (SerialAT.available())
  {   
    input = SerialAT.readString();
    for (int i = 0; i < input.length(); i++)
    {
      if (input.substring(i, i + 1) == "\n")
      {
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


 
  modem.disableGPS();

  // Set SIM7000G GPIO4 LOW ,turn off GPS power
  // CMD:AT+SGPIO=0,4,1,0
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,0");
  if (modem.waitResponse(10000L) != 1) {
    DBG(" SGPIO=0,4,1,0 false ");
  }
  Serial.println("\n---End of GPRS TEST---\n");


  uint8_t  chargeState = -99;
  int8_t   percent     = -99;
  uint16_t milliVolts  = -9999;
  modem.getBattStats(chargeState, percent, milliVolts);
  Serial.println("Battery charge state:" +String(chargeState));
  Serial.println("Battery charge 'percent':"+ String(percent));
  Serial.println("Battery voltage:"+String(milliVolts/1000.0F));


  // --------TESTING SENDING SMS--------+
  //res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
  //DBG("SMS:", res ? "OK" : "fail");


  modem.sendAT("+CMGF=1");                                                       // Set the message format to text mode
  modem.waitResponse(1000L);
  modem.sendAT("+CNMI=2,2,0,0,0\r");
  delay(100);

}

String receivedMessage;
unsigned long previousMillis;

 void loop()
 {  
      
      
      modem.sendAT("+CMGL=\"ALL\"");  
      if(modem.waitResponse(1000L,"+")==1)
      {
      while(!SerialAT.available());
    
      receivedMessage = SerialAT.readString();
      
      Serial.println("************************");
      Serial.println(receivedMessage);
      Serial.println("************************");

      if(receivedMessage.indexOf("\"") > 0)
        {          
          receivedMessage.replace(",,", ",");
          receivedMessage.replace("\r", ",");
          receivedMessage.replace("\"", "");
          Serial.println(receivedMessage);

          String for_mess = receivedMessage;
          char delimiter = ',';
          char delimiter_b = '-';
          String date_str =  parse_SMS_by_delim(for_mess, delimiter,1);
          
          String time_str_raw =  parse_SMS_by_delim(for_mess, delimiter,2);
          String time_str = parse_SMS_by_delim(time_str_raw, delimiter_b,0);
          
          String message_str =  parse_SMS_by_delim(for_mess, delimiter,3);
          

          Serial.println("************************");
          Serial.println(date_str);
          Serial.println(time_str);
          Serial.println(message_str);
          Serial.println("************************");
          
          currentMessage = message_str; 
          currentTime = time_str;
          currentDate = date_str;

          if (message_str.indexOf("battery") >= 0) {
           Serial.println("battery message");
           int battery_value  = battery_charge();
           bool res = modem.sendSMS(SMS_TARGET, "Battery level: " + String(battery_value)+"\%");
           DBG("SMS:", res ? "OK" : "fail");
           currentMessage = "Battery level: "+ String(battery_value)+"\%";

            }

          if(message_str.indexOf("ON")>=0)
          {
          // Turn on relay
          digitalWrite(LED_PIN, LOW);          
          //Serial.println("LED on");
          currentMessage = "LED is on";           
          }
          
          if(message_str.indexOf("OFF")>=0)
          {
          // Turn off relay
          digitalWrite(LED_PIN, HIGH);           
          //Serial.println("LED off");
          currentMessage = "LED is off";
          }

          modem.sendAT("+cmgd=,4");
          modem.waitResponse(1000L);      
        }
      }

    Serial.println("message:");
    Serial.println(currentMessage);      
    scrollText(currentMessage,currentTime,currentDate);
   
      
          
}
       
String parse_SMS_by_delim(String sms, char delimiter, int targetIndex)
 {
  // Tokenize the SMS content using the specified delimiter
  int delimiterIndex = sms.indexOf(delimiter);
  int currentIndex = 0;

  while (delimiterIndex != -1)
  {
    if (currentIndex == targetIndex)
    {    
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
      






void scrollText(String curr_message,String curr_time, String curr_date) {
  
  char oled_time_str[15];
  char oled_date_str[15];

  sprintf(oled_time_str, "Time: %s", curr_time);
  sprintf(oled_date_str, "Date: %s", curr_date);
  
  
  
  int16_t curr_messageWidth = curr_message.length() * 6 * 2; // Width of the text in pixels
  //int16_t curr_timeWidth = oled_time_str.length() * 6 * 2; // Width of the text in pixels
  //int16_t curr_dateWidth = oled_date_str.length() * 6 * 2; // Width of the text in pixels

 

 //int16_t myArray[3] = {curr_messageWidth, curr_timeWidth, curr_dateWidth};
 int16_t myArray[3] = {curr_messageWidth, 15, 15};

   //This is to determine the maximum size of the message/date/time for scrolling
   int maxVal = myArray[0];
   int minVal = myArray[0];
   //Serial.print("Size of myArray is: "); Serial.println(sizeof(myArray));

   for (int i = 0; i < (sizeof(myArray) / sizeof(myArray[0])); i++) {
      if (myArray[i] > maxVal) {
         maxVal = myArray[i];
      }
      if (myArray[i] < minVal) {
         minVal = myArray[i];
      }
   }
   //Serial.print("The maximum value of the array is: "); Serial.println(maxVal);
     
  int16_t x = SCREEN_WIDTH;

  while (x >= -maxVal) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print(oled_date_str);
    display.setCursor(0,20);
    display.setTextSize(1);
    display.print(oled_time_str);
    display.setCursor(x, 40);
    display.setTextSize(2);
    display.print(curr_message);
    display.display();

    //delay(5); // Adjust the delay to control the scrolling speed
    x--;
  }
}

int8_t battery_charge() {
  uint8_t  chargeState = -99;
  int8_t   percent     = -99;
  uint16_t milliVolts  = -9999;
  modem.getBattStats(chargeState, percent, milliVolts);
  DBG("Battery charge state:", chargeState);
  DBG("Battery charge 'percent':", percent);
  DBG("Battery voltage:", milliVolts / 1000.0F);
  return percent;
}
  
