
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
#define pinCowork 5
#define pinCroix  6
#define pinButton 2

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS_Cowork  3
#define NUMPIXELS_Croix   3

// Led Strip
Adafruit_NeoPixel pixelsCroix = Adafruit_NeoPixel(NUMPIXELS_Croix, pinCroix, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsCowork = Adafruit_NeoPixel(NUMPIXELS_Cowork, pinCowork, NEO_GRB + NEO_KHZ800);
int delayval = 10; // delay for half a second

uint16_t stripR, stripG, stripB;

// Color sensor
/* Initialise with default values (int time = 2.4ms, gain = 1x) */
Adafruit_TCS34725 tcs = Adafruit_TCS34725();
#define maxIn   128
int startC = -1;
bool rgbIsConnected;

// RGB Converter
RGBConverter rgbConvert;

// Night/Day
bool dayMode;

// Pulse
float heartBeat;
float heartFreq = 5000;

// If I want to log, define it
// If you don't want, comment it
#define iCanLog

void setup() {
  
  Serial.begin(9600);
  Serial.println("Init --------------------------------------");

  rgbIsConnected = tcs.begin();
  if (rgbIsConnected) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  
  pixelsCowork.begin(); // This initializes the NeoPixel library.
  pixelsCroix.begin(); // This initializes the NeoPixel library.

  dayMode = 1;
  attachInterrupt(digitalPinToInterrupt(pinButton), changeNightAndDay, RISING);
  
}

void loop() {
  
  // --- Calculation of heartBeat
  heartBeat = fmod(millis(),heartFreq) / heartFreq;
  heartBeat = 0.5f * (sin(TWO_PI * heartBeat) + 1);
  
  #ifdef iCanLog
    Serial.print("Day/Night : ");Serial.print(dayMode, DEC);Serial.print(" - ");
    Serial.print("HeartBeat : "); Serial.print(heartBeat, DEC);Serial.print(" - ");
  #endif
  
  // --
  getColorSensor();
  // --
  setCowork();
  setCroix();

  // End of log /n
  Serial.println("");
  
}

void setCowork(){
  
  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
  for (int i=0; i < NUMPIXELS_Cowork; i++){
    byte    rgb[3];
    rgbConvert.hslToRgb(1.0f, 1.0f, 1.0f, rgb);
    pixelsCowork.setPixelColor(i, pixelsCowork.Color(rgb[0], rgb[1], rgb[2]));
  } 
  
  // CoWork is Bright or Not
  if(dayMode == 1){
    // DAY -> On éteint
    pixelsCowork.setBrightness(0);
  }else{
    // Night -> On allume
    pixelsCowork.setBrightness(255);
  }
  
  // This sends the updated pixel color to the hardware.
  pixelsCowork.show(); 
  
  #ifdef iCanLog
    Serial.print("Cowork : ");
    Serial.print(pixelsCowork.getBrightness(), DEC);
  #endif


}

void setCroix(){

  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
  for (int i=0; i < NUMPIXELS_Croix; i++){
    pixelsCroix.setPixelColor(i, pixelsCroix.Color(stripR,stripG,stripB));
  }
  
  // Set the brightnes with bright ratio
  pixelsCroix.setBrightness(255.0f * heartBeat);
  
  // This sends the updated pixel color to the hardware.
  pixelsCroix.show(); 
  
  #ifdef iCanLog
    Serial.print("Croix : ");
    Serial.print(pixelsCroix.getBrightness(), DEC);
  #endif

  
}

void getColorSensor() {
  uint16_t r, g, b, c, colorTemp, lux;
  byte    rgb[3];
  double  hsl[3];
    
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);

  // Initialization with the first value
  if(startC == -1){
    startC = c;
  }
  
  rgbConvert.rgbToHsl(r, g, b, hsl);

  //if(hsl[1] >= 1){
  if(c <= startC){
    // Nothing
    rgbConvert.hslToRgb(hsl[0], 0.0f, 0.0f, rgb);
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
  }else{
    // Something
    rgbConvert.hslToRgb(hsl[0], hsl[1] * 1.4f, 0.5f, rgb);
  }
  
  // Calculation 
  stripR = map(rgb[0], 0, 255, 0, c);
  stripG = map(rgb[1], 0, 255, 0, c);
  stripB = map(rgb[2], 0, 255, 0, c);

#ifdef iCanLog
  //Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  //Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  
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
  Serial.print("startC: "); Serial.print(startC, DEC); Serial.print(" ");

#endif

}

void changeNightAndDay(){
    
    if(dayMode == 0){
      dayMode = 1;
    }else{    
      dayMode = 0;
    }

    Serial.println("Changing Night to Day, and vice-versa");
    
}

