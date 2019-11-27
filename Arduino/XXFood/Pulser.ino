void PulserSetup()
{
  if (sysStartUp)
  {
    // Pulse Generator
    cli();
    // Clear all settings of Timer5
    TCCR5A = 0;
    TCCR5B = 0;
    // Reset current count value to zero
    TCNT5 = 0;
    // Set Prescaler to 8 for counting every 0.5us
    TCCR5B |= (1 << CS51);
    // Turn on No.14 fast PWM mode
    TCCR5A |= (1 << WGM51); 
    TCCR5B |= (1 << WGM53) | (1 << WGM52);
    sei();
  }
}


volatile boolean pulsing = false;
volatile byte pulGenPin = 255;
volatile unsigned int pulCycle = 0;
volatile unsigned int pulLeft = 0;

void PulseGen(byte pin, double duration)
{
  duration = duration * 1000; // Convert millisecond to microsecond
  if (!pulsing)
  {
    pulGenPin = pin;
    digitalWrite(pulGenPin, HIGH);
    TCNT5 = 0;
    // Divide duration
    pulCycle = int(duration / 10000); // How many 10000us in duration?
    pulLeft = int(duration) % 10000; // How many us left?
    // Set timer according to duration
    if (pulCycle == 0)
      ICR5 = pulLeft * 2;
    else
      ICR5 = 10000 * 2;
    // Start timer
    pulsing = true;
    TIMSK5 |= (1 << TOIE5);
  }
}

ISR(TIMER5_OVF_vect)
{
  if (pulCycle == 0)
    PulseEnd();
  else
  {
    pulCycle--;
    if (pulCycle == 0)
    {
      if (pulLeft == 0)
        PulseEnd();
      else
        ICR5 = pulLeft * 2;
    }
  }
}

void PulseEnd()
{
  digitalWrite(pulGenPin, LOW);
  TIMSK5 = 0;
  pulsing = false;
  long t = millis();
  Serial.println(t);
}




//unsigned long timeCounter = 0;
//
//void Timer5()
//{
//  cli();
//  TCCR5A = 0;
//  TCCR5B = 0;
//  TCNT5 = 0;
//  ICR5 = 16000000/(8*1000000)-1; //presaler=8,frequence=1000000
//  TCCR5A |= (1 << WGM51); //fast PWM mode
//  TCCR5B |= (1 << WGM53)|(1 << WGM52)| (1 << CS51); // turn on fast PWM mode,Set prescaler of 8
//  TIMSK5 |= (1 << TOIE5);
//  sei();
//}  
//
//
//ISR(TIMER5_OVF_vect)
//{
//  timeCounter++;
//  Flash3();
//}
//
//void Flash3()
//{
//  unsigned long durationNumber1 = 50000; // duration different from my expect?
//  
//  if ( timeCounter < durationNumber1 )
//    digitalWrite(pinTest,LOW);
//  else
//  {
//    digitalWrite(pinTest,HIGH);
//    TIMSK5 = 0;
//    timeCounter = 0;
//  }
//}

//void Flash2(byte pin)
//{
//  int durationNumber = 50;
//  ledCounter = timeCounter;
//  digitalWrite(pin, HIGH);
//  while((ledCounter - timeCounter) != durationNumber){}
//  digitalWrite(pin, LOW);
//  
//}

