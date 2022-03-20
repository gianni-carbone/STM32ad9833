#include "STM32ad9833.h"

#ifndef USE_HAL_SPI	
#include <SPI.h>	
#endif

#define AD_MCLK   	25000000UL    	// Clock speed of the AD9833 reference clock in Hz


#define AD_MODE 	1      			// When MODE = 1, the SIN ROM is bypassed, resulting in a triangle output from the DAC. When MODE = 0, the SIN ROM is used which results in a sinusoidal signal at the output.
#define AD_DIV2 	3      			// When DIV2 = 1, the MSB of the DAC data is passed to the VOUT pin. When DIV2 = 0, the MSB/2 of the DAC data is output at the VOUT pin.
#define AD_OPBITEN  5   			// When OPBITEN = 1, the output of the DAC is no longer available at the VOUT pin, replaced by MSB (or MSB/2) of the DAC. When OPBITEN = 0, the DAC is connected to VOUT.
#define AD_SLEEP12  6   			// SLEEP12 = 1 powers down the on-chip DAC. SLEEP12 = 0 implies that the DAC is active.
#define AD_SLEEP1 	7    			// When SLEEP1 = 1, the internal MCLK clock is disabled, and the DAC output remains at its present value. When SLEEP1 = 0, MCLK is enabled.
#define AD_RESET	8     			// Reset = 1 resets internal registers to 0, which corresponds to an analog output of midscale. Reset = 0 disables reset.
#define AD_PSELECT  10  			// Defines whether the PHASE0 register or the PHASE1 register data is added to the output of the phase accumulator.
#define AD_FSELECT 	11  			// Defines whether the FREQ0 register or the FREQ1 register is used in the phase accumulator.
#define AD_B28 		13      		// B28 = 1 allows a complete word to be loaded into a frequency register in two consecutive writes. When B28 = 0, the 28-bit frequency register operates as two 14-bit registers.

#define AD_PHASE 	13	    		// Select the phase register
#define AD_FREQ0 	14    			// Select frequency 0 register
#define AD_FREQ1 	15    			// Select frequency 1 register

#define SEL_FREQ0  (1<<AD_FREQ0)
#define SEL_FREQ1  (1<<AD_FREQ1)
#define SEL_PHASE0 (1<<AD_FREQ0 | 1<<AD_FREQ1 | 0<<AD_PHASE)
#define SEL_PHASE1 (1<<AD_FREQ0 | 1<<AD_FREQ1 | 1<<AD_PHASE)

#define AD_2POW28 	(1UL << 28)   // Used when calculating output frequency

#ifdef USE_HAL_GPIO
#define sfy_set()   st.sfyGpioPort->ODR |= st.sfyGpioPin
#define sfy_reset() st.sfyGpioPort->ODR &= ~st.sfyGpioPin
#else
#define sfy_set()   digitalWrite(st.sfyPin, HIGH)
#define sfy_reset() digitalWrite(st.sfyPin, LOW)
#endif



#ifdef USE_HAL_SPI	
#define sfy_delay()		asm("NOP")	
#define spi_wait_rdy()	while(!(st.hspi.Instance->SR  & SPI_FLAG_TXE)); while ((st.hspi.Instance->SR  & SPI_FLAG_BSY))
#define spi_send(d)		sfy_reset(); sfy_delay(); st.hspi.Instance->DR = (d); spi_wait_rdy(); sfy_set()
#else
#define spi_send(d)		sfy_reset(); SPI.beginTransaction(SPISettings(36000000, MSBFIRST, SPI_MODE2)); SPI.transfer16((d)); SPI.endTransaction(); sfy_set()	
#endif


#ifdef USE_HAL_SPI		
STM32ad9833::STM32ad9833(uint32_t _sfy_pin, SPI_TypeDef* SPIinstance, bool initSPI)
#else
STM32ad9833::STM32ad9833(uint32_t _sfy_pin, bool initSPI)	
#endif
{
	st.sfyPin = _sfy_pin;
	
	if (initSPI) {
#ifdef USE_HAL_SPI		
		st.hspi.Instance = SPIinstance;	
		SPI_MspInit();
		SPI_Init();			// TODO: manage returns
#else
		SPI.begin(); 
#endif
	}
	
#ifdef USE_HAL_GPIO	
	PinName pn = digitalPinToPinName(st.sfyPin);
	st.sfyGpioPort = get_GPIO_Port(STM_PORT(pn));
	st.sfyGpioPin = STM_GPIO_PIN(pn);
	GPIO_Init();
#else
	pinMode(st.sfyPin, OUTPUT);
#endif
	reset(true);		// keeep chip resetted
}

