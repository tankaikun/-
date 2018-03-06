#include "includes.h"
#include "gps.h"
#include "usart3.h"
#include "onenet.h"
#include "gprs.h"
#include "nrf24l01.h"
#include "string.h"

 /*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK App_TaskStartStk[APP_TASK_START_STK_SIZE];
static  OS_STK App_SendOnenetStk[APP_TASK_ONENET_STK_SIZE];
static  OS_STK App_AnalyGPS_Data[APP_TASK_GPS_STK_SIZE];
static  OS_STK App_GetWirelessNode_Data[APP_TASK_NODE_STK_SIZE];



 /*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
//static  void App_TaskCreate(void);
static  void App_DispScr_SignOn(void);

static  void App_TaskStart(void* p_arg);

static void App_Send_Data_Onenet(void);

static void App_Analy_GPS_Data(void);

static void App_Get_WirelessNode_Data(void);



/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument : none.
*
* Return   : none.
*********************************************************************************************************
*/

void TestPrintf(void)
{
	printf("OSRunning = %d.\n", OSRunning);
	printf("--test for handler--\r\n");
}

 
int main(void)
{ 
		 	
	CPU_INT08U os_err;

	CPU_IntDis();		
	
	OSInit();                                                   /* Initialize "uC/OS-II, The Real-Time Kernel".         */

	
	BSP_Init();                                                 /* Initialize BSP functions.  */
  
	App_DispScr_SignOn();

	printf("OS Init OK\r\n");
	
	os_err = OSTaskCreate((void (*) (void *)) App_TaskStart,
	                      (void *) 0,
	           (OS_STK *) &App_TaskStartStk[APP_TASK_START_STK_SIZE - 1],
	           (INT8U) APP_TASK_START_PRIO);
	printf("main | Creat App_TaskStartr\n");
														
	OSTaskNameSet(APP_TASK_START_PRIO, (CPU_INT08U *) "Start Task", &os_err);
           
   OSTimeSet(0);
													
   OSStart();     /* Start multitasking (i.e. give control to uC/OS-II).  */
													
													
   return (0);
}



/*
*********************************************************************************************************
*                                          App_TaskStart()
*
* Description : The startup task.  The uC/OS-II ticker should only be initialize once multitasking starts.
*
* Argument : p_arg       Argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
*
* Return   : none.
*
* Caller   : This is a task.
*
* Note     : none.
*********************************************************************************************************
*/
static  void App_TaskStart(void* p_arg)
{
	CPU_INT08U os_err;

	// 调整ticks时钟
	OS_CPU_SysTickInit();                                        	/* Initialize the SysTick.       */
	// 统计cpu的使用率
	OSStatInit();                                               	/* Determine CPU capacity.                              */


	// 用户创建任务
	// GPS的数据分析
	os_err = OSTaskCreate((void (*) (void *)) App_Analy_GPS_Data,
	                      (void *) 0,
	           (OS_STK *) &App_AnalyGPS_Data[APP_TASK_GPS_STK_SIZE - 1],
	           (INT8U) APP_TASK_ANALY_GPS_PRIO);
	printf("App_TaskStart | Creat App_Analy_GPS_Data\n");												
	OSTaskNameSet(APP_TASK_ANALY_GPS_PRIO, (CPU_INT08U *) "Analy GPS Data Task", &os_err);

	// 无线节点的通信获取胎压信息
	os_err = OSTaskCreate((void (*) (void *)) App_Get_WirelessNode_Data,
	                      (void *) 0,
	           (OS_STK *) &App_GetWirelessNode_Data[APP_TASK_NODE_STK_SIZE - 1],
	           (INT8U) APP_TASK_GET_WIRELESS_NODE_PRIO);
	printf("App_TaskStart | Creat App_Get_WirelessNode_Data\n");												
	OSTaskNameSet(APP_TASK_GET_WIRELESS_NODE_PRIO, (CPU_INT08U *) "Get Wireless Data Task", &os_err);

	// GPRS的数据发送
	os_err = OSTaskCreate((void (*) (void *)) App_Send_Data_Onenet,
	                      (void *) 0,
	           (OS_STK *) &App_SendOnenetStk[APP_TASK_ONENET_STK_SIZE - 1],
	           (INT8U) APP_TASK_SEND_ONENET_PRIO);
	printf("App_TaskStart | Creat App_Send_Data_Onenet\n");														
	OSTaskNameSet(APP_TASK_SEND_ONENET_PRIO, (CPU_INT08U *) "Send Onenet Task", &os_err);

	
	// 交出cpu，以后要在这里将任务删除掉
	while (DEF_TRUE)
	{
		printf("Creat App_Send_Data_Onenet\n");
		OSTimeDlyHMSM(10, 0, 0, 200);
	}
}




/*
---------------------------------------------------------------------------
*                             App_Analy_GPS_Data()
*
* Description : 通过GPS获取经纬度信息，并且将数据解析
*
* Argument	: none.
*
* Return   : none.
*
* Caller   : App_TaskStart().
*
* Note    : none.
=======================================================================
*/
extern int g_flag_gps;                        // gps数据获取成功标志    
extern uint8_t g_gps_rbuff[GPS_RBUFF_SIZE];   // gps串口等待解析数据
extern gps_msg_s g_gpsx;                      // gps数据解析后的结构体封装

