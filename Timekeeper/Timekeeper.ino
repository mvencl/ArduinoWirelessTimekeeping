// Wireless functions using a Arduino WiFi modul nRF24L01
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include <RTClib.h>


bool leftRun, rightRun;
int exLeftButton = 3;
int exRightButton = 4;
long lastSendDataTime = 0;


//RTC - Start
  RTC_DS3231 rtc;
  char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  DateTime beforeTime, currentTime, finishedLeft, finishedRight;
  long beforeMillis, currentMillis, finishedLeftMillis, finishedRightMillis;
//RTC - End

//NRF - Start
  #define CE_PIN_1 8
  #define CSN_PIN_1 7
  #define CE_PIN_2 10
  #define CSN_PIN_2 9  
  const byte adressReceiverTime[5] = {'R','x','T','i','M'};
  const byte adressReceiverControll[5] = {'R','x','C','o','T'};
  RF24 radio1(CE_PIN_1, CSN_PIN_1); // Create a Radio
  RF24 radio2(CE_PIN_2, CSN_PIN_2); // Create a Radio

  char dataToSend[24] = "L:00:00:000-R:00:00:000";
  int dataReceived = 0; // this must match dataToSend in the TX "reset = 2" "start = 1"
  char startReceived[6] = "start";
  char resetReceived[6] = "reset";
  bool newDataReceived = false;
//NRF - End
 
void setup(void)
{ 
  Serial.begin(9600);
  delay(3000); // wait for console opening

  //RTC Setup
  if(! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
    
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    setDefaultRtcTime();    
  }
  
  //NRF setup
  Serial.println("NRF setup");
  if(! radio1.begin()){
    Serial.println("Couldn't find NRF 1");
    while (1);
  }
  if(! radio2.begin()){
    Serial.println("Couldn't find NRF 2");
    while (1);
  }

  radio1.setPALevel(RF24_PA_MAX);  // možnosti jsou RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio1.setDataRate( RF24_250KBPS );
  radio1.setRetries(0,5); // delay, count
  radio1.openWritingPipe(adressReceiverTime);

  radio2.setPALevel(RF24_PA_MAX); // možnosti jsou RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio2.setDataRate( RF24_250KBPS );
  radio2.setRetries(0,5); // delay, count
  radio2.openReadingPipe(1, adressReceiverControll);
  radio2.startListening(); 
  
  //Pin setup
  pinMode(exLeftButton, INPUT_PULLUP);
  pinMode(exRightButton, INPUT_PULLUP); 
  
  resetTimer();
  
  Serial.println("Ready for using");
}
 
void loop(void)
{
  if (digitalRead(exLeftButton) == HIGH) stopLeft();   
  if (digitalRead(exRightButton) == HIGH) stopRight();
  setCurrentTime();
  writeLeft();
  writeRight();  

  listeningDataFromWireless();
  if (newDataReceived) {
    newDataReceived = false;
    if(dataReceived == 2){ resetTimer(); }
    if(dataReceived == 1){ startTimer(); }
  }   
  
  
  if(leftRun || rightRun || (millis() - lastSendDataTime > 1000 )){
    writeDataToWireless();
    lastSendDataTime = millis();
  }  

  if (digitalRead(exLeftButton) == HIGH) stopLeft();   
  if (digitalRead(exRightButton) == HIGH) stopRight();
  setCurrentTime();
  writeLeft();
  writeRight();  
}

void setCurrentTime()
 {
    currentTime = rtc.now();    
    if((currentTime - beforeTime).totalseconds() != 0)
    {      
      beforeTime = currentTime;  
      beforeMillis = millis();  
    }        
    currentMillis = millis() - beforeMillis;
 }

void writeLeft()
{  
  if(leftRun)
  {
    finishedLeft = currentTime;
    finishedLeftMillis = currentMillis;
  }
}

void writeRight()
{
  if(rightRun)
  {
    finishedRight = currentTime;
    finishedRightMillis = currentMillis;
  }
}

void startTimer() { 
   if(leftRun || rightRun) return;   
    resetTimer();
    leftRun = true;
    rightRun = true;
    Serial.println("Start timing");
 }

 void stopLeft(){
    if(leftRun)
    {
      leftRun = false;
      Serial.println("Stop left");
    }
 }

 void stopRight(){
    if(rightRun)
    {
      rightRun = false;
      Serial.println("Stop right");
    }
      
 }

 void resetTimer(){
    leftRun = false;
    rightRun = false;
    setDefaultRtcTime();
    finishedLeft = rtc.now();
    finishedRight = rtc.now();
    finishedLeftMillis = 0;
    finishedRightMillis = 0;     
 }

 void setDefaultRtcTime()
 {
    rtc.adjust(DateTime(2017, 12, 28, 0, 0, 0));
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    DateTime currNow = rtc.now();
    Serial.print(currNow.year(), DEC);
    Serial.print('/');
    Serial.print(currNow.month(), DEC);
    Serial.print('/');
    Serial.print(currNow.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[currNow.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(currNow.hour(), DEC);
    Serial.print(':');
    Serial.print(currNow.minute(), DEC);
    Serial.print(':');
    Serial.print(currNow.second(), DEC);
    Serial.println();
 }

 void listeningDataFromWireless()
 {       
     if ( radio2.available() ) {
        radio2.read( &dataReceived, sizeof(dataReceived) );
        newDataReceived = true;
     }    
 }

 void writeDataToWireless(){ //"L:00:00:000-R:00:00:000"
  
    char left[12] = "L:00:00:000";
    sprintf(left, "L:%02d:%02d:%03d",  finishedLeft.minute(),finishedLeft.second(),finishedLeftMillis );
    if(leftRun){ left[0] = 'C'; }
    
    char right[12] = "L:00:00:000";
    sprintf(right, "R:%02d:%02d:%03d",  finishedRight.minute(),finishedRight.second(),finishedRightMillis );
    if(rightRun){ right[0] = 'C'; }
       
    sprintf(dataToSend, "%s-%s",left, right );
    
    bool rslt;
    Serial.print("Data Sent ");
    Serial.println(dataToSend);    
    rslt = radio1.write( &dataToSend, sizeof(dataToSend) );   

//    if (rslt) {
//        Serial.println("  Has been sent");    
//    }
//    else {
//        Serial.println("  Tx failed");
//    }

 }

