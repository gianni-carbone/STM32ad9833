# STM32ad9833

## Description 
Arduino STM32 library for Analog Device ad9833 module. This library simplifies the use of dds analog devices ad9833 modules. It also allowed for high module update rates compared to using the standard SPI library.

## AD9833 modules
The AD9833 is a programmable waveform generator capable of producing sine, triangular, and square wave outputs. The output frequency and phase are software programmable. With a 25 MHz clock rate, resolution of 0.1 Hz can be achieved. The AD9833 is written to via a 3-wire serial interface. This serial interface operates at clock rates up to 40 MHz. The device operates with a power supply from 2.3 V to 5.5 V.

## Library operations
The STM32ad9833 library offers a series of features for controlling the ad9833 module. Among them, the instantiation of the control object, the control of frequency, phase, waveform and output channel. The added value of this library is the modality of access to the module which can be chosen between the standard arduino SPI and a dedicated driver, optimized for the writing speed. For example, for a bluepill 32F103 card clocked at 72MHz, the maximum SPI write speed (@32MHz SPI clock) for complete frequency change command requires 117 uS, the dedicated driver mode only 14 uS.

## pros
Such high update rates allow the development of user functions (eg arbitrary frequency sweeps) with higher resolution than using the standard Arduino SPI driver.

## cons
The use of the dedicated SPI library is not recommended for configurations where the serial is occupied by several slaves with different SPI modes or clock frequencies. In this case, the library can still be used with the native arduino SPI driver, adopting transactions.
