#include<math.h>
#include<String.h>

// DIGITAL PINS ASSIGNMENT
// 0, 1: Serial port
// 2(it_0): Trigger In
// 3: Imaging Synchronization LED
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
};
typedef struct command Command;

Command lastCmd;
Command newCmd;



// 1) SERVOS PARAMETERS
volatile boolean rotated = false;
// Constants
const byte servoHpin = 9;
const byte servoVpin = 10;
double servoCtrlPeriod = 20; // ms
double deadZone = 4; // us
// User defined paras
double servoH_begin = 570; // us
double servoH_end = 2150; // us
double servoV_begin = 640; // us
double servoV_end = 2250; // us
double servoAcceleration = 90; // degree/s^2
double sweepAngle = 15; // degree
// Derived paras
volatile double ar; // from servoAcceleration and servoCtrlPeriod
volatile int itCounterMax; // from servoCtrlPeriod and deadZone




// 2) STEPPER MOTOR PARAMETERS
boolean sysStartUp = true;
// Constants
const byte pulPin = 12;
const byte dirPin = 13;
// User defined paras
int pulseTime = 100; // us
double motorStepSize = 0.01; // mm
double motorMaxSpeed = 20; // mm/s
double motorAcceleration = 40; // mm/s^2
// Derived paras
double minStepTime; // us
int accSteps; // Steps needed for accelerating to maximum speed



// 3) FOOD LOADER
byte startPoint = 4;
volatile boolean loaded = false;
// Constants
byte loaderPins[] = { 5, 6, 7, 8 };
const double pi = M_PI; // from AVR library
byte loaderStepTime = 75; // ms



// 4) BEHAVIOR DETECTOR
boolean monitorPiezo = false; // Indicates whether to monitor analog signal or report result
long timeLastPeakG = 0; // ms - Stores last read peak of Grab piezo
int peaksContG = 0; // Continuous peaks read
long timeLastPeakD = 0; // ms - Stores last read peak of Drop piezo
int peaksContD = 0; // Continuous peaks read
boolean canceled = false;
byte arrayLength = 64;
long grabTimes[64];
int grabDuras[64];
int gIndex = -1;
long timeDrop = 0; // Time of dropping
boolean dropAfter = false;
// Constants
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
const byte piezoGPin = A0; // Grab detecting piezo
const byte piezoDPin = A5; // Drop detecting piezo
const byte anaSinkPin = A1; // This pin sink any residual voltage in capacitor
// User defined paras
double multiplierD = 1.5; // Sensitivity of Drop piezo - 1.5 is good
double multiplierG = 7;
byte pNumThresholdD = 5; // Reliablity of Drop piezo
byte pNumThresholdG = 10; // Reliablity of Grab piezo
byte criticalInterval = 150; // ms - Two peaks are deemed continuous if their interval is no bigger than this value
int holdTime = 1000; // ms - Minimal duration that the animal keeping the food for a successful grab
// Derived paras
double readThresholdD;
double readThresholdG;



// 5) RESET
const byte resetPin = 4;

// 6) TRIGGER-IN INTERRUPT
const byte trgInPin = 2;
volatile static boolean trgDetected = false;

// 7) TRIGGER-OUT SIGNAL
const byte trgOutPin = 11;
const byte trgSynPin = 3;




// INITIALIZATION
void setup()
{
  lastCmd.lPos = 0; // Initialize position info
  lastCmd.hPos = 180;
  lastCmd.vPos = 180;
  newCmd.lPos = 0; // Initialize position info
  newCmd.hPos = 180;
  newCmd.vPos = 180;
  
  Serial.begin(115200);
  
  DetectorSetup();
  RailSetup();
  ServoSetup();
  LoaderSetup();
  TriggerSetup();
  
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
  NaturalRotate(lastCmd.hPos, newCmd.hPos, lastCmd.vPos, newCmd.vPos);
  NaturalMove(lastCmd.lPos, newCmd.lPos);
  lastCmd = newCmd;
  
  while(!rotated)
  { }
  rotated = false;
  
  delay(200); // Wait for motors to stablize
  Serial.println("executed");
}
