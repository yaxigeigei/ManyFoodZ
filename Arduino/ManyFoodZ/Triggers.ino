void TriggerSetup()
{
  // Trigger-in
//  pinMode(trgInPin, INPUT);
//  attachInterrupt(0, TrigInInterrupt, RISING);
  
  // Trigger-out
  pinMode(trgOutPin, OUTPUT);
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
  delay(1);
  digitalWrite(trgOutPin, LOW);
}
