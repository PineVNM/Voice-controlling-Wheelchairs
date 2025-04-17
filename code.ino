/*
Set up EasyVR (voice voice recognition module)
*/

#include "Arduino.h"
#if !defined(SERIAL_PORT_MONITOR)
#error "Arduino version not supported. Please update your IDE to the latest version."
#endif

#if defined(__SAMD21G18A__)
// Shield Jumper on HW (for Zero, use Programming Port)
#define port SERIAL_PORT_HARDWARE
#define pcSerial SERIAL_PORT_MONITOR
#elif defined(SERIAL_PORT_USBVIRTUAL)
// Shield Jumper on HW (for Leonardo and Due, use Native Port)
#define port SERIAL_PORT_HARDWARE
#define pcSerial SERIAL_PORT_USBVIRTUAL
#else
// Shield Jumper on SW (using pins 12/13 or 8/9 as RX/TX)
#include "SoftwareSerial.h"
SoftwareSerial port(12, 13);
#define pcSerial SERIAL_PORT_MONITOR
#endif

#include "EasyVR.h"

EasyVR easyvr(port);

//Grammars and Words
enum Wordsets
{
  SET_1  = -1,
  SET_2  = -2,
};

enum Wordset1
{
  S1_ACTION = 0,
  S1_MOVE = 1,
  S1_TURN = 2,
  S1_RUN = 3,
  S1_LOOK = 4,
  S1_ATTACK = 5,
  S1_STOP = 6,
  S1_HELLO = 7,};

enum Wordset2
{
  S2_LEFT = 0,
  S2_RIGHT = 1,
  S2_UP = 2,
  S2_DOWN = 3,
  S2_FORWARD = 4,
  S2_BACKWARD = 5,
};

// use negative group for wordsets
int8_t group, idx;


/*
Set up Motors
*/

// Include the Stepper library:
#include <Stepper.h>

// Define number of steps per revolution:
const int stepsPerRevolution = 200;

// Give the motor control pins names:
  // Right Motor

    #define pwmR1 5
    #define pwmR2 6

    #define dirR1 4
    #define dirR2 7

  // Left Motor

    #define pwmL1 3
    #define pwmL2 11

    #define dirL1 8
    #define dirL2 9


// Initialize the stepper library on the motor shield:
  // Right Motor
    Stepper rightStepper(stepsPerRevolution, dirR1, dirR2);
  // Left Motor
    Stepper leftStepper(stepsPerRevolution, dirL1, dirL2);


/*
Set up Ultrasonic sensors
*/

// defines pins numbers
  // sensor 1
    const int trigPin1 = 1;
    const int echoPin1 = 2;
  // sensor 2
    // const int trigPin2 = 37;
    // const int echoPin2 = 38;

// defines variables
  byte stopDist = 50;   
  // sensor 1
    long duration1;
    int distance1;
  // sensor 2
    // long duration2;
    // int distance2;


