#ifndef STM32ad9833_h
#define STM32ad9833_h

#define USE_HAL_GPIO  // enable the fast SFY pin toggle. Suggestion: stay it enabled
#define USE_HAL_SPI   // enable the fastest SPI write. 
                      // Suggestion: if the spi interface is for the exclusive use of the module then stay it enabled.
                      // If you have to share the spi interface with other slaves with different settings than MODE2 @MCLK/2
                      // then comment the line and use spi transactions.
#include <Arduino.h>
#include "stm32yyxx_hal_conf.h"

#define STM32ad9833_VER	902


#define AD_MCLK   	25000000UL    // default clock speed of the AD9833 reference clock in Hz
#define AD_DEF_FREQ	1000.0
#define AD_DEF_PHAS	0
#define AD_DEF_SHAP	AD_SINE
#define AD_DEF_CHAN	0

#ifndef USE_HAL_GPIO
#pragma message("HAL_GPIO is not defined. The performance is not optimal")
#endif

#ifndef USE_HAL_SPI
#pragma message("HAL_SPI is not defined. The performance is not optimal")
#endif

#if !defined(STM32F1xx) && !defined(STM32F4xx)
#pragma message("This code is tested for STM32F1xx and STM32F4xx series and could not work with current board")
#endif	

enum shape_t {
	AD_OFF,
	AD_SQUARE,
	AD_SINE,
	AD_TRIANGLE,
	AD_SQUARE2
};

typedef struct {
	uint32_t		masterClock = AD_MCLK;
	uint32_t		sfyPin = 0;

	uint16_t  		regCtl = 0;     	// control register image	
	uint32_t  		regFreq[2];     	// frequency registers
	uint32_t  		regPhase[2]; 		// phase registers
	
	float     		freq[2];     		// last frequencies set
	uint16_t  		phase[2];       	// last phase setting
	shape_t			shape;				// last shape setting

	uint8_t			freqRegister;		// last active frequency register
	uint8_t			phaseRegister;		// last active phase register

#ifdef USE_HAL_SPI	
	SPI_HandleTypeDef	hspi;
#endif
	
#ifdef USE_HAL_GPIO	
	GPIO_TypeDef*	sfyGpioPort = {0};
	uint16_t		sfyGpioPin = 0;
#endif

} ad9833_status_t;

class STM32ad9833 {
	
	public:
#ifdef USE_HAL_SPI		
	STM32ad9833(uint32_t _sfy_pin, SPI_TypeDef *SPIinstance = SPI1, bool initSPI = true);
#else
	STM32ad9833(uint32_t _sfy_pin, bool initSPI = true);
#endif
	uint32_t	version();
	void 		masterClock(uint32_t _ck);
	void 		reset(bool _hold = false);
	bool 		begin(shape_t shape = AD_DEF_SHAP, float freq = AD_DEF_FREQ, uint16_t phase = AD_DEF_PHAS, uint8_t reg = AD_DEF_CHAN);
	bool 		setFrequency(float freq, uint8_t reg = 0);
	bool 		setPhase(uint16_t phase, uint8_t reg = 0);
	void 		setShape(shape_t _shape);
	bool 		freqRegister(uint8_t reg);
	bool 		phaseRegister(uint8_t reg);
	
	private:
	ad9833_status_t st;
#ifdef USE_HAL_SPI
	bool SPI_MspInit(void);
	bool SPI_Init(void);
#endif
#ifdef USE_HAL_GPIO
	void 			GPIO_Init(void);
#endif	
};

#endif