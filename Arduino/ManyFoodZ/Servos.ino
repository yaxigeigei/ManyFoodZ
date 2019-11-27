void ServoSetup()
{
  if (!sysStartUp)
  {
    servoH.detach(); // To avoid bug in Servo firmware
    servoV.detach(); // To avoid bug in Servo firmware
  }
  
  servoH.attach(servoHPin, servoH_BEGIN, servoH_END);
  servoV.attach(servoVPin, servoV_BEGIN, servoV_END);
  
  // Calculate servo working acceleration
  ar = RotationAcceleration / square(1000.0 / servoStepTime);
  
  // Configure shaking movement timer
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0; // Initialize counter value to 0
  OCR2A = 249; // i.e. 16000000(CPU_f) / 1000(final_f) / 64(prescaler) - 1
  TCCR2A |= (1 << WGM21); // Turn on CTC mode
  TCCR2B |= (1 << CS22); // Set CS21 bit for 64 prescaler
  TIMSK2 |= (1 << OCIE2A); // Enable timer compare interrupt
  sei();
}


void Shake(byte t)
{
  shakeTimes = t;
}


ISR(TIMER2_COMPA_vect)
{
  if(shakeTimes > 0)
  {
    shakeCounter++;
    if (shakeCounter == shakeInterval)
    {
      servoV.write(lastCmd.vPos - 15);
      servoH.write(lastCmd.hPos);
    }
    else if (shakeCounter == 2 * shakeInterval)
    {
      servoV.write(lastCmd.vPos);
      servoH.write(lastCmd.hPos - 15);
    }
    else if (shakeCounter == 3 * shakeInterval)
    {
      servoV.write(lastCmd.vPos - 15);
      servoH.write(lastCmd.hPos - 30);
    }
    else if (shakeCounter == 4 * shakeInterval)
    {
      shakeCounter = 0;
      shakeTimes--;
      servoV.write(lastCmd.vPos);
      servoH.write(lastCmd.hPos - 15);
    }
  }
}

// Two Servo Rotate Naturally Together
void CoNaturalRotate(int iniAngle_H, int finalAngle_H, int iniAngle_V, int finalAngle_V)
{
  double sH = abs(finalAngle_H - iniAngle_H);
  double sV = abs(finalAngle_V - iniAngle_V);
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
  
  double timePoint = 1;
  double v = 0;
  double a;
  double vMaxAbs;
  double k_H = 1;
  double k_V = 1;
  
  if(abs(sH) >= abs(sV))
  {
    vMaxAbs = vMaxAbs_H;
    a = a_H;
    k_V = vMax_V / vMax_H; // i.e. Vv = Vh * k
  }
  else
  {
    vMaxAbs = vMaxAbs_V;
    a = a_V;
    k_H = vMax_H / vMax_V; // i.e. Vv = Vh * k
  }
  
  for( ; abs(v) < vMaxAbs; v += a, timePoint++)
  {
    servoH.write(double(iniAngle_H) + v * k_H * timePoint / 2.0);
    servoV.write(double(iniAngle_V) + v * k_V * timePoint / 2.0);
    delay(servoStepTime);
  }

  for( ; abs(v) > ar; v -= a, timePoint--)
  {
    servoH.write(double(finalAngle_H) - v * k_H * timePoint / 2);
    servoV.write(double(finalAngle_V) - v * k_V * timePoint / 2);
    delay(servoStepTime);
  }

  servoH.write(finalAngle_H);
  servoV.write(finalAngle_V);
}


// One Servo Rotate Naturally
void NaturalRotate(Servo s, int iniAngle, int finalAngle)
{
  double vMaxAbs = sqrt(abs(finalAngle - iniAngle) * ar);
  double aAbs = ar;
  
  double vMax = vMaxAbs;
  double a = ar;
  if(finalAngle - iniAngle < 0)
  {
    vMax *= -1;
    a *= -1;
  }
  
  double timePoint = 1;
  double v = 0;
  for( ; abs(v) < vMaxAbs; v += a, timePoint++)
    {
      s.write(double(iniAngle) + v * timePoint / 2);
      delay(servoStepTime);
    }

  for( ; abs(v) > aAbs; v -= a, timePoint--)
    {
      s.write(double(finalAngle) - v * timePoint / 2);
      delay(servoStepTime);
    }

  s.write(finalAngle);
}


// One Servo Rotate with Constant Speed
void ConstRotate(Servo s, int iniAngle, int finalAngle)
{
  double angle;
  double timePoint = 0;
  if(iniAngle < finalAngle)
  {
    for(angle = iniAngle; angle < finalAngle; angle = iniAngle + vr * timePoint)
    {
      s.write(angle);
      delay(servoStepTime);
      timePoint++;
    }
  }
  else
  {
    for(angle = iniAngle; angle > finalAngle; angle = iniAngle - vr * timePoint)
    {
      s.write(angle);
      delay(servoStepTime);
      timePoint++;
    }
  }
  s.write(finalAngle);
}

