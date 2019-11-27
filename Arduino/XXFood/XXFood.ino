#include<math.h>
#include<String.h>

/*

 PIN ASSIGNMENT
 
 21(it_2): Remote Signal In
 11(h), 12(v), 13(r): Horizontal, Vertical and Rotational Servos (Fixed Pin number for Timer1)
 22: Trigger Out
 24: Field Illumination LED
 25: Reset Hall Sensor
 26, 27, 28, 29: Loader Stepper
 30: Buzzer(sourceless)
 31: Bad Pin, Never Use It
 32(pul), 33(dir): Rail Stepper
 34: Grab Counter
 35: Food Counter
 36: Door Servo
 38, 39, 40, 41: Four Channels of Remote
 42(+), 43(+), 44(-), 45(-): Shutter
 
 A8: Drop Piezo

*/





// SYSTEM STATUS
boolean sysStartUp = true; // The flag indicating the start of system (set to false at the end of setup())
                           
struct command // Encapsulating motor positions into a struct
{
  double lPos; // Position of rail stepper in the number of steps
  double hPos; // Position of horizontal servo
  double rPos; // Position of horizontal compensatory (or called rotational) servo
  double vPos; // Position of vertical servo
};
typedef struct command Command; // The end of encapsulation

Command lastCmd; // Struct variable storing the latest executed command
Command newCmd; // Struct variable storing the unexecuted incoming command


// SERVO PARAMETERS
const byte doorServoPin = 36;
byte doorOpenDeg = 30;
byte doorCloseDeg = 150;
const byte servoCtrlPeriod = 20; // ms; industrial standard pulse period
double servoBegins[] = { 570, 570, 500, 1000 }; // us; pulse width corresponding to 0 degree
double servoEnds[] = { 2220, 2175, 1600, 2000 }; // us; pulse width corresponding to 180 degree
double servoAcc; // deg/frequency^2; maximum acceleration of natrual servos


// RAIL MOVEMENT PARAMETERS
const byte railPulPin = 32; // Pulsing for stepping
const byte railDirPin = 33; // Controlling direction of stepping
const byte railResetPin = 25; // Hall sensor input for detecting zero position
unsigned int railPulDur = 100; // us; the width of pulse for stepping
double railStepSize = 0.01; // mm; Displacement of platform on one step
double railMaxSpeed = 20; // mm/s
double railAcceleration = 40; // mm/s^2
unsigned int resetTimeOut = 8000; // The reset movement will be aborted after 5 seconds


// FOOD LOADER
const byte loaderPins[] = { 26, 27, 28, 29 };
byte thisStep = 3; // An arbitrary start point (0-3) for stepper initiation
byte remoteAdjSteps = 3; // Steps of adjustment made by one remote command
int loaderStepTime = 75; // ms; Determining the speed of rotation


// INFRARED SENSORS
const byte grabPin = 34;
const byte foodPin = 35;
byte sNumThrhIR = 2; // Spike Number Threshold for IR sensors;
                     // at least this many of consecutive spikes will be deemed as a reliable signal
byte critItvlIR = 50; // ms; critical Interval for IR sensors;
                      // two spikes are deemed consecutive if their interval is no bigger than this value

// DROP PIEZO
const byte piezoDPin = A8; // Drop detecting piezo
byte sNumThrhD = 3; // Spike Number Threshold for Drop piezo;
                    // at least this many of consecutive spikes will be deemed as a reliable signal
byte critItvlD = 150; // ms; critical Interval for IR sensors;
                      // two spikes are deemed consecutive if their interval is no bigger than this value
double multiplierD = 1.5; // Baseline times this value yielding the 'readThreshold' (see "Detector" tab)


// OTHER
const byte shutterPins[] = { 42, 43, 44, 45 };
const byte trgOutPin = 22;
const byte lumLedPin = 24; // Field illumination LED
volatile int lumLedStat = HIGH; // Status of field illumination LED

const byte remotePins[] = { 38, 39, 40, 41 };
unsigned long timeLastCmd = 0; // Time of last remote command executed

const byte buzzerPin = 30;
unsigned int toneFreq = 3000; // Hz
unsigned int toneDur = 300; // ms




// INITIALIZATION
void setup()
{
  Serial.begin(115200);
  
  // Initialize position 
  lastCmd.lPos = 0;
  lastCmd.hPos = 180;
  lastCmd.vPos = 180;
  lastCmd.rPos = 0;
  
  newCmd.lPos = 0;
  newCmd.hPos = 180;
  newCmd.vPos = 180;
  newCmd.rPos = 0;
  
  // Run setup for every modules
  RailSetup();
  ServoSetup();
  LoaderSetup();
  RemoteSetup();
  DetectorSetup();
  PulserSetup();
  MiscSetup();
  
  // Indicate that the system has finished startup
  sysStartUp = false;
}



// MAIN
void loop()
{
  ReadSerialCommand(); // Process serial input from computer
  MonitorBehavior(); // Monitor and report behavior information
}



// Movement Command Response
volatile boolean sliding = false; // A flag indicating whether the platform is sliding
volatile boolean rotating = false; // A flag indicating whether natural servos are rotating

void Move2Pos()
{
  NaturalRotate();
  NaturalMove(lastCmd.lPos, newCmd.lPos);
  lastCmd = newCmd;
  
  while(rotating | sliding)
  { }
  
  delay(200); // Waiting for motors to stabilize
  Serial.println("executed");
}