void setup()
{
  // Sets the trigPin as Outputs
    pinMode(trigPin1, OUTPUT); 
    // pinMode(trigPin2, OUTPUT); 
  // Sets the echoPin as an Input 
    pinMode(echoPin1, INPUT); 
    // pinMode(echoPin2, INPUT); 

  // Set the PWM pins so that the direction pins can be used to control the motor:
    pinMode(pwmR1, OUTPUT);
    pinMode(pwmR2, OUTPUT);
    pinMode(pwmL1, OUTPUT);
    pinMode(pwmL2, OUTPUT);

    digitalWrite(pwmR1, HIGH);
    digitalWrite(pwmR2, HIGH);
    digitalWrite(pwmL1, HIGH);
    digitalWrite(pwmL2, HIGH);

  // Set the motor speed (RPMs):
    rightStepper.setSpeed(120);
    leftStepper.setSpeed(120);

  // setup PC serial port
  pcSerial.begin(9600);
bridge:
  // bridge mode?
  int mode = easyvr.bridgeRequested(pcSerial);
  switch (mode)
  {
    case EasyVR::BRIDGE_NONE:
      // setup EasyVR serial port
      port.begin(9600);
      // run normally
      pcSerial.println(F("Bridge not requested, run normally"));
      pcSerial.println(F("---"));
      break;

    case EasyVR::BRIDGE_NORMAL:
      // setup EasyVR serial port (low speed)
      port.begin(9600);
      // soft-connect the two serial ports (PC and EasyVR)
      easyvr.bridgeLoop(pcSerial);
      // resume normally if aborted
      pcSerial.println(F("Bridge connection aborted"));
      pcSerial.println(F("---"));
      break;

    case EasyVR::BRIDGE_BOOT:
      // setup EasyVR serial port (high speed)
      port.begin(115200);
      pcSerial.end();
      pcSerial.begin(115200);
      // soft-connect the two serial ports (PC and EasyVR)
      easyvr.bridgeLoop(pcSerial);
      // resume normally if aborted
      pcSerial.println(F("Bridge connection aborted"));
      pcSerial.println(F("---"));
      break;
  }

  // initialize EasyVR
  while (!easyvr.detect())
  {
    pcSerial.println(F("EasyVR not detected!"));
    for (int i = 0; i < 10; ++i)
    {
      if (pcSerial.read() == '?')
        goto bridge;
      delay(100);
    }
  }

  pcSerial.print(F("EasyVR detected, version "));
  pcSerial.print(easyvr.getID());

  if (easyvr.getID() < EasyVR::EASYVR3)
    easyvr.setPinOutput(EasyVR::IO1, LOW); // Shield 2.0 LED off

  if (easyvr.getID() < EasyVR::EASYVR)
    pcSerial.print(F(" = VRbot module"));
  else if (easyvr.getID() < EasyVR::EASYVR2)
    pcSerial.print(F(" = EasyVR module"));
  else if (easyvr.getID() < EasyVR::EASYVR3)
    pcSerial.print(F(" = EasyVR 2 module"));
  else
    pcSerial.print(F(" = EasyVR 3 module"));
  pcSerial.print(F(", FW Rev."));
  pcSerial.println(easyvr.getID() & 7);

  easyvr.setDelay(0); // speed-up replies
  // easyvr.setmicroDist(3); //set the mic distance 
  easyvr.setTimeout(5); //Set the number of seconds to listen for each command.
  easyvr.setLanguage(0); //Set language to English

  group = SET_1; //<-- start group (customize)
}

void loop()
{
  // Clears the trigPin
    digitalWrite(trigPin1, LOW);
    // digitalWrite(trigPin2, LOW);
    delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin1, HIGH);
    // digitalWrite(trigPin2, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);
    // digitalWrite(trigPin2, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
    duration1 = pulseIn(echoPin1, HIGH);
    // duration2 = pulseIn(echoPin2, HIGH);

  // Calculating the distance
    distance1 = duration1 * 0.034 / 2;  // Distance equation: Speed of sound x Time of flight / 2
    // distance2 = duration2 * 0.034 / 2;

  // Prints the distance on the Serial Monitor
    Serial.print("Distance 1: ");
    Serial.print(distance1);
    Serial.println(" cm");    
    // Serial.print("Distance 2: ");
    // Serial.print(distance2);
    // Serial.println(" cm");
  
  // Stop and move backward when having obstacle  
  // if (distance1 <= stopDist) 
  // // || (distance2 <= stopDist))
  // {
  //   setStepperIdle();
  //   Serial.println("TOO CLOSE");
  //   delay(300);
  //   moveBackward();
  // }


  if (easyvr.getID() < EasyVR::EASYVR3)
    easyvr.setPinOutput(EasyVR::IO1, HIGH); // LED on (listening)

    pcSerial.print("Say a word in Wordset: ");
    pcSerial.println(-group);
    easyvr.recognizeWord(-group);

  do
  {
    // allows Commander to request bridge on Zero (may interfere with user protocol)
    if (pcSerial.read() == '?')
    {
      setup();
      return;
    }
    // <<-- can do some processing here, while the module is busy
  }
  while (!easyvr.hasFinished());

  if (easyvr.getID() < EasyVR::EASYVR3)
    easyvr.setPinOutput(EasyVR::IO1, LOW); // LED off

  // handle voice recognition
  idx = easyvr.getWord();
  if (idx >= 0)
  {
    // beep
    easyvr.playSound(0, EasyVR::VOL_FULL);
    // print debug message
    uint8_t flags = 0, num = 0;
    char name[32];
    pcSerial.print("Word: ");
    pcSerial.print(idx);
    if (easyvr.dumpGrammar(-group, flags, num))
    {
      for (uint8_t pos = 0; pos < num; ++pos)
      {
        if (!easyvr.getNextWordLabel(name))
          break;
        if (pos != idx)
          continue;
        pcSerial.print(F(" = "));
        pcSerial.println(name);
        break;
      }
    }
    // perform some action
    action();
    return;
  }
  idx = easyvr.getCommand();
  if (idx >= 0)
  {
    // beep
    easyvr.playSound(0, EasyVR::VOL_FULL);
    // print debug message
    uint8_t train = 0;
    char name[32];
    pcSerial.print("Command: ");
    pcSerial.print(idx);
    if (easyvr.dumpCommand(group, idx, name, train))
    {
      pcSerial.print(" = ");
      pcSerial.println(name);
    }
    else
      pcSerial.println();
    // perform some action
    action();
  }
  else // errors or timeout
  {
    if (easyvr.isTimeout())
      pcSerial.println("Timed out, try again...");
    int16_t err = easyvr.getError();
    if (err >= 0)
    {
      pcSerial.print("Error ");
      pcSerial.println(err, HEX);
    }
  }
}