uint32_t STM32ad9833::version(void) {
	return STM32ad9833_VER;
}

void STM32ad9833::masterClock(uint32_t _ck) {
	st.masterClock = _ck;
}

bool STM32ad9833::setFrequency(float freq, uint8_t reg) {
	// Bluepill @72MHz (SPI1: SPI_BAUDRATEPRESCALER_2)
	// takes aprox 14uS SPI@36 MHz with USE_HAL_SPI and USE_HAL_GPIO enabled
	// takes aprox 117uS SPI@36 MHz with USE_HAL_SPI and USE_HAL_GPIO disabled
	// takes aprox 112uS SPI@36 MHz with USE_HAL_SPI enabled and USE_HAL_GPIO disabled
	
	uint16_t  f_sel;

	if (freq>((float)st.masterClock/2.0)) return false;
	if (reg>1) return false;

	st.freq[reg] = freq;
	st.regFreq[reg] = (uint32_t)((freq * AD_2POW28/st.masterClock) + 0.5);
	
	switch (reg) {
		case 0:  f_sel = SEL_FREQ0; break;
		case 1:  f_sel = SEL_FREQ1; break;
	}
	
	spi_send(st.regCtl);   // set B28 to send both
	spi_send(f_sel | (uint16_t)(st.regFreq[reg] & 0x3fff));
	spi_send(f_sel | (uint16_t)((st.regFreq[reg] >> 14) & 0x3fff));
	
	return true;
}

bool STM32ad9833::setPhase(uint16_t phase, uint8_t reg) {
	uint16_t  ph_sel;

	if (reg>1) return false;
	// TODO: check on phase value?

	st.phase[reg] = phase;
	st.regPhase[reg] = (uint16_t)((512.0 * (phase/10) / 45) + 0.5);

	switch (reg) {
		case 0:  ph_sel = SEL_PHASE0; break;
		case 1:  ph_sel = SEL_PHASE1; break;
	}

	spi_send(ph_sel | (0xfff & st.regPhase[reg]));

	return true;
}


void STM32ad9833::setShape(shape_t _shape){
 
	switch (_shape) {
	case AD_OFF:
	  bitClear(st.regCtl, AD_OPBITEN);
	  bitClear(st.regCtl, AD_MODE);
	  bitSet(st.regCtl, AD_SLEEP1);
	  bitSet(st.regCtl, AD_SLEEP12);
	break;
	case AD_SINE:
	  bitClear(st.regCtl, AD_OPBITEN);
	  bitClear(st.regCtl, AD_MODE);
	  bitClear(st.regCtl, AD_SLEEP1);
	  bitClear(st.regCtl, AD_SLEEP12);
	break;
	case AD_SQUARE:
	  bitSet(st.regCtl, AD_OPBITEN);
	  bitClear(st.regCtl, AD_MODE);
	  bitSet(st.regCtl, AD_DIV2);
	  bitClear(st.regCtl, AD_SLEEP1);
	  bitClear(st.regCtl, AD_SLEEP12);
	break;
	case AD_SQUARE2:
	  bitSet(st.regCtl, AD_OPBITEN);
	  bitClear(st.regCtl, AD_MODE);
	  bitClear(st.regCtl, AD_DIV2);
	  bitClear(st.regCtl, AD_SLEEP1);
	  bitClear(st.regCtl, AD_SLEEP12);
	break;
	case AD_TRIANGLE:
	  bitClear(st.regCtl, AD_OPBITEN);
	  bitSet(st.regCtl, AD_MODE);
	  bitClear(st.regCtl, AD_SLEEP1);
	  bitClear(st.regCtl, AD_SLEEP12);
	break;
	}

	spi_send(st.regCtl);
	st.shape = _shape;
}

bool STM32ad9833::freqRegister(uint8_t reg) {
	if (reg>1) return false;

	switch (reg) {
		case 0: bitClear(st.regCtl, AD_FSELECT); break;
		case 1: bitSet(st.regCtl, AD_FSELECT);   break;
	}
	spi_send(st.regCtl);
	st.freqRegister =  reg;
	return true;
}

bool STM32ad9833::phaseRegister(uint8_t reg){
	if (reg>1) return false;
	switch (reg) {
		case 0: bitClear(st.regCtl, AD_PSELECT); break;
		case 1: bitSet(st.regCtl, AD_PSELECT);   break;
	}
	spi_send(st.regCtl);
	st.phaseRegister = reg;
	return true;
}


void STM32ad9833::reset(bool _hold) {
	bitSet(st.regCtl, AD_RESET);
	spi_send(st.regCtl);
	if (!_hold) {
		bitClear(st.regCtl, AD_RESET);
		spi_send(st.regCtl);
	}
}

