void RemoteSetup()
{
  if (sysStartUp)
  {
    for(int i = 0; i < 4; i++)
      pinMode(remotePins[i], INPUT);
    
    attachInterrupt(2, ReadRemoteCommand, RISING); // RF Remote Reading
  }
}



// Reading RF Remote Command
void ReadRemoteCommand()
{
  if (millis() - timeLastCmd > 500)
  {
    int keyState[4];
    for (int i = 0; i < 4; i++)
      keyState[i] = digitalRead(remotePins[i]);
    
    if (keyState[0]) // D
      Stepping(remoteAdjSteps);
    else if (keyState[1]) // C
      ImgTrigOut(1000);
    else if (keyState[2]) // B
    {
      BuzzerOn();
      ShutterOpen();
    }
    else if (keyState[3]) // A
      ShutterClose();
  }
  
  timeLastCmd = millis();
}




// Reading Command
double incomingVal = 0;
boolean decimal = false;
double factor = 1;
String incomingCmd = String();

void ResetSerialRead()
{
  incomingVal = 0;
  factor = 1;
  decimal = false;
  incomingCmd = String();
}

void ReadSerialCommand()
{
  if(Serial.available())
  {
    char ch = Serial.read();
    
    if(isDigit(ch))
    {
      if (!decimal)
        incomingVal = incomingVal * 10 + ch - '0'; 
      else
      {
        factor *= 0.1;
        incomingVal = incomingVal + (ch - '0') * factor; 
      }
    }
    else if(ch == '.')
      decimal = true;
    else if(islower(ch))
    {
      switch(ch)
      {
        case 'e':
          Move2Pos(); break;
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
        case 'r':
          newCmd.rPos = incomingVal;
          Serial.print("R ");
          Serial.println(incomingVal); break;
        case 'f':
          Stepping(incomingVal); break;
        case 'i':
          ImgTrigOut(incomingVal); break;
        case 'o': 
          ShutterOpen(); break;
        case 'c': 
          ShutterClose(); break;
        case 'b':
          BuzzerOn(); break;
        case 's':
          noTone(buzzerPin);
          Serial.println("Silenced"); break;
        default: break;
      }
      ResetSerialRead();
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
      else if(incomingCmd.equals("SHB"))
      {
        servoBegins[0] = incomingVal;
        Serial.print("H 0 deg = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SHE"))
      {
        servoEnds[0] = incomingVal;
        Serial.print("H 180 deg = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SVB"))
      {
        servoBegins[1] = incomingVal;
        Serial.print("V 0 deg = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SVE"))
      {
        servoEnds[1] = incomingVal;
        Serial.print("V 180 deg = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SRB"))
      {
        servoBegins[2] = incomingVal;
        Serial.print("R 0 deg = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SRE"))
      {
        servoEnds[2] = incomingVal;
        Serial.print("R 180 deg = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SDB"))
      {
        servoBegins[3] = incomingVal;
        Serial.print("D 0 deg = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SDE"))
      {
        servoEnds[3] = incomingVal;
        Serial.print("D 180 deg = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("SRA"))
      {
        servoAcc = ConvertServoAcc(incomingVal);
        Serial.print("Servo's acceleration = ");
        Serial.print(incomingVal);
        Serial.println(" deg/s^2");
      }
      else if(incomingCmd.equals("RSS"))
      {
        ResetStepper();
        Serial.println("Stepper Reset");
      }
      else if(incomingCmd.equals("MMS"))
      {
        railMaxSpeed = incomingVal;
        Serial.print("Platform's maximum speed = ");
        Serial.print(incomingVal);
        Serial.println(" mm/s");
      }
      else if(incomingCmd.equals("MLA"))
      {
        railAcceleration = incomingVal;
        Serial.print("Platform's acceleration = ");
        Serial.print(incomingVal);
        Serial.println(" mm/s^2");
      }
      else if(incomingCmd.equals("MSS"))
      {
        railStepSize = double(incomingVal) / 1000.0;
        Serial.print("Motor's step increment = ");
        Serial.print(incomingVal);
        Serial.println(" um");
      }
      else if(incomingCmd.equals("MPT"))
      {
        railPulDur = incomingVal;
        OCR3A = ceil(railPulDur/4);
        Serial.print("Motor's pulse duration = ");
        Serial.print(incomingVal);
        Serial.println(" us");
      }
      else if(incomingCmd.equals("PDM"))
      {
        multiplierD = incomingVal / 100;
        Serial.print("Drop piezo multiplier = ");
        Serial.println(multiplierD);
      }
      else if(incomingCmd.equals("CID"))
      {
        critItvlD = incomingVal;
        Serial.print("Drop critical interval = ");
        Serial.print(critItvlD);
        Serial.println(" ms");
      }
      else if(incomingCmd.equals("PND"))
      {
        sNumThrhD = incomingVal;
        Serial.print("Drop piezo threshold peaks number = ");
        Serial.println(sNumThrhD);
      }
      else if(incomingCmd.equals("FTH"))
      {
        FindThreshold();
        Serial.println("calibrated");
      }
      else if(incomingCmd.equals("CIC"))
      {
        critItvlIR = incomingVal;
        Serial.print("IR critical interval = ");
        Serial.print(critItvlIR);
        Serial.println(" ms");
      }
      else if(incomingCmd.equals("PNC"))
      {
        sNumThrhD = incomingVal;
        Serial.print("Counter threshold peaks number = ");
        Serial.println(sNumThrhD);
      }
      else if(incomingCmd.equals("TFQ"))
      {
        toneFreq = incomingVal;
        Serial.print("Tone frequency = ");
        Serial.print(incomingVal);
        Serial.println(" Hz");
      }
      else if(incomingCmd.equals("TDR"))
      {
        toneDur=incomingVal;
        Serial.print("Tone duration = ");
        Serial.print(incomingVal);
        Serial.println(" ms");
      }
      
      ResetSerialRead();
    }
  }
}


