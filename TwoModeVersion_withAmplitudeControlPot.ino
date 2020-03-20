#include <Servo.h> 


Servo myservo;



int potpin = A0; // analog Pin for potentiometer
int val; // variable for value of potentiometer


const byte ledPinOne = 13; // define pin for LED 13
const byte ledPinTwo = 6; // define pin for LED 6
const byte ledPinWarning = 5; // define pin for LED 5

// define interruptpins
const byte interruptPinOne = 1;
const byte interruptPinTwo = 0;
const byte interruptPinThree = 7;

// Define globale volatile variables for the status of the LEDs
volatile byte stateOne = HIGH;
volatile byte stateTwo = LOW;

// to prevent debouncing define debounce time
long debouncing_time = 50;
volatile unsigned long last_micros;
//in debouncing routine we need to use several timestamps
long timestamp = 0;
long timestamptwo = 0;
long timeone = 1000;
long timetwo = 2000;

//variable to define the duration of a breath-cycle
long amplitude;

//variable to count main-loops --> we want warning after defined number of loops
long int loopcount = 0;
long int loopcount_warning = 10;
// if this flag is set by interrupt-routine, LED will blink and signal reset of loopcount
bool blinking_warning = false;
// define number of blinking for reset_warning
int ResetBlinkRepititions = 5;


//---------------------------------------------------------------------------------------------------------
// main loop of the program
void loop() {
  
  // Schalte die LEDs an
  digitalWrite(ledPinOne, stateOne);
  digitalWrite(ledPinTwo, stateTwo);

  val = analogRead(potpin); // liest das Potentiometer aus (Wert zwischen 0 und 1023)
  val = map(val, 0, 1023, 12, 26); // rechnet den Wert in den Wertebereich des Servomotors (zwischen 0 und180)

  //duration of one breath-cycles per minute
  // is adjusted with the potentiometer value between 12 and 26 cycles per minute
  amplitude = 60000/val;
  
  
  //routine for mode 1:
  if(stateOne){
    modeOne(amplitude);
  }

  //routine for mode 2:
  if(stateTwo){
    modeTwo(amplitude);   
  }

  //number of completed loops is increased after every loop
  loopcount++;
  // as a warning the LED is turned off after defined number of loops
  if (loopcount == loopcount_warning){
    digitalWrite(ledPinWarning, HIGH);
  }

  //let LED blink warning if reset was done
  if (blinking_warning == true){
    blinkLED();
    blinking_warning = false;
  }
  
}
