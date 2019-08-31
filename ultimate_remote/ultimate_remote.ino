/*
 * Code for "Ultimate Remote" which handles transmission of IR codes for TV, cable, Blu-ray etc.
 * as well as a Bluetooth mode to connect to an iOS device such as iPad or iPhone to use Bluetooth
 * switch control features
 * 
 * Designed for use with 3 switches that are "Left", "Select", and "Right". See "my_inputs.h" for details.
 * 
 * All files in his package are open source under Creative Commons CC BY-SA
 * 
 * Developed by Chris Young cy_borg5@cyborg5.com
 * http://tech.cyborg5.com/
 * 
 */

#define MY_DEBUG 0
#define USE_BLE 1
#define USE_TOUCHSCREEN 1

/***************************
 * Feather TFT
 ***************************/
#include <Wire.h>
#include <Adafruit_ILI9341.h>
//Assumes Feather M0 or M4 or other compatible
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
#include <Fonts/FreeMono18pt7b.h>
#include "SymbolMono18pt7b.h"
Adafruit_ILI9341 display= Adafruit_ILI9341(TFT_CS, TFT_DC);
#define BACKLIGHT_PIN  13

void Message(const char msg[],uint16_t Duration) {
#if MY_DEBUG
  Serial.println(msg);
#endif
  display.fillRect(0,0,240,50,ILI9341_BLACK); 
  display.setCursor(0,0);
  display.println(msg); 
  delay(Duration);
}

void error(const char err[]) {
  Message(err, 2000);
  //while (1);
}

/***************************
 * Global Variables
 ***************************/

//Row and Col are the current position of the cursor.
//pRow and pCol are the previous position of the cursor.
uint8_t Row, pRow, Col, pCol; 

//The number of rows in the current page.
uint8_t Num_Rows;

//The width of the current page.
uint8_t Page_Width;

//Status of the display backlight. Turns itself off after certain amount of time.
//Also the "select" command doesn't work when the display is off just in case.
bool Backlight_Status;  

//"Page" is the currently displayed page. "Old_Page" remembers what page we were on
// when we switched to Bluetooth mode so that when we exit Bluetooth we will go back to the
// page that we were on.
uint8_t Page, Old_Page;

//Timestamp of the last time we updated the screen i.e. processed a command
uint32_t Time_of_Last_Update;

//The most recent results from Read_Buttons()
uint8_t Current_Buttons;

//Each command consists of a character display, the 32-bit IR value to be
// transmitted, the protocol number, and a special flag hich alternatively may indicate
// the number of bits.
typedef struct cmnd {
  uint8_t Glyph;    //The character to be displayed
  int32_t Value;    //string of bits to be transmitted or page number two change to
                    // if Special == SPL_CHANGE_PAGE
  uint8_t Protocol; //Sony, NEC etc.
  uint8_t Special;  //special handling flag or number of bits
} cmnd_t;

//The current command that the cursor rests on.
struct cmnd Current;

//The full keyboard screen PAGE_KB needs special handling so that it properly displays
// shifted characters. The flag kb_shift is true if we have toggled on Shift.
// This array maps on shifted display glyphs to their shifted value.
bool kb_shift;
uint8_t Shifted [] = {
          0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
         10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
         20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
         //      sp   !   "   #   $   %   &   ' 
         30, 31, 32, 33, 34, 35, 36, 37, 38,'"',
       // (   )   *   +   ,   -   .   /   0   1  
         40, 41, 42, 43,'<','_','>','?',')','!',
       // 2   3   4   5   6   7   8   9   ;   ;  
         '@','#','$','%','^','&','*','(',':',59,
         60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
         70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
         80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
         90,'{','|','}', 94, 95,'~','A','B','C',
        'D','E','F','G','H','I','J','K','L','M',
        'N','O','P','Q','R','S','T','U','V','W',
        'X','Y','Z',123,124,125,126,127};
  
//Types of special commands stored in Current.Special
#define SPL_QUIT    1
#define SPL_REC     2
#define SPL_GOTO_RIGHT 3
#define SPL_GOTO_DOWN 4
#define SPL_GOTO_TVAR 5
#define SPL_CHANGE_PAGE 6
#define SPL_HOLD 7
#define SPL_KB_SHIFT 8

//The maximum number of commands in a display column
#define COL_MAX   14

//The number of minutes of inactivity before the backlight turns off
#define LIGHT_TIMEOUT 5ul

