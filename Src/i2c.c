
#include "i2c.h"

#include "gpio.h"

#define I2C_TX_MODE		0
#define I2C_RX_MODE		1

#define I2C_GEN_ADDRESS(ADR, MODE) ((ADR << 1) | MODE)


#define I2C_BUS_ERROR	(I2C1->SR1 & I2C_SR1_BERR)		//Set by hardware when the interface detects an SDA rising or
														//falling edge while SCL is high, occurring in a non-valid 
														//position during a byte transfer
														
#define I2C_TIMEOUT_ERR	(I2C1->SR1 & I2C_SR1_TIMEOUT)	//SCL remained low for 25ms
#define I2C_PEC_ERROR	(I2C1->SR1 & I2C_SR1_PECERR)	//Packet Error Correction 
#define I2C_OVR			(I2C1->SR1 & I2C_SR1_OVR)		//Overrun or underrun
#define I2C_ACK_FAIL	(I2C1->SR1 & I2C_SR1_AF)		//Acknowledge failure
#define I2C_ARLO		(I2C1->SR1 & I2C_SR1_ARLO)		//Arbitration lost

#define I2C_ERROR_CHECK	(I2C_TIMEOUT_ERR | I2C_PEC_ERROR | I2C_OVR | I2C_ACK_FAIL | I2C_ARLO) //Return 0 if errors not detected

#define I2C_MODE		(I2C1->SR2 & I2C_SR2_TRA)		//0 - Receive Mode ; 1 - Transmit Mode  
#define I2C_BUS_BUSY	(I2C1->SR2 & I2C_SR2_BUSY)		//0 - No communication on the bus ; 1 - Communication ongoing on the bus
#define I2C_MSL			(I2C1->SR2 & I2C_SR2_MSL)		//0 - Slave Mode ; 1 - Master mode

#define I2C_TIMEOUT		50000


uint32_t i2c_timeout = 0;

/* I2C1 init function */
void MX_I2C1_Init(void)
{
	LL_I2C_InitTypeDef I2C_InitStruct;

	LL_GPIO_InitTypeDef GPIO_InitStruct;

	/**I2C1 GPIO Configuration  
	PB6   ------> I2C1_SCL
	PB7   ------> I2C1_SDA 
	*/
	GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* Peripheral clock enable */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

	/* I2C1 interrupt Init */
	NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
	NVIC_EnableIRQ(I2C1_EV_IRQn);

	/**I2C Initialization 
	*/
	LL_I2C_DisableOwnAddress2(I2C1);

	LL_I2C_DisableGeneralCall(I2C1);

	LL_I2C_EnableClockStretching(I2C1);

	I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
	I2C_InitStruct.ClockSpeed = 400000;
	I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;
	I2C_InitStruct.OwnAddress1 = 0;
	I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
	I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
	LL_I2C_Init(I2C1, &I2C_InitStruct);

	LL_I2C_SetOwnAddress2(I2C1, 0);

}

ErrorStatus I2C_Send(uint8_t data, uint8_t address){
	
	LL_I2C_DisableBitPOS(I2C1);
	LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
	
	LL_I2C_GenerateStartCondition(I2C1);
	while(!LL_I2C_IsActiveFlag_SB(I2C1)){};
	(void) I2C1->SR1;
		
	LL_I2C_TransmitData8(I2C1, I2C_GEN_ADDRESS(address, I2C_TX_MODE));
		
	while(!LL_I2C_IsActiveFlag_ADDR(I2C1)){};
	LL_I2C_ClearFlag_ADDR(I2C1);
		
	LL_I2C_TransmitData8(I2C1, data);
	while(!LL_I2C_IsActiveFlag_TXE(I2C1)){};
		
	LL_I2C_GenerateStopCondition(I2C1);

	return SUCCESS;
}

