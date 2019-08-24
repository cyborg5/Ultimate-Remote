/*
 * This page contains all of the code producing the touchscreen to control the device.
 * We divide the screen into three segments: Left, Center, and Right where the left and right
 * sections move the cursor left and right in the center section selects. Unfortunately the screen is not
 * capable of multitouch so we cannot use corded commands.
 */

#if (USE_TOUCHSCREEN)
#include "Adafruit_STMPE610.h"

Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

void Initialize_Touch(void) {
  if(ts.begin()) {
    Message("Touchscreen initialized", 1000); 
  } else {
    Message("Touchscreen error", 2000);
  }
};

uint8_t Read_Touchscreen(void) {
  if(! ts.bufferEmpty()) {
    TS_Point p = ts.getPoint();
    p.x=map(p.x,0,4095,-150,150);
    p.y=map(p.y,0,4095,-150,150);
    #if(MY_DEBUG)
      Serial.print("Touched at ("); Serial.print(p.x); Serial.print(","); Serial.print(p.y); Serial.print(") ");
    #endif
    if( (abs(p.y) < 50) && (abs(p.x < 50) ) ) {
      #if(MY_DEBUG)
        Serial.println("Touched select.");
      #endif
      return PUSHED_SELECT;
    } else if(p.y > abs(p.x)) {
      #if(MY_DEBUG)
        Serial.println("Touched right.");
      #endif
      return PUSHED_RIGHT;
    } else if(-p.y > abs(p.x) ) {
      #if(MY_DEBUG)
        Serial.println("Touched left.");
      #endif
      return PUSHED_LEFT;
    } else if(p.x > abs(p.y)) {
      #if(MY_DEBUG)
        Serial.println("Touched down.");
      #endif
      return PUSHED_DOWN;
    } else if(-p.x > abs(p.y)) {
      #if(MY_DEBUG)
        Serial.println("Touched up.");
      #endif
      return PUSHED_UP;
    } else {
      #if(MY_DEBUG)
        Serial.println("unknown.");
      #endif      
    }
  }
  return PUSHED_NONE;
};
#else
void Initialize_Touch(void) {};
uint8_t Read_Touchscreen(void) {
  return PUSHED_NONE;
};
#endif
