2018 - 01 - 03

开发硬件：
	主芯片：stm32f103vet6
编程环境：
	SI
编译环境：
	MDK5.23

typedef unsigned              char              uint8_t;     u8
typedef unsigned              short  int       uint16_t;
typedef unsigned              int                 uint32_t;
typedef unsigned              __INT64          uint64_t;




 操作系统的任务安排
 任务1：获取gps数据，将数据打包，通过GPRS发送到onenet上去
 任务2：读取433无线通信模块的数据 DMA
 任务3：
 
gps的特性：
更新频率：1HZ
 
 定位协议：NMEA-0813
 控制协议：SkyTraq
 
 
 注意：
 初始化BSP的最后是等待GPS的pps接口触发
 中断触发，全局变量flag_pps，中断后关闭失能中断
 
 


时间：1月20号 - 1月26号
	GPS数据解析：

能力锻炼：
阅读代码，看懂代码的编写过程，画出相关的流程图
建立代码库
通过文件能自行写出相关的代码并且调试成功



1. 字符串转数字

int NMEA_Str2num(u8 *buf,u8*dx)
返回值：转换后的数值
buf：	要转换的字符串；
dx： 	小数点的位数（输出型参数）

异常情况：
负数      '-'
小数      '.'
结束符    '*' ','
非法数    c<'0' 和 c>'9'  // 前面的必须


2. 字符串中查找逗号




3. 解析gps数据



4. 设置gps参数

 
 
 