/**
 *******************************************************************************************************************
 *  @file AD7705.h
 *  @brief Библиотека для работы с 16 битным сигма-дельта АЦП с встроенной цифровой фильтрацией AD7705/MS7705/TM7705
 *  @author Волков Олег
 *  @date 23.04.2024
 *
 *******************************************************************************************************************
 * @attention
 *
 *  AD7705 имеет 2 полностью дифференциальных входа АЦП
 *  Нелинейность 0.003%
 *  Программное усиление от 1 до 128
 *  Имеется буфер аналогового входа
 *  Есть фильтры. Советую использовать только 50 Гц
 *  Питание от 2.7В до 3.3В, либо от 4.75В до 5.25В (По ощущениям, будто от 5В шума меньше).
 *  Также заметил, что при сильном изменении питающего напряжения могут уплывать значения после калибровки,
 *  не смотря на стабильный Vref.
 *  Поэтому питание VDD должно быть стабильным.
 *  Для нормальной работы АЦП требуется кварцевый резонатор 4.9512 МГц, либо 2.4576 МГц.
 *  Нагрузочные конденсаторы я ставил 30pF.
 *
 *  Общение по SPI:
 *  				CS (NSS) в активном режиме подтянут к земле
 *  				CPOL = 1
 *  				CPHA = 1
 *  				Работа с данными осуществляется в 8-битном формате.
 *
 *  YouTube: https://www.youtube.com/channel/UCzZKTNVpcMSALU57G1THoVw
 *  GitHub: https://github.com/Solderingironspb
 *  ВК: https://vk.com/solderingiron.stm32
 *  Документация: https://github.com/Solderingironspb/AD7705/blob/main/AD7705_datasheet.pdf

   Псевдокод для примера:
    #include "main.h"
    #include "stm32f103xx_CMSIS.h"
    #include <stdbool.h>
    #include "AD7705.h"
    uint16_t ADC_val = 0;
   	int main(void) {
	CMSIS_Debug_init(); //Настройка дебага
	CMSIS_RCC_SystemClock_72MHz(); //Настройка МК на 72 МГц
	CMSIS_SysTick_Timer_init(); //Настройка системного таймера
	CMSIS_GPIO_init(GPIOA, 4, GPIO_GENERAL_PURPOSE_OUTPUT, GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_50_MHZ); //CS
	CMSIS_GPIO_init(GPIOA, 3, GPIO_GENERAL_PURPOSE_OUTPUT, GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_50_MHZ); //RESET
	CMSIS_GPIO_init(GPIOA, 2, GPIO_INPUT, GPIO_INPUT_FLOATING, GPIO_SPEED_RESERVED); //DRDY
	CMSIS_SPI1_init(); //Настройка SPI 18 MBits/s CPOL = 1 CPHA = 1
	Delay_ms(100);
	AD7705_Init(SPI1, AD7705_ADC_CHANNEL_1, AD7705_CRYSTAL_2_4576, AD7705_BUFFER_ON, AD7705_UNIPOLAR_MODE, AD7705_GAIN_16, AD7705_MD_SELF_CALIBRATION); //Настроим 1 канал перед опросом
	while (1) {
		AD7705_Wait_DRDY(); //Дождемся, пока DRDY не упадет в 0
		ADC_val = AD7705_ReadData(SPI1, AD7705_ADC_CHANNEL_1); //Опросим 1 канал
	}
}
 *******************************************************************************************************************
 */

#ifndef INC_AD7705_H_
#define INC_AD7705_H_

#include "main.h"
#include "stm32f103xx_CMSIS.h"
#include <stdbool.h>

/*-------------------------------------------------МАКРОСЫ-------------------------------------------------------*/
/*Настройка ножек МК*/
#define AD7705_CS_PORT 						GPIOA
#define AD7705_CS_PIN  						4

#define AD7705_RESET_PORT 					GPIOA
#define AD7705_RESET_PIN  					3

#define AD7705_DRDY_PORT					GPIOA
#define AD7705_DRDY_PIN						2

/*CMSIS*/
#define AD7705_CS_ON						AD7705_CS_PORT->BSRR = (1U << (AD7705_CS_PIN + 16U))
#define AD7705_CS_OFF						AD7705_CS_PORT->BSRR = (1U << AD7705_CS_PIN)
#define AD7705_RESET_ON						AD7705_RESET_PORT->BSRR = (1U << (AD7705_RESET_PIN + 16U))
#define AD7705_RESET_OFF					AD7705_RESET_PORT->BSRR = (1U << AD7705_RESET_PIN)
#define AD7705_CHECK_DRDY					READ_BIT(AD7705_DRDY_PORT->IDR, 1U << AD7705_DRDY_PIN)

/*Относится непосредственно к настройкам АЦП*/
#define AD7705_ADC_CHANNEL_1				0x0
#define AD7705_ADC_CHANNEL_2				0x1
#define AD7705_CRYSTAL_4_9512				0xC
#define AD7705_CRYSTAL_2_4576				0x4
#define AD7705_BUFFER_ON 					0x2
#define AD7705_BUFFER_OFF 					0x0
#define AD7705_BIPOLAR_MODE					0x0
#define AD7705_UNIPOLAR_MODE				0x4
#define AD7705_GAIN_1						0x0
#define AD7705_GAIN_2						0x8
#define AD7705_GAIN_4						0x10
#define AD7705_GAIN_8						0x16
#define AD7705_GAIN_16						0x20
#define AD7705_GAIN_32						0x28
#define AD7705_GAIN_64						0x30
#define AD7705_GAIN_128						0x38
#define AD7705_MD_NORMAL_MODE				0x00
#define AD7705_MD_SELF_CALIBRATION			0x40
#define AD7705_MD_ZERO_SCALE_CALIBRATION	0x80
#define AD7705_MD_FULL_SCALE_CALIBRATION	0xC0
/*-------------------------------------------------МАКРОСЫ-------------------------------------------------------*/



void AD7705_Init(SPI_TypeDef* SPI, uint8_t Channel, uint8_t Crystal, uint8_t Buffer_on_off, uint8_t Bipolar_Unipolar, uint8_t Gain, uint8_t Calibration);
uint16_t AD7705_ReadData(SPI_TypeDef* SPI, uint8_t Channel);
void AD7705_Wait_DRDY(void);



#endif /* INC_AD7705_H_ */
