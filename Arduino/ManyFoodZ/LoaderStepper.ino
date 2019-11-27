void LoaderSetup()
{
  for(int i = 0; i < 4; i++)
  {
    pinMode(loaderPins[i], OUTPUT);
    digitalWrite(loaderPins[i], LOW); 
  }
  RunToEnd(4);
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
}



void RunMakeup(byte stage)
{
  for(int i = 2; i >= 0; i--)
  {
    if(stage > i)
      StepUnit(4 - i);
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
  
  RunMakeup(stepMakeup);
  
  for (int i = 0; i < stepRemain; i += 4)
  {
    if(stepRemain - i >= 4)
     RunToEnd(4);
    else
     RunToEnd(stepRemain - i);
  }
}
