#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_NeoPixel.h>

#define LEDNUM      23      // Number of LEDs
#define HEADLIGHT   3       // Half of Number of LEDs for Headlight
#define PIN         10      // Digital Output for LED-Strip (FLORA 10)
#define LUXSDA      A2      // Analog-Input for Lux-Sensor SDA (Flora 2)
#define LUXSCL      A3      // Analog-Input for Lux-Sensor SCL (Flora 3)
#define MAXF        12      // Max Count of Modes

const uint8_t interruptPinState   = 1;    // Digital Output for Interrupt-Button (State) (FLORA 1)
const uint8_t interruptPinBright  = 0;    // Digital Output for Interrupt-Button (Brightness) (FLORA 0)
volatile uint8_t state            = 0;    // Current Mode
volatile uint8_t bright           = 1;    // Current Brightness-State
volatile uint8_t brightness       = 50;   // Current Brightness
volatile uint16_t broadband       = 0;    // for Lux-Sensor getLuminosity() (broadband)
volatile uint16_t infrared        = 0;    // for Lux-Sensor getLuminosity() (infrared)

// NeoPixel-Strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDNUM, PIN, NEO_GRB + NEO_KHZ800);

// Status LED
Adafruit_NeoPixel statusLED = Adafruit_NeoPixel(1, 8, NEO_GRB + NEO_KHZ800);

// Lux-Sensor
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

/*
 * Setup-Function
 */
void setup() {
  // Start Serial-Communication
  Serial.begin(9600);

  // Set PIN-Mode for Buttons
  pinMode(interruptPinState, INPUT_PULLUP);
  pinMode(interruptPinBright, INPUT_PULLUP);

  if(!tsl.begin()) {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  // Configure Lux-Sensor
  configureSensor();

  strip.setBrightness(brightness);
  strip.begin();
  strip.show();

  strip.setBrightness(40);
  statusLED.begin();
  statusLED.show();

  statusLED.setPixelColor(0, statusLED.Color(0, 255, 0));      
  statusLED.show();

  // Attach Handlers to Interrupt
  attachInterrupt(digitalPinToInterrupt(interruptPinState), handlerState, CHANGE);
  attachInterrupt(digitalPinToInterrupt(interruptPinBright), handlerBright, CHANGE);
}

/*
 * Loop-Function
 */
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
      rainbowCycle(20, 4);
      break;
    case 5:
      circulate(5);
      break;
    case 6:
      showLight();
      break;
    case 7:
      twinkle();
      break;
    case 8:
      strobe();
      break;
    case 9:
      randomFade(random(255), random(255), random(255), 10, 9);
      break;
    case 10:
      rainbowChase(50, 10);
      break;
    case 11:
      ray(random(255), random(255), random(255), 11);
      break;
  }

  /* Measure brightness and convert it */ 
  if(bright == 3) {
    tsl.getLuminosity(&broadband, &infrared);
    luxToBrightness(broadband);
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
        statusLED.setPixelColor(0, statusLED.Color(0, 255, 0));      
        statusLED.show();
        break;
      case 1:
        bright = 2;
        brightness = 75;
        break;
      case 2:
        bright = 3;
        brightness = 100;
        break;
      case 3:
        bright = 4;
        brightness = 100;
        break;
      case 4:
        bright = 0;
        brightness = 0;

        /* Set Status LED */
        statusLED.setPixelColor(0, statusLED.Color(255, 0, 0));  
        statusLED.show();
        break;
    }
  }
}

/*
 * Configure the Lux-Sensor
 */
void configureSensor() {
  tsl.setGain(TSL2561_GAIN_1X);
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
}

/*
 * Convert Lux to 8 Bit and set Brightness
 * 
 * @lux: Measured lux-value to convert
 */
void luxToBrightness(int lux) {
  Serial.println(lux);
  if(lux < 10) {
      brightness = 100;
  } else if(lux < 15) {
      brightness = 80;
  } else if(lux < 25) {
      brightness = 70;
  } else if(lux < 35) {
      brightness = 60;
  } else if(lux < 50) {
      brightness = 50;
  } else if(lux < 65) {
      brightness = 40;
  } else if(lux < 80) {
      brightness = 30;
  } else if(lux < 100) {
      brightness = 20;
  }
}

/*
 * Fill the dots one after the other with a color
 * 
 * @c:        Color to be set
 * @wait:     Time to wait till change
 * @fnumber:  Number of the function
 */
