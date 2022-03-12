// *************************************************
// example for bluepill SPI1
// Module     MCU
//  SFY       B9
//  CLK       A5
//  DAT       A7
//  VCC       +3.3v
// ************************************************* 

#define USE_HAL_GPIO  // this one enable the fast SFY pin toggle. Suggestion: stay it enabled
#define USE_HAL_SPI   // this one enables the fastest SPI write. 
                      // Suggestion: if the spi interface is for the exclusive use of the module then stay it enabled.
                      // If you have to share the spi interface with other slaves with different settings than MODE2
                      // then comment the line and use spi transactions.

#include "STM32ad9833.h"

typedef struct {
  float               frequency = 1000.0;   // current frequency
  uint16_t            phase = 0;            // current phase (in 10th of herz)
  shape_t             shape = AD_SINE;
  uint8_t             channel = 0;
} status_t;

status_t st;

String  cmdLine="";                  // serial command buffer 
#define SFY_PIN   PB9

STM32ad9833       dds(SFY_PIN);      // ad9833 driver

void setup() {
  Serial.begin(115200);
  dds.begin(st.frequency, st.phase, st.shape, st.channel);
}


void loop() {
  char      c;                                                // incoming character                      
  while(Serial.available() > 0) {
    c = Serial.read();
    if (c != '\n') {
      cmdLine=cmdLine+String(c);
    } else {
      cmdLine.toUpperCase();
      parseCmd();
      cmdLine = "";
    }
  }
}
