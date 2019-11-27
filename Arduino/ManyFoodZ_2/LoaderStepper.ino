void LoaderSetup()
{
  if (sysStartUp)
  {
    for(int i = 0; i < 4; i++)
    {
      pinMode(loaderPins[i], OUTPUT);
      digitalWrite(loaderPins[i], LOW); 
    }
    RunToEnd(32);
  }
}



void StepUnit(byte index)
{
  int level[] = { LOW, LOW, LOW, LOW };
  
  for(int i = 0; i < 4; i++)
  {
    if(i + 1 == index)
    {
      level[3 - i] = HIGH;
      startPoint = i + 1;
    }
    digitalWrite(loaderPins[3 - i], level[3 - i]);
  }
  
  delay(loaderStepTime);
  
  for(int i = 0; i < 4; i++)
  {
    digitalWrite(loaderPins[3 - i], LOW);
  }
}



void RunMakeup(byte steps)
{
  for(int i = 2; i >= 0; i--)
  {
    if(steps > i)
      StepUnit(4 - i);
  }
}

void RunLittle(byte steps, byte left)
{
  byte counter = 0;
  for(int i = 2; i >= 0; i--)
  {
    if(steps > i && counter < left)
    {
      StepUnit(4 - i);
      counter++;
    }
  }
}

void RunToEnd(byte stage)
{
  for(int i = 0; i < 4; i++)
  {
    if(stage > i)
      StepUnit(i + 1);
  }
}



void OneMoreFood(int n)
{
  int stepMakeup = 4 - startPoint;
  int stepRemain = n - stepMakeup;
  
  if (n >= 4)
    RunMakeup(stepMakeup);
  else
    RunLittle(stepMakeup, n);
  
  for (int i = 0; i < stepRemain; i += 4)
  {
    if(stepRemain - i >= 4)
     RunToEnd(4);
    else
     RunToEnd(stepRemain - i);
  }
  
  Serial.println("executed");
}
