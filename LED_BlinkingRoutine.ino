

// routine to led LED blink
void blinkLED(){
  
  for(int i=0; i<ResetBlinkRepititions; i++){
    digitalWrite(ledPinWarning, HIGH);
    delay(300);
    digitalWrite(ledPinWarning, LOW);
    delay(300);  
  }

  // reset blinking_warning flag to false
  blinking_warning = false;
  
  //Serial.print("blinkLED finished");
  
}




//---------------------------------------------------------------------------------------------------------
// if both interrupts are pushed, this reset loop will reset loopcount variable and set the blinking flag
void reset_loop_warning(){
  
  //Serial.print("start reset loop");
  
  // reset global loopcount variable to 0
  loopcount = 0;
  
  // blink LED to signal resetting
  blinking_warning = true;
  
}
