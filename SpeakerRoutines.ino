void acoustic_warning_loopcount(){
    for(int i=0; i<ResetBlinkRepititions; i++){
      digitalWrite(acousticPin, HIGH);
      delay(300);
      digitalWrite(acousticPin, LOW);
      delay(300);
    }
}
