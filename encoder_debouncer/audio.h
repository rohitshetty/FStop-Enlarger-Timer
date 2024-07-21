#ifndef AUDIO_H
#define AUDIO_H

#include <Arduino.h>



class AudioBeeper {
  public:
    AudioBeeper(uint8_t speaker_pin);

    void run();

    void mark_for_beeping(unsigned long dur, unsigned long freq);
    bool ENABLE_SOUND = false;

  private: 
    unsigned long audio_millis = 0;
    unsigned long period = 0; 
    unsigned long duration = 1000;
    uint8_t SPEAKER_PIN;
 
};

#endif