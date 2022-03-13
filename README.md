# STM32ad9833

# Description 
Arduino STM32 library for Analog Device ad9833 module. This library simplifies the use of dds analog devices ad9833 modules. It also allowed for high module update rates compared to using the standard SPI library.

## AD9833 modules
The AD9833 is a programmable waveform generator capable of producing sine, triangular, and square wave outputs. The output frequency and phase are software programmable. With a 25 MHz clock rate, resolution of 0.1 Hz can be achieved. The AD9833 is written to via a 3-wire serial interface. This serial interface operates at clock rates up to 40 MHz. The device operates with a power supply from 2.3 V to 5.5 V. The AD9833 contains two frequency registers and two phase registers. It is possible to load each register with a distinct value and then subsequently select the value for frequency/phase synthesis. Eg. it is possible to load the frequency f1 in reg0 and that f2 in reg1, and operate an FSK by activating alternatively the register to be used for the synthesis.

## Library operations
The STM32ad9833 library offers a series of features for controlling the ad9833 module. Among them, the instantiation of the control object, the control of frequency, phase, waveform and output register. The added value of this library is the modality of access to the module which can be chosen between the standard arduino SPI and a dedicated driver, optimized for the writing speed. For example, for a bluepill 32F103 clocked at 72MHz, the standard SPI write speed (@36MHz SPI clock) for complete frequency change command requires 117 uS, the dedicated driver mode only 14 uS.

## pros
Such high update rates allow the development of user functions (eg arbitrary frequency sweeps) with higher resolution than using the standard Arduino SPI driver.

## cons
The use of the dedicated SPI library is not recommended for configurations where the serial line is occupied by several slaves with different SPI modes or clock frequencies. In this case, the library can still be used with the native arduino SPI driver, adopting transactions.

# Examples
Various examples are included among which, a more complex one that allows the rapid use of the module through a serial command line interpreter. With the latter it is possible to immediately start synthesizing waveforms and adjusting their parameters through command sent via the arduino standard serial line (serial monitor)

# Compile options
The use of the dedicated driver or the standard SPI driver is governed by a compile directive into the header file. By placing the #define USE_HAL_SPI at the begining of STM32ad9833.h file, the library will use the dedicated high-speed driver. Otherwise the arduino standard SPI driver will be used. In the latter case, the SPI1 serial port will always be used. With the use of the #define USE_HAL_GPIO directive, the library will use a high speed GPIO driver which allows to gain a few more micro seconds in the module calls. This last directive has no contraindications, it is advisable to keep it always active.

# Syntax
- STM32ad9833 dds(p)        generates the dds object of type STM32ds1833 using the p pin for the FSYNC signal of the chip. By default, SPI1 will be used and it will be initialized on creation. This Syntax can be used with or without USE_HAL_SPI defined 
- STM32ad9833 dds(p, s, i)  generates the dds object of type STM32ds1833 using the p pin for the FSYNC signal of the chip. s SPI will be used and it will be initialized on creation if i = true. Example: STM32ad9833 dds(PB9, SPI1, true);. This Syntax can be used ONLY with USE_HAL_SPI defined 
- STM32ad9833 dds(p, i)     generates the dds object of type STM32ds1833 using the p pin for the FSYNC signal of the chip. Always SPI1 will be used and it will be initialized on creation if i = true. Example: STM32ad9833 dds(PB9, true);. This Syntax can be used ONLY with USE_HAL_SPI undefined
- .begin(s, f, p, r)        initializes the chip and starts the synthesis using the waveform s (shape_t), at the frequency f (float) with the phase p (uint16_t) using the register r (uint_8). Phase p is expressed in tenths of a degree (e.g. 105 = 10.5 °), the frequency f in Hertz (e.g. 1000.5 = 1000.5 Hz). Shape s can be one of: AD_OFF, AD_SQUARE, AD_SINE, AD_TRIANGLE, AD_SQUARE2. When AD_SQUARE2 is selected, the output frequency will be half of that chosen with the parameter f, in the remaining cases the frequency will be exactly f. When called, this function sets both registers of the chip to the same frequency f and phase p, but uses the register specified in parameter r for synthesis.
- .setFrequency(f, r)       Set frequency f for register r. If, at the time of the call, the active register is the same as r, the frequency will be immediately synthesized when the function is executed.
- .setPhase(p, r)           Set phase p for register r. If, at the time of the call, the active register is the same as r, the phase will be immediately applied when the function is executed.
- .setShape(s)              Select the s waveform to synthesize. See .begin() for the available shapes and see further in this document to more detailed information on output signal characteristics.
- freqRegister(r)           Select register r for frequency synthesis. The frequency loaded into reg r (0 or 1) will be used for output. The difference between setting the frequency with the .setFrequency () method and that of setting distinct frequencies in the two registers and then selecting one with the .freqRegister () method lies in the fact that in the first case, it is necessary to write 48 bits, in the second only 16 In terms of execution time, for example with an SPI @ 36MHz, 1.5 uS are needed compared to the 14 required by the .setFrequency () call (in modes with dedicated SPI driver). This, for example, allows to obtain FSK modulations with much higher bitrates.
- .phaseRegister()          Select register r for phase synthesis. The observations made for the call to freqRegister () apply.

# Output signal characteristics
The output characteristics of the signal largely depend on the selected waveform and the required output frequency. For the square waveforms (AD_SQUARE and AD_SQUARE2) the nominal amplitude of the signal is approximately equal to VDD (supply voltage between 2.3 and 5.5V). For the other forms (AD_SINE, AD_TRIANGLE), the nominal amplitude is equal to 612 mV. But while in the case of the square shapes, the amplitude is almost constant throughout the frequency range, for the remaining shapes, the amplitude is strongly influenced by the frequency. In bench test cases, with a VDD = 5.1V and a sine output I detected 624mVpp @1KHz, 608mVpp @1MHz, 575mVpp @2MHz, 450mVpp @5MHz, 350mvpp @10MHz
