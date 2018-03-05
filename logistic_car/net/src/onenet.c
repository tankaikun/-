//网络设备
#include "gprs.h"	
//协议文件
#include "onenet.h"
//硬件驱动
#include "usart.h"
#include "includes.h"
//C库
#include <string.h>
#include <stdio.h>


#define DEVID	"23370018"
#define APIKEY	"s=Zna3ID6kI8=hQMmpHJAfN1VOA="


// 假定的经纬度
float lon = 0;
float lat = 0;

void OneNet_FillBuf(char *buf)
{
	
	char text[64];
	char buf1[128];

	// 清空text和buf1内存空间
	memset(text, 0, sizeof(text));
	memset(buf1, 0, sizeof(buf1));
	
	strcpy(buf1, "{");
	
	memset(text, 0, sizeof(text));
	sprintf(text, "\"GPS\":{\"lon\":%f,\"lat\":%f}", lon, lat);
	strcat(buf1, text);
	
	strcat(buf1, "}");

	// buf与HTTP协议相关
	sprintf(buf, "POST /devices/%s/datapoints?type=3 HTTP/1.1\r\napi-key:%s\r\nHost:api.heclouds.com\r\n"
					"Content-Length:%d\r\n\r\n",
					DEVID, APIKEY, strlen(buf1));
					
	strcat(buf, buf1);
// "POST /devices/ DEVID /datapoints?type=3 HTTP/1.1\r\napi-key: APIKEY \r\nHost:api.heclouds.com\r\n" "Content-Length:strlen(buf1) \r\n\r\n"

}

//==========================================================
//	函数名称：	OneNet_SendData
//
//	函数功能：	上传数据到平台
//
//	入口参数：	type：发送数据的格式
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_SendData(void)
{
	
	char buf[256];
	char res;
	
	OneNet_FillBuf(buf);								//封装数据流
	
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
//	函数名称：	OneNet_RevPro
//
//	函数功能：	平台返回数据检测
//
//	入口参数：	dataPtr：平台返回的数据
//
//	返回参数：	无
//
//	说明：		
//==========================================================
void OneNet_RevPro(unsigned char *dataPtr)
{
	// 判断接受到的数据是不是平台发过来的？
	if(strstr((char *)dataPtr, "CLOSED"))
	{
		//UsartPrintf(USART_DEBUG, "TCP CLOSED\r\n");
	}
	else
	{
		// 这里用来检测是否发送成功
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


