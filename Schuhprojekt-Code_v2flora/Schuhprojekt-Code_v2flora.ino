/*#include <Adafruit_TSL2591.h>
#include <Adafruit_Sensor.h>*/
#include <Adafruit_NeoPixel.h>

#define LEDNUM      44      // Number of LEDs
#define HEADLIGHT   3       // Number of LEDs for Headlight
#define PIN         10      // Digital Output for LED-Strip (FLORA 10)
#define LUXSDA      A4      // Analog-Input for Lux-Sensor SDA
#define LUXSCL      A5      // Analog-Input for Lux-Sensor SCL
#define MAXF        6       // Max Count of Modes

const uint8_t interruptPinState   = 1;    // Digital Output for Interrupt-Button (State) (FLORA 1)
const uint8_t interruptPinBright  = 0;    // Digital Output for Interrupt-Button (Brightness) (FLORA ?)
volatile uint8_t state            = 0;    // Current Mode
volatile uint8_t bright           = 50;   // Current Brightness

// NeoPixel-Strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDNUM, PIN, NEO_GRB + NEO_KHZ800);

// Lux-Sensor
// Adafruit_TSL2591 tsl = Adafruit_TSL2591(1);

/*
 * Setup-Function
 */
void setup() {
  // Start Serial-Communication
  Serial.begin(9600);

  // Set PIN-Mode for Buttons
  pinMode(interruptPinState, INPUT_PULLUP);
  pinMode(interruptPinBright, INPUT_PULLUP);
  
  strip.begin();
  strip.show();

  // Attach Handlers to Interrupt
  attachInterrupt(digitalPinToInterrupt(interruptPinState), handlerState, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptPinBright), handlerBright, RISING);
}

/*
 * Loop-Function
 */
void loop() {
  strip.setBrightness(bright);

  if(bright) {
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
}

/*
 * Interrupt-Handler for Button-Press to set the State
 */
void handlerState() {
  uint8_t sensorVal = digitalRead(interruptPinState);
  
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

/*
 * Interrupt-Handler for Button-Press to set the Brightness
 */
void handlerBright() {
  uint8_t sensorVal = digitalRead(interruptPinState);

  Serial.print("Button2 Value: ");
  Serial.println(sensorVal);
  Serial.print("Brightness: ");
  Serial.println(bright);

  if(sensorVal == HIGH) {
    switch (bright) {
      case 0:
        bright = 50;
        break;
      case 50:
        bright = 150;
        break;
      case 150:
        bright = 255;
        break;
      case 255:
        bright = 0;
        break;
    }
  }
}

/*
 * Triggers all modes in a row
 */
void wheelstart() {
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50, 0); // Red
  colorWipe(strip.Color(0, 255, 0), 50, 1); // Green
  colorWipe(strip.Color(0, 0, 255), 50, 2); // Blue
  rainbow(20, 3);
  rainbowCycle(20, 2);
}

/*
 * Fill the dots one after the other with a color
 * 
 * @c:        Color to be set
 * @wait:     Time to wait till Change
 * @fnumber:  Number of the Function
 */
void colorWipe(uint32_t c, uint8_t wait, uint8_t fnumber) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setBrightness(bright);
      strip.setPixelColor(i, c);
      strip.show();
      
      if(state != fnumber){
        break;
      }

      delay(wait);
  }
}

/*
 * Shows a rainbow
 * 
 * @wait:     Time to wait till Change
 * @fnumber:  Number of the Function
 */
void rainbow(uint8_t wait, uint8_t fnumber) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setBrightness(bright);
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

/*
 * Slightly different, this makes the rainbow equally distributed throughout
 * 
 * @wait:     Time to wait till Change
 * @fnumber:  Number of the Function
 */
void rainbowCycle(uint8_t wait, uint8_t fnumber) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setBrightness(bright);
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

/*
 * Input a value 0 to 255 to get a color value.
 * The colours are a transition r - g - b - back to r.
 * 
 * @WheelPos: Curren Position in Wheel
 */
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

/*
 * Light-Mode for LED-Strip
 */
void showLight() {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    double m = 1;
    
    if(i > (LEDNUM / 2 - HEADLIGHT) && i < (LEDNUM / 2 + HEADLIGHT)) {
      m = 255;
    } else {
      m = 0;
    }

    strip.setBrightness(bright);
    strip.setPixelColor(i, strip.Color(m, m, m));
    strip.show();
  }
}