static void App_Analy_GPS_Data(void)
{
	
	while(1)
	{
		debug("App_Analy_GPS_Data\r\n");
		
		if(g_flag_gps == 1)
		{
			g_flag_gps = 0;

			GPS_Analysis(&g_gpsx, g_gps_rbuff);
			
			Gps_Msg_Show();		
		}
		
		OSTimeDlyHMSM(0, 0, 1, 0);
	}

}



/*
---------------------------------------------------------------------------
*                             App_Get_WirelessNode_Data()
*
* Description : 通过无线模块获取汽车的胎压信息
*
* Argument	: none.
*
* Return   : none.
*
* Caller   : App_TaskStart().
*
* Note    : none.
=======================================================================
*/

uint8_t g_wireless_rxbuf[WIRELESS_RECEIVE_DATA_LENGTH];

static void App_Get_WirelessNode_Data(void)
{
	
	uint16_t ret = 0;
	int i =0;

	while(1)
	{
		debug("App_Get_WirelessNode_Data\r\n");

		NRF_RX_Mode();

		memset(g_wireless_rxbuf, 0 ,4);
		
		ret = NRF_Rx_Dat(g_wireless_rxbuf);
		if( ret == 0)
		{
			debug("wireless receive error\r\n");
		}
		else 
		{
			debug("receive ret = 0x%x\r\n",ret);
			for(i = 0; i < 4; i++)
			{
				debug("receive data = %d\r\n", g_wireless_rxbuf[i]);
			}	
		}

		OSTimeDlyHMSM(0, 0, 3, 0);
	}

}















/*
---------------------------------------------------------------------------
*                             App_Send_Data_Onenet()
*
* Description : 通过GPS获取经纬度信息，将经纬度信息通过GPRS网络发送到onenet平台
*
* Argument	: none.
*
* Return   : none.
*
* Caller   : App_TaskStart().
*
* Note    : none.
=======================================================================
*/


static void App_Send_Data_Onenet(void)
{
	gprs_test();  
	
	while(1)
	{
		printf("App_Send_Data_Onenet\r\n");
		
		// 将经纬度信息通过GPRS网络发送到onenet平台
		OneNet_SendData();
		
		// 任务挂起，30S
		OSTimeDlyHMSM(0, 0, 0, 900);
	}

}



/*
*********************************************************************************************************
*                                          App_DispScr_SignOn()
*
* Description : Display uC/OS-II system information on the USART.
*
* Argument: none.
*
* Return   : none.
*
* Caller   : App_TaskKbd().
*
* Note    : none.
*********************************************************************************************************
*/
static  void App_DispScr_SignOn(void)
{
   printf("\r\n  Micrium uC/OS-II  \r\n");
   printf("  ST STM32 (Cortex-M3)\r\n\r\n");

   printf("  uC/OS-II:  V%d.%d% d\r\n", OSVersion() / 100,(OSVersion() % 100) / 10, (OSVersion() % 10));
   printf("  TickRate: %d  \r\n", OS_TICKS_PER_SEC);
   printf("  CPU Usage: %d %% \r\n", OSCPUUsage);
   printf("  CPU Speed:%ld MHz  \r\n", BSP_CPU_ClkFreq() / 1000000L);
   printf("  #Ticks: %d  \r\n", OSTime);
   printf("  #CtxSw: %d  \r\n\r\n", OSCtxSwCtr);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          uC/OS-II APP HOOKS
*********************************************************************************************************
*********************************************************************************************************
*/

#if (OS_APP_HOOKS_EN > 0)
/*
*********************************************************************************************************
*                                      TASK CREATION HOOK (APPLICATION)
*
* Description : This function is called when a task is created.
*
* Argument : ptcb   is a pointer to the task control block of the task being created.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void App_TaskCreateHook(OS_TCB* ptcb)
{
}

/*
*********************************************************************************************************
*                                    TASK DELETION HOOK (APPLICATION)
*
* Description : This function is called when a task is deleted.
*
* Argument : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void App_TaskDelHook(OS_TCB* ptcb)
{
   (void) ptcb;
}

/*
*********************************************************************************************************
*                                      IDLE TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskIdleHook(), which is called by the idle task.  This hook
*               has been added to allow you to do such things as STOP the CPU to conserve power.
*
* Argument : none.
*
* Note     : (1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 251
void App_TaskIdleHook(void)
{
}
#endif

/*
*********************************************************************************************************
*                                        STATISTIC TASK HOOK (APPLICATION)
*
* Description : This function is called by OSTaskStatHook(), which is called every second by uC/OS-II's
*               statistics task.  This allows your application to add functionality to the statistics task.
*
* Argument : none.
*********************************************************************************************************
*/

void App_TaskStatHook(void)
{
}

/*
*********************************************************************************************************
*                                        TASK SWITCH HOOK (APPLICATION)
*
* Description : This function is called when a task switch is performed.  This allows you to perform other
*               operations during a context switch.
*
* Argument : none.
*
* Note     : 1 Interrupts are disabled during this call.
*
*            2  It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the
*                  task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

#if OS_TASK_SW_HOOK_EN > 0
void App_TaskSwHook(void)
{
}
#endif

/*
*********************************************************************************************************
*                                     OS_TCBInit() HOOK (APPLICATION)
*
* Description : This function is called by OSTCBInitHook(), which is called by OS_TCBInit() after setting
*               up most of the TCB.
*
* Argument : ptcb    is a pointer to the TCB of the task being created.
*
* Note     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if OS_VERSION >= 204
void App_TCBInitHook(OS_TCB* ptcb)
{
   (void) ptcb;
}
#endif

#endif



