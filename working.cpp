#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// Define keypad layout
// Define keypad layout
// Define keypad layout
const byte ROWS = 5; // 5 rows
const byte COLS = 4; // 4 columns
char *specialKeys[] = {
    "F1", "F2", "#", "*",
    "1", "2", "3", "UP",
    "4", "5", "6", "DOWN",
    "7", "8", "9", "ESC",
    "LEFT", "0", "RIGHT", "ENTER"};

char specialKeysID[] = {
    'A', 'B', '#', '*',
    '1', '2', '3', 'C',
    '4', '5', '6', 'D',
    '7', '8', '9', 'E',
    'F', '0', 'G', 'H'};

char keys[ROWS][COLS] = {
    {specialKeysID[0], specialKeysID[1], specialKeysID[2], specialKeysID[3]},
    {specialKeysID[4], specialKeysID[5], specialKeysID[6], specialKeysID[7]},
    {specialKeysID[8], specialKeysID[9], specialKeysID[10], specialKeysID[11]},
    {specialKeysID[12], specialKeysID[13], specialKeysID[14], specialKeysID[15]},
    {specialKeysID[16], specialKeysID[17], specialKeysID[18], specialKeysID[19]}};

byte rowPins[ROWS] = {38, 36, 34, 32, 30}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {22, 24, 26, 28};     // connect to the column pinouts of the kpd

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Define LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD I2C address

// Define pins
const int motorPin1 = 9;
const int motorPin2 = 10;
const int limitswitch1 = 2; // pin for the start button (limit switch 1)
const int limitswitch2 = 3; // pin for the stop button (limit switch 2)
const int relayPin = 13;

// Define variables
int loopCount = 0;
int N = 0;
int limitSwitch2Count = 0;
const int limitswitch1InterruptPin = 2; // The pin number for Limit Switch 1 should match the signal pin connected to it.
volatile bool isMotorRunning = false;
volatile bool motorActive = false; // This flag controls the state of the motor loop.
volatile bool restartMotorSequence = false;
volatile bool pause = false;
int storedLoopCount = 0;

// Function prototypes
void startMotorSequence();
void checkMotorDirection();
void fetchNValue();
void askNValueConfirmation();
void stopEverything();
void resetArduino();

bool directionConfirmed = false;
// int N = 0;              // Global variable to store the N value
bool nValueSet = false; // Global flag to determine if N has been set

void setup()
{
    // Initialize the LCD
    lcd.init();
    lcd.backlight();

    // Initialize pins
    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    pinMode(limitswitch1, INPUT_PULLUP);
    pinMode(limitswitch2, INPUT_PULLUP);
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH); // Assume relay is active LOW

    // Show welcome message
    lcd.print("Welcome");
    delay(500);
    lcd.clear();

    attachInterrupt(digitalPinToInterrupt(limitswitch1InterruptPin), limitSwitch1InterruptHandler, FALLING);

    checkMotorDirection();
}
void restartMotorSequenceHandler()
{
    delay(100); // Simple debouncing
    if (digitalRead(limitswitch1) == LOW)
    {
        restartMotorSequence = true;
    }
}

void limitSwitch1InterruptHandler()
{
    // delay(100); // Simple debouncing
    if (digitalRead(limitswitch1) == LOW)
    {
        // isMotorRunning = !isMotorRunning;
        pause = !pause;
        digitalWrite(relayPin, HIGH);
        digitalWrite(motorPin1, LOW);
        // digitalWrite(motorPin2, LOW);

        // lcd.print("big motor off");
    }
}

void checkMotorDirection()
{
    lcd.clear();
    lcd.print("Select direction");

    bool directionSelected = false; // Need this flag to exit the loop once direction is selected

    while (!directionSelected)
    {
        char key = keypad.getKey();
        if (key)
        { // Check if a key is pressed
            switch (key)
            {
            case 'G': // If LEFT button is pressed
                // print the direction on the LCD
                lcd.clear();
                lcd.print("Direction: Right");
                motorForwardForShortDuration();
                directionSelected = true;
                break;

            case 'F': // If RIGHT button is pressed
                // print the direction on the LCD
                lcd.clear();
                lcd.print("Direction: Left");
                motorReverseUntilLimitSwitch2();
                directionSelected = true;
                break;

            case 'D': // If DOWN button is pressed
            case 'E': // If ESCAPE button is pressed
                directionConfirmed = true;
                fetchNValue();
                directionSelected = true;
                break;

            case '*': // If * button is pressed, skip direction check
                directionConfirmed = true;
                // Proceed to fetch N value
                fetchNValue();
                directionSelected = true;
                break;

            default:
                // Do nothing if any other key is pressed
                break;
            }
            delay(100); // Wait a bit before checking again for key press, to avoid bouncing
        }
    }
}
void stopOrResetIfNeeded()
{
    char key = keypad.getKey();
    if (key)
    { // if a key is pressed

        switch (key)
        {

        case '*':
            lcd.clear();
            lcd.print("Wait for reset");
            break;

        case '#':
            stopEverything();
            break;

            // other keys if necessary...
        }
    }
}

