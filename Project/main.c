

#include <reg52.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "1602.h"
#include "delay.h"
#define uchar unsigned char
#define uint unsigned int


							
sbit SDA=P1^1;	//接在TEA5767的数据端口	自行更改端口适用于自己的开发板						
sbit SCL=P1^0;	//接在TEA5767的时钟端口								
sbit KEY1=P3^3;		//频率增加按键端口							
sbit KEY2=P3^4;		//频率减小按键端口							


unsigned char temp[16];//定义显示区域临时存储数组
/////////////////////////////////////////////////////////////////////////////////////////////////////改

unsigned char radio_write_data[5]={0x30,0x50,0x70,0x16,0x80};        //要写入TEA5767的数据,101.1M
unsigned long frequency=101100;	//开机初始化的电台频率为101.1MHz
unsigned int pll=101100;		//计算频率合成时用的变量





void Delayus(unsigned char x)	//延时函数
{
 for(;x>0;x--);
}

void iic_start()		//起始信号   
{
 SDA=1;
 Delayus(4);
 SCL=1;
 Delayus(4);
 SDA=0;
 Delayus(4);
 SCL=0;
 Delayus(4);
}

void iic_stop()			 //停止信号
{
 SCL=0;
 Delayus(4);
 SDA=0;
 Delayus(4);
 SCL=1;
 Delayus(4);
 SDA=1;
 Delayus(4);
}

void iic_ack()			  //应答对应
{
 SDA=0;
 Delayus(4);
 SCL=1;
 Delayus(4);
 SCL=0;
 Delayus(4);
 SDA=1;
 Delayus(4);
}


bit iic_testack()			// 检测对方回答
{
 bit ErrorBit;
 SDA=1;
 Delayus(4);
 SCL=1;
 Delayus(4);
 ErrorBit=SDA;
 Delayus(4);
 SCL=0;
 return ErrorBit;
}

void iic_write8bit(unsigned char input)		  	//写8位串行数据
{
 unsigned char temp;
 for(temp=8;temp>0;temp--)
 {
  SDA=(bit)(input&0x80);
  Delayus(4);
  SCL=1;
  Delayus(4);
  SCL=0;
  Delayus(4);
  input=input<<1;
 }
}


void delays (unsigned char b)	//按键消抖用的延时程序
{
  unsigned char i;
  for(b;b>0;b--)
  for(i=0;i<250;i++);
}

void radio_write(void)		   //收音机写数据
{
    unsigned char i;
    iic_start();
    iic_write8bit(0xc0);        //TEA5767写地址
    if(!iic_testack())
    {
        for(i=0;i<5;i++)
        {
            iic_write8bit(radio_write_data[i]);
            iic_ack();
        }
    }
    iic_stop();    
}


 //将显示的频率算好后写到TEA5767芯片里面实现调节,不用考虑TEA5767用于搜台的相关位:SM,SUD
void search()				   //搜台
{        
    pll=(unsigned int)((float)((frequency+225))/(float)8.192);    //计算合成频率数据，频率单位:k
    radio_write_data[0]=pll/256;	//将算好的频率高位数据写入TEA5767
    radio_write_data[1]=pll%256;	//将算好的频率低位数据写入TEA5767
    radio_write_data[2]=0x70;		//这是TEA5767开立体声，关静音
    radio_write_data[3]=0x16;		//中国制式，采用32768晶振，软静音关，立体声消噪声开
    radio_write_data[4]=0x80;		//6.5M锁相环开，去加重时间75US
    radio_write();
}




void display(uint frq)				  //显示函数
{ 
  sprintf(temp,"   %5.1f MHz",((float)frq)/10);//浮点型显示，带一位小数点，首先把int类型强制变为float型，然后除以10运算
  LCD_Write_String(0,1,temp);//显示第一行
}

void main()				//主函数
{
 LCD_Init();           //初始化液晶
 DelayMs(20);          //延时有助于稳定
 LCD_Clear();          //清屏
 LCD_Write_String(0,0,"   FM TEA5767   ");//显示第一行
 search();
 radio_write();		   //写入初始化电台频率
 while(1)
 {
  	if(!KEY1) 	   //手动设置频率,+0.1MHz;
  	{
  	  delays(250); //延时消抖
	  if(!KEY1)
	  {
  	  frequency+=100;
      if(frequency>108500)	//频率如果大于108.5MHz
          frequency=87500;	//频率设定为87.5MHz
	      search();
		  display(frequency/100);  //调用显示
	  }
  	}

    if(!KEY2) 		//手动设置频率,-0.1MHz;
    {
     delays(250);   //延时消抖
        if(!KEY2)
	    {
        frequency-=100;
        if(frequency<87500)	//频率如果小于87.5MHz
          frequency=108500;	//频率设定为108.5MHz
		  search();
		  display(frequency/100);  //调用显示
    }
	display(frequency/100);  //调用显示
   } 
  }
}















