// *************************************************
// example for bluepill SPI1
// Module     MCU
//  SFY       B9
//  CLK       A5
//  DAT       A7
//  VCC       +3.3v
// ************************************************* 

#define USE_HAL_GPIO  // enable the fast SFY pin toggle. Suggestion: stay it enabled
#define USE_HAL_SPI   // enable the fastest SPI write. 
                      // Suggestion: if the spi interface is for the exclusive use of the module then stay it enabled.
                      // If you have to share the spi interface with other slaves with different settings than MODE2 @MCLK/2
                      // then comment the line and use spi transactions.

#include "STM32ad9833.h"

#define SFY_PIN   PB9

STM32ad9833       dds(SFY_PIN);      // ad9833 driver

uint32_t f = 1000;

void setup() {
  dds.begin(AD_SINE, (float)f);    // start sintesys: sine wave 1.0 KHz
}



void loop() {
}