//Return codes from Read_Buttons(). 
#define PUSHED_NONE     0
#define PUSHED_LEFT     1
#define PUSHED_SELECT   2
#define PUSHED_RIGHT    4
#define PUSHED_DOWN     (PUSHED_RIGHT + PUSHED_SELECT)
#define PUSHED_UP       (PUSHED_LEFT  + PUSHED_SELECT)
#define PUSHED_ALL      (PUSHED_LEFT  + PUSHED_SELECT + PUSHED_RIGHT)
//Not used but here just for illustrative purposes
#define PUSHED_SPLIT    (PUSHED_RIGHT + PUSHED_LEFT)

//Specifies the pages of commands to be displayed
#include "my_pages.h"

//Handles touchscreen input
#include "my_touch.h"

//Hardware specific routines to read the buttons
#include "my_inputs.h"

//All of my code for handling BLE switch control
#include "my_ble_code.h"

IRsend mysend; //Infrared sending object

//Turns the backlight on or off depending on flag.
void setBacklight(bool Flag) {
  Backlight_Status = Flag;
  if(Backlight_Status) {
    digitalWrite(BACKLIGHT_PIN, HIGH);
    //Record the time so that we know when to turn off the backlight for being idle too long
    Time_of_Last_Update = millis(); 
  } else {
    #if(MY_DEBUG)
      Serial.println("Backlight off");
    #endif
    digitalWrite(BACKLIGHT_PIN, LOW);    
  }
}

//Displays an "are you sure?" message on the display
// user must press "Down" button for yes. Any other press
// or a timeout results in no.
bool Are_You_Sure() {
  bool Result=false;
  unsigned long Amount = millis();    //Start the timeout clock
  display.fillScreen(ILI9341_BLACK);
  display.setCursor(0, 0); display.print(F("R U sure?"));
  display.setCursor(0, 25); display.print(F("Press Down"));
  display.setCursor(0, 50); display.print(F("For Yes"));
  do {    //Read the buttons until one is pressed for time runs out
    if (Read_Buttons() ) break;
  } while ( (millis() - Amount) < (3000ul) ); //3 second time limit
  display.setCursor(0, 75);
  if (Current_Buttons == PUSHED_DOWN) {
    display.print("Success!");
    Result=true;
  } else {
    display.print("Time Out");
  }
  delay (3000);
  display.fillScreen(ILI9341_BLACK);
  Update_Page();
  return Result; //Returns true if you confirmed yes otherwise false
};

//Handles special circumstances. Returns true if you should not
//do anything else. If false you should go ahead and transmit the code.
bool Do_Special (uint8_t Special) {
  bool Results = false;
  uint32_t Amount;
  switch (Special) {
    case SPL_QUIT:
      //Turn off the backlight and do nothing for 10 seconds
      Results= true;  //This means do not send any code. We've handled it.
      setBacklight(false); //Turn off the backlight
      delay(10000); 
      break;
    case SPL_REC:
      //Ask "Are you sure?" after pressing a record button.
      //The function returns true for yes which means we want to send the code
      Results = (!Are_You_Sure()); 
      Update();   //Redraw the screen to clear the "Are you sure?" message.
      break;
    case SPL_CHANGE_PAGE:
      //Change to a different page or enter Bluetooth mode
      Old_Page = Page;      //Remember where we were so if we exit Bluetooth we can come back
      Page = Current.Value; //The page we are switching to. 
      kb_shift = false;     //Always start with a fresh shift status
      mysend.send(CYKM,CYKM_TOGGLE+CYKM_TOGGLE_RESET,0);//Release all toggles when changing pages
      Update_Page();        //Redraw the entire new page.
      Results = true;       //Don't send any code. We've handled it.
      #if(MY_DEBUG)
        Serial.print("changed page to:"); Serial.println(Page,DEC);
      #endif
      break;
    case SPL_HOLD:
      //Some cable commands require that you hold down the button for an extended
      //period of time. All of these commands are the "SA" protocol so we use
      //Current.Protocol field to specify the number of seconds.
      Amount = millis();
      do {
        mysend.send(SA, Current.Value,0);
        delay(30);
      } while ((millis() - Amount) < (1000ul *Current.Protocol));
      Results = 1;  //We handled it so don't send again.
      break;
    //These three special commands force the cursor to move to a particular 
    // row and column. They still send the command.
    case SPL_GOTO_RIGHT:
      Row = 3; Col = 1; Update();   break;
    case SPL_GOTO_DOWN:
      Row = 3; Col = 3; Update();   break;
    case SPL_GOTO_TVAR:
      Row = 2; Col = 3; Update();   break;
    case SPL_KB_SHIFT:
      //Toggle the shift mode for PAGE_KB
      kb_shift=!kb_shift;
      Update_Page();
      Row = 4; Col = 10; Update();//Remain on the shift command
      break;
  };
  return Results;
};