void setStepperIdle() { 
  digitalWrite(pwmR1, LOW);
  digitalWrite(pwmR2, LOW);
  digitalWrite(pwmL1, LOW);
  digitalWrite(pwmL2, LOW);
  digitalWrite(dirR1, LOW);
  digitalWrite(dirR2, LOW);
  digitalWrite(dirL1, LOW);
  digitalWrite(dirL2, LOW);
}

void moveBackward(){
  rightStepper.step(-stepsPerRevolution);
  leftStepper.step(-stepsPerRevolution);
  delay(2000);
  setStepperIdle();
  Serial.print("backward");
  delay(10);
}

void action()
{
  switch (group)
  {
    case SET_1:
      switch (idx)
      {
        case S1_ACTION:
          // write your action code here
          group = SET_2; // <-- or jump to another group or wordset for composite commands
          break;
        case S1_MOVE:
          // write your action code here
          // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
          break;
        case S1_TURN:
          // write your action code here
          // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
          break;
        case S1_RUN:
          // write your action code here
          // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
          break;
        case S1_LOOK:
          // write your action code here
          // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
          break;
        case S1_ATTACK:
          // write your action code here
          // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
          break;
        case S1_STOP:
          setStepperIdle();
          Serial.print("stop");
          delay(10);
          break;
        case S1_HELLO:
          // write your action code here
          // group = GROUP_X\SET_X; <-- or jump to another group or wordset for composite commands
          break;
      }
      break;
    case SET_2:
      switch (idx)
      {
        case S2_LEFT:
          rightStepper.step(stepsPerRevolution);
          leftStepper.step(-stepsPerRevolution);
          delay(2000);
          setStepperIdle();
          Serial.print("left");
          delay(10);
          break;

        case S2_RIGHT:
          rightStepper.step(-stepsPerRevolution);
          leftStepper.step(stepsPerRevolution);
          delay(2000);
          setStepperIdle();
          Serial.print("left");
          delay(10);
          break;

        case S2_UP:
          rightStepper.step(stepsPerRevolution);
          leftStepper.step(stepsPerRevolution);
          delay(2000);
          setStepperIdle();
          Serial.print("forward");
          delay(10);
          break;

        case S2_DOWN:
          rightStepper.step(-stepsPerRevolution);
          leftStepper.step(-stepsPerRevolution);
          delay(2000);
          setStepperIdle();
          Serial.print("backward");
          delay(10);
          break;

        case S2_FORWARD:
          rightStepper.step(stepsPerRevolution);
          leftStepper.step(stepsPerRevolution);
          delay(2000);
          setStepperIdle();
          Serial.print("forward");
          delay(10);
          break;

        case S2_BACKWARD:
          rightStepper.step(-stepsPerRevolution);
          leftStepper.step(-stepsPerRevolution);
          delay(2000);
          setStepperIdle();
          Serial.print("backward");
          delay(10);
          break;
      }
    break;
  }
}