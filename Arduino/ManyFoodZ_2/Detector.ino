void DetectorSetup()
{
  if (sysStartUp)
  {
    ADCSRA &= ~PS_128; // Remove bits set by Arduino library
    ADCSRA |= PS_32; // Operation freq. is 16MHz/32 = 500kHz; sampling rate is 500kHz/13 = 38.46kHz; three samples cost ~100us
  }
  FindThreshold();
}



void FindThreshold()
{
  double sumD = 0;
  double sumG = 0;
  
  for(int i = 0; i < 1000; i++)
  {
    sumD += analogRead(piezoDPin);
    analogRead(anaSinkPin);
    int temp = analogRead(piezoGPin);
    sumG += analogRead(piezoGPin);
    delay(random(1,8));
  }
  
  readThresholdD = sumD / 1000.0 * multiplierD;
  Serial.print("Drop piezo's threshold = ");
  Serial.println(readThresholdD);
  
  readThresholdG = (sumG / 1000.0 + 1) * multiplierG;
  Serial.print("Grab piezo's threshold = ");
  Serial.println(readThresholdG);
}



void PiezoEntrance()
{
  if (!monitorPiezo)
  {
    monitorPiezo = true;
    ClearPiezoVars();
    Serial.println("Start listening......");
  }
}

void ClearPiezoVars()
{
  timeLastPeakG = 0;
  peaksContG = 0;
  timeLastPeakD = 0;
  peaksContD = 0;
  canceled = false;
}

void PiezoListen()
{
  if(monitorPiezo)
  {
    long now = millis(); // ms - Stores time when milli() is called
    analogRead(anaSinkPin);
    
    // Grab Piezo Signal
    int piezoGReading = analogRead(piezoGPin);
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
  }
}



void ListenAfterward()
{
  if (monitorPiezo)
  {
    // Sweeping off the food (if there is any)
    if (newCmd.hPos > 180 - sweepAngle)
      newCmd.hPos = 180;
    else
      newCmd.hPos += sweepAngle;
    NaturalRotate(lastCmd.hPos, newCmd.hPos, lastCmd.vPos, newCmd.vPos);
    lastCmd = newCmd;
    
    // To see whether there is a drop
    ClearPiezoVars();
    int timer = 3000; // 3000ms of looping
    boolean exit = false;
    unsigned long timeEnd = millis();
    
    while(!exit)
    {
      long now = millis();
      int piezoDReading = analogRead(piezoDPin);
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
