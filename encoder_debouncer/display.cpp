#include "display.h"
#include <TM1637Display.h>
#include <Arduino.h>

Display::Display(uint8_t clk, uint8_t dio) 
  : ssd_display(clk, dio) {
    ssd_display.setBrightness(1);
    ssd_display.clear();
  };


void Display::show_splash_screen(enum SPLASH_SCREEN_MESSAGE show_msg) {
  display_state = SPLASH_SCREEN_MODE;
  UPDATE_DISPLAY_FLAG = true;
  splash_message = show_msg;
  splash_screen_shown_at = millis();
}

void Display::render_splash_screen() {
    if (millis() - splash_screen_shown_at > SPLASH_SCREEN_DURATION) {
      display_state = INTEGER_MODE; 
      UPDATE_DISPLAY_FLAG = true;
    } else {
      // We display the message that is inside the splash screen message 
      switch(splash_message) {
        case SPLASH_PRNT:
          ssd_display.setSegments(PRNT_SEGS);
          break;
        case SPLASH_STRP:
          ssd_display.setSegments(STRP_SEGS);
          break;
        case SPLASH_READY:
        default:
          ssd_display.setSegments(READY_SEGS);
      }

    }
}

void Display::render_numerics() {
    ssd_display.showNumberDec(display_number);
    UPDATE_DISPLAY_FLAG = false;
}

void Display::show_numerics(uint16_t number) {
  display_state = INTEGER_MODE;
  UPDATE_DISPLAY_FLAG = true;
  display_number = number;
}


void Display::run() {
  if (UPDATE_DISPLAY_FLAG) {
    switch(display_state) {


      case SPLASH_SCREEN_MODE:
        render_splash_screen();
      break;

      case INTEGER_MODE:
        render_numerics();
      break;

      case ALPHA_NUMERIC_MODE:
      break;
    }
}
//  else if (UPDATE_DISPLAY_FLAG) {

//     // This needs complex logic, depending on the major and minor mode, to show what we need. 
//     // ex we show the fstop etc.
//     display.showNumberDec(encoder_counter);
//     UPDATE_DISPLAY_FLAG = false;
//   }

}
