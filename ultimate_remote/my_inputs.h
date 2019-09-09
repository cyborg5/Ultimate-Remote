/*
 * In this file we need to define a function "Read_Buttons()"
 * This function will be continuously called waiting for you to do something.
 * If you aren't doing anything, it returned zero. If you are pushing a button, it
 * returns a value from the chart below. 
 * 
 * It also sets the global variable "Current_Buttons" to the returned value.
 * 
 * This could be the result of actually pushing buttons, input from sip and puff,
 *  capacitive touch or any other method of signaling.
 * In this particular example we have 2 methods of doing input:
 *    1) Three pushbuttons connected to GPIO pins
 *    2) If debugging is enabled you can enter commands through the serial monitor.
 *       'u'=Up, 'd'=Down, 'l'=Left, 'r'=Right, 's'=Select
 * If you are not using 3 input pins connected to 3 switches you will need to rewrite
 *   the function Read_Buttons() for your particular hardware setup. But it still should
 *   return values from the chart below.
 *   
 * Also define function "Initialize_Buttons()" which will be called by "setup()" at the beginning.
 */

/*
 * Everything below here is hardware specific
 */
//GPIO pin numbers for pushbuttons
//Although we used 3 analog GPIO pins we are using them as
// digital inputs. It just so happens the hardware makes these pins handy.
#define SELECT    A1
#define NEXT      A0
#define PREVIOUS  A2

//This macro encodes the three pushbuttons into a single value 0-7. 
//Note that digitalRead() will return "1" when the button is NOT pressed and
// returns "0" when it is pressed. That's the reason for the logical not "~" in the
// formula below.
#define  READ_SWITCHES (7&(~(digitalRead(PREVIOUS)*4+digitalRead(SELECT)*2+digitalRead(NEXT))) )

//The time stamp when the buttons first started to be pressed.
uint32_t Start_of_Press;

uint8_t Read_Buttons() {
  //Save the most recent button press for future comparison
  uint8_t Previous_Buttons = Current_Buttons;
  //Length of time that buttons have been pressed
  uint32_t Length_of_Press;
  //The following statement is not a mistake. We want to get the value, save it in Current_Buttons and 
  //  if it is nonzero do something.
  if (Current_Buttons = READ_SWITCHES) {
    //At this point we know that one or more buttons is currently being pressed.
    if(Current_Buttons==Previous_Buttons) {
      //If the value is the same as last time we called Read_Buttons then compute how long we've been this way.
      Length_of_Press=millis()-Start_of_Press;
    } else {
      //If the button state was different from a risetime we checked, reset the clock.
      Length_of_Press=0; 
      Start_of_Press=millis();
    }
    //For all pages except Bluetooth page, we want to use a chorded system.
    //The chances of pressing 2 buttons absolutely simultaneously is pretty slim 
    // so we continue to sample the buttons for 200 milliseconds and do a bitwise 
    // logical OR on the button states. That way if you press one button, then 
    // while holding it you press another one, and then while holding the second 
    // one you release the first one, it will register as a press of both buttons 
    // simultaneously. The bitwise OR also ensures that if the press is less than 200
    // milliseconds it will still register properly.
    if (Page!=PAGE_BLE) {//Use chorded
      uint8_t Sample;
      do {
        delay(50);  //We already know the buttons were pressed. Wait a fraction of a second.
        //Take a sample and bitwise OR
        Current_Buttons |= (Sample = READ_SWITCHES);
        Length_of_Press=millis()-Start_of_Press; //Recompute length of press
        if(Length_of_Press>200) {
          Sample=0;   //If we've been here long enough, get out
        }
      } while (Sample);
    } else {
      //Is BLE mode. Test to see if we should exit BLE mode.
      //There are three ways to exit.
      //  1) Press all three buttons simultaneously resulting in Current_Buttons==PUSHED_ALL
      //      This method takes care of itself.
      //  2) Hold any combination of one or more buttons for 8 seconds
      //  3) Hold Up or Down chord for more than one half second.
      //We could have combined these statements into a single condition but it was complicated
      // enough as it was. Separated it for readability sake.
      if (Length_of_Press > 8000)  {
        Current_Buttons = PUSHED_ALL;
      }
      if ((Length_of_Press > 500) && ((Current_Buttons>PUSHED_RIGHT) || (Current_Buttons == PUSHED_UP))) {
        Current_Buttons = PUSHED_ALL;
      }
    }
  } else {
    //Buttons are not pressed
    Previous_Buttons=Length_of_Press=0;
  }
//Alternate input method using serial monitor for debugging purposes
#if (MY_DEBUG)
  if(Serial.available ()) {
    uint8_t c= Serial.read ();
    switch  (c) {
      case 'u': Current_Buttons=PUSHED_UP; break;
      case 'd': Current_Buttons=PUSHED_DOWN; break;
      case 'l': Current_Buttons=PUSHED_LEFT; break;
      case 's': Current_Buttons=PUSHED_SELECT; break;
      case 'r': Current_Buttons=PUSHED_RIGHT;  break;
    }
  }
#endif

//Alternate input method using touchscreen
#if (USE_TOUCHSCREEN)
  if(Current_Buttons== PUSHED_NONE) {
    Current_Buttons= Read_Touchscreen();
  }
#endif
  return  Current_Buttons;
};

void Initialize_Buttons(void) {
  Start_of_Press = 0; 
  pinMode (SELECT, INPUT_PULLUP);
  pinMode (NEXT, INPUT_PULLUP);
  pinMode (PREVIOUS, INPUT_PULLUP);
}
