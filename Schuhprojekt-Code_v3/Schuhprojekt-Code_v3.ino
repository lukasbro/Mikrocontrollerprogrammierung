#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_NeoPixel.h>

#define LEDNUM      23      // Number of LEDs
#define HEADLIGHT   3       // Half of Number of LEDs for Headlight
#define PIN         6       // Digital Output for LED-Strip (FLORA 10)
#define LUXSDA      A4      // Analog-Input for Lux-Sensor SDA (Flora 2)
#define LUXSCL      A5      // Analog-Input for Lux-Sensor SCL (Flora 3)
#define MAXF        6       // Max Count of Modes

const uint8_t interruptPinState   = 2;    // Digital Output for Interrupt-Button (State) (FLORA 1)
const uint8_t interruptPinBright  = 3;    // Digital Output for Interrupt-Button (Brightness) (FLORA 0)
volatile uint8_t state            = 0;    // Current Mode
volatile uint8_t bright           = 1;    // Current Brightness-State
volatile uint8_t brightness       = 50;   // Current Brightness

// NeoPixel-Strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDNUM, PIN, NEO_GRB + NEO_KHZ800);

// Lux-Sensor
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 1);

/*
 * Setup-Function
 */
void setup() {
  // Start Serial-Communication
  Serial.begin(9600);

  // Set PIN-Mode for Buttons
  pinMode(interruptPinState, INPUT_PULLUP);
  pinMode(interruptPinBright, INPUT_PULLUP);

  strip.setBrightness(brightness);
  strip.begin();
  strip.show();

  // Attach Handlers to Interrupt
  attachInterrupt(digitalPinToInterrupt(interruptPinState), handlerState, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptPinBright), handlerBright, FALLING);

  if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  // Configure Lux-Sensor
  configureSensor();
}

/*
 * Loop-Function
 */
void loop() {
  // strip.setBrightness(brightness);
  
  switch (state) {
    case 0:
      colorWipe(strip.Color(0, 0, 255, brightness), 50, 0);
      break;
    case 1:
      colorWipe(strip.Color(0, 255, 0, brightness), 50, 1);
      break;
    case 2:
      colorWipe(strip.Color(255, 0, 0, brightness), 50, 2);
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

  /* Measure Brightness and convert to 0-255 */ 
  if(bright == 4) {
    sensors_event_t event;
    tsl.getEvent(&event);

    if (event.light) {
      luxToBrightness(event.light);
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
  uint8_t sensorVal = digitalRead(interruptPinBright);

  Serial.print("Button2 Value: ");
  Serial.println(sensorVal);
  Serial.print("Brightness: ");
  Serial.println(bright);

  if(sensorVal == LOW) {
    switch (bright) {
      case 0:
        bright = 1;
        brightness = 50;
        break;
      case 1:
        bright = 2;
        brightness = 125;
        break;
      case 2:
        bright = 3;
        brightness = 255;
        break;
      case 3:
        bright = 4;
        brightness = 255;
        break;
      case 4:
        bright = 0;
        brightness = 0;
        break;
    }
  }
}

/*
 * Configure the Lux-Sensor
 */
void configureSensor() {
  tsl.enableAutoRange(true);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);
}

/*
 * Convert Lux to 8 Bit and set Brightness
 * 
 * @lux: Measured Lux-Value to Convert
 */
void luxToBrightness(uint16_t lux) {
  brightness = (int)(lux / 66000 * 255);
}

/*
 * Fades the Brightness to nbrightness
 * 
 * @nbrightness: The new Brightness to be set
 */
uint8_t fadeBrightness(uint8_t nbrightness) {
  for(uint8_t i = 0; i < nbrightness; i++) {
    return i;
  }

  return nbrightness;
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
      strip.setBrightness(brightness);
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
      strip.setBrightness(brightness);
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
      strip.setBrightness(brightness);
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
  for(uint8_t i=0; i<strip.numPixels(); i++) {
    double m = 1;
    
    if(i + 1 > (LEDNUM / 2 - HEADLIGHT) && i < (LEDNUM / 2 + HEADLIGHT)) {
      m = 255;
    } else {
      m = 0;
    }

    strip.setBrightness(brightness);
    strip.setPixelColor(i, strip.Color(m, m, m));
    strip.show();
  }
}


