/*
 * flash.h
 *
 *  Created on: 2020��1��8��
 *      Author: ��������
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

void write_Seg(uint8_t* ptr);
void param_save(void);
void param_read(void);

#endif /* FLASH_H_ */