void colorWipe(uint32_t c, uint8_t wait, uint8_t fnumber) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setBrightness(brightness);
      strip.setPixelColor(i, c);
      strip.show();
      
      if(state != fnumber || bright == 0){
        break;
      }

      delay(wait);
  }
}

/*
 * Shows a rainbow
 * 
 * @wait:     Time to wait till change
 * @fnumber:  Number of the function
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
 * @wait:     Time to wait till change
 * @fnumber:  Number of the function
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
 * @WheelPos: Current position in wheel
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
 * Knight Rider-Style circulation
 * 
 * @fnumber:  Number of the function
 */
void circulate(uint8_t fnumber) {
  uint8_t i = 0;
  uint8_t dir = 0;
  uint8_t wait = 210;
  
  while(1) {   
    for(uint8_t j = 0; j < strip.numPixels(); j++) {
      if (j > i-2 && j < i+2) {
        strip.setBrightness(brightness);
        strip.setPixelColor(j, strip.Color(255, 0, 0)); 
      } else {
        strip.setBrightness(brightness);
        strip.setPixelColor(j, strip.Color(0, 0, 0));
      }
      
      if(state != fnumber){
        exit;
      }
    }

    strip.show();

    if(state != fnumber){
      break;
    }

    if(i < 6) {
      if(dir) {
        wait += 15 + 6 - i; 
      } else {
        wait -= 15 + 6 - i; 
      }
    } else if(i > strip.numPixels() - 6) {
      if(dir) {
        wait -= 15 + strip.numPixels() - i; 
      } else {
        wait += 15 + strip.numPixels() - i; 
      }
    } else {
      wait = 30;
    }

    if(dir == 0) {
      if(i == strip.numPixels() - 3) {
        dir = 1;
      }
      
      i++;
    } else if(dir == 1) {
      if (i == 2) {
        dir = 0;
      }
      
      i--;
    }

    delay(wait);
  }
}

/*
 * light mode
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

/*
 * twinkle mode
 */
void twinkle() {
  clearStrip();
  uint8_t  j = random(LEDNUM);
  strip.setBrightness(brightness);
  strip.setPixelColor(j, 255, 255, 255);
  strip.show();
  delay(50);
  strip.setPixelColor(j, 0, 0, 0);
}

/*
 * strobe mode
 */
void strobe() {
  clearStrip();
  delay(50);
  for(uint8_t j=0; j<strip.numPixels(); j++) {
    strip.setBrightness(brightness);
    strip.setPixelColor(j, 255, 255, 255);
  }
  strip.show();
  delay(50);
}

/*
 * Fade in and out random color mode
 * 
 * @green:    Green color to be set
 * @red:      Red color to be set
 * @blue:     Blue color to be set
 * @wait:     Time to wait till change
 * @fnumber:  Number of the function
 */
void randomFade(uint8_t green, uint8_t red, uint8_t blue, uint8_t wait, uint8_t fnumber) {
  for(uint8_t b=0; b <255; b++) {
    for(uint8_t i=0; i < strip.numPixels(); i++) {
      strip.setBrightness(brightness);
      strip.setPixelColor(i, green*b/255, red*b/255, blue*b/255, brightness);
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

  for(uint8_t b=255; b > 0; b--) {
    for(uint8_t i=0; i < strip.numPixels(); i++) {
      strip.setBrightness(brightness);
      strip.setPixelColor(i, green*b/255, red*b/255, blue*b/255, brightness);
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
 * rainbowChase mode (aus der Beispielbibliothek)
 * @wait:     Time to wait till change
 * @fnumber:  Number of the function
 */
void rainbowChase(uint8_t wait, uint8_t fnumber) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setBrightness(brightness);
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        if(state != fnumber){
          break;
        }
      }
      strip.show();
      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setBrightness(brightness);
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
        if(state != fnumber){
          break;
        }
      }
      if(state != fnumber){
        break;
      }
    }
    if(state != fnumber){
      break;
    }
  }
}

/*
 * ray mode
 * 
 * @green:    Green color to be set
 * @red:      Red color to be set
 * @blue:     Blue color to be set
 * @fnumber:  Number of the function
 */
void ray(uint8_t green, uint8_t red, uint8_t blue, uint8_t fnumber) {
  clearStrip();
  for(uint16_t i=0; i<LEDNUM+7; i++) {
    strip.setBrightness(brightness);
    strip.setPixelColor(i, green, red, blue);
    strip.show();
    delay(50);
    strip.setPixelColor(i-7, 0, 0, 0);
    strip.show();
    if(state != fnumber){
      break;
    }
  }
}

/*
 * clear strip function
 */
void clearStrip() {
  for(uint8_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}