#define DELTA_R 29
#define DELTA_C 22
#define BASE_C 0
#define BASE_R 22
//Displays the glyph at particular column and row
void Show_Cmd(uint8_t c,uint8_t r) {
  uint16_t fg,bg,Glyph; //Foreground color, background, and character to be displayed
  if ( (r==Row) && (c==Col)) {//This is a cursor
    fg=ILI9341_BLACK;  
    bg=display.color565(255,255,0);
    //To the right of the last column is a pair of virtual columns that allow you to 
    //go up or down a row without having to use chords.
    if(c==(Page_Width+1)) { //Go 2 presses right of the last command and select to go up
      c--;
      Glyph=MY_UP_ARROW;
    } else if(c==Page_Width) { //Go one press right of the last command and select to go down
      Glyph=MY_DOWN_ARROW;
    } else {  //Otherwise display the actual character
      Glyph=MyCommands[Pages[Page][r]][c].Glyph;
      if(kb_shift && (r>0) && (Glyph<127)) {
        Glyph= Shifted [Glyph];
      }
    }
  } else {//Not a cursor
    fg=ILI9341_WHITE; bg=ILI9341_BLACK;
    //If we are in a virtual column, erase the up or down arrow that was temporarily displayed
    //and then draw the proper character in the last column.
    if(c==(Page_Width+1)) c--;
    if(c==Page_Width) {
      Glyph=' ';
    } else {
      Glyph=MyCommands[Pages[Page][r]][c].Glyph;
      if(kb_shift && (r>0) && (r<5) && (Glyph<127)) {
        Glyph= Shifted [Glyph];
      }
    }
  }
  //First draw the background rectangle in the background color
  display.fillRect(DELTA_C*c-1, DELTA_R*r,DELTA_C+1,DELTA_R,bg);
  //Decide which font to use
  if( (Glyph>=32) && (Glyph<=126) ){ //If it's 33-126 then use standard mono 18 font
    display.setFont(&FreeMono18pt7b);
  } else {
    display.setFont(&SymbolMono18pt7b);//Otherwise use special symbol font
    if (Glyph>126) {      //Remap anything above 127 to be in the range 32 and upwards
      Glyph-=(127-32);
    }
  }
  //Draw the actual character glyph
  display.drawChar(BASE_C+c*DELTA_C,BASE_R+r*DELTA_R,Glyph,fg,bg,1);
  display.setFont();//Reset to default font for messages
}

//Draw or redraw the entire page
void Update_Page (void) {
  display.fillScreen(ILI9341_BLACK);
  if (Page == PAGE_BLE) {
    Message("Switch Cntrl.", 1000);
    return;
  }
  Row = 1; Col = 0; pRow=1; pCol=0;
  Num_Rows = Num_Rows_in_Page[Page]; 
  uint8_t i,j;
  for (i = 0; i < Num_Rows; i++) {
    if (i==0) {
      Page_Width = PAGE_BLE+1;
    } else {
      Page_Width = Page_Widths[Page]; //The width of the current page
    }
    for (j =0; j < Page_Width; j++) {
      //delay (1000);
      Show_Cmd(j,i);
    }
  };
  Update();
};

//We've completed a command do some housekeeping
//Update the time of the last command so that we know when to turn 
// the backlight off again.
//
void Update () {
  setBacklight(true);   //Turn on the backlight
  Show_Cmd(pCol,pRow);  //Erase the previous cursor
  Show_Cmd(Col,Row);    //Draw the new cursor
  pRow=Row;pCol=Col;    //Record the cursor current position so we can erase it next time
};

//These next four functions move the cursor Up, Down, Left, or Right and wrap around
// if you go off the edge.
void Do_Up() {
  Row = (Row - 1 + Num_Rows) % Num_Rows; Update ();
};

void Do_Down () {
  ++Row;
  Row = Row % Num_Rows; Update ();
};

void Do_Right () {
  Col = (Col + 1) % (Page_Width+2); Update();
};

void Do_Left () {
  Col = (Col - 1 + Page_Width+2) % (Page_Width+2); Update();
};

