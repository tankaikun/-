#include "gps.h"
#include <string.h>
#include <stdio.h>
#include "bsp.h"


/*
* �������ƣ�NMEA_Comma_Pos()
* �������ܣ�����buf�еĵ�cx�����ţ���������ƫ����
* ����������
*          buf��Ҫ���ҵ��ַ���
*          cx�� �ڼ�������
* ����ֵ��    ���������ַ���buf��ƫ����
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
* �������ƣ�NMEA_Pow()
* �������ܣ���m^n
* ����������
*          m������
*          n�� ָ��
* ����ֵ��    ����ֵ:m^n
*		  -1: ��������
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
* �������ƣ�NMEA_Str2num()
* �������ܣ��ַ���ת����
* ����������
*         buf ����������ַ���
*         dx�� ����Ͳ�����С�����λ��
* ����ֵ��   ת�������ֵ 
*/
uint32_t NMEA_Str2num(uint8_t *buf,uint8_t* dx)
{
	uint8_t *p=buf;
	
	uint8_t ilen=0, flen=0;
	uint32_t ires=0, fres=0;
	uint8_t mask=0,i;
	uint32_t res=0;
	
	// ���ж���ֵ�Ƿ��Ǹ���
	if(*p == '-')
	{
		mask |= 0x01;
		p++;
	}
	// �����ַ���
	while(1)
	{
	
		// �ж��Ƿ��ǽ�����
		if(*p=='*' || *p==',' ) break;
		
		// debug("%c ", *p);
		
		// �ж��Ƿ���С��
		if(*p=='.')
		{
			mask |= 0x02;
			p++;
		}
		
		// �ж��Ƿ��ǷǷ��ַ�
		if(*p>'9' || *p<'0')
		{
			printf("*p = %c\n",*p);
			return 0;
		}

		// ���ȼ���ֵ����
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
	
	// ��������
	if(mask & 0x01) p++;
	
	// �������ִ���  
	for(i=0; i<ilen; i++)
	{
		ires += NMEA_Pow(10, ilen-1-i) * (p[i]-'0');
	}
	
	// С�����ִ���
	if(flen>5) 
		flen=5;
	*dx = flen;
	for(i=0; i<flen; i++)
	{
		fres += NMEA_Pow(10, flen-1-i) * (p[ilen+1+i]-'0');
	}
	// debug("str2num()_fres = %d\r\n",fres);
	
	// ������������С������
	res = ires * NMEA_Pow(10, flen) + fres;
	if(mask & 0x01) res = -res;
	
	// debug("str2num()_res = %d\r\n",res);
	// debug("str2num()_dx = %d \r\n",*dx);
	
	return res;
}


/*
* �������ƣ� GPS_Analysis()
* �������ܣ� ����gps����
* ����������
*         g_gpsx ��gps�����ݰ�
*         buf�� ���ڽ��յ�����
* ����ֵ��   none
*/
void GPS_Analysis(gps_msg_s *g_gpsx,uint8_t *buf)
{
	uint32_t temp;     // ��ʱ������
	char *pt;
	uint8_t *p, dx=0;    // �ַ��������Լ�С����
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
	// γ�ȣ�ddmm.mmmmm
	posx = NMEA_Comma_Pos(p, 3);
	if(posx != 0xff)
	{
		temp = NMEA_Str2num(p+posx, &dx);
		// debug("jingdu | temp = %d; dx = %d \r\n", temp,dx);
		
		g_gpsx->latitude = temp / NMEA_Pow(10, dx+2);  // �õ�����
		rs = temp % NMEA_Pow(10, dx+2);               // �õ�����
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

	// ���ȣ�dddmm.mmmmm
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

	posx=NMEA_Comma_Pos(p,9);								//�õ�UTC����
	if(posx!=0XFF)
	{
		temp=NMEA_Str2num(p+posx,&dx);		 				//�õ�UTC����
		g_gpsx->utc.day=temp/10000;
		g_gpsx->utc.month=(temp/100)%100;
		g_gpsx->utc.year=2000+temp%100;	 	 
	} 
}



// SkyTraq Э�������ļ�
uint8_t SkyTra_Cfg_Prt(uint8_t baud_id)
{
	uint8_t i = 0;
	i=i;
	return i;
}









