
#include <Adafruit_NeoPixel.h>

// PIN DECLARATIONS
int pinMUX1A = 3;
int pinMUX1B = 4;
int pinMUX1C = 5;
int pinMUX2A = 6;
int pinMUX2B = 7;
int pinMUX2C = 8;
int pinHEARTBEAT = 9;
int pinLEDDATA = 2;



// VARIABLE DECLARATIONS
int heartBeatCounter = 0; // only for dividing down timer2 to a visible speed
boolean heartBeatStatus = false; // keeps track of heartbeat on or off.
int numLEDs = 24;
byte byteCommand = 0b00000000;
int intCommand = 0;
int activeChannel = 0;
int ledStatus[24] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // 0 means untested, 1 means passed, 2 means failed.
unsigned long lngDisplayTimer = 0;

Adafruit_NeoPixel pixels(numLEDs, pinLEDDATA, NEO_GRB + NEO_KHZ800);

// HEARTBEAT!!
ISR(TIMER2_COMPA_vect){
  heartBeatCounter ++;
  if (heartBeatCounter >= 50){
    heartBeatCounter = 0;
    if (heartBeatStatus){
      digitalWrite(pinHEARTBEAT,LOW);
      heartBeatStatus = false;
    }
    else{
      digitalWrite(pinHEARTBEAT,HIGH);
      heartBeatStatus = true;
    }
  }
}

void setup() {
  // SETTING UP HEARTBEAT
  cli();//stop interrupts
  //set timer0 interrupt at 100 hz
  TCCR2A = 0; // clear entire control register
  TCCR2B = 0;
  TCNT2  = 0; //initialize counter value to 0
  OCR2A = 155;// set compare match register for 100Hz increments. OCR0A = (16*10^6) / (100*1024) - 1 (must be <256)
  TCCR2A |= (1 << WGM21); // turn on CTC mode
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // Set CS01 and CS00 bits for 64 prescaler
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei();//allow interrupts
  pinMode(pinHEARTBEAT, OUTPUT);
  digitalWrite(pinHEARTBEAT, LOW);
  
  Serial.begin(9600);
  pixels.begin();
  pixels.clear();
  pixels.show();
  lngDisplayTimer = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){ // just stall for time until another byte is available
    byteCommand = Serial.read();
    intCommand = (int) byteCommand;
    Serial.println(intCommand);
    if (intCommand == 43){ // recieved character '+', means success for the current channel
       ledStatus[activeChannel]=1;
    }
    if (intCommand == 45){ // recieved character '-', means failure for the current channel
       ledStatus[activeChannel]=2;
    }
    if (intCommand == 61){ // recieved character '=', means neutral for the current channel, ie, reset color to off
       ledStatus[activeChannel]=0;
    }
    intCommand -= 65; // adjust to use capital letters
    if (intCommand >=0 && intCommand < 24){ // set the channel to the new location.
      activeChannel = intCommand;
      setMux1(activeChannel%8);
      setMux2(activeChannel/8);
    }
    else{Serial.println("ERROR, INVALID CODE RECIEVED.");}
  }
  if (millis()-lngDisplayTimer > 30){
    for(int i=0; i<numLEDs; i++) {
      if (ledStatus[i]==0){pixels.setPixelColor(i, pixels.Color(0, 0, 0));}
      else if (ledStatus[i]==1){pixels.setPixelColor(i, pixels.Color(0, 100, 0));}
      else if (ledStatus[i]==2){pixels.setPixelColor(i, pixels.Color(100, 0, 0));}
      if (activeChannel == i){pixels.setPixelColor(i, pixels.Color(0, 0, 100));}
    }
    lngDisplayTimer = millis();
    pixels.show();
  }
}

// allows a single function call for setting a mux channel. Note: this function currently
// only supports a single group of muxes (all using the same select lines).
// Note there are OOP libraries for analog muxes but this code is really simple.
// channel input is 0-7
void setMux1(int channel){
  // if we are worried about speed, these digital writes need to be replaced. (and from hardware, pin selects
  // need to end up on one port!)
  if (channel%2 == 1){digitalWrite(pinMUX1A, HIGH);} else{digitalWrite(pinMUX1A, LOW);}
  if ((channel/2)%2 == 1){digitalWrite(pinMUX1B, HIGH);} else{digitalWrite(pinMUX1B, LOW);}
  if ((channel/4)%2 == 1){digitalWrite(pinMUX1C, HIGH);} else{digitalWrite(pinMUX1C, LOW);}
  // This hardware is only for an 8 to 1 mux
  //if ((channel/8)%2 == 1){digitalWrite(pinANALOGSELECT3, HIGH);} else{digitalWrite(pinANALOGSELECT3, LOW);}
  delayMicroseconds(8); // give settling/ capacitor charge shuffle equilization time.
}
void setMux2(int channel){
  // if we are worried about speed, these digital writes need to be replaced. (and from hardware, pin selects
  // need to end up on one port!)
  if (channel%2 == 1){digitalWrite(pinMUX2A, HIGH);} else{digitalWrite(pinMUX2A, LOW);}
  if ((channel/2)%2 == 1){digitalWrite(pinMUX2B, HIGH);} else{digitalWrite(pinMUX2B, LOW);}
  if ((channel/4)%2 == 1){digitalWrite(pinMUX2C, HIGH);} else{digitalWrite(pinMUX2C, LOW);}
  // This hardware is only for an 8 to 1 mux
  //if ((channel/8)%2 == 1){digitalWrite(pinANALOGSELECT3, HIGH);} else{digitalWrite(pinANALOGSELECT3, LOW);}
  delayMicroseconds(8); // give settling/ capacitor charge shuffle equilization time.
}