//Handle the pressing of the Select button which can have different meanings
//depending on context.
void Do_Select () {
  if(Col==Page_Width) {  //If we are in the first virtual column then go down
    Do_Down();
    delay(300);
    return;
  } else if(Col==(Page_Width+1)) { //If we are in the second virtual column then go up
    Do_Up();
    delay(300);
    return;
  }
  //Pick the current command out of the command table
  Current=MyCommands[Pages[Page][Row]][Col];
#if(MY_DEBUG)
  Serial.print("Page:"); Serial.print(byte(Page));  Serial.print(" Row:"); Serial.print(byte(Row)); Serial.print(" Col:"); Serial.println(byte(Col));
  Serial.print("sent: "); Serial.print(Pnames(Current.Protocol)); Serial.print(" 0x"); Serial.print(Current.Value, HEX);
  Serial.print(" bits:"); Serial.println (Current.Special);
#endif
  uint16_t Bits=0;
  if(Backlight_Status) {//only do select if display active
    //If Do_Special returns true then we don't need to do anything else
    if (Do_Special(Current.Special)) {
      return;
    }
    //SONY protocol has different number of bits but it's easier to calculate them than to
    //specify them in the table.
    if (Current.Protocol== SONY) {
      if(Current.Value>0x7fff) Bits= 20; else Bits= 15;
    } else {
      if (Current.Protocol== SAMSUNG36) {
        //Although this is called "Bits" is actually the address portion of a 36 bit protocol.
        //Fortunately the address doesn't vary between commands for this device. I only have one
        // device that uses this protocol.
        Bits=0x400; 
      }
    }
    if(Current.Protocol==RC6) {
      //For this protocol we need to specify the number of bits in the Current.Special field.
      //This means that we can't use special commands with this protocol and the number of
      // types of special commands cannot exceed 19 because there is a 20-bit protocol option here.
      Bits= Current.Special;
    }
    //If the keyboard shift is on and this is a keyboard command then add in the shift parameter
    if (kb_shift && (Current.Protocol==CYKM)) {
      Current.Value |= CYKM_SHIFT;
    }
    //Actually send the IR command
    mysend.send(Current.Protocol, Current.Value, Bits);
    Update();
  } else { //If backlight is off, turn it back on and refresh the page but don't process "Select".
    Update_Page();
  }
};

//Initialize everything
void setup() {
  #if(MY_DEBUG)
    while (! Serial) {}; delay (500);
    Serial.begin(9600); Serial.println("testing 123");
  #endif
  Initialize_Buttons(); //This function defined in my_inputs.h
  pinMode (BACKLIGHT_PIN, OUTPUT); //Backlight connected to pin 13
  display.begin();
  display.setRotation(1);
  display.setCursor(0, 0);
  display.setTextSize(3); //Default font size for messages
  display.setTextColor(ILI9341_WHITE);
  display.fillRect(0,0,240,320,ILI9341_BLACK); 
  Initialize_Touch();   //Defined in my_touch.h
  Initialize_BLE();
  Message("Updated 2019-08-06", 5000);
  setBacklight(true);
  //Start out on the cable page
  Page = PAGE_CBL; 
  kb_shift=false;
  Update_Page();
}

void loop() {
  if(Read_Buttons()) {
    setBacklight(true);
    //If buttons were pressed and you are in BLE mode, handle BLE mode.
    if (Page == PAGE_BLE) {
      switch (Current_Buttons) {
        case PUSHED_LEFT:   
          Message("LEFT",0);    
          send_BLE_KeyCode('l'); 
          break;
        case PUSHED_SELECT: 
          Message("SELECT",0);  
          send_BLE_KeyCode('s'); 
          break;
        case PUSHED_RIGHT:  
          Message("RIGHT",0);   
          send_BLE_KeyCode('r');  
          break;
        case PUSHED_ALL:
          exitBLE();  //Change back to IR mode
          break; 
      }
    } else {
      //If buttons were pressed and you are in IR mode, handle cursor and select commands
      #define ARROW_SPEED 150
      switch(Current_Buttons) {
        case PUSHED_RIGHT:  
          Do_Right();   delay(ARROW_SPEED); 
          break;
        case PUSHED_LEFT:   
          Do_Left();    delay(ARROW_SPEED); 
          break;
        case PUSHED_SELECT: 
          Do_Select();  delay(0); 
          break;
        case PUSHED_UP:     
          Do_Up();      delay(ARROW_SPEED + 200);
          break;
        case PUSHED_DOWN:   
          Do_Down();    delay(ARROW_SPEED + 200);
          break;
      };
    };
  };
  //If it's been too long since we processed a command, then turn off the backlight.
  //Any call to Update() will reset the timer.
  if (Backlight_Status){
    if ((millis()-Time_of_Last_Update)>(LIGHT_TIMEOUT*60ul*1000ul)) {
      setBacklight(false);//Blank display after certain number of minutes.
    }
  };
};
