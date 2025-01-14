#include <Stepper.h>

// Define stepper motor parameters
const int stepsPerRevolution = 200; // Change this to match your stepper motor
const int motorPin1 = 8;
const int motorPin2 = 9;
const int motorPin3 = 10;
const int motorPin4 = 11;
const int NO_KEY = 0;

// Define keypad parameters
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; // Connect to the column pinouts of the keypad

// Password and input variables
const char* correctPassword = "5538";
char enteredPassword[5]; // To store up to 4 characters plus null terminator
int passwordIndex = 0;

// Timing variables
unsigned long unlockTime = 0;
unsigned long lockTime = 0;

// Create Stepper object
Stepper myStepper(stepsPerRevolution, motorPin1, motorPin3, motorPin2, motorPin4);

// KeypadHandler Class for handling keypad input
class KeypadHandler {
public:
  KeypadHandler() {
    for (int i = 0; i < ROWS; i++) {
      pinMode(rowPins[i], OUTPUT);
    }
    for (int i = 0; i < COLS; i++) {
      pinMode(colPins[i], INPUT_PULLUP);
    }
  }

  char getKey() {
    for (int i = 0; i < ROWS; i++) {
      digitalWrite(rowPins[i], LOW);
      for (int j = 0; j < COLS; j++) {
        if (digitalRead(colPins[j]) == LOW) {
          while (digitalRead(colPins[j]) == LOW); // Wait for key release
          digitalWrite(rowPins[i], HIGH);
          return keys[i][j];
        }
      }
      digitalWrite(rowPins[i], HIGH);
    }
    return NO_KEY;
  }

  bool checkPassword(char* enteredPassword) {
    return strcmp(enteredPassword, correctPassword) == 0;
  }
};

// Safe Class for handling the motor actions
class Safe {
public:
  Safe(Stepper& stepper) : myStepper(stepper) {}

  void unlock() {
    Serial.println("Unlocking safe...");
    myStepper.step(stepsPerRevolution / 2); // Adjust steps as needed
    unlockTime = millis();
    lockTime = 0; // Reset lock time
  }

  void lock() {
    Serial.println("Locking safe...");
    myStepper.step(-stepsPerRevolution / 2); // Reverse direction
    lockTime = millis();
    unlockTime = 0; // Reset unlock time
  }

  void updateLockStatus() {
    if (millis() - unlockTime >= 5 * 60 * 1000 && unlockTime != 0) { // 5 minutes
      lock();
    }
  }

private:
  Stepper& myStepper;
};

// Global objects
KeypadHandler keypadHandler;
Safe safe(myStepper);

// Timing variables
void setup() {
  Serial.begin(9600);
  myStepper.setSpeed(60); // Set stepper motor speed
}

void loop() {
  char key = keypadHandler.getKey();

  if (key) {
    Serial.print(key);
    enteredPassword[passwordIndex++] = key;
    enteredPassword[passwordIndex] = '\0'; // Null terminate the string

    if (passwordIndex >= 4) {
      if (keypadHandler.checkPassword(enteredPassword)) {
        Serial.println(" - Password Correct");
        safe.unlock();
      } else {
        Serial.println(" - Incorrect Password"); 
      }
      passwordIndex = 0; // Reset for next entry
    }
  }

  // Check if it's time to lock the safe
  safe.updateLockStatus();
}
