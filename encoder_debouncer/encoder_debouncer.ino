#include <Arduino.h>
#include <TM1637Display.h>
#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.h"
#include "audio.h"
#include "display.h"

#define USING_TIM_DIV1                false           // for shortest and most accurate timer
#define USING_TIM_DIV16               true           // for medium time and medium accurate timer
#define USING_TIM_DIV256              false            // for longest timer but least accurate. Default

#define HW_TIMER_INTERVAL_MS      1L
ESP8266_ISR_Timer ISR_Timer;
ESP8266Timer ITimer;


// Module connection pins (Digital Pins)
#define DISP_CLK D2
#define DISP_DIO D1

#define ENC_CLK_PIN D7 
#define ENC_DT_PIN D6
#define ENC_BTN D5

#define MODE_CHANGE_BTN D4

#define PEDAL_BTN D3


#define SPKR_PIN D8

#define ENLARGER_CNTRL D0

#define LONG_PRESS_PERIOD 2000
#define SHORT_PRESS_PERIOD 1000


// TM1637Display display(DISP_CLK, DISP_DIO);
AudioBeeper audio_beeper(SPKR_PIN);

Display display(DISP_CLK, DISP_DIO);



unsigned long last_mode_change_button_down_pressed_at = 0;
bool START_RESET_TIMER = false;

volatile static uint8_t prevNextCode = 0;
volatile static uint16_t store=0;


enum MAJOR_MODES {
  TEST_STRIP_MODE, 
  PRINT_MODE
};

enum MINOR_MODES {
  SETUP_MODE, 
  RUN_MODE
};


volatile enum MAJOR_MODES major_mode = TEST_STRIP_MODE;
volatile enum MINOR_MODES minor_mode = SETUP_MODE;

volatile unsigned long running_millis = 0;
volatile unsigned long running_timer_counter = 0;

void IRAM_ATTR TimerHandler() {
  ISR_Timer.run();

}




volatile uint16_t encoder_counter = 0;



