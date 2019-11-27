// Runtime paras (it ctrl)
const byte LINR_ACC = 1;
const byte SHAKE = 2;
volatile byte SMODE = 0;
volatile boolean rotating = false;
volatile int servoIT_us_counter_H;
volatile int servoIT_us_counter_V;

// Runtime paras (spd ctrl)
volatile boolean speedUp = true;
volatile double iniAngleIT_H;
volatile double iniAngleIT_V;
volatile double finalAngleIT_H;
volatile double finalAngleIT_V;
volatile double servoIT_v;
volatile double servoIT_vMaxAbs;
volatile double servoIT_a;
volatile double servoIT_k_H;
volatile double servoIT_k_V;
volatile double servoIT_timePoint = 1;

// Shake related paras
byte vShakePos[4];
byte hShakePos[4];
volatile double intervalMulti = 4; // Duration of each movement will be 20ms * this value
volatile byte lastMoveIndex;
volatile double thisMoveIndex;
volatile byte shakeCycles = 2;
volatile byte thisCycle = 1;


void ServoSetup()
{
  // Calculate derived parameters
  ar = servoAcceleration / square(1000.0 / servoCtrlPeriod); // Increment of velocity used in interrupt
  itCounterMax = 16000000 / (1000 / servoCtrlPeriod) / 64 - 1; // Time for counting to this value equals 20ms (servo ctrl period)
  
  if (sysStartUp)
  {
    // Assign pins for servos
    pinMode(servoHpin, OUTPUT);
    pinMode(servoVpin, OUTPUT);
    
    // Configure servo timer
    // Clear all existing settings
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    // Turn on Fast PWM mode with ICR1 set as TOP value
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13);
    // Set prescaler of 64
    TCCR1B |= (1 << CS11) | (1 << CS10);
    // Interrupt Points
    ICR1 = int(itCounterMax); // Set the TOP value for interruptting every 20ms
    OCR1A = int(Angle2count(180, servoH_begin, servoH_end)); // Set startup position of servo H to 180 degree
    OCR1B = int(Angle2count(180, servoV_begin, servoV_end)); // Set startup position of servo V to 180 degree
    TIMSK1 |= (1 << TOIE1); // Enable the overflow interrupt set by ICR1
    TIMSK1 |= (1 << OCIE1A) | (1 << OCIE1B); // Enable two comparing interrupts which stop pulses
    sei();
  }
}



unsigned long t = 0;

ISR(TIMER1_COMPA_vect) // for servoH
{
  digitalWrite(servoHpin, LOW);
}

ISR(TIMER1_COMPB_vect) // for servoV
{
  digitalWrite(servoVpin, LOW);
}

ISR(TIMER1_OVF_vect) // for firing up
{
  digitalWrite(servoHpin, HIGH);
  digitalWrite(servoVpin, HIGH);
  
  if (rotating)
  {
    static double currentAngleH;
    static double currentAngleV;
    
    switch(SMODE)
    {
      case LINR_ACC:
        if (speedUp && abs(servoIT_v) < servoIT_vMaxAbs)
        {
          currentAngleH = iniAngleIT_H + servoIT_v * servoIT_k_H * servoIT_timePoint;
          currentAngleV = iniAngleIT_V + servoIT_v * servoIT_k_V * servoIT_timePoint;
          servoIT_v += servoIT_a;
          servoIT_timePoint++;
        }
        else
        {
          if (abs(servoIT_v) > ar)
          {
            speedUp = false;
            currentAngleH = finalAngleIT_H - servoIT_v * servoIT_k_H * servoIT_timePoint;
            currentAngleV = finalAngleIT_V - servoIT_v * servoIT_k_V * servoIT_timePoint;
            servoIT_v -= servoIT_a;
            servoIT_timePoint--;
          }
          else
          {
            currentAngleH = finalAngleIT_H;
            currentAngleV = finalAngleIT_V;
            servoIT_timePoint = 1;
            speedUp = true;
            rotating = false;
            rotated = true;
          }
        } break;
      
//      case SINE_ACC:
//        break;
//      
//      case CONST:
//        break;
//        
//      case MAX: 
//        currentAngleH = finalAngleIT_H;
//        currentAngleV = finalAngleIT_V;
//        rotating = false; break;
      
      case SHAKE:
        if (thisCycle <= shakeCycles)
        {
          if (thisMoveIndex < lastMoveIndex + 1)
          {
            byte index = floor(thisMoveIndex);
            currentAngleH = hShakePos[index];
            currentAngleV = vShakePos[index];
            thisMoveIndex += 1.0 / intervalMulti;
            
            if (thisMoveIndex == lastMoveIndex + 1)
            {
              thisMoveIndex = 0;
              thisCycle++;
            }
          }
        }
        else
        {
          rotating = false;
          rotated = true;
        }
        break;
        
      default: break;
    }
    
    servoIT_us_counter_H = Angle2count(currentAngleH, servoH_begin, servoH_end);
    servoIT_us_counter_V = Angle2count(currentAngleV, servoV_begin, servoV_end);
    OCR1A = servoIT_us_counter_H - 1;
    OCR1B = servoIT_us_counter_V - 1;
  }
}





