

// routine to led LED blink
void blinkLED_loopcount(){
  
  for(int i=0; i<ResetBlinkRepititions; i++){
    digitalWrite(ledPinWarning, HIGH);
    digitalWrite(acousticPin, HIGH);
    
    delay(300);
    digitalWrite(ledPinWarning, LOW);
    digitalWrite(acousticPin, LOW);
    delay(300);  
  }

  // reset blinking_warning flag to false
  blinking_warning_loopcount = false;
  
  //Serial.print("blinkLED finished");
  
}