#define ROTARY_ENCODER_POLL_MS 1L
void rotary_encoder_poll() {
  // Rotary encoder debouncing from: https://www.best-microcontroller-projects.com/rotary-encoder.html
  // Follows a state machine approach to get valid transitions

  if (minor_mode == SETUP_MODE  ) {
      static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
  prevNextCode <<= 2;

  if (digitalRead(ENC_DT_PIN)) prevNextCode |= 0x02;
  if (digitalRead(ENC_CLK_PIN)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

   // If valid then store as 16 bit data.
   if  (rot_enc_table[prevNextCode] ) {
      store <<= 4;
      store |= prevNextCode;
      if ((store&0xff)==0x2b) {
        encoder_counter = encoder_counter -1;
        display.show_numerics(encoder_counter);
        // UPDATE_DISPLAY_FLAG = true;
      };
      if ((store&0xff)==0x17) {
        encoder_counter = encoder_counter +1 ;
        display.show_numerics(encoder_counter);
        // UPDATE_DISPLAY_FLAG = true;
      };
   }

  }
}

#define COUNTDOWN_RUNNING_TIMER_MS 1L
void countdown_running_timer() {
  // For every 100 ms we reduce the time by that count? 
  // 60.0 second is 60000ms/100ms = 600 counts 

  if (major_mode == PRINT_MODE && minor_mode == RUN_MODE) {


    if (running_timer_counter > 0) {
      digitalWrite(ENLARGER_CNTRL, LOW);
      
      if (running_timer_counter % 100 == 0) { 
        display.show_numerics(uint16_t (running_timer_counter/100));
      }
      // Avoid missing beeps for whatever reasons - mainly because of display I guess
      if (running_timer_counter % 1000 >=0 && running_timer_counter % 1000 < 5 && !audio_beeper.ENABLE_SOUND) {
        audio_beeper.mark_for_beeping(10, 1000);
      }
      running_timer_counter--;
      
    } else {
      digitalWrite(ENLARGER_CNTRL, HIGH);
        audio_beeper.mark_for_beeping(100, 1000);

      change_minor_mode();
    }

    // we do comparison if our value is reached 
    // if condition is good, we turn on relay, and on false, we turn off 
    // every 100 ms of the counter - we call the screen update 
    // every 1000 ms we call the beeper 
    // At the end of the timer, we switch
  }
}


#define PEDAL_BTN_DEBOUNCE_TIME_MS 1L
void pedal_btn_debounce_timer() {
  // Debouncing code from: https://www.ganssle.com/debouncing-pt2.htm 
  // Method two
  static uint16_t State = 0;
  State = (State<<1) | digitalRead(PEDAL_BTN) | 0xe000;
  if (State == 0xf000) {
    change_minor_mode();
  }
}

#define MODE_CHANGE_BTN_DEBOUNCE_TIME_MS 1L
void mode_change_btn_down_debounce_timer() {
  // Debouncing code from: https://www.ganssle.com/debouncing-pt2.htm 
  // Method two
  static uint16_t State = 0;
  State = (State<<1) | digitalRead(MODE_CHANGE_BTN) | 0xe000;
  if (State == 0xf000) {
    START_RESET_TIMER = true;
    last_mode_change_button_down_pressed_at = millis();
  }
}

void mode_change_btn_up_debounce_timer() {
    // Debouncing code from: https://www.ganssle.com/debouncing-pt2.htm 
  // Method two
  static uint16_t State = 0xffff;
  State = (State<<1) | digitalRead(MODE_CHANGE_BTN) | 0xe000;
  if (State == 0xe7ff) {
      START_RESET_TIMER = false;

      if (millis() - last_mode_change_button_down_pressed_at <= SHORT_PRESS_PERIOD) {
        change_major_mode();
      }
  }
}

void change_major_mode() {
  switch(major_mode) {
    case PRINT_MODE:
      init_test_strip_mode_setup();
    break;

    case TEST_STRIP_MODE:
    default:
      init_print_setup_mode();
    break;
  }
}

void change_minor_mode() {

  if (major_mode == PRINT_MODE && minor_mode == SETUP_MODE) {
 

    running_timer_counter = encoder_counter * 100;
    running_millis = millis();
    minor_mode = RUN_MODE;
  } else if (major_mode==PRINT_MODE && minor_mode == RUN_MODE) {
    minor_mode = SETUP_MODE;
    // encoder_counter = running_timer_counter / 100;
    display.show_numerics(encoder_counter);
  }
}

void init_print_setup_mode(){
  major_mode = PRINT_MODE;
  minor_mode = SETUP_MODE;

  display.show_splash_screen(SPLASH_PRNT);

}

void init_test_strip_mode_setup() {
  major_mode = TEST_STRIP_MODE;
  minor_mode = SETUP_MODE;

  display.show_splash_screen(SPLASH_STRP);
}


#define ENCODER_BTN_DEBOUNCE_TIME_MS 1L 
void encoder_btn_debounce_timer() {
  static uint16_t State = 0;
  if (minor_mode == SETUP_MODE) {
    State = (State<<1) | digitalRead(ENC_BTN) | 0xe000;
    if (State == 0xf000 ) {
      encoder_counter++;
      display.show_numerics(encoder_counter);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(ENC_CLK_PIN, INPUT_PULLUP);
  pinMode(ENC_DT_PIN, INPUT_PULLUP); 
  pinMode(ENC_BTN, INPUT_PULLUP); 
  pinMode(MODE_CHANGE_BTN, INPUT_PULLUP);


  pinMode(PEDAL_BTN, INPUT_PULLUP);

  pinMode(SPKR_PIN, OUTPUT);
  digitalWrite(SPKR_PIN, LOW);

  pinMode(ENLARGER_CNTRL, OUTPUT);
  pinMode(ENLARGER_CNTRL, HIGH);


  

  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS*1000, TimerHandler))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));

  ISR_Timer.setInterval(ROTARY_ENCODER_POLL_MS ,rotary_encoder_poll);
  ISR_Timer.setInterval(COUNTDOWN_RUNNING_TIMER_MS, countdown_running_timer);
  ISR_Timer.setInterval(PEDAL_BTN_DEBOUNCE_TIME_MS, pedal_btn_debounce_timer);
  ISR_Timer.setInterval(ENCODER_BTN_DEBOUNCE_TIME_MS, encoder_btn_debounce_timer);
  ISR_Timer.setInterval(MODE_CHANGE_BTN_DEBOUNCE_TIME_MS, mode_change_btn_up_debounce_timer);
  ISR_Timer.setInterval(MODE_CHANGE_BTN_DEBOUNCE_TIME_MS, mode_change_btn_down_debounce_timer);

  init_test_strip_mode_setup();
  

}



void loop() {
  check_for_reset();
  update_ui();
}

void check_for_reset() {
  if (minor_mode == SETUP_MODE && START_RESET_TIMER && (millis() - last_mode_change_button_down_pressed_at > LONG_PRESS_PERIOD)) {
    START_RESET_TIMER = false;
    encoder_counter = 0;
    display.show_numerics(encoder_counter);
  }
}
void update_ui() {

  audio_beeper.run();
  display.run();
}
