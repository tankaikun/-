#ifndef __GPS_H
#define __GPS_H

#include "stm32f10x.h"

// UTC事件
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
	utc_time_s utc;					//UTC时间
	uint32_t latitude;				//纬度 分扩大100000倍,实际要除以100000
	uint8_t nshemi;					//北纬/南纬,N:北纬;S:南纬				  
	uint32_t longitude;			    //经度 分扩大100000倍,实际要除以100000
	uint8_t ewhemi;					//东经/西经,E:东经;W:西经

	uint32_t altitude;				//海拔高度,放大了10倍,实际除以10.单位:0.1m	 
	uint16_t speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时	 
}gps_msg_s;


void GPS_Analysis(gps_msg_s *g_gpsx,uint8_t *buf);



#endif /* __GPS_H */

