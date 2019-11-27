void TriggerSetup()
{
  if (sysStartUp)
  {
    // Trigger-in
  //  pinMode(trgInPin, INPUT);
  //  attachInterrupt(0, TrigInInterrupt, RISING);
    
    // Trigger-out
    pinMode(trgOutPin, OUTPUT);
    pinMode(trgSynPin, OUTPUT);
  }
}



// Feed Trigger-in //
// Once receiving this interrupt, it sets flag to true, 
// allowing the CheckTriggered() in loop() to ask computer for food grabing commands. 
//void TrigInInterrupt() 
//{
//  trgDetected = true;
//}
//void CheckTriggered()
//{
//  if(trgDetected)
//  {
//    Serial.println("triggered");
//    trgDetected = false;
//  }
//}



// Imaging Trigger-out //
void ImgTrigOut() // Send a Pulse to Imaging System
{
  digitalWrite(trgOutPin, HIGH);
  digitalWrite(trgSynPin, HIGH);
  delay(1);
  digitalWrite(trgOutPin, LOW);
  delay(99);
  digitalWrite(trgSynPin, LOW);
}