int Angle2count(double angle, double sBegin, double sEnd)
{
  double us = angle / 180.0 * (sEnd - sBegin) + sBegin;
  return int(round(us / deadZone));
}





// Two Servo Rotate Naturally Together
void NaturalRotate(double iniAngle_H, double finalAngle_H, double iniAngle_V, double finalAngle_V)
{
  if (!rotating)
  {
    // Find the biggest displacement
    double sH = abs(finalAngle_H - iniAngle_H);
    double sV = abs(finalAngle_V - iniAngle_V);
    
    // Calculate maximum speeds of two servos based on the same duration of rotation time
    double vMaxAbs_H;
    double vMaxAbs_V;
    if(abs(sH) >= abs(sV))
    {
      vMaxAbs_H = sqrt(sH * ar);
      vMaxAbs_V = sqrt(sV * (sV / sH) * ar);
    }
    else
    {
      vMaxAbs_V = sqrt(sV * ar);
      vMaxAbs_H = sqrt(sH * (sH / sV) * ar);
    }
    
    // Determine signs of velocity and acceleration respectively
    double vMax_H = vMaxAbs_H;
    double vMax_V = vMaxAbs_V;
    double a_H = ar;
    double a_V = ar;
    if(finalAngle_H - iniAngle_H < 0)
    {
      vMax_H *= -1;
      a_H *= -1;
    }
    if(finalAngle_V - iniAngle_V < 0)
    {
      vMax_V *= -1;
      a_V *= -1;
    }
    
    // Calculate the scaling factor (signed) for the maximum speed of the slower servo
    servoIT_k_H = 1;
    servoIT_k_V = 1;
    if(abs(sH) >= abs(sV))
    {
      servoIT_vMaxAbs = vMaxAbs_H;
      servoIT_a = a_H;
      servoIT_k_V = vMax_V / vMax_H; // i.e. Vv = Vh * k
    }
    else
    {
      servoIT_vMaxAbs = vMaxAbs_V;
      servoIT_a = a_V;
      servoIT_k_H = vMax_H / vMax_V; // i.e. Vv = Vh * k
    }
    
    // Wraps up variables for the interrupt
    servoIT_k_H /= 2.0; // simplifies the calculation in interrupt
    servoIT_k_V /= 2.0; // simplifies the calculation in interrupt
    servoIT_v = servoIT_a;
    
    iniAngleIT_H = iniAngle_H;
    iniAngleIT_V = iniAngle_V;
    finalAngleIT_H = finalAngle_H;
    finalAngleIT_V = finalAngle_V;
    
    // Start
    SMODE = LINR_ACC;
    rotating = true;
  }
}




void Shake()
{
  if (!rotating)
  {
    vShakePos[0] = lastCmd.vPos - 5;
    hShakePos[0] = lastCmd.hPos - 20;
    vShakePos[1] = lastCmd.vPos;
    hShakePos[1] = lastCmd.hPos;
    vShakePos[2] = lastCmd.vPos - 5;
    hShakePos[2] = lastCmd.hPos - 20;
    vShakePos[3] = lastCmd.vPos;
    hShakePos[3] = lastCmd.hPos;
    
    SMODE = SHAKE;
    lastMoveIndex = sizeof(vShakePos) / sizeof(byte) - 1;
    thisMoveIndex = 0;
    thisCycle = 1;
    
    rotating = true;
  }
}




void Sweep()
{

}
