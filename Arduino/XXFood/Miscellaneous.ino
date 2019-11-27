void MiscSetup()
{
  if (sysStartUp)
  {
    pinMode(trgOutPin, OUTPUT); // Trigger-out to microscope
    pinMode(lumLedPin, OUTPUT); // Field Illumination LED
    
    // Shutter
    for(int i = 0; i < 4; i++)
    {
      pinMode(shutterPins[i], OUTPUT);
      digitalWrite(shutterPins[i], LOW);
    }
  }
}



// Imaging Trigger-out
void ImgTrigOut(double dur) // Send a Pulse to Imaging System
{
  // Record the timing of pulse initiation
  long t = millis();
  // Give pulses
  PulseGen(trgOutPin, dur);
  // Report timing to computer
  Serial.println(t);
  Serial.println("ImgTriggered");
}



void ToggleFieldLed()
{
  lumLedStat = !lumLedStat; // Change the state between HIGH and LOW
  digitalWrite(lumLedPin, lumLedStat); // Apply the altered state
}



void ShutterOpen()
{
  long t = millis();
  
  // Shutter Door Open
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(shutterPins[i], HIGH);
    digitalWrite(shutterPins[i+2], LOW);
  }
  // Servo Door Open
  RotateTo(doorOpenDeg);
  
  Serial.println(t);
  Serial.println("dOpen");
  ShutterRest();
}

void ShutterClose()
{
  long t = millis();
  
  // Shutter Door
  for (int i = 0; i < 2; i++)
  {
    digitalWrite(shutterPins[i+2], HIGH);
    digitalWrite(shutterPins[i], LOW);
  }
  // Servo Door
  RotateTo(doorCloseDeg);
  
  Serial.println(t);
  Serial.println("dClose");
  ShutterRest();
}

void ShutterRest()
{
  delay(100);
  for (int i = 0; i < 4; i++)
    digitalWrite(shutterPins[i],LOW);
}



void BuzzerOn()
{
  long t = millis();
  tone(buzzerPin, toneFreq, toneDur);
  Serial.println(t);
  Serial.println("Buzz");
}