void loop()
{
    if (pause)
    {
        lcd.clear();
        lcd.print("Loop paused");
        while (pause)
        {
            // delay(100); // Small delay to allow other processes
        }
        lcd.clear();
        lcd.print("Resuming loop");
        delay(1000); // Give some time to read the message before continuing
    }

    if (!pause)
    {
        if (!nValueSet)
        {
            fetchNValue();
            lcd.clear();
            lcd.print("limit switch 1");
        }
        else if (nValueSet)
        {

            // Check if Limit Switch 1 is pressed to start the motor sequence
            if (digitalRead(limitswitch1) == LOW)
            { // Assuming LOW when pressed
                // Debounce the limit switch
                delay(50);

                if (digitalRead(limitswitch1) == LOW)
                {
                    isMotorRunning = true;
                    startMotorSequence();
                }
            }
        }
    }

    // All main logic is handled within sub-functions
}

void motorReverseUntilLimitSwitch2()
{
    // Code for reversing motor until limit switch 2 is triggered
    // This part of the code will involve motor control logic and monitoring limit switch 2

    // Example logic (you need to fill in with actual motor control logic):
    lcd.clear();
    lcd.print("Motor reversing");

    // Activate motor in reverse direction
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);

    // Wait for limit switch 2 to be triggered
    while (digitalRead(limitswitch2) != LOW)
    {
        // Continuously check the limit switch
    }

    // Once limit switch 2 is triggered, stop and increment counter
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW); // Stop motor
    limitSwitch2Count++;
    lcd.clear();
    lcd.print("Motor Forward");

    // Move motor forward for 0.2 seconds
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW); // Forward direction
    delay(2000);                  // Wait for 0.2 seconds

    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW); // Stop motor

    lcd.clear();
    lcd.print("Motor stopped");

    // Confirm direction and move to next step
    directionConfirmed = true;
    fetchNValue();
}

void motorForwardForShortDuration()
{
    // Code for moving motor forward for a short duration

    // Example logic (you need to fill in with actual motor control logic):
    lcd.clear();
    lcd.print("Motor forward");

    // Activate motor in forward direction
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);

    delay(2000); // Wait for 0.2 seconds

    // Stop motor
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);

    lcd.clear();
    lcd.print("Motor stopped");
    delay(200); // Wait for 0.2 seconds

    // Confirm direction and move to next step
    directionConfirmed = true;
    fetchNValue();
}

void fetchNValue()
{
    lcd.clear();
    lcd.print("Give the N value:");

    N = 0; // Reset the N value
    bool enterPressed = false;

    // Collect user input until the "ENTER" key is pressed
    while (!enterPressed)
    {
        char key = keypad.getKey();
        if (key)
        { // if a key is pressed
            if (key >= '0' && key <= '9')
            { // if the key is a digit
                if (N < 10)
                {                             // This is a simple check. Adjust it to allow for larger numbers as needed for your use case.
                    N = N * 10 + (key - '0'); // Append the digit to the current value of N
                    lcd.setCursor(0, 1);      // Move cursor to the second line of the LCD
                    lcd.print(N);             // Display current value of N
                }
            }
            else if (key == 'C')
            {
                // Go back to previous prompt which is "checkMotorDirection()"
                checkMotorDirection();
                return;
            }
            else if (key == 'D')
            {
                // Skip the current function and keep going
                enterPressed = true;
            }
            else if (key == 'H')
            {
                // The user has confirmed the N value
                enterPressed = true;
            }
        }
        // Add necessary delay if key debouncing is required
    }

    // Confirm the entered N value
    askNValueConfirmation();
}

void askNValueConfirmation()
{
    // After the user has entered the N value, we should ask for confirmation
    lcd.clear();
    lcd.print("N is ");
    lcd.print(N);
    lcd.setCursor(0, 1);
    lcd.print("F1-Yes F2-No");

    bool confirmation = false;
    while (!confirmation)
    {
        char key = keypad.getKey();
        if (key)
        { // if a key is pressed
            if (key == 'A' || key == 'H')
            {
                nValueSet = true;
                lcd.clear();
                lcd.print("Press limit switch 1");
                confirmation = true;
                // The user has confirmed the N value

                // No longer call startMotorSequence() here
                // Proceed to other logic
            }
            else if (key == 'B' || key == 'E')
            {
                // The user has rejected the N value. Prompt for a new value.
                fetchNValue();
                confirmation = true; // Exit the loop
            }
        }
    }
}

