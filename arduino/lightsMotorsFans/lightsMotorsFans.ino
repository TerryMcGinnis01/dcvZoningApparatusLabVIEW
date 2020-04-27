/*
   Controls 10 stepper motor positions, and 4 relays.
   START WITH ALL VALVES CLOSED!!!
   The angle/step is equal to 360/(number steps per revolution).
   Relay off --> 0
   Relay on --> 1

   Flow:
   Sends a "ready to receive data" byte.
   Waits for "acknowledged" byte.
   Received data and stores into array
   Stores sliced array into variables for each motor and relay.
   Turns on/off each relay.
   Moves the motors to the specified positions.
   Starts over
 */

// Define stepper motor connections:

int bytesRead = 0; //characters that are stored from incoming data
int incomingByte = 0; // for incoming serial data
int got = 0; //about to got a message
//Positions of all are set equal to zero at start.
int incoming[54] = {}; // 1-50 are Motor #1-10 positions; 5 digits to each motor. 51-54 are for relay on/off
int relay[4] = {2, 3, 4, 5}; //Pins for controlling each relay
int onoff[4] = {0, 0, 0, 0}; //0 --> off, 1 --> on.
int motoPin[10]= {22, 23, 28, 29, 34, 35, 40, 41, 46, 47}; //Each Motor has 3 pins assigned to it.  EX. Pins 22,24,26; 23,25,27 --> Step, Direction, Enable
long newPosition[10]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  //Desired Positions of motors 1-10
long originalPosition[10]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  //Current Positions of motors 1-10
long compare[10]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  //Displacement between Current and Desired Positions of motors 1-10
long maximum = 0;  //Used to measure maximum displacement (Minimum steps to take to get every motor where it needs to go)
long minimum = 0;  //A helper to measure maximum displacement (if the absolute maximum value is a negative number)
byte i = 0;        //This is a global variable counter.  Somehow this takes up less SRAM than local variables being used for counters (Using local variables in this code corrupts the long arrays)

void setup() {
        // Declare pins as output:
        for (int q = 0; q < 4; q++) {
                pinMode(relay[q],OUTPUT);
                digitalWrite(relay[q],HIGH);
        }
        for (int j = 22; j < 52; j++) {
                pinMode(j, OUTPUT);
                digitalWrite(motoPin[j] + 2, HIGH);
        }
        //Setup for Serial Communication
        Serial.begin(115200); // opens serial port, sets data rate to 115200 bps
        delay(500);
}
void loop() {
        Serial.println("o"); // sends a "ready to receive" byte

        while (0==0) {
                //We'll read anything available until an "acknowledged" byte is sent from the other device.  Note no data is stored during this loop
                if (Serial.available() > 0) {
                        got = Serial.read();
                        if (got == 33) { //33 is the "acknowledged" byte we're waiting for ('!' character)
                                break;
                        }
                }
        }

        while (bytesRead < 54) //We'll receive and store 54 characters.  This will move the specified motor to the specified position.  Then clear the stored characters and reset.
        {
                if (Serial.available() > 0) {
                        incomingByte = Serial.read() - 48;
                        if (incomingByte >= 0 && incomingByte <= 9) {
                                incoming[bytesRead] = incomingByte;
                                bytesRead++;
                        }
                }
        }

        bytesRead = 0; //Resets the counter for storing data, so it will go through that loop again once the full loop finishes.

        for (i = 0; i < 10; i++) {
                newPosition[i] = incoming[i * 5] * 10000L + incoming[i * 5 + 1] * 1000 + incoming[i * 5 + 2] * 100 + incoming[i * 5 + 3] * 10 + incoming[i * 5 + 4] * 1;
                //1-5 stored characters get mushed into a 5 digit number and stored.  Then 6-10, 11-15, and so on until 36-40.  8 5-digit numbers total get stored to desired positions.
                compare[i] = newPosition[i] - originalPosition[i]; //Finds the displacement for each motor and stores it.
                maximum = max(maximum, compare[i]); //These two lines find the min and max steps between all motor displacements.
                minimum = min(minimum, compare[i]); //
                if (compare[i] != 0) {
                        //If the motor needs to be moved, arduino powers it on.
                        digitalWrite(motoPin[i] + 4, LOW);
                        //arduino sets the direction the motor will turn
                        if (compare[i] < 0) {
                                digitalWrite(motoPin[i] + 2, LOW);
                        }
                        else {
                                digitalWrite(motoPin[i] + 2, HIGH);
                        }
                }
        }
        minimum = -1 * minimum;      //These two lines find the absolute max displacement for the motors.
        maximum = max(maximum, minimum); //

        //Switch the relays on/off
        for (i = 0; i < 4; i++) {
                onoff[i] = incoming[50+i];
                if (onoff[i] == 1) {
                        digitalWrite(relay[i],LOW);
                }
                else {
                        digitalWrite(relay[i],HIGH);
                }
        }

        //Put the motors where they need to go.  Each motor will take a step at the same time if it needs to.
        for (long p = 0; p < maximum + 1; p++) {
                //If the motor is where it needs to be, power off.  Otherwise take a step.
                for (i = 0; i < 10; i++) {
                        if (compare[i] == 0) {
                                digitalWrite(motoPin[i] + 4, HIGH);
                        }
                        if (compare[i] < 0) {
                                digitalWrite(motoPin[i], HIGH);
                        }
                        else if (compare[i] > 0) {
                                digitalWrite(motoPin[i], LOW);
                        }
                }

                delayMicroseconds(90);

                for (i = 0; i < 10; i++) {
                        if (compare[i] < 0) {
                                digitalWrite(motoPin[i], LOW);
                                compare[i]++; //Remember that a step was taken
                        }
                        else {
                                digitalWrite(motoPin[i], HIGH);
                                compare[i]--; //Remember that a step was taken
                        }
                }
                delayMicroseconds(90);
        }

        for (i = 0; i < 10; i++) {
                originalPosition[i] = newPosition[i];
        }

        //Reset max & min
        maximum = 0;
        minimum = 0;

}
