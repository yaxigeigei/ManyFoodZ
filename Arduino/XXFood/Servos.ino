// Runtime Parameters for Natural Servos
volatile boolean speedUp = true;
volatile double servoIT_timePoint = 1;
volatile double servoIT_v;
double servoIT_vMaxAbs;
double servoIT_a;

double iniAngleIT[3];
double finalAngleIT[3];
double servoIT_k[3];



void ServoSetup()
{
  if (sysStartUp)
  {
    // Assign pins for servos
    for (int i = 0; i < 3; i++)
      pinMode(11 + i, OUTPUT);
      
    pinMode(doorServoPin, OUTPUT);
      
    // Translate deg/s^2 to deg/frequency^2
    servoAcc = ConvertServoAcc(90);
    
    // Clear all existing settings
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    // Turn on No.14 Fast PWM mode with ICR1 set as TOP value
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13);
    // Enable OC1A, OC1B, and OC1C on Pin 11, 12, and 13 respectively (Clear on match and set at bottom)
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << COM1C1);
    // Set prescaler of 8 for counting every 0.5us
    TCCR1B |= (1 << CS11);
    // Interrupt Points
    ICR1 = int(16000000 / 8 / (1000 / servoCtrlPeriod) - 1); // Set the TOP value for interruptting every 20ms
    OCR1A = us2count(servoEnds[0]); // Set startup position of servo H to 180 degree
    OCR1B = us2count(servoEnds[1]); // Set startup position of servo V to 180 degree
    OCR1C = us2count(servoBegins[2]); // Set startup position of servo R to 0 degree
    
    TCCR4A = 0;
    TCCR4B = 0;
    TCNT4 = 0;
    ICR4 = 0.02 * 16000000/8 - 1; //  39999
    RotateTo(doorCloseDeg);
    TCCR4A |= (1 << WGM41);//fast PWM mode
    TCCR4B |= (1 << WGM43)|(1 << WGM42)|(1 << CS41); //fast PWM mode and Set prescaler of 8
    TIMSK4 |= (1 << OCIE4A)|(1 << TOIE4);//make COMPA and OVF effectable
    sei();
  }
}



unsigned long t = 0;

ISR(TIMER1_OVF_vect) // for firing up
{
  static double currentAngle[3];
  
  if (speedUp && abs(servoIT_v) < servoIT_vMaxAbs)
  {
    double tempVar = servoIT_v * servoIT_timePoint;
    for (int i = 0; i < 3; i++)
      currentAngle[i] = iniAngleIT[i] + tempVar * servoIT_k[i];
    servoIT_v += servoIT_a;
    servoIT_timePoint++;
  }
  else
  {
    if (abs(servoIT_v) > servoAcc)
    {
      speedUp = false;
      double tempVar = servoIT_v * servoIT_timePoint;
      for (int i = 0; i < 3; i++)
        currentAngle[i] = finalAngleIT[i] - tempVar * servoIT_k[i];
      servoIT_v -= servoIT_a;
      servoIT_timePoint--;
    }
    else
    {
      for (int i = 0; i < 3; i++)
        currentAngle[i] = finalAngleIT[i];
      servoIT_timePoint = 1;
      TIMSK1 = 0;
      rotating = false;
    }
  }
  
  OCR1A = Angle2count(currentAngle[0], servoBegins[0], servoEnds[0]) - 1;
  OCR1B = Angle2count(currentAngle[1], servoBegins[1], servoEnds[1]) - 1;
  OCR1C = Angle2count(currentAngle[2], servoBegins[2], servoEnds[2]) - 1;
}




// Three Servo Rotate Naturally Together
void NaturalRotate()
{
  if (!rotating)
  {
    // Assignment
    double iniAngle[] = { lastCmd.hPos, lastCmd.vPos, lastCmd.rPos };
    double finalAngle[] = { newCmd.hPos, newCmd.vPos, newCmd.rPos };
    double dist[3]; // Signed distances to rotate
    double vMax[3]; // Signed maximum speed during rotation
    double acc[3]; // Signed acceleration
    int ref = 0;
    
    for (int i = 0; i < 3; i++)
    {
      dist[i] = finalAngle[i] - iniAngle[i];
      acc[i] = servoAcc;
      servoIT_k[i] = 1;
    }
    
    // Find the biggest displacement out of three servos and take it as the reference
    double maxDist = 0;
    for (int i = 0; i < 3; i++)
    {
      maxDist = max(maxDist, abs(dist[i]));
      if (maxDist == abs(dist[i]))
        ref = i;
    }
    
    // Given the rotation time is the same, calculate respective maximum speeds and signs
    for (int i = 0; i < 3; i++)
    {
      vMax[i] = sqrt(abs(square(dist[i])/dist[ref]) * servoAcc);
      if (dist[i] < 0)
        vMax[i] *= -1;
    }
    
    // Calculate the scaling factor (signed) for the maximum speed of three servos
    servoIT_vMaxAbs = abs(vMax[ref]); // Unsigned maximum speed value of the fastest servo
    for (int i = 0; i < 3; i++)
      servoIT_k[i] = vMax[i] / servoIT_vMaxAbs;
    
    // Wrap up variables for the interrupt
    servoIT_a = acc[ref];
    servoIT_v = servoIT_a;
    for (int i = 0; i < 3; i++)
    {
      servoIT_k[i] /= 2.0; // simplifies the calculation in interrupt
      iniAngleIT[i] = iniAngle[i];
      finalAngleIT[i] = finalAngle[i];
    }
    
    // Start
    speedUp = true;
    rotating = true;
    TIMSK1 |= (1 << TOIE1); // Enable the overflow interrupt that calculates next toggles
  }
}



double ConvertServoAcc(double degAcc)
{
  return degAcc / square(1000.0/double(servoCtrlPeriod));
}




ISR(TIMER4_COMPA_vect)
{
  digitalWrite(doorServoPin, HIGH);
}

ISR(TIMER4_OVF_vect)
{
  digitalWrite(doorServoPin, LOW);
}

void RotateTo(double deg) // => myservo.write(deg)
{
  double us = servoBegins[3] + deg * (servoEnds[3] - servoBegins[3])/180.0;
  OCR4A = (20000 - us) * ICR4 / 20000;
}



double Angle2us(double angle, double sBegin, double sEnd)
{
  return angle / 180.0 * (sEnd - sBegin) + sBegin;
}


int us2count(double us)
{
  return round(us * 2);
}


int Angle2count(double angle, double sBegin, double sEnd)
{
  return us2count(Angle2us(angle, sBegin, sEnd));
}
