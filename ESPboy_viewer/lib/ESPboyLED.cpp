/*
ESPboy LED class
for www.ESPboy.com project by RomanS
*/

#include "ESPboyLED.h"


void ESPboyLED::begin(Adafruit_MCP23017 *mcpGUI){
  mcp = mcpGUI;
  pinMode(LEDPIN, OUTPUT);
  mcp->pinMode(LEDLOCK, OUTPUT);
  LEDflagOnOff = 1;
  LEDr = 0; 
  LEDg = 0; 
  LEDb = 0;
  ledset(LEDr, LEDg, LEDb);
}


void ESPboyLED::off(){
  LEDflagOnOff = 0;
  ledset(0, 0, 0);
}


void ESPboyLED::on(){
  LEDflagOnOff = 1;
  ledset(LEDr, LEDg, LEDb);
}


uint8_t ESPboyLED::getState(){
  return (LEDflagOnOff);
}

void ESPboyLED::setRGB (uint8_t red, uint8_t green, uint8_t blue){
  LEDr = red;
  LEDg = green;
  LEDb = blue;
  if (LEDflagOnOff) ledset(LEDr, LEDg, LEDb);
}


void ESPboyLED::setR (uint8_t red){
  LEDr = red;
  if (LEDflagOnOff) ledset(LEDr, LEDg, LEDb);
}


void ESPboyLED::setG (uint8_t green){
  LEDg = green;
  if (LEDflagOnOff) ledset(LEDr, LEDg, LEDb);
}


void ESPboyLED::setB (uint8_t blue){
  LEDb = blue;
  if (LEDflagOnOff) ledset(LEDr, LEDg, LEDb);
}


uint32_t ESPboyLED::getRGB(){
  return (((uint32_t)LEDb<<16) + ((uint32_t)LEDg<<8) + ((uint32_t)LEDr) );
}


uint8_t ESPboyLED::getR(){
  return (LEDr);
}


uint8_t ESPboyLED::getG(){
  return (LEDg);
}


uint8_t ESPboyLED::getB(){
  return (LEDb);
}


#define T0H (F_CPU/2000000)
#define T1H (F_CPU/833333)
#define TT0T (F_CPU/400000)
 
#define PINMASK (1<<LEDPIN) 
 
 
void ICACHE_RAM_ATTR ESPboyLED::ledset(uint8_t rled, uint8_t gled, uint8_t bled) {
 static uint_fast32_t i, t, c, startTime, pixel;
 static uint_fast32_t mask; 
 static uint8_t rled_, gled_, bled_;
 static uint32_t timer;
 
 if(!getState()) return;
 
 if(timer+10>millis()) return;
 timer=millis();
 
 if(rled_==rled && gled_==gled && bled_==bled) return;
 rled_=rled; gled_=gled; bled_=bled;

  GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, PINMASK);
  mcp->digitalWrite(LEDLOCK, HIGH); 
  pixel = (gled<<16) + (rled<<8) + bled;
  mask = 0x800000;
  
  os_intr_lock();
  startTime = 0;
  for (i=0; i<24; i++){
    if (pixel & mask) t = T1H;
    else t = T0H;
    while (((c=ESP.getCycleCount()) - startTime) < TT0T);// Wait for the previous bit to finish
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, PINMASK);      // digitalWrite HIGH 
    startTime = c;   
    while (((c=ESP.getCycleCount()) - startTime) < t);   // Wait for high time to finish
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, PINMASK);      // digitalWrite LOW
    mask>>=1;
  }
  os_intr_unlock();
  
  GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, PINMASK);

  mcp->digitalWrite(LEDLOCK, LOW); 
}
