void DetectorSetup()
{
  ADCSRA &= ~PS_128; // Remove bits set by Arduino library
  ADCSRA |= PS_32; // Operation freq. is 16MHz/32 = 500kHz; sampling rate is 500kHz/13 = 38.46kHz; three samples cost ~100us
  pinMode(freqPin, OUTPUT);
  FindThreshold();
}



void FindThreshold()
{
  double sumD = 0;
  for(int i = 0; i < 1000; i++)
  {
    sumD += analogRead(piezoDPin);
    delay(random(1,9));
  }
  readThresholdD = sumD / 1000.0 * multiplierD;
  Serial.print("The threshold of drop piezo is ");
  Serial.println(readThresholdD);
}


void PiezoEntrance()
{
  if (!monitorPiezo)
  {
    monitorPiezo = true;
    for(int i = 0; i < 500; i++)
    {
      analogRead(piezoGPin);
      analogRead(anaSinkPin);
      analogRead(piezoDPin);
      delay(1);
    }
    Serial.println("Start listening......");
  }
}



void PiezoListen()
{
  if(monitorPiezo)
  {
    static int cycleState = HIGH; // Indicates analog sampling cycle
    long now = millis(); // ms - Stores time when milli() is called
    
    // Grab Piezo Signal
    analogRead(anaSinkPin);
    int piezoGReading = analogRead(piezoGPin);
    static long timeLastPeakG = 0; // ms - Stores last read peak of Grab piezo
    static int peaksContG = 0; // Continuous peaks read
    
    if (piezoGReading > readThresholdG)
    {
      if (now - timeLastPeakG > criticalInterval)
        peaksContG = 1;
      else // (now - timeLastPeakG <= criticalInterval)
      {
        if (peaksContG < pNumThresholdG)
          peaksContG++;
        else if (peaksContG == pNumThresholdG)
        {
          peaksContG++;
          gIndex++;
          if (gIndex < arrayLength)
            grabTimes[gIndex] = now;
          Serial.println("g");
        }
        else if (gIndex < arrayLength)// (&& peaksContG > pNumThresholdG)
          grabDuras[gIndex] = now - grabTimes[gIndex];
      }
      timeLastPeakG = now;
    }
    
    // Drop Piezo Signal
    int piezoDReading = analogRead(piezoDPin);
    static long timeLastPeakD = 0; // ms - Stores last read peak of Drop piezo
    static int peaksContD = 0; // Continuous peaks read
    static boolean canceled = false;
    
    if (piezoDReading > readThresholdD)
    {
      if (now - timeLastPeakD > criticalInterval)
        peaksContD = 1;
      else // (now - timeLastPeakD <= criticalInterval)
      {
        if (peaksContD < pNumThresholdD)
          peaksContD++;
        else if (peaksContD == pNumThresholdD)
        {
          peaksContD++;
          Serial.println("d");
          if (timeDrop == 0)
            timeDrop = now;
        }
      }
      timeLastPeakD = now;
    }
    
    cycleState = !cycleState;
    digitalWrite(freqPin, cycleState);
  }
}



void ListenAfterward()
{
  if (monitorPiezo)
  {
    Shake(3);
    
    unsigned long timeEnd = millis();
    int timer = 1500; // 1500ms of looping
    boolean exit = false;
    
    while(!exit)
    {
      long now = millis();
      int piezoDReading = analogRead(piezoDPin);
      static long timeLastPeakD = 0; // ms - Stores last read peak of Grab piezo
      static int peaksContD = 0; // Continuous peaks read
      
      if(piezoDReading > readThresholdD)
      {
        if(now - timeLastPeakD > criticalInterval)
          peaksContD = 1;
        else // (now - timeLastPeakD <= criticalInterval)
        {
          if(peaksContD < pNumThresholdD)
            peaksContD++;
          else if(peaksContD == pNumThresholdD)
            dropAfter = true;
        }
        timeLastPeakD = now;
      }
      
      if (now - timeEnd > timer)
        exit = true;
    }
    
    DetectorReport();
    monitorPiezo = false;
  }
}


void DetectorReport()
{
  // Preliminary Calculations
  byte grabNum = 0; // Number of grabs animal did in total
  byte eventPosition = 0; // The number means hit or fall occured at grabTimes[num]
  boolean anaHit = false;
  
  for (int i = 0; i < arrayLength; i++)
  {
    long grabEndTime = grabTimes[i] + grabDuras[i];
    
    if (grabTimes[i] != 0) // When there was a grab
    {
      grabNum++;
      if (grabTimes[i + 1] != 0) // When "i"th grab was not the last grab
      {
        if (timeDrop >= grabTimes[i] && timeDrop < grabTimes[i + 1]) // i.e. drop happened in this attempt
        {
          eventPosition = i + 1;
          if (timeDrop - grabEndTime >= holdTime) // For "Hit" to satisfy
            anaHit = true;
        }
      }
      else // (grabTime[i + 1] == 0) // It was the last grab
      {
        if (timeDrop >= grabTimes[i]) // i.e. drop happened in this attempt
        {
          eventPosition = i + 1;
          if (timeDrop - grabEndTime >= holdTime) // For "Hit" to satisfy
            anaHit = true;
        }
      }
    }
    
    grabTimes[i] = 0;
    grabDuras[i] = 0;
  }
  
  // Judgement
  if (dropAfter || grabNum == 0)
    Serial.println("fail");
  else
  {
    if (timeDrop == 0)
      Serial.println("hit");
    else if (anaHit)
      Serial.println("hit");
    else
      Serial.println("fail");
  }
  
  // Details
  Serial.print("result");
  Serial.print(grabNum);
  Serial.print(",");
  Serial.println(eventPosition);
  
  // Clear Global Variables
  timeDrop = 0;
  gIndex = -1;
  dropAfter = false;
  
  // Completion Signal
  Serial.println("executed");
}
