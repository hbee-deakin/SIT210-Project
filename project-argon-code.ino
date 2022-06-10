// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_DHT_Particle.h>

// This Library is used for i2c communication between devices
#include<Wire.h>

// This is to prevent my argon device from going offline
SYSTEM_MODE(AUTOMATIC) 

// This Ensures  application loop is not interrupted by background processes and network management
SYSTEM_THREAD(ENABLED) 

// Initialise the LEDs to corresponding Pins
const pin_t GRN_LED = D4; // Green LED
const pin_t ORG_LED = D5; // Orange LED
const pin_t RED_LED = D6; // Red LED

// The state of the cat: inside, outside or missing
int status = 0; 

// Used to determine a new RFID reading
int received = 0; 

// Time limit for timer (12 hours in milliseconds = 1000 * 60 * 60 * 12 )
int timeLimit = 43200000; 

// Timer used to determine whether the pet is missing
// If pet is away and timer reaches the time limit it will trigger the pet missing function 
Timer timer(timeLimit, petMissing); 

// This reset LED function is used to reset all the LED lights to OFF except one
// This function takes a single parameter 'x' used to turn the corresponding LED ON
void resetLED(int x) {
    digitalWrite(GRN_LED, LOW);
    digitalWrite(ORG_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(x, HIGH);
}

// This set up function is used to set up and initialised the variables
// This function only runs once, at the begining of the program
void setup()
{
    // Begin the wire used for I2C communication as slave with address 8
    Wire.begin(0x8);
    
    // Initialise the event to run when there is a reading from I2C master device
    Wire.onReceive(receiveEvent);
  
    // Initialse the pinmodes for LEDs and set the pin mode as outputs
    pinMode(GRN_LED, OUTPUT);
    pinMode(ORG_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
  
    // Turn off all LEDs except for GREEN
    resetLED(GRN_LED);
  
    // Establish serial communication
    Serial.begin();
  
    // Prevent my argon device from going offline
    Particle.keepAlive(30); 
  
}

// This function is used to handle the event when data is received from the master device
// The message received from the I2C master communicates the current state which is set to the status variable
void receiveEvent(int howMany) {
    // Used in for loop to determine a new reading
    received = howMany;
    
    // Read the message while the wire is available and store to the state to the status variable
    while (Wire.available()){
        status = Wire.read();
    }
}

// Initialise LEDs to beginning state when the program begins
void init(){
    // Sets all lights OFF except for GREEN
    resetLED(GRN_LED);
}

// The function which is called when the Pet enters the door
void petEnter(){
    // Publishes event to Particle.io, which is used by IFTTT to send phone alert that pet has entered
    Particle.publish("petdoor", "Enter", PRIVATE);
    // Sets all lights OFF except GREEN
    resetLED(GRN_LED);
    // Sets the timer to stop
    timer.stop();
}

// The function which is called when the Pet exits the door
void petExit(){
    // Publishes event to Particle.io, which is used by IFTTT to send phone alert that pet has left
    Particle.publish("petdoor", "Exit", PRIVATE);
    // Sets all lights OFF except ORANGE
    resetLED(ORG_LED);
    // Sets the timer to start
    timer.start();
}

// The function which is called when the Pet is missing and hasnt returned within the time limit
void petMissing(){
    // Publishes event to Particle.io, which is used by IFTTT to send phone alert that pet is missing
    Particle.publish("petdoor", "Missing", PRIVATE);
    // Sets the status to 2 = missing
    status = 2;
    // Sets all lights OFF except RED
    resetLED(RED_LED);
    // Sets the timer to stop
    timer.stop();
}

// This function repeats and runs every time frame of the program
void loop()
{
    // Check to see if there has been a new reading receieved from the RFID reader
    // If there is a reading, the status determines the function to call
    if (received > 0) {
        // If the status is 0, then run the pet enter function
        // Delay for 15 seconds, this prevents overloading of particle publish events but keeping to a limit of 1 publish per 15 seconds
        if (status == 0){
            petEnter();
            delay(15000);
        }
        // If the status is 1, then run the pet exit function
        // Delay for 15 seconds, this prevents overloading of particle publish events but keeping to a limit of 1 publish per 15 seconds
        else if (status == 1){
            petExit();
            delay(15000);
        }
        // If the status is 2, then run the initialise function
        else if (status == 2){
            init();
        }
        // Reset the received variable to 0, ready to store a new message
        received = 0;
    }
}