#ifndef __GPS_H
#define __GPS_H

#include "stm32f10x.h"

// UTC�¼�
typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
}utc_time_s;


typedef struct
{
	utc_time_s utc;					//UTCʱ��
	uint32_t latitude;				//γ�� ������100000��,ʵ��Ҫ����100000
	uint8_t nshemi;					//��γ/��γ,N:��γ;S:��γ				  
	uint32_t longitude;			    //���� ������100000��,ʵ��Ҫ����100000
	uint8_t ewhemi;					//����/����,E:����;W:����

	uint32_t altitude;				//���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m	 
	uint16_t speed;					//��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ	 
}gps_msg_s;


void GPS_Analysis(gps_msg_s *g_gpsx,uint8_t *buf);



#endif /* __GPS_H */

