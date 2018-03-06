#include "gps.h"
#include <string.h>
#include <stdio.h>
#include "bsp.h"


/*
* 函数名称：NMEA_Comma_Pos()
* 函数功能：查找buf中的第cx个逗号，返回他的偏移量
* 函数参数：
*          buf：要查找的字符串
*          cx： 第几个逗号
* 返回值：    逗号所在字符串buf的偏移量
*/
uint8_t NMEA_Comma_Pos(uint8_t *buf, uint8_t cx)
{
	uint8_t *p = buf;

	while(cx != 0)
	{
		if(*buf==' ' || *buf=='*' || *buf>'z')
			return 0xff;
		if(*buf==',')
			cx--;
		buf++;        
	}
	return buf-p;
}


/*
* 函数名称：NMEA_Pow()
* 函数功能：求m^n
* 函数参数：
*          m：基数
*          n： 指数
* 返回值：    计算值:m^n
*		  -1: 参数错误
*/
uint32_t NMEA_Pow(uint8_t m,uint8_t n) 
{
	uint32_t result=1;

	while(n--)
	{
		result *= m;
	}
	
	return result;
}


/*
* 函数名称：NMEA_Str2num()
* 函数功能：字符串转数字
* 函数参数：
*         buf ：待处理的字符串
*         dx： 输出型参数，小数点的位数
* 返回值：   转换后的数值 
*/
uint32_t NMEA_Str2num(uint8_t *buf,uint8_t* dx)
{
	uint8_t *p=buf;
	
	uint8_t ilen=0, flen=0;
	uint32_t ires=0, fres=0;
	uint8_t mask=0,i;
	uint32_t res=0;
	
	// 先判断数值是否是负数
	if(*p == '-')
	{
		mask |= 0x01;
		p++;
	}
	// 遍历字符串
	while(1)
	{
	
		// 判断是否是结束符
		if(*p=='*' || *p==',' ) break;
		
		// debug("%c ", *p);
		
		// 判断是否是小数
		if(*p=='.')
		{
			mask |= 0x02;
			p++;
		}
		
		// 判断是否是非法字符
		if(*p>'9' || *p<'0')
		{
			printf("*p = %c\n",*p);
			return 0;
		}

		// 长度计数值处理
		if(mask & 0x02)
		{
			flen++;
			if(flen>6)
				return 0;
		}
		else
		{
			ilen++;
			if(ilen>6)
				return 0;
		}
		
		p++;
		
	}
	// debug("\r\n");
	
	p = buf;
	
	// 负数处理
	if(mask & 0x01) p++;
	
	// 整数部分处理  
	for(i=0; i<ilen; i++)
	{
		ires += NMEA_Pow(10, ilen-1-i) * (p[i]-'0');
	}
	
	// 小数部分处理
	if(flen>5) 
		flen=5;
	*dx = flen;
	for(i=0; i<flen; i++)
	{
		fres += NMEA_Pow(10, flen-1-i) * (p[ilen+1+i]-'0');
	}
	// debug("str2num()_fres = %d\r\n",fres);
	
	// 负数，整数，小数整合
	res = ires * NMEA_Pow(10, flen) + fres;
	if(mask & 0x01) res = -res;
	
	// debug("str2num()_res = %d\r\n",res);
	// debug("str2num()_dx = %d \r\n",*dx);
	
	return res;
}


/*
* 函数名称： GPS_Analysis()
* 函数功能： 解析gps数据
* 函数参数：
*         g_gpsx ：gps的数据包
*         buf： 串口接收的数据
* 返回值：   none
*/
void GPS_Analysis(gps_msg_s *g_gpsx,uint8_t *buf)
{
	uint32_t temp;     // 临时操作数
	char *pt;
	uint8_t *p, dx=0;    // 字符串操作以及小数点
	uint8_t posx;
	float rs;

	pt = strstr((const char *)buf, "GNRMC");
	p = (uint8_t *)pt;
	posx = NMEA_Comma_Pos(p, 1);
	
	if(posx != 0xff)
	{
		temp = NMEA_Str2num((p+posx), &dx) / NMEA_Pow(10, dx);
		
		g_gpsx->utc.hour = (uint8_t)(temp / NMEA_Pow(10, 4));
		g_gpsx->utc.min = temp / NMEA_Pow(10, 2) % NMEA_Pow(10, 2);
		g_gpsx->utc.sec = temp % NMEA_Pow(10, 2);
		
	}
	// 纬度：ddmm.mmmmm
	posx = NMEA_Comma_Pos(p, 3);
	if(posx != 0xff)
	{
		temp = NMEA_Str2num(p+posx, &dx);
		// debug("jingdu | temp = %d; dx = %d \r\n", temp,dx);
		
		g_gpsx->latitude = temp / NMEA_Pow(10, dx+2);  // 得到度数
		rs = temp % NMEA_Pow(10, dx+2);               // 得到分数
		// debug("jingdu | dushu = %d; fenshu = %f \r\n", g_gpsx->latitude,rs);
		
		g_gpsx->latitude = g_gpsx->latitude*NMEA_Pow(10, dx) + rs/60;
		// debug("g_gpsx->latitude = %d \r\n", g_gpsx->latitude);
		// debug("\r\n");
	}

	posx = NMEA_Comma_Pos(p, 4);
	if(posx != 0xff)
	{
		g_gpsx->nshemi = *(p+posx);
	}

	// 经度：dddmm.mmmmm
	posx = NMEA_Comma_Pos(p, 5);
	if(posx != 0xff)
	{
		temp = NMEA_Str2num(p+posx, &dx);
		g_gpsx->longitude = temp / NMEA_Pow(10, dx+2);
		rs = temp % NMEA_Pow(10, dx+2);
		g_gpsx->longitude = g_gpsx->longitude*NMEA_Pow(10, dx) + rs/60;
	}

	posx = NMEA_Comma_Pos(p, 6);
	if(posx != 0xff)
	{
		g_gpsx->ewhemi = *(p+posx);
	}

	posx=NMEA_Comma_Pos(p,9);								//得到UTC日期
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p+posx,&dx);		 				//得到UTC日期
		g_gpsx->utc.day=temp/10000;
		g_gpsx->utc.month=(temp/100)%100;
		g_gpsx->utc.year=2000+temp%100;	 	 
	} 
}



// SkyTraq 协议配置文件
uint8_t SkyTra_Cfg_Prt(uint8_t baud_id)
{
	uint8_t i = 0;
	i=i;
	return i;
}









