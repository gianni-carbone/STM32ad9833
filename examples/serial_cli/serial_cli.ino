// *************************************************
// example for bluepill SPI1
// Module     MCU
//  SFY       B9
//  CLK       A5
//  DAT       A7
//  VCC       +3.3v
// ************************************************* 

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
  dds.begin(st.shape, st.frequency, st.phase, st.channel);
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
