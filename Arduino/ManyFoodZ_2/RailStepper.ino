void RailSetup()
{
  if (sysStartUp)
  {
    pinMode(pulPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(resetPin, INPUT); // Reset sensor setup
  }
  minStepTime = motorStepSize / motorMaxSpeed * 1000000;
  accSteps = square(motorMaxSpeed) / (2 * motorAcceleration * motorStepSize);
}



// Find the Zero Point of Linear Axis
void ResetStepper()
{
  // HIGH means moving backward
  digitalWrite(dirPin, HIGH);
  // Keep going until sensor is triggered and sensor pin goes HIGH
  while(digitalRead(resetPin) == HIGH)
    Go(1, minStepTime);
  // Reset corresponding variable
  lastCmd.lPos = 0;
}



// Moving Naturally
void NaturalMove(int iniPos, int finalPos)
{
  if(iniPos < finalPos)
    digitalWrite(dirPin, LOW);
  else
    digitalWrite(dirPin, HIGH);
  
  double length = abs(finalPos - iniPos);
  double aSteps = min(accSteps, length/2);
  double stepCount;
  double compound = motorStepSize / 2 / motorAcceleration;
  
  unsigned long consume;
  double interval;
  
  for(stepCount = 1; stepCount <= aSteps; stepCount++)
  {
    consume = micros();
    interval = 1000000 * sqrt(compound / stepCount);
    consume = micros() - consume;
    
    interval = interval - consume;
    if (interval < pulseTime)
      interval = pulseTime;
    
    Go(1, interval);
  }
  int milestone_1 = stepCount;
  for( ; stepCount < length - milestone_1; stepCount++)
  {
    Go(1, minStepTime);
  }
  for( ; stepCount < length; stepCount++)
  {
    consume = micros();
    interval = 1000000 * sqrt(compound / (double(length) - stepCount));
    consume = micros() - consume;
    
    interval = interval - consume;
    if (interval < pulseTime)
      interval = pulseTime;
    
    Go(1, interval);
  }
}



void Go(int steps, int period)
{
  for(int i = 0; i < steps; i++)
  {
    digitalWrite(pulPin, HIGH);
    delayMicroseconds(pulseTime);
    digitalWrite(pulPin, LOW);
    delayMicroseconds(period - pulseTime);
  }
}



// Moving with Constant Speed
void ConstMove(int iniPos, int finalPos)
{
  int length = abs(finalPos - iniPos);
  
  if(iniPos < finalPos)
    digitalWrite(dirPin, LOW);
  else
    digitalWrite(dirPin, HIGH);
    
  for(int i = 0; i < length; i++)
    Go(1, minStepTime);
}
