#include <Adafruit_NeoPixel.h>

#define LEDNUM 23   //Number of LEDs
#define PIN 6       //Digital Output for LED-Strip (FLORA 10)
#define MAXF 6      //Max Count of Modes

const int interruptPin = 2; //Digital Output for Interrupt-Button (FLORA 1)
volatile int state = 0;     //Current Mode

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDNUM, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  //Start Serial-Communication
  Serial.begin(9600);
  
  pinMode(interruptPin, INPUT_PULLUP);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  attachInterrupt(digitalPinToInterrupt(interruptPin), handler, CHANGE);
}

void loop() {
  switch (state) {
    case 0:
      colorWipe(strip.Color(0, 0, 255), 50, 0);
      break;
    case 1:
      colorWipe(strip.Color(0, 255, 0), 50, 1);
      break;
    case 2:
      colorWipe(strip.Color(255, 0, 0), 50, 2);
      break;
    case 3:
      rainbow(20, 3);
      break;
    case 4:
      rainbowCycle(20, 2);
      break;
    case 5:
      showLight();
      break;
  }
}

//Interrupt-Handler for Button-Press
void handler() {
  int sensorVal = digitalRead(interruptPin);
  
  Serial.print("Button Value: ");
  Serial.println(sensorVal);
  Serial.print("State: ");
  Serial.println(state);
  
  if(sensorVal == LOW) {
    if(state + 1 < MAXF) {
      state++;
    } else {
      state = 0;
    }
  }
}

int breakFunction(int num) {
  if(state != num) {

  }
}

//Triggers all modes in a row
void wheelstart() {
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50, 0); // Red
  colorWipe(strip.Color(0, 255, 0), 50, 1); // Green
  colorWipe(strip.Color(0, 0, 255), 50, 2); // Blue
  rainbow(20, 3);
  rainbowCycle(20, 2);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait, int stat) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      
      if(state != stat){
        break;
      }

      delay(wait);
  }
}

//Shows a rainbow
void rainbow(uint8_t wait, uint8_t fnumber) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
      if(state != fnumber){
        break;
      }
    }
    strip.show();
    if(state != fnumber){
      break;
    }
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait, uint8_t fnumber) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      if(state != fnumber){
        exit;
      }
    }
    strip.show();
    if(state != fnumber){
      break;
    }
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

//Light-Mode for LED-Strip
void showLight() {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    double m = 1;
    
    if(i > (LEDNUM / 2 - 3) && i < (LEDNUM / 2 + 3)) {
      m = 255;
    } else {
      m = 0;
    }
    
    strip.setPixelColor(i, strip.Color(m, m, m));
    strip.show();
  }
}

