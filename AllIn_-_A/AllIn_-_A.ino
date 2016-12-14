
// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>
#include <RGBConverter.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define pinLed    6
#define pinButton 2

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      3

// Led Strip
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, pinLed, NEO_GRB + NEO_KHZ800);
int delayval = 10; // delay for half a second

uint16_t stripR, stripG, stripB;

// Color sensor
/* Initialise with default values (int time = 2.4ms, gain = 1x) */
Adafruit_TCS34725 tcs = Adafruit_TCS34725();
#define maxIn   128

// RGB Converter
RGBConverter rgbConvert;

// Night/Day
bool dayMode;

// Pulse
float brightRatio;
float brightFreq = 5000;

void setup() {
  
  Serial.begin(9600);
  
  Serial.println("Init --------------------------------------");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  Serial.println("-");
  
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  pixels.begin(); // This initializes the NeoPixel library.

  dayMode = 1;
  attachInterrupt(digitalPinToInterrupt(pinButton), changeNightAndDay, RISING);
  
}

void loop() {
  // 
  brightRatio = fmod(millis(),brightFreq) / brightFreq;
  brightRatio = 0.5f * (sin(TWO_PI * brightRatio) + 1);
  
  getColorSensor();
  setLeds();
  
}

void setLeds(){
  setCowork();
  setCroix();
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void setCowork(){
  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
  // CoWork is White or Not
  if(dayMode == 1){
    // DAY -> On éteint
    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
  }else{
    // Night -> On allume
    pixels.setPixelColor(0, pixels.Color(255,255,255));
    pixels.setPixelColor(1, pixels.Color(255,255,255));
  }
  
}

void setCroix(){
  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
  pixels.setPixelColor(2, pixels.Color(stripR,stripG,stripB)); // Moderately bright green color.
}

void getColorSensor() {
  uint16_t r, g, b, c, colorTemp, lux;
  
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);
  
  double hsl[3];
  rgbConvert.rgbToHsl(r, g, b, hsl);

  byte rgb[3];
  if(hsl[1] >= 1){
    // Nothing
    rgbConvert.hslToRgb(hsl[0], hsl[1], hsl[2], rgb);
  }else{
    // Something
    rgbConvert.hslToRgb(hsl[0], hsl[1] * 1.4f, 0.5f, rgb);
  }
  
  // Calculation 
  stripR = map(rgb[0], 0, 255, 0, c) * brightRatio;
  stripG = map(rgb[1], 0, 255, 0, c) * brightRatio;
  stripB = map(rgb[2], 0, 255, 0, c) * brightRatio;

  //Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  //Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("brightRatio : ");
  Serial.print(brightRatio, DEC);
  Serial.print(" - ");
  
  Serial.print("[R,G,B] : ");
  Serial.print("[");
  Serial.print(r, DEC); Serial.print(","); Serial.print(g, DEC); Serial.print(",");Serial.print(b, DEC);
  Serial.print("] - ");

  Serial.print("[h,s,l] : ");
  Serial.print("[");
  Serial.print(hsl[0], DEC); Serial.print(","); Serial.print(hsl[1], DEC); Serial.print(",");Serial.print(hsl[2], DEC);
  Serial.print("] - ");
    
  Serial.print("Strip [R,G,B] : ");
  Serial.print("[");
  Serial.print(rgb[0], DEC); Serial.print(","); Serial.print(rgb[1], DEC); Serial.print(",");Serial.print(rgb[2], DEC);
  Serial.print("] - ");

  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
  
}

void changeNightAndDay(){
    
    if(dayMode == 0){
      dayMode = 1;
    }else{    
      dayMode = 0;
    }

    Serial.println("Changing Night to Day, and vice-versa");
    
}

