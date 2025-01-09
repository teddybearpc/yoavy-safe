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

// Create stepper motor object
Stepper myStepper(stepsPerRevolution, motorPin1, motorPin3, motorPin2, motorPin4);

// Password and input variables
const char* correctPassword = "5538";
char enteredPassword[5]; // To store up to 4 characters plus null terminator
int passwordIndex = 0;

// Timing variables
unsigned long unlockTime = 0;
unsigned long lockTime = 0;

void setup() {
  Serial.begin(9600);

  // Set keypad pin modes
  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
  }
  for (int i = 0; i < COLS; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }

  // Set stepper motor speed
  myStepper.setSpeed(60); 
}

void loop() {
  char key = getKey();

  if (key) {
    Serial.print(key);
    enteredPassword[passwordIndex++] = key;
    enteredPassword[passwordIndex] = '\0'; // Null terminate the string

    if (passwordIndex >= 4) {
      if (strcmp(enteredPassword, correctPassword) == 0) {
        Serial.println(" - Password Correct");
        unlockSafe();
      } else {
        Serial.println(" - Incorrect Password"); 
      }
      passwordIndex = 0; // Reset for next entry
    }
  }

  // Check if it's time to lock the safe
  if (millis() - unlockTime >= 5 * 60 * 1000 && unlockTime != 0) { // 5 minutes
    lockSafe();
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

void unlockSafe() {
  Serial.println("Unlocking safe...");
  myStepper.step(stepsPerRevolution / 2); // Adjust steps as needed
  unlockTime = millis(); 
  lockTime = 0; // Reset lock time
}

void lockSafe() {
  Serial.println("Locking safe...");
  myStepper.step(-stepsPerRevolution / 2); // Reverse direction
  lockTime = millis();
  unlockTime = 0; // Reset unlock time
}

void SetLockStatus(int status) {
  if (status == 0) {
    lockSafe();
  } else if (status == 1) {
    unlockSafe();
  }
}
