// *************************************************
// example for bluepill SPI1
// Module     MCU
//  SFY       B9
//  CLK       A5
//  DAT       A7
//  VCC       +3.3v
// ************************************************* 

#include "STM32ad9833.h"

#define SFY_PIN   PB9

STM32ad9833       dds(SFY_PIN);      // ad9833 driver

uint32_t f = 1000;

void setup() {
  dds.begin(AD_SINE, (float)f);    // start sintesys: sine wave 1.0 KHz
}


void loop() {
}
