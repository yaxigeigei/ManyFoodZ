// Runtime Parameters for IR sensors
boolean foodState = false; // The flag indicating
unsigned long timeLastSpikeG = 0; // ms; Time of Last read Spike of Grab IR
unsigned long timeLastSpikeF = 0; // ms; Time of Last read Spike of Food IR
unsigned int spikesContG = 0; // Count of consecutive spikes
unsigned int spikesContF = 0; // Count of consecutive spikes

// Runtime Parameters for Drop Piezo
double readThresholdD; // Threshold for converting analogous signal to 0 or 1
long timeLastSpikeD = 0; // ms; Time of Last read Spike of Drop piezo
unsigned int spikesContD = 0; // Count of consecutive spikes


void DetectorSetup()
{
  if (sysStartUp)
  {
    const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
    const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRA &= ~PS_128; // Remove bits set by Arduino library
    ADCSRA |= PS_32; // Operation freq. is 16MHz/32 = 500kHz; sampling rate is 500kHz/13 = 38.46kHz
    
    pinMode(grabPin, INPUT);
    pinMode(foodPin, INPUT);
  }
  foodState = digitalRead(foodPin);
  FindThreshold();
}



void FindThreshold()
{
  double sumD = 0;
  
  for(int i = 0; i < 1000; i++)
  {
    sumD += analogRead(piezoDPin);
    delay(random(1,8));
  }
  
  readThresholdD = sumD / 1000.0 * multiplierD;
  Serial.print("Drop piezo's threshold = ");
  Serial.println(readThresholdD);
}



void MonitorBehavior()
{
  long now = millis(); // ms - Update current time
  
  // Food Signal
  if (digitalRead(foodPin) != foodState)
  {
    if (now - timeLastSpikeF > critItvlIR)
      spikesContF = 1;
    else // (now - timeLastspikeG <= criticalInterval)
    {
      if (spikesContF < sNumThrhIR)
        spikesContF++;
      else if (spikesContF == sNumThrhIR)
      {
        spikesContF++;
        foodState = digitalRead(foodPin);
        if (foodState == HIGH)
        {
          Serial.println(now);
          Serial.println("fIn");
        }
        else
        {
          Serial.println(now);
          Serial.println("fOut");
        }
      }
    }
    timeLastSpikeF = now;
  }
  
  // Grab Signal
  now = millis(); // ms - Update current time
  if (digitalRead(grabPin) == HIGH)
  {
    if (now - timeLastSpikeG > critItvlIR)
      spikesContG = 1;
    else // (now - timeLastspikeG <= criticalInterval)
    {
      if (spikesContG < sNumThrhIR)
        spikesContG++;
      else if (spikesContG == sNumThrhIR)
      {
        spikesContG++;
        Serial.println(now);
        Serial.println("grab");
      }
    }
    timeLastSpikeG = now;
  }
  
  // Drop Signal
  now = millis(); // ms - Update current time
  int piezoDReading = analogRead(piezoDPin);
  if (piezoDReading > readThresholdD)
  {
    if (now - timeLastSpikeD > critItvlD)
      spikesContD = 1;
    else // (now - timeLastspikeD <= criticalInterval)
    {
      if (spikesContD < sNumThrhD)
        spikesContD++;
      else if (spikesContD == sNumThrhD)
      {
        spikesContD++;
        Serial.println(now);
        Serial.println("drop");
      }
    }
    timeLastSpikeD = now;
  }
}
