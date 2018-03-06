#ifndef __GPRS_H
#define __GPRS_H

#include "stm32f10x.h"
#include "bsp.h"
#include "gps.h"


extern gps_msg_s g_gpsx;


uint8_t* gprs_check_cmd(uint8_t *str);
uint8_t gprs_send_cmd(uint8_t *cmd,uint8_t *ack,uint16_t waittime);
uint8_t gprs_word_test(void);
uint8_t gprs_connet_server(uint8_t *IP_ADD,uint8_t *COM);
uint8_t gprs_send_data(uint8_t *DATA);
void gprs_test(void);

void Gps_Msg_Show(void);


#endif /* __GPRS_H */


