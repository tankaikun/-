//�����豸
#include "gprs.h"	
//Э���ļ�
#include "onenet.h"
//Ӳ������
#include "usart.h"
#include "includes.h"
//C��
#include <string.h>
#include <stdio.h>


#define DEVID	"23370018"
#define APIKEY	"s=Zna3ID6kI8=hQMmpHJAfN1VOA="


// �ٶ��ľ�γ��
float lon = 0;
float lat = 0;

void OneNet_FillBuf(char *buf)
{
	
	char text[64];
	char buf1[128];

	// ���text��buf1�ڴ�ռ�
	memset(text, 0, sizeof(text));
	memset(buf1, 0, sizeof(buf1));
	
	strcpy(buf1, "{");
	
	memset(text, 0, sizeof(text));
	sprintf(text, "\"GPS\":{\"lon\":%f,\"lat\":%f}", lon, lat);
	strcat(buf1, text);
	
	strcat(buf1, "}");

	// buf��HTTPЭ�����
	sprintf(buf, "POST /devices/%s/datapoints?type=3 HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\n"
					"Content-Length:%d\r\n\r\n",
					DEVID, APIKEY, strlen(buf1));
					
	strcat(buf, buf1);
// "POST /devices/ DEVID /datapoints?type=3 HTTP/1.1\r\napi-key: APIKEY \r\nHost:api.heclouds.com\r\n" "Content-Length:strlen(buf1) \r\n\r\n"

}

//==========================================================
//	�������ƣ�	OneNet_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_SendData(void)
{
	
	char buf[256];
	char res;
	
	OneNet_FillBuf(buf);								//��װ������
	
	res = gprs_send_data((unsigned char *)buf);
	memset(buf, 0, sizeof(buf));
	if(res)
	{
		printf("OneNet_SendData.\r\n");
		printf("send to onenet error %d.\r\n", res);
	}
	else
	{
		printf("send to onenet success.\r\n");
	}
}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_RevPro(unsigned char *dataPtr)
{
	// �жϽ��ܵ��������ǲ���ƽ̨�������ģ�
	if(strstr((char *)dataPtr, "CLOSED"))
	{
		//UsartPrintf(USART_DEBUG, "TCP CLOSED\r\n");
	}
	else
	{
		// ������������Ƿ��ͳɹ�
		if(strstr((char *)dataPtr, "succ"))
		{
			//UsartPrintf(USART_DEBUG, "Tips:	Send OK\r\n");
		}
		else
		{
			//UsartPrintf(USART_DEBUG, "Tips:	Send Err\r\n");
		}
	}
	

}