void checkForImmediateStop()
{
    char key = keypad.getKey();
    if (key == '#')
    {
        stopEverything();
    }
}
void startMotorSequence()
{
    motorActive = true;
    int motorDelayTime = N * 1000 / 1; // Calculate delay time (t) in milliseconds.

    for (loopCount; isMotorRunning && loopCount < 4;)
    {

        if (!pause)
        {
            // Check if the motor is still running after each step
            while (!isMotorRunning)
            {
                delay(10);             // Wait for a short period to prevent tightly locked loop
                stopOrResetIfNeeded(); // <--- HERE
            }

            // Check again if motorActive is still true, since it might be changed by "stopEverything()"
            if (motorActive)
            {
                digitalWrite(relayPin, LOW);
                lcd.clear();
                lcd.print("Motor is ON ");
                lcd.print(loopCount + 1);

                while (!isMotorRunning)
                {
                    delay(10);             // Wait for a short period to prevent tightly locked loop
                    stopOrResetIfNeeded(); // <--- HERE
                }

                while (pause)
                {
                    lcd.clear();
                    lcd.print("paused");
                }
                // Forward loop operation
                delay(motorDelayTime);
                while (!isMotorRunning)
                {
                    delay(10);             // Wait for a short period to prevent tightly locked loop
                    stopOrResetIfNeeded(); // <--- HERE
                }

                digitalWrite(motorPin1, HIGH);
                digitalWrite(motorPin2, LOW);
                delay(3000);
                digitalWrite(motorPin1, LOW);
                digitalWrite(motorPin2, LOW);

                lcd.clear();
                lcd.print("Loop ");
                lcd.print(loopCount + 1);
                lcd.print(" done");
                loopCount++;
            }

            // After each operation, check if the motor is still running
            while (!isMotorRunning)
            {
                delay(10);               // Wait for a short period to prevent tightly locked loop
                checkForImmediateStop(); // Check for immediate stop request
            }
        }
        // After 4 loops, go reverse until it touches Limit Switch 2
        if (motorActive && loopCount == 4)
        {
            char key = keypad.getKey();
            lcd.clear();
            lcd.print("Relay pin off");
            digitalWrite(relayPin, HIGH);
            lcd.clear();
            lcd.print("Reversing");
            digitalWrite(motorPin1, LOW);
            digitalWrite(motorPin2, HIGH);

            while (digitalRead(limitswitch2) != LOW)
            {
            }

            digitalWrite(motorPin1, LOW);
            digitalWrite(motorPin2, LOW);

            lcd.clear();
            lcd.print("Reverse complete");

            // Update the count of how many times Limit Switch 2 was triggered
            limitSwitch2Count++;
            lcd.setCursor(0, 1);
            lcd.print("LS2 Count: ");
            lcd.print(limitSwitch2Count);
            loopCount = 0;
            while (key != '*')
            {
                key = keypad.getKey();
                delay(20);
                if (digitalRead(limitswitch1) == LOW)
                {
                    isMotorRunning = true;
                    startMotorSequence();
                }
            }
            resetArduino();
        }
    }
}

void stopEverything()
{
    char key = keypad.getKey();
    while (key != '#')
    {
        key = keypad.getKey(); // This line will keep checking the keypad for '*' key to be pressed
        delay(20);             // Adding a small delay to avoid bouncing effect and to give system some time for other tasks
    }

    // Stop all operations
    // This function could be called during an emergency stop or a stop command from the user.
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW); // Stop the motor
    digitalWrite(relayPin, HIGH); // Turn the relay off (assuming active LOW)

    motorActive = false; // Update motorActive flag to stop the loop operation

    lcd.clear();
    lcd.print("All stopped!"); // Notify the user
    while (key != '*')
    {
        key = keypad.getKey(); // This line will keep checking the keypad for '*' key to be pressed
        delay(20);             // Adding a small delay to avoid bouncing effect and to give system some time for other tasks
    }

    // resetArduino(); // Resets the system to initial state
    softwareReset();

    // isMotorRunning = false;
}

void softwareReset()
{
    asm volatile("  jmp 0");
}

void resetArduino()
{
    // Reset Arduino to the initial state
    // This function could reset all variables and states back to the beginning
    // and be invoked when a reset is requested by the user or system.
    loopCount = 0;
    limitSwitch2Count = 0;
    motorActive = false; // Ensure motor is not in a running state
    nValueSet = false;   // Reset the N value set flag
    isMotorRunning = false;
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW); // Stop the motor
    digitalWrite(relayPin, HIGH); // Turn the relay off (assuming active LOW)
    lcd.clear();
    lcd.print("System Reset!");
    delay(1000); // Wait for 1 second
    lcd.clear();
    softwareReset();

    // Call the initial functions again to start over
    checkMotorDirection(); // Optionally call the direction function if that's the intended reset behaviour
    fetchNValue();         // Comment this out if you want to start with motor direction after reset
}

// Add other necessary functions ...