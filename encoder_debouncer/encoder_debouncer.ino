#include <Arduino.h>
#include <TM1637Display.h>
#include "ESP8266TimerInterrupt.h"
#include "ESP8266_ISR_Timer.h"

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
#define ENC_SW D8

#define PEDAL_BTN D3
TM1637Display display(DISP_CLK, DISP_DIO);

volatile static uint8_t prevNextCode = 0;
volatile static uint16_t store=0;

enum GLOBAL_ENLARGER_TIMER_MODES {
  IDLE, RUNNING
} ;

static enum GLOBAL_ENLARGER_TIMER_MODES global_enlarger_timer_mode = IDLE;

void IRAM_ATTR TimerHandler() {
  ISR_Timer.run();

}


volatile uint16_t encoder_counter = 0;
volatile bool UPDATE_DISPLAY_FLAG = false;
volatile bool MAKE_TICK_FLAG = false;

#define ROTARY_ENCODER_POLL_MS 1L
void rotary_encoder_poll() {
  // Rotary encoder debouncing from: https://www.best-microcontroller-projects.com/rotary-encoder.html
  // Follows a state machine approach to get valid transitions

  if (global_enlarger_timer_mode == IDLE) {
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
        UPDATE_DISPLAY_FLAG = true;
      };
      if ((store&0xff)==0x17) {
        encoder_counter = encoder_counter +1 ;
        UPDATE_DISPLAY_FLAG = true;
      };
   }

  }
}

#define COUNTDOWN_RUNNING_TIMER_MS 10L
void countdown_running_timer() {
  // For every 100 ms we reduce the time by that count? 
  // 60.0 second is 60000ms/100ms = 600 counts 
  if (global_enlarger_timer_mode == RUNNING) {
    // Reduce the global timer.
  }
}


#define PEDAL_BTN_DEBOUNCE_TIME_MS 1L
void pedal_btn_debounce_timer() {
  // Debouncing code from: https://www.ganssle.com/debouncing-pt2.htm 
  // Method two
  static uint16_t State = 0;
  State = (State<<1) | digitalRead(PEDAL_BTN) | 0xe000;
  if (State == 0xf000) {
    encoder_counter++;
    UPDATE_DISPLAY_FLAG=true;
  }

}

void setup() {
  Serial.begin(115200);

  display.setBrightness(1);
  display.clear();
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(ENC_CLK_PIN, INPUT_PULLUP);
  pinMode(ENC_DT_PIN, INPUT_PULLUP); 

  pinMode(PEDAL_BTN, INPUT_PULLUP);


  if (ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_MS*1000, TimerHandler))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));


  ISR_Timer.setInterval(ROTARY_ENCODER_POLL_MS ,rotary_encoder_poll);
  ISR_Timer.setInterval(COUNTDOWN_RUNNING_TIMER_MS, countdown_running_timer);
  ISR_Timer.setInterval(PEDAL_BTN_DEBOUNCE_TIME_MS, pedal_btn_debounce_timer);
}

void loop() {

// We use this to update the display and make the audio feedback
// all the inputs are run from Timer interrupts at regular interval. 
// We disable some of those based on the current mode to save time
  if (UPDATE_DISPLAY_FLAG) {
    display.showNumberDec(encoder_counter);   
    UPDATE_DISPLAY_FLAG = false;
  }

  if (MAKE_TICK_FLAG) {
    // make sound for 10 ms ;
    make_tick_sound();
    MAKE_TICK_FLAG = false;
  }
  

}

void make_tick_sound() {
  // Here a square wave of 1 ms is made for 10ms 
  // Basically 1khz sound, for 10ms - 
}
