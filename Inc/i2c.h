
#ifndef __i2c_H
#define __i2c_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_ll_i2c.h"
#include "main.h"

extern void _Error_Handler(char *, int);

void MX_I2C1_Init(void);
ErrorStatus I2C_Send(uint8_t data, uint8_t address);


#ifdef __cplusplus
}
#endif
#endif /*__ i2c_H */
