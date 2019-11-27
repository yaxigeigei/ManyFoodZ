#include<Servo.h>
#include<math.h>

// DIGITAL PINS ASSIGNMENT
// 0, 1: Serial port
// 2(it_0): Trigger In
// 3: Piezo sampling frequency indicator
// 4: Reset Sensor
// 5, 6, 7, 8: Loader Stepper
// 9 (theta), 10 (phi): Servos
// 11: Trigger Out
// 12 (pul), 13 (dir): Rail Stepper

// ANALOG PINS ASSIGNMENT
// A0(grab), A1(drop): Piezos
// A2 - A5: (Empty)



// CONSTRUCT DECLARATION
// Movement command paras
struct command
{
  double lPos;
  double hPos;
  double vPos;
  boolean linear2Angular;
};
typedef struct command Command;

Command lastCmd;
Command newCmd;



// 1) SERVOS PARAMETERS
// Constants
Servo servoH;
Servo servoV;
const int servoHPin = 9;
const int servoVPin = 10;
double servoStepTime = 20; // ms - control the smoothness of rotation
// User defined paras
static int servoH_BEGIN = 600; // 600
static int servoH_END = 2175; // 2175
static int servoV_BEGIN = 640; // 640
static int servoV_END = 2270; // 2270
double RotationAcceleration = 90; // degree/s^2
// Derived paras
double vr; // Machine operational angular speed (for constant rotate only)
double ar; // Machine operational angular acceleration (for natural rotate only)



// 2) STEPPER MOTOR PARAMETERS
boolean sysStartUp = true;
// Constants
const int pulPin = 12;
const int dirPin = 13;
// User defined paras
int pulseTime = 100; // us
double motorStepSize = 0.01; // mm
double motorMaxSpeed = 30; // mm/s
double motorAcceleration = 20; // mm/s^2
// Derived paras
double minStepTime; // us
int accSteps; // Steps needed for accelerating to maximum speed



// 3) FOOD LOADER
byte startPoint = 4;
// Constants
byte loaderPins[] = { 5, 6, 7, 8 };
double pi = M_PI; // from AVR library
byte loaderStepTime = 20; // ms



// 4) BEHAVIOR DETECTOR
boolean monitorPiezo = false; // Indicates whether to monitor analog signal or report result
volatile byte shakeTimes = 0;
volatile unsigned int shakeCounter = 0;
byte arrayLength = 64;
long grabTimes[64];
int grabDuras[64];
int gIndex = -1;
long timeDrop = 0; // Time of dropping
boolean dropAfter = false;
// Constants
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
const byte piezoGPin = A0; // Grab detecting piezo
const byte piezoDPin = A5; // Drop detecting piezo
const byte anaSinkPin = A1; // This pin sink any residual voltage in cap. 
const byte freqPin = 3; //  Output a square wave to oscilloscope whose frequency is a half of piezo sampling frequency
// User defined paras
double multiplierD = 1.5; // Sensitivity of Drop piezo - 1.5 is good
double readThresholdG = 10;
byte pNumThresholdD = 5; // Reliablity of Drop piezo
byte pNumThresholdG = 20; // Reliablity of Grab piezo
byte criticalInterval = 200; // ms - Two peaks are deemed continuous if their interval is no bigger than this value
volatile int shakeInterval = 50;
int holdTime = 1000; // ms - Minimal duration that the animal keeping the food for a successful grab
// Derived paras
double readThresholdD;




// 5) RESET
const int resetPin = 4;

// 6) TRIGGER-IN INTERRUPT
const int trgInPin = 2;
volatile static boolean trgDetected = false;

// 7) TRIGGER-OUT SIGNAL
const int trgOutPin = 11;





// INITIALIZATION
void setup()
{
  Serial.begin(115200);
  DetectorSetup();
  RailSetup();
  ServoSetup();
  LoaderSetup();
  TriggerSetup();

  lastCmd.lPos = 0; // Initialize position info
  lastCmd.hPos = 180;
  lastCmd.vPos = 180;
  newCmd.linear2Angular = true;
  
  sysStartUp = false;
}



// MAIN
void loop()
{
  ReadCommand();
//  CheckTriggered();
  PiezoListen();
}




// Movement Command Response
void Move2Pos()
{
  if(newCmd.linear2Angular)
  {
    CoNaturalRotate(lastCmd.hPos, newCmd.hPos, lastCmd.vPos, newCmd.vPos);
    NaturalMove(lastCmd.lPos, newCmd.lPos);
  }
  else
  {
    NaturalMove(lastCmd.lPos, newCmd.lPos);
    CoNaturalRotate(lastCmd.hPos, newCmd.hPos, lastCmd.vPos, newCmd.vPos);
  }
  lastCmd = newCmd;
  Serial.println("executed");
}
