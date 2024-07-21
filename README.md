## Logs

1. TM1637 takes few milliseconds to push the data out - this was causing issue with the sound feedback
2. The timer interrupts are kinda sorta software interrupt - so other places, if I am doing too much operations, can sink time - but have been aware of that, and has been avoided at places.
3. Major mode switches between test strip print mode, and print mode. Minor mode switches between running and setup mode.
4. Print mode is ready - you can set time, and let it run down to zero.
5. Test strip print mode needs

- in setup mode, acquire base time, f stop, and number of breakpoints.
- in run mode, we go from zero, stopping at each break points.
- rotary encoder will need to handle changing of different values. Including, base time, f stops etc.
- Display will need alpha numeric mode to show f stop
- Setup mode for test strip print will need a state machine of sorts so that it can go between all 3 acquisition points.
  - the encoder btn will switch between the acquisition stages
  - will be set to defaults
  - pedal will start the run
  - different mode switch will need to convey to display somehow what to show.
  - Having WS821b would make signaling on the breakpoints easier.
