/*******************************************************************************
 * These routines handle BLE mode which sends Bluetooth keystrokes to an iOS 
 *   device to use the iOS switch control features. It emulates a Bluetooth keyboard
 *   and sends 'l' for "Left", 's' for Select, and 'r' for "Right".
 *   
 *  The code also allows for long presses which gives you access to other iOS functions.
 *  It does not use chorded commands. When you press a button, it sends a "key down"
 *  command via Bluetooth and when you release the button it sends "key up".
 */

#if (USE_BLE)
  //Bluefruit stuff
  #include <SPI.h>
  #include "Adafruit_BLE.h"
  #include "Adafruit_BluefruitLE_SPI.h"
  #include "BluefruitConfig.h"
  Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
#endif

//This routine is called during "setup()"
void Initialize_BLE(void) {
  display.fillScreen(ILI9341_BLACK); 
  #if(USE_BLE)
    if ( !ble.begin(VERBOSE_MODE))
    {
      error("NO BLE?");
    } else {
      Message("Found BLE OK!", 1000);
      if (! ble.sendCommandCheckOK("AT+GAPDEVNAME=Bluefruit Switch Access") ) {
        error("err:name?");
      } else {
        ble.echo(false);
        Message("Enable KB:", 500);
        if(!ble.sendCommandCheckOK("AT+BleHIDEn=On")) {
          error("err:enable Kb");
        } else {
          /* Add or remove service requires a reset */
          Message("SW reset", 500); 
          if (! ble.reset() ) {
            error("err:SW reset");
          } else {
            Message("Completed BLE initialization", 1000);
          }
        }
      }
    }
  #else
    Message("BLE not enabled in this compile.", 1000);
  #endif
  display.fillScreen(ILI9341_BLACK); 
}

//Exit Bluetooth mode and go back to IR mode
void exitBLE(void) {
  #if (USE_BLE)
    Message("ALL3", 500); 
    Page = Old_Page;  //Go back to the page we were on before we went into Bluetooth mode
    Update_Page(); 
  #endif
}

//This function translates the Bluetooth keystroke to the iOS device.
//The parameter passed is an ASCII character such as 'l', 's', or 'r'.
//This has to be translated into a keyboard code in hexadecimal format.
//The entire command for keypress is 
//    "AT+BLEKEYBOARDCODE=00-00-xx-00-00-00-00"
//    where xx is replaced with a 2 digit hex code for the particular key.
void send_BLE_KeyCode (uint8_t c) {
  #if (USE_BLE)
    ble.print("AT+BLEKEYBOARDCODE=00-00-");
    uint8_t Code = c - 'a' + 4;
    if (Code < 0x10) {  //If less than 0x10 we have to include the leading zero
      ble.print("0");
    }
    ble.print(Code, HEX);
    ble.println("-00-00-00-00");  //The string is not completed until we do println
    #if(MY_DEBUG)
      Message("Pressed.",0);
    #endif
    delay(100);//de-bounce
    //This condition basically says "if only one button is pressed"
    while (Current_Buttons && ( (Current_Buttons == PUSHED_RIGHT) || (Current_Buttons < PUSHED_UP) ) ) { 
      //Keep reading the button until it is released.
      Read_Buttons();
    };
    //Now release the Bluetooth keypress
    ble.println("AT+BLEKEYBOARDCODE=00-00");
    #if(MY_DEBUG)
      Message("Released",0);
    #endif
    //If you pushed all three buttons or other means of exiting then do exit BLE mode
    if(Current_Buttons==PUSHED_ALL) {
      exitBLE();
    }
    delay(200);//de-bounce release
  #endif
}