bool STM32ad9833::begin(shape_t shape, float freq, uint16_t phase, uint8_t reg) {
	sfy_set();
	
	bitSet(st.regCtl, AD_B28); 
	spi_send(st.regCtl);	
	reset(true);
	if (!setFrequency(freq, 0)) return false;	
	if (!setFrequency(freq, 1)) return false;	
	if (!setPhase(phase, 0)) return false;
	if (!setPhase(phase, 1)) return false;
	setShape(shape);
	freqRegister(reg);
	phaseRegister(reg);
	reset();                  				// full transition	
	return true;
}

// Private
#ifdef USE_HAL_SPI	

bool STM32ad9833::SPI_MspInit(){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	
	// assuming PinMap_SPI_MISO, PinMap_SPI_MOSI and PinMap_SPI_SCLK same size
	uint8_t i=0;
	while (PinMap_SPI_MISO[i].pin != NC) {
		
		if (st.hspi.Instance == PinMap_SPI_MISO[i].peripheral) {
			GPIO_InitStruct.Pin = STM_GPIO_PIN(PinMap_SPI_MISO[i].pin);
			HAL_GPIO_Init(get_GPIO_Port(STM_PORT(PinMap_SPI_MISO[i].pin)), &GPIO_InitStruct);
			set_GPIO_Port_Clock(STM_PORT(PinMap_SPI_MISO[i].pin));
			GPIO_InitStruct.Pin = STM_GPIO_PIN(PinMap_SPI_MOSI[i].pin);
			HAL_GPIO_Init(get_GPIO_Port(STM_PORT(PinMap_SPI_MOSI[i].pin)), &GPIO_InitStruct);
			set_GPIO_Port_Clock(STM_PORT(PinMap_SPI_MOSI[i].pin));
			GPIO_InitStruct.Pin = STM_GPIO_PIN(PinMap_SPI_SCLK[i].pin);
			HAL_GPIO_Init(get_GPIO_Port(STM_PORT(PinMap_SPI_SCLK[i].pin)), &GPIO_InitStruct);
			set_GPIO_Port_Clock(STM_PORT(PinMap_SPI_SCLK[i].pin));
		}
		i++;
	}

	return true;
}


bool STM32ad9833::SPI_Init(void){
	// enable clock here is mandatory
#ifdef SPI1	
	if(st.hspi.Instance == SPI1) __HAL_RCC_SPI1_CLK_ENABLE();
#endif
#ifdef SPI2	
	if(st.hspi.Instance == SPI2) __HAL_RCC_SPI2_CLK_ENABLE();
#endif
#ifdef SPI3	
	if(st.hspi.Instance == SPI3) __HAL_RCC_SPI3_CLK_ENABLE();
#endif
#ifdef SPI4	
	if(st.hspi.Instance == SPI4) __HAL_RCC_SPI4_CLK_ENABLE();
#endif

	st.hspi.Init.Mode = SPI_MODE_MASTER;
	st.hspi.Init.Direction = SPI_DIRECTION_2LINES;
	st.hspi.Init.CLKPhase = SPI_PHASE_1EDGE;					// SPI MODE2
	st.hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;				// SPI MODE2
	st.hspi.Init.DataSize = SPI_DATASIZE_16BIT;					// always write 16 bit of data
	st.hspi.Init.NSS = SPI_NSS_SOFT;
	st.hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;	// SPI1: prescaler = 2 tested with STM32F103@72MHz
	st.hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	st.hspi.Init.TIMode = SPI_TIMODE_DISABLE;
	st.hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	st.hspi.Init.CRCPolynomial = 7;

	bool ok = (HAL_SPI_Init(&st.hspi) == HAL_OK);
	
	if (ok) 
		__HAL_SPI_ENABLE(&st.hspi);                        // MANDATORY

  return ok;	
}
#endif

#ifdef USE_HAL_GPIO
void STM32ad9833::GPIO_Init(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// enable GPIO port clock for sfyPin
#ifdef GPIOA
	if (st.sfyGpioPort == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
#endif
#ifdef GPIOB
	if (st.sfyGpioPort == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
#endif
#ifdef GPIOC
	if (st.sfyGpioPort == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
#endif
#ifdef GPIOD
	if (st.sfyGpioPort == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
#endif
#ifdef GPIOE
	if (st.sfyGpioPort == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
#endif


	HAL_GPIO_WritePin(st.sfyGpioPort, st.sfyGpioPin, GPIO_PIN_RESET);

	GPIO_InitStruct.Pin = st.sfyGpioPin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(st.sfyGpioPort, &GPIO_InitStruct);
}
#endif