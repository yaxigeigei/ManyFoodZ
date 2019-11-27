void LoaderSetup()
{
  if (sysStartUp)
  {
    for(int i = 0; i < 4; i++)
      pinMode(loaderPins[i], OUTPUT);
    
    Stepping(1);
  }
}



void Stepping(int n)
{
  for (int i = n; i > 0; i--)
  {
    if (thisStep < 3)
      thisStep++;
    else
      thisStep = 0;
    
    UnitStep(thisStep);
  }
  Serial.println("executed");
}



void UnitStep(byte index)
{
  // Set the pin specified by "index"
  int level[] = { LOW, LOW, LOW, LOW };
  for(int i = 0; i < 4; i++)
  {
    if(i == index)
      level[3 - i] = HIGH;
    digitalWrite(loaderPins[3 - i], level[3 - i]);
  }
  
  // Keep this output for a while before next step
  delay(loaderStepTime);
  
  // Turn off all pins for reducing heating
  for(int i = 0; i < 4; i++)
    digitalWrite(loaderPins[3 - i], LOW);
}


