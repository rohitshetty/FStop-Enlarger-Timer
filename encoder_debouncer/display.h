#ifndef DISPLAY_H 
#define DISPLAY_H
#include<Arduino.h>
#include <TM1637Display.h>

#define SPLASH_SCREEN_DURATION 1500L

enum DISPLAY_STATES {
  SPLASH_SCREEN_MODE, 
  INTEGER_MODE, 
  ALPHA_NUMERIC_MODE
};

enum SPLASH_SCREEN_MESSAGE {
  SPLASH_READY, 
  SPLASH_PRNT, 
  SPLASH_STRP
};

/*   A
  ---
F|   |B
 | G |
  ---
E|   |C
 | D |
  ---
*/
const uint8_t READY_SEGS[] = {
  SEG_E | SEG_F | SEG_A,  // r
  SEG_D | SEG_E | SEG_F | SEG_A | SEG_B | SEG_G,  // e
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G, //d 
  SEG_F | SEG_G | SEG_B | SEG_C // y
};

const uint8_t PRNT_SEGS[] = {
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,  // p
  SEG_E | SEG_F | SEG_A,                  // r
  SEG_E | SEG_C | SEG_G,                  // n
  SEG_E | SEG_F | SEG_G | SEG_D           // t
};

const uint8_t STRP_SEGS[] = {
  SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,  // s
  SEG_E | SEG_F | SEG_G | SEG_D,          // t
  SEG_E | SEG_F | SEG_A,                  // r
  SEG_A | SEG_B | SEG_E | SEG_F | SEG_G   // p
};


/* this thing needs to show 
splash screen 
integers 
f stop count

*/


class Display {
  public: 
    Display(uint8_t clk, uint8_t dio);

    void run();
    void render_splash_screen();
    void show_splash_screen(enum SPLASH_SCREEN_MESSAGE show_msg);
    void render_numerics();
    void show_numerics(uint16_t number);
    // void set_data_to_show();


  private: 
    TM1637Display ssd_display;
    unsigned long splash_screen_shown_at = 0;
    DISPLAY_STATES display_state = SPLASH_SCREEN_MODE;
    bool UPDATE_DISPLAY_FLAG = false;
    SPLASH_SCREEN_MESSAGE splash_message = SPLASH_READY;

    uint16_t display_number = 0;


};

#endif