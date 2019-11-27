// Runtime Parameters
double compound;
double accSteps;
double moveDist;
double decStep;
volatile double stepCount = 1;
volatile unsigned int ovfCount = 0;
volatile unsigned int cmpCount = 0;


void RailSetup()
{
  if (sysStartUp)
  {
    pinMode(railPulPin, OUTPUT);
    pinMode(railDirPin, OUTPUT);
    pinMode(railResetPin, INPUT);
    
    // Clear all existing settings
    TCCR3A = 0;
    TCCR3B = 0;
    // Turn on Fast PWM mode with ICR1 set as TOP value
    TCCR3A |= (1 << WGM31);
    TCCR3B |= (1 << WGM32) | (1 << WGM33);
    // Set prescaler to 64, counting every 4us
    TCCR3B |= (1 << CS31) | (1 << CS30);
    OCR3A = ceil(railPulDur/4);
  }
}


ISR(TIMER3_OVF_vect)
{
  ovfCount++;
  digitalWrite(railPulPin, HIGH);
  
  if (stepCount <= accSteps)
    ICR3 = int(250000 * sqrt(compound / stepCount));
  else if (stepCount > decStep && stepCount < moveDist)
    ICR3 = int(250000 * sqrt(compound / (double(moveDist) - stepCount)));
  else if (stepCount >= moveDist)
    sliding = false;
}


ISR(TIMER3_COMPA_vect)
{
  cmpCount++;
  stepCount++;
  digitalWrite(railPulPin, LOW);
  
  if (!sliding)
  {
    TIMSK3 = 0;
//    Serial.println(ovfCount);
//    Serial.println(cmpCount);
  }
}


// Moving Naturally
void NaturalMove(int iniPos, int finalPos)
{
  moveDist = abs(finalPos - iniPos);
  
  if (moveDist != 0)
  {
    // Determine the direction of movement
    if(iniPos < finalPos)
      digitalWrite(railDirPin, LOW);
    else
      digitalWrite(railDirPin, HIGH);
    
    // Calculate the number of steps required for accelerating to maximum speed
    accSteps = square(railMaxSpeed) / (2 * railAcceleration * railStepSize); 
    accSteps = min(accSteps, moveDist/2); // In case the distance is too short to reach the maximum speed
    
    // Calculate the index of step of deceleration begin
    decStep = moveDist - accSteps;
    
    // Pre-calculate a constant component used in the calculation in interrupt
    compound = railStepSize / 2 / railAcceleration;
    
    // Initialize some variables for this move
    stepCount = 1;
    sliding = true;
    ovfCount = 0;
    cmpCount = 0;
    
    // Initialize Timer3
    cli();
    ICR3 = 0;
    TIMSK3 |= (1 << TOIE3) | (1 << OCIE3A);
    TCNT3 = 0;
    sei();
  }
}



// Find the Zero Point of the Rail
void ResetStepper()
{
  long start = millis(); // ms - Stores time when milli() is called
  digitalWrite(railDirPin, HIGH); // HIGH means moving backward
  
  // Keep going until the Hall sensor is triggered and its pin goes HIGH
  while(digitalRead(railResetPin) == HIGH && millis() - start < resetTimeOut)
    Go(railStepSize / 20 * 1000000);
    
  // Clear corresponding variable
  lastCmd.lPos = 0;
}



void Go(int period)
{
    digitalWrite(railPulPin, HIGH);
    delayMicroseconds(railPulDur);
    digitalWrite(railPulPin, LOW);
    delayMicroseconds(period - railPulDur);
}
