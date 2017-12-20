#include <Adafruit_NeoPixel.h>

#define PIN 6
#define MAXF 5

const int interruptPin = 3;
volatile int state = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(22, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
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
      rainbow(20);
      break;
    case 4:
      rainbowCycle(20);
      break;
  }
}

void handler() {
  if(state + 1 < MAXF) {
    state++;
  } else {
    state = 0;
  }
}

int breakFunction(int num){
  if(state != num){

  }
}

void wheelstart() {
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50, 0); // Red
  colorWipe(strip.Color(0, 255, 0), 50, 1); // Green
  colorWipe(strip.Color(0, 0, 255), 50, 2); // Blue
  rainbow(20);
  rainbowCycle(20);
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

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
      if(state != 1){
        break;
      }
    }
    strip.show();
    if(state != 1){
      break;
    }
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      if(state != 2){
        exit;
      }
    }
    strip.show();
    if(state != 2){
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
