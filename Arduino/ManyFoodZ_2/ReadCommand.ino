// Reading Command 
int incomingVal = 0;
String incomingCmd = String();

void ReadCommand()
{
  if(Serial.available())
  {
    char ch = Serial.read();
    
    if(ch == 'e')
      Move2Pos();
    else if(isDigit(ch))
      incomingVal = incomingVal * 10 + ch - '0'; 
    else if(islower(ch))
    {
      switch(ch)
      {
        case 'l': 
          newCmd.lPos = incomingVal;
          Serial.print("L ");
          Serial.println(incomingVal); break;
        case 'h':
          newCmd.hPos = 180 - incomingVal;
          Serial.print("H ");
          Serial.println(incomingVal); break;
        case 'v':
          newCmd.vPos = 180 - incomingVal;
          Serial.print("V ");
          Serial.println(incomingVal); break;
        case 'p':
          PiezoEntrance(); break;
        case 'q':
          ListenAfterward(); break;
        case 'k':
          Shake(); break;
        case 'w':
          Sweep(); break;
        case 'f':
          OneMoreFood(incomingVal); break;
        case 'r':
          ResetStepper(); break;
        case 'i':
          ImgTrigOut(); break;
        default: break;
      }
      incomingVal = 0;
    }
    else if(isalpha(ch))
    {
      incomingCmd += ch;
    }
    
    if(incomingCmd.length() == 3)
    {
      if(incomingCmd.equals("MFZ"))
      {
        Serial.println("YES");
      }
      if(incomingCmd.equals("SHB"))
      {
        servoH_begin = incomingVal;
        Serial.print("0 deg theta = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SHE"))
      {
        servoH_end = incomingVal;
        Serial.print("180 deg theta = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SVB"))
      {
        servoV_begin = incomingVal;
        Serial.print("0 deg phi = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SVE"))
      {
        servoV_end = incomingVal;
        Serial.print("180 deg phi = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SRA"))
      {
        servoAcceleration = incomingVal;
        Serial.print("Servo's acceleration = ");
        Serial.print(incomingVal);
        Serial.println(" deg/s^2");
      }
      else if(incomingCmd.equals("MMS"))
      {
        motorMaxSpeed = incomingVal;
        Serial.print("Platform's maximum speed = ");
        Serial.print(incomingVal);
        Serial.println(" mm/s");
      }
      else if(incomingCmd.equals("MLA"))
      {
        motorAcceleration = incomingVal;
        Serial.print("Platform's acceleration = ");
        Serial.print(incomingVal);
        Serial.println(" mm/s^2");
      }
      else if(incomingCmd.equals("MSS"))
      {
        motorStepSize = double(incomingVal) / 1000.0;
        Serial.print("Motor's step increment = ");
        Serial.print(incomingVal);
        Serial.println(" um");
      }
      else if(incomingCmd.equals("MPT"))
      {
        pulseTime = incomingVal;
        Serial.print("Motor's pulse duration = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("PDM"))
      {
        multiplierD = double(incomingVal) / 100.0;
        Serial.print("Drop piezo multiplier = ");
        Serial.println(multiplierD);
      }
      else if(incomingCmd.equals("PGM"))
      {
        multiplierG = double(incomingVal) / 100.0;
        Serial.print("Grab piezo multiplier = ");
        Serial.println(multiplierG);
      }
      else if(incomingCmd.equals("CTI"))
      {
        criticalInterval = incomingVal;
        Serial.print("Critical interval = ");
        Serial.print(criticalInterval);
        Serial.println(" ms");
      }
      else if(incomingCmd.equals("PND"))
      {
        pNumThresholdD = incomingVal;
        Serial.print("Drop piezo's threshold peaks number = ");
        Serial.println(pNumThresholdD);
      }
      else if(incomingCmd.equals("PNG"))
      {
        pNumThresholdG = incomingVal;
        Serial.print("Grab piezo's threshold peaks number = ");
        Serial.println(pNumThresholdG);
      }
      else if(incomingCmd.equals("HLD"))
      {
        holdTime = incomingVal;
        Serial.print("Holding time critirium = ");
        Serial.print(holdTime);
        Serial.println(" ms");
      }
      else if(incomingCmd.equals("CLB"))
      {
        FindThreshold();
        Serial.println("calibrated");
      }
      else if(incomingCmd.equals("CFR"))
      {
        RailSetup();
        Serial.println("configured");
      }
      else if(incomingCmd.equals("CFS"))
      {
        ServoSetup();
        Serial.println("configured");
      }
      incomingVal = 0;
      incomingCmd = "";
    }
  }
}
