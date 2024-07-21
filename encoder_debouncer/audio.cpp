#include "audio.h"
#include <Arduino.h>


AudioBeeper::AudioBeeper(uint8_t speaker_pin) {
  SPEAKER_PIN = speaker_pin;
}


void AudioBeeper::mark_for_beeping(unsigned long dur, unsigned long freq) {
  audio_millis = millis();
  ENABLE_SOUND = true;
  period = 3;
  duration = dur;
}


void AudioBeeper::run() {
  unsigned long currentMillis = millis();

  if (ENABLE_SOUND && (currentMillis - audio_millis <= duration)) {
      digitalWrite(SPEAKER_PIN, HIGH);
  } else {
    digitalWrite(SPEAKER_PIN, LOW);
    ENABLE_SOUND = false;
  }
  
}