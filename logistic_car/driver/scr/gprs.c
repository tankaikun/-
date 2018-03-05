#include "gprs.h"
#include "string.h" 
#include "usart2.h"
#include "includes.h"
#include "onenet.h"

uint8_t GPRS_CSQ[3];
u8 Flag_send_message=0;		//1--启动发送经纬度；0--不发送
u8 dtbuf[50];   		//打印缓存器	

void delay(void);


/*
* 函数名称： gprs_check_cmd(uint8_t *str)
* 函数功能： 模块发送命令后,检测接收到的应答
* 函数参数： str:期待的应答结果 
* 返回值：    0,没有得到期待的应答结果
*		其他,期待应答结果的位置(str的位置)
*/
uint8_t* gprs_check_cmd(uint8_t *str)
{
	char *pstr = NULL;
	if(USART2_RX_STA&0X8000)
	{
		USART2_RX_BUF[USART2_RX_STA&0X7FFF] = 0;	//添加结束符
		pstr = strstr((const char*)USART2_RX_BUF,(const char*)str);
	}
	return (uint8_t*)pstr;
}

/*
* 函数名称： uint8_t gps_send_cmd(uint8_t *cmd,uint8_t *ack,uint16_t waittime)
* 函数功能： 向模块发送命令
* 函数参数： 
* cmd:发送的命令字符串,当cmd<0XFF的时候,发送数字(比如发送0X1A), 大于的时候发送字符串
* ack:期待的应答结果,如果为空,则表示不需要等待应答
* waittime:等待时间(单位:10ms)
* 返回值：   0,发送成功(得到了期待的应答结果)
*        1,发送失败
*/
uint8_t gprs_send_cmd(uint8_t *cmd,uint8_t *ack,uint16_t waittime)
{
	uint8_t res = 0;
	USART2_RX_STA = 0;
	if((u32)cmd <= 0XFF)
	{
		while(DMA1_Channel7->CNDTR!=0);	
		USART2->DR=(u32)cmd;
	}
	else 
	{
		u2_printf("%s\r\n",cmd);		// 发送命令
	}
		
	if(ack&&waittime)					// 需要等待应答
	{
		while(--waittime)				// 等待倒计时
		{
			OSTimeDlyHMSM(0, 0, 0, 10);
			//delay();
			if(USART2_RX_STA&0X8000)// 接收到期待的应答结果
			{
				if(gprs_check_cmd(ack))
					break;//得到有效数据 
				USART2_RX_STA=0;
			} 
		}
		// printf("gprs_send_cmd()等待超时\r\n");
		if(waittime==0)
			res=1; 
	}
	return res;
}


/*
* 函数名称： uint8_t gprs_word_test(void)
* 函数功能： 模块的测试
* 函数参数： 
* 返回值：0，连接正常   
*		 1,通信不上
*        2,没有sim卡
*        3,等待网络
*/
uint8_t gprs_word_test(void)
{
	if(gprs_send_cmd((u8 *)"AT",(u8 *)"OK",100))
	{
		printf("gprs_word_test | 通信不上\r\n");
		if(gprs_send_cmd((u8 *)"AT",(u8 *)"OK",100))
			return 1;	//通信不上
	}		
	if(gprs_send_cmd((u8 *)"AT+CPIN?",(u8 *)"READY",200))
		return 2;		//没有SIM卡
	if(gprs_send_cmd((u8 *)"AT+CREG?",(u8 *)"0,1",200))
	{
		 if(!gprs_send_cmd((u8 *)"AT+CSQ",(u8 *)"OK",100))	
		 {
				memcpy(GPRS_CSQ,USART2_RX_BUF+15,2);
		 }
		 return 3;	//等待附着到网络
	}	
	return 0;
}


/*
* 函数名称： uint8_t gprs_connet_server(uint8_t *IP_ADD,uint8_t *COM)
* 函数功能： 连接服务器
* 函数参数： 
* 返回值：0，连接正常   
*		 其他：故障
*/
uint8_t gprs_connet_server(uint8_t *IP_ADD,uint8_t *COM)
{		
	if(gprs_send_cmd((uint8_t *)"AT+CGATT?",(uint8_t *)": 1",100))	 	return 1;
	if(gprs_send_cmd((uint8_t *)"AT+CIPSHUT",(uint8_t *)"OK",500))		return 2;
	if(gprs_send_cmd((uint8_t *)"AT+CSTT",(uint8_t *)"OK",200))			return 3;
	if(gprs_send_cmd((uint8_t *)"AT+CIICR",(uint8_t *)"OK",600))			return 4;
	if(!gprs_send_cmd((uint8_t *)"AT+CIFSR",(uint8_t *)"ERROR",200))		return 5;		
		sprintf((char*)dtbuf,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"",IP_ADD,COM);
	if(gprs_send_cmd((uint8_t *)dtbuf,(uint8_t *)"CONNECT OK",200))		return 6;		
		return 0;
}	


/*
* 函数名称： uint8_t gprs_send_data(uint8_t *DATA)
* 函数功能： 发送数据
* 函数参数： 
* 返回值：0，连接正常   
*		 其他：故障
*/
uint8_t gprs_send_data(uint8_t *DATA)
{		
	if(gprs_send_cmd((uint8_t *)"AT+CIPSEND",(uint8_t *)">",100))	return 1;
	if(gprs_send_cmd(DATA,"",0))	return 2;
	if(gprs_send_cmd((uint8_t *)0x1a,(uint8_t *)"SEND OK",1500))	return 3;		
		return 0;
}	


/*
* 函数名称： void gprs_test(uint8_t *res)
* 函数功能：
* 函数参数： 
* 返回值：
*/
void gprs_test(void)
{
	uint8_t res;
	res=gprs_word_test();	  
	printf("res=gprs_word_test() = %d\r\n", res);
	while(res)
	{
		res = gprs_word_test();	   
	  	switch(res)
	  	{
		  	case 1:												  //无通信失败
				printf("communtion fail \r\n");		   
			  	break;
		  	case 2:												  //无SIM卡
			 	printf("NO SIMCARD...   \r\n"); 
			  	break;
		  	case 3:												  //等待注册到网络
			  	printf("REGISTERING...  \r\n");
			  	break;
		  	default:
			  	break;			  
	  }   
	  OSTimeDlyHMSM(0, 0, 0, 100);
	  //delay();
	}   

	res = 1;
	while(res)
	{ 	  
		res = gprs_connet_server((uint8_t*)"183.230.40.33",(uint8_t*)"80");
		printf("gprs_connet_server() = %d.\r\n", res);
		OSTimeDlyHMSM(0, 0, 0, 10);
		//delay();
	}
}


//extern gps_msg_s gpsx;

gps_msg_s gpsx;


//显示GPS定位信息 
void Gps_Msg_Show(void)
{
 	float tp;		
	
	tp = gpsx.longitude;
		lon = tp/10000;
	printf("Longitude:%.5f %1c   ", lon, gpsx.ewhemi);	//得到经度字符串
	printf("\n");
  
	tp = gpsx.latitude;	
	lat = tp/10000;
	printf("Latitude:%.5f %1c   ",lat,gpsx.nshemi);	     //得到纬度字符串
	printf("\n");
	
	printf("UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.day);	//显示UTC日期
  printf("\n");  
	
	printf("UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//显示UTC时间
	printf("\n");
}








