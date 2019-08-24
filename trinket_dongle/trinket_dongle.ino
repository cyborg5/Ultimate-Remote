#include <IRLibRecv.h>
#include <IRLibDecodeBase.h>
#include <IRLib_P12_CYKM.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <HID.h>

#define MY_DEBUG 0
IRrecv My_Receiver(3);

IRdecodeCYKM My_Decoder;
uint16_t myBuffer[RECV_BUF_LENGTH];

bool Unit_Active;
uint32_t time, diff;

void setup() {
  #if(MY_DEBUG)
    Serial.begin(9600);
    while (! Serial) {};
    Serial.println("Trinket Dongle Working");
  #endif
  Mouse.begin();
  Keyboard.begin();
  My_Receiver.enableIRIn(); // Start the receiver
  My_Decoder.mouseSpeed=10;
  My_Receiver.enableAutoResume(myBuffer);
  Unit_Active=true;  time=millis(); 
}

#define HOLD_TIMEOUT 2ul //Release toggle after two minutes
void loop() {
  if (Unit_Active){
    diff=millis()-time;
    if (diff>(HOLD_TIMEOUT*60ul*1000ul)){
      Unit_Active=false; 
      #if(MY_DEBUG)
        Serial.println("releasing toggles");
      #endif
      My_Decoder.doMouseKeyboard(CYKM_TOGGLE+CYKM_TOGGLE_RESET);
      My_Decoder.mouseSpeed=10;
    }
  };
  if (My_Receiver.getResults()) {
    if(My_Decoder.decode()) {
      My_Decoder.doMouseKeyboard();
      #if(MY_DEBUG)
        My_Decoder.dumpResults();
      #endif
      Unit_Active=true;  time=millis(); 
    }
    My_Receiver.enableIRIn(); 
  }
}
