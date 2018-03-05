#include "gprs.h"
#include "string.h" 
#include "usart2.h"
#include "includes.h"
#include "onenet.h"

uint8_t GPRS_CSQ[3];
u8 Flag_send_message=0;		//1--�������;�γ�ȣ�0--������
u8 dtbuf[50];   		//��ӡ������	

void delay(void);


/*
* �������ƣ� gprs_check_cmd(uint8_t *str)
* �������ܣ� ģ�鷢�������,�����յ���Ӧ��
* ���������� str:�ڴ���Ӧ���� 
* ����ֵ��    0,û�еõ��ڴ���Ӧ����
*		����,�ڴ�Ӧ������λ��(str��λ��)
*/
uint8_t* gprs_check_cmd(uint8_t *str)
{
	char *pstr = NULL;
	if(USART2_RX_STA&0X8000)
	{
		USART2_RX_BUF[USART2_RX_STA&0X7FFF] = 0;	//��ӽ�����
		pstr = strstr((const char*)USART2_RX_BUF,(const char*)str);
	}
	return (uint8_t*)pstr;
}

/*
* �������ƣ� uint8_t gps_send_cmd(uint8_t *cmd,uint8_t *ack,uint16_t waittime)
* �������ܣ� ��ģ�鷢������
* ���������� 
* cmd:���͵������ַ���,��cmd<0XFF��ʱ��,��������(���緢��0X1A), ���ڵ�ʱ�����ַ���
* ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
* waittime:�ȴ�ʱ��(��λ:10ms)
* ����ֵ��   0,���ͳɹ�(�õ����ڴ���Ӧ����)
*        1,����ʧ��
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
		u2_printf("%s\r\n",cmd);		// ��������
	}
		
	if(ack&&waittime)					// ��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)				// �ȴ�����ʱ
		{
			OSTimeDlyHMSM(0, 0, 0, 10);
			//delay();
			if(USART2_RX_STA&0X8000)// ���յ��ڴ���Ӧ����
			{
				if(gprs_check_cmd(ack))
					break;//�õ���Ч���� 
				USART2_RX_STA=0;
			} 
		}
		// printf("gprs_send_cmd()�ȴ���ʱ\r\n");
		if(waittime==0)
			res=1; 
	}
	return res;
}


/*
* �������ƣ� uint8_t gprs_word_test(void)
* �������ܣ� ģ��Ĳ���
* ���������� 
* ����ֵ��0����������   
*		 1,ͨ�Ų���
*        2,û��sim��
*        3,�ȴ�����
*/
uint8_t gprs_word_test(void)
{
	if(gprs_send_cmd((u8 *)"AT",(u8 *)"OK",100))
	{
		printf("gprs_word_test | ͨ�Ų���\r\n");
		if(gprs_send_cmd((u8 *)"AT",(u8 *)"OK",100))
			return 1;	//ͨ�Ų���
	}		
	if(gprs_send_cmd((u8 *)"AT+CPIN?",(u8 *)"READY",200))
		return 2;		//û��SIM��
	if(gprs_send_cmd((u8 *)"AT+CREG?",(u8 *)"0,1",200))
	{
		 if(!gprs_send_cmd((u8 *)"AT+CSQ",(u8 *)"OK",100))	
		 {
				memcpy(GPRS_CSQ,USART2_RX_BUF+15,2);
		 }
		 return 3;	//�ȴ����ŵ�����
	}	
	return 0;
}


/*
* �������ƣ� uint8_t gprs_connet_server(uint8_t *IP_ADD,uint8_t *COM)
* �������ܣ� ���ӷ�����
* ���������� 
* ����ֵ��0����������   
*		 ����������
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
* �������ƣ� uint8_t gprs_send_data(uint8_t *DATA)
* �������ܣ� ��������
* ���������� 
* ����ֵ��0����������   
*		 ����������
*/
uint8_t gprs_send_data(uint8_t *DATA)
{		
	if(gprs_send_cmd((uint8_t *)"AT+CIPSEND",(uint8_t *)">",100))	return 1;
	if(gprs_send_cmd(DATA,"",0))	return 2;
	if(gprs_send_cmd((uint8_t *)0x1a,(uint8_t *)"SEND OK",1500))	return 3;		
		return 0;
}	


/*
* �������ƣ� void gprs_test(uint8_t *res)
* �������ܣ�
* ���������� 
* ����ֵ��
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
		  	case 1:												  //��ͨ��ʧ��
				printf("communtion fail \r\n");		   
			  	break;
		  	case 2:												  //��SIM��
			 	printf("NO SIMCARD...   \r\n"); 
			  	break;
		  	case 3:												  //�ȴ�ע�ᵽ����
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


//��ʾGPS��λ��Ϣ 
void Gps_Msg_Show(void)
{
 	float tp;		
	
	tp = gpsx.longitude;
		lon = tp/10000;
	printf("Longitude:%.5f %1c   ", lon, gpsx.ewhemi);	//�õ������ַ���
	printf("\n");
  
	tp = gpsx.latitude;	
	lat = tp/10000;
	printf("Latitude:%.5f %1c   ",lat,gpsx.nshemi);	     //�õ�γ���ַ���
	printf("\n");
	
	printf("UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.day);	//��ʾUTC����
  printf("\n");  
	
	printf("UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//��ʾUTCʱ��
	printf("\n");
}








