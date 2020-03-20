
//---------------------------------------------------------------------------------------------------------
// interrupt-routine for first interrupt-button
// has a debouncing routine and starts the actual interrupt function
void debounceInterrupt() {
  if ((long)(micros() - last_micros) >= debouncing_time*1){
    Interrupt();
    last_micros = micros();
  }
}


// interrupt-routine for second interrupt-button
// has a debouncing routine and starts the actual interrupt function
void debounceInterruptTwo() {
  if ((long)(micros() - last_micros) >= debouncing_time*1){
    InterruptTwo();
    last_micros = micros();
  }
}




//---------------------------------------------------------------------------------------------------------
// actual interrupt-routine for first interrupt-button
void Interrupt() {
  // Invert status: LED from HIGH to LOW
  stateOne = HIGH;
  stateTwo = LOW;
  
  //Serial.print("I1 started...");
  
  //check, if second interrupt pin is also pushed (for reset mode)
  if (digitalRead(interruptPinTwo)==LOW){
    reset_loop_warning();
    //Serial.print("worked!");
  }
  
}
// actual interrupt-routine for second interrupt-button
void InterruptTwo() {
  stateTwo = HIGH;
  stateOne = LOW;
}




//---------------------------------------------------------------------------------------------------------
// if both interrupts are pushed, this reset loop will reset loopcount variable and set the blinking flag
void reset_loop_warning(){
  
  //Serial.print("start reset loop");
  
  // reset global loopcount variable to 0
  loopcount = 0;
  
  // blink LED to signal resetting
  blinking_warning_loopcount = true;
  
}
