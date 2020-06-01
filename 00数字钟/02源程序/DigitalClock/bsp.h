/*
 * bsp.h
 *
 *  Created on: 2020年1月8日
 *      Author: 白鸟无言
 */

#ifndef BSP_H_
#define BSP_H_

void set_freq_4MHz(void);

void GPIO_Init(void);

void Uart_Init(void);

void TimerA0_Init(void);

void TimerB0_Init(void);

#endif /* BSP_H_ */
