#ifndef _ONENET_H_
#define _ONENET_H_

#include "stm32f10x.h"

extern float lon;
extern float lat;

void OneNet_SendData(void);

void OneNet_RevPro(unsigned char *dataPtr);

#endif
