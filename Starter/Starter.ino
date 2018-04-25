// Wireless functions using a Arduino WiFi modul nRF24L01
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "LedControl.h" 

int exStartButton = 3;
int exResetButton = 2;

//NRF - Start
  #define CE_PIN 7
  #define CSN_PIN 8
  const byte adressReceiverTime[5] = {'R','x','T','i','M'};
  const byte adressReceiverControll[5] = {'R','x','C','o','T'};
  RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

  char dataReceived[24] = "L:00:00:000-R:00:00:000";
  int dataToSend = 0; // this must match dataToSend in the TX "reset = 2" "start = 1"
  bool newDataReceived = false;
//NRF - End

#define LED_DIN_PIN 4
#define LED_CS_PIN  5
#define LED_CLK_PIN 6

LedControl lc=LedControl(LED_DIN_PIN,LED_CLK_PIN,LED_CS_PIN,2); 
 
void setup(void)
{ 
  Serial.begin(9600);
  delay(3000); // wait for console opening
   
  //NRF setup
  if(! radio.begin()){
    Serial.println("Couldn't find NRF");
    while (1);
  }

  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate( RF24_250KBPS );
  radio.setRetries(0,5); // delay, count
  radio.openWritingPipe(adressReceiverControll);
  radio.openReadingPipe(1, adressReceiverTime);

  radio.startListening();

  //Pin setup
  pinMode(exStartButton, INPUT_PULLUP);
  pinMode(exResetButton, INPUT_PULLUP);

  // Initialize the MAX7219     
  lc.shutdown(0,false); // To Enable the Display    
  lc.setIntensity(0,15); // To set the brightness level (0 is min, 15 is max)  
  lc.clearDisplay(0); // To Clear the display register 
  lc.shutdown(1,false); // To Enable the Display
  lc.setIntensity(1,15); // To set the brightness level (0 is min, 15 is max) 
  lc.clearDisplay(1); // To Clear the display register 
  
  
  Serial.println("Ready for using");
}
 
void loop(void)
{
    getData();
    showData();
    
    if (digitalRead(exStartButton) == HIGH){ //start
      sendData(1);      
    }    
    
    if (digitalRead(exResetButton) == HIGH){ // reset
      sendData(2);      
    }      
}

void getData() {
  
    if ( radio.available() ) {
        radio.read( &dataReceived, sizeof(dataReceived) );
        newDataReceived = true;
    }
}

void showData() {
    if (newDataReceived == true) {
        Serial.print("Data received ");
        Serial.println(dataReceived);
        newDataReceived = false;    

        Serial.print("array ");
        Serial.println(dataReceived[2]);
        
        //show data on display 2
        //lc.clearDisplay(0); // To Clear the display register     
        lc.setChar(0,7,dataReceived[2], false); 
        lc.setChar(0,6,dataReceived[3], false);
        lc.setRow(0,5,B00000001);
        lc.setChar(0,4,dataReceived[5], false); 
        lc.setChar(0,3,dataReceived[6], false); 
        lc.setRow(0,2,B00000001);
        lc.setChar(0,1,dataReceived[8], false);   
        lc.setChar(0,0,dataReceived[9], false);    
        
        //show data on display 2
        //lc.clearDisplay(1); // To Clear the display register         
        lc.setChar(1,7,dataReceived[14], false); 
        lc.setChar(1,6,dataReceived[15], false);
        lc.setRow(1,5,B00000001);
        lc.setChar(1,4,dataReceived[17], false); 
        lc.setChar(1,3,dataReceived[18], false);
        lc.setRow(1,2,B00000001);
        lc.setChar(1,1,dataReceived[20], false);   
        lc.setChar(1,0,dataReceived[21], false);       
    }
}

void sendData(int data){
    radio.stopListening();
    dataToSend = data;
    bool rslt;
    Serial.print("Data Send ");
    Serial.print(dataToSend);    
    rslt = radio.write( &dataToSend, sizeof(dataToSend) );   

    if (rslt) {
        Serial.println("  Has been sent");    
    }
    else {
        Serial.println("  Tx failed");
    }
  radio.startListening();
}

