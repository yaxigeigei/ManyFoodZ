// Reading Command 
int incomingVal = 0;
String incomingCmd = "";

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
          Serial.print("L-axis position is ");
          Serial.println(incomingVal); break;
        case 'h':
          newCmd.hPos = 180 - incomingVal;
          Serial.print("H-axis angle is ");
          Serial.println(incomingVal); break;
        case 'v':
          newCmd.vPos = 180 - incomingVal;
          Serial.print("V-axis angle is ");
          Serial.println(incomingVal); break;
        case 'p':
          PiezoEntrance(); break;
        case 'q':
          ListenAfterward(); break;
        case 'k':
          Shake(incomingVal); break;
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
      incomingCmd += ch;
    
    if(incomingCmd.length() == 3)
    {
      if(incomingCmd.equals("SHB"))
      {
        servoH_BEGIN = incomingVal;
        Serial.print(incomingVal);
        Serial.println(" corresponds to 0 degree theta");
      }
      else if(incomingCmd.equals("SHE"))
      {
        servoH_END = incomingVal;
        Serial.print(incomingVal);
        Serial.println(" corresponds to 180 degree theta");
      }
      else if(incomingCmd.equals("SVB"))
      {
        servoV_BEGIN = incomingVal;
        Serial.print(incomingVal);
        Serial.println(" corresponds to 0 degree phi");
      }
      else if(incomingCmd.equals("SVE"))
      {
        servoV_END = incomingVal;
        Serial.print(incomingVal);
        Serial.println(" corresponds to 180 degree phi");
      }
      else if(incomingCmd.equals("SRA"))
      {
        RotationAcceleration = incomingVal;
        Serial.print("Servo's acceleration set to ");
        Serial.print(incomingVal);
        Serial.println(" degree/s^2");
      }
      else if(incomingCmd.equals("MMS"))
      {
        motorMaxSpeed = incomingVal;
        Serial.print("Platform's maximum speed set to ");
        Serial.print(incomingVal);
        Serial.println(" mm/s");
      }
      else if(incomingCmd.equals("MLA"))
      {
        motorAcceleration = incomingVal;
        Serial.print("Platform's acceleration set to ");
        Serial.print(incomingVal);
        Serial.println(" mm/s^2");
      }
      else if(incomingCmd.equals("MSS"))
      {
        motorStepSize = double(incomingVal) / 1000.0;
        Serial.print("Motor's step increment set to ");
        Serial.print(incomingVal);
        Serial.println(" mm");
      }
      else if(incomingCmd.equals("MPT"))
      {
        pulseTime = incomingVal;
        Serial.print("Motor's pulse duration (voltage-high time) set to ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("PDM"))
      {
        multiplierD = double(incomingVal) / 100.0;
        Serial.print("Drop piezo multiplier set to ");
        Serial.println(multiplierD);
      }
      else if(incomingCmd.equals("CTI"))
      {
        criticalInterval = incomingVal;
        Serial.print("Critical interval set to ");
        Serial.println(criticalInterval);
      }
      else if(incomingCmd.equals("PND"))
      {
        pNumThresholdD = incomingVal;
        Serial.print("Drop piezo's threshold peaks number set to ");
        Serial.println(pNumThresholdD);
      }
      else if(incomingCmd.equals("PNG"))
      {
        pNumThresholdG = incomingVal;
        Serial.print("Grab piezo's threshold peaks number set to ");
        Serial.println(pNumThresholdG);
      }
      else if(incomingCmd.equals("HLD"))
      {
        holdTime = incomingVal;
        Serial.print("Holding time critirium set to ");
        Serial.println(holdTime);
      }
      else if(incomingCmd.equals("ASD"))
      {
        shakeInterval = incomingVal;
        Serial.print("Arm shake delay set to "); 
        Serial.println(shakeInterval);
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
