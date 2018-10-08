/*********************************************************************************
文件包含区
*********************************************************************************/
#include "include.h"
/*********************************************************************************
常量定义区
*********************************************************************************/

/*********************************************************************************
公共变量定义区
*********************************************************************************/
/*********************************************************************************
外部变量声明区
*********************************************************************************/
/*********************************************************************************
私有变量定义区
*********************************************************************************/ 
struct BC95_Str BC95;            //BC95 用的寄存器
/*********************************************************************************
测试变量定义区
*********************************************************************************/
/*********************************************************************************
内部函数定义区
*********************************************************************************/
/*********************************************************************************
功能代码定义区
*********************************************************************************/
/*********************************************************************************
 Function:      //void BC95_Power_On(void)
 Description:   //
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void BC95_Power_On(void)        //BC95上电
{
  RCC_Configuration();
  USART2_Configuration();       //初始化串口2
  
  GPIO_SetBits(GPIOE,GPIO_Pin_2);       //VBAT拉高        3.1-4.2V，典型值3.6V
  GPIO_SetBits(GPIOE,GPIO_Pin_1);      //复位脚拉低
  
  BC95.Report_Bit = 1;
  
  Create_Timer(ONCE,1,
                     BC95_Reset,0,PROCESS); 
}
/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void BC95_Power_Off(void)        //BC95断电
{
  GPIO_ResetBits(GPIOE,GPIO_Pin_2); 
}
/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void BC95_Reset(void)
{
  GPIO_ResetBits(GPIOE,GPIO_Pin_1);     //复位脚拉高
  
  Create_Timer(ONCE,5,
                     BC95_Start,0,PROCESS); 
}
/*********************************************************************************
 Function:      //
 Description:   ////TPB21启动
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void BC95_Start(void)        
{
  Uart2.Sent_Length = 0;                                //清空串口2发送长度
  Uart2.Receive_Length = 0;                             //清空串口2接收长度
  Uart2.Send_Busy = FALSE;                              //清空串口2发送忙标志  
  Uart2.Receive_Busy = FALSE;                           //清空串口2接收忙
  Uart2.Receive_Pend = FALSE;                           //清空串口2挂起
  
  BC95.Start_Process = NRB; //指向下一个流程
  BC95.Incident_Pend = TRUE; //事件标志挂起
  BC95.Err_Conner.Connect = SEND_ERROR_NUM;     //连接超时次数
}

/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void BC95_Process(void)                         //BC95主进程
{
  char *str = NULL;
  char *str1 = NULL;
  char *str2 = NULL;

  //如果需要上报消息，启动BC95
  if(BC95.Report_Bit != 0)
  {
    if(BC95.Start_Process == BC95_RECONNECT)
    {
      if(BC95.Reconnect_Times >= 3)  //重连超次数则睡眠
      {
        BC95.Reconnect_Times = 0;  
        BC95.FailTimes++;  
        MCU_DeInit(); 
      }
      else     //否则重连
      {
        MeterParameter.DeviceStatus = RUN;
        BC95_Start();
      }
    }
  }
  
  if((BC95.Incident_Pend != FALSE)||(Upgrade.Incident_Pend != FALSE)) //检测是否有事件挂起
  {
    BC95.Incident_Pend = FALSE; //清除事件挂起
    Upgrade.Incident_Pend = FALSE;
    switch(BC95.Start_Process)
    {
    case NRB:                  //重启
      {
        BC95_Data_Send("AT+NRB\r\n",8);
        Create_Timer(ONCE,10,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;
    case AT:                  //同步波特率
      {
        BC95_Data_Send("AT\r\n",4);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;
    case GETNBAND:               //查询频段
      {
        
        BC95_Data_Send("AT+NBAND?\r\n",11);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;
    case GETCFUN:                //查询电话功能
      {
        BC95_Data_Send("AT+CFUN?\r\n",10);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      } 
      break;
    case CGSN:                 //查询IMEI      
      {
        BC95_Data_Send("AT+CGSN=1\r\n",11);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;  
     case CCID:                 //查询CCID
      {
        BC95_Data_Send("AT+NCCID\r\n",10); 
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;  
    case CSQ:                //查询信号强度
      {
        BC95_Data_Send("AT+CSQ\r\n",8);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      } 
      break;
    case GETCGATT:                //查询网络激活状态    
      {
        BC95_Data_Send("AT+CGATT?\r\n",11);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;    
    case CEREG:                //查询网络注册状态     
      {
        BC95_Data_Send("AT+CEREG?\r\n",11);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;
    case CCLK:                //查询实时时间 
      {
        BC95_Data_Send("AT+CCLK?\r\n",10);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;
    case GETNCDP:                 //查询CDP服务器
      {
        BC95_Data_Send("AT+NCDP?\r\n",10);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;
    case SETNCDP:                 //设置CDP服务器 
      {
        BC95_Data_Send("AT+NCDP=180.101.147.115,5683\r\n",30);   
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break; 
    case NSMI:                 //设置发送消息指示
      {
        BC95_Data_Send("AT+NSMI=1\r\n",11);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;
    case NNMI:                 //设置接收消息指示
      {
        BC95_Data_Send("AT+NNMI=1\r\n",11);
        Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                     BC95_Recv_Timeout_CallBack,0,PROCESS); 
      }
      break;
    case NMGS:                 //发送消息     
      { 
        //先执行上报，以保证联网成功
        if(BC95.Report_Bit != 0)
        {
          Report_History_Data();          //发送数据
          Create_Timer(ONCE,BC95_R_TIMEROUT_TIME,
                       BC95_Recv_Timeout_CallBack,0,PROCESS); 
        }
        else
        {
          if(Upgrade.Process != IDLE)
          {
            Upgrade_Send_Process();       //发送升级
          }
          else
          {
            Create_Timer(ONCE,10,
                         MCU_DeInit,0,PROCESS); //10s后关机
          }
        }
      }
      break;
    case BC95_CONNECT_ERROR:      //连接失败
      BC95.Start_Process = BC95_RECONNECT;
      BC95.Reconnect_Times++;
      break;
    case BC95_POWER_DOWN:       //发送接收完成则直接睡眠  
      MCU_DeInit(); 
      break;
    default:
      break;
    }
  }
  
  if(Uart2.Receive_Pend != FALSE)//判断有数据
  { 
    
    switch(BC95.Start_Process)
    {
    case NRB:                  //重启
      {
        if(strstr(Uart2.R_Buffer,"REBOOT_CAUSE_APPLICATION_AT") != NULL)
        {
          BC95.Start_Process = AT;
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
          Create_Timer(ONCE,20,
                       BC95_Delay_CallBack,0,PROCESS); 
        }
      }
      break;
    case AT:            //同步波特率
      {
        if(strstr(Uart2.R_Buffer,"OK") != NULL)
        {         
          BC95.Start_Process = GETNBAND;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }
      }
      break;
    case GETNBAND:               //查询频段
      {       
        if(strstr(Uart2.R_Buffer,"+NBAND:5") != NULL)    //支持频段5
        {
          BC95.Start_Process = GETCFUN;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }
      }
      break;
    case GETCFUN:                //查询电话功能
      {
        if(strstr(Uart2.R_Buffer,"+CFUN:1") != NULL)     //全功能
        {         
          BC95.Start_Process = CGSN;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }
      } 
      break;
    case CGSN:         // 查询IMEI
      {
        str = strstr(Uart2.R_Buffer,"+CGSN");
        if( str != NULL)//获取到IMEI
        {
          BC95.Start_Process = CCID;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }
      }
      break;
     case CCID:          //查询CCID
      {
        str = strstr(Uart2.R_Buffer,"+NCCID");
        if( str != NULL)
        {
          BC95.Start_Process = CSQ;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }
      }
      break;
    case CSQ:           //查询信号强度
      {
        str = strstr(Uart2.R_Buffer,"+CSQ");
        if( str != NULL)
        {
          BC95.Rssi =0;//保存信号强度
          if( (str[5] >= '0') && (str[5] <= '9') )
          {
            BC95.Rssi += str[5]-0x30;
          }
          if( (str[6] >= '0') && (str[6] <= '9') )
          {
            BC95.Rssi *=10;
            BC95.Rssi += str[6]-0x30;
          }
    
          if(BC95.Rssi < 99)
          {
            BC95.Start_Process = GETCGATT;
            BC95.Incident_Pend = TRUE;//标记挂起
            Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
          }
        }
      }
      break;
    case GETCGATT:       //查询网络激活状态
      {
        if( strstr(Uart2.R_Buffer,"+CGATT:1") != NULL)//网络激活
        {        
          BC95.Start_Process = CEREG;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }       
      }
      break;
    case CEREG:       //查询网络注册状态
      {
        if( strstr(Uart2.R_Buffer,"+CEREG:0,1") != NULL)//网络注册
        {        
          BC95.Start_Process = CCLK;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }
      }
      break;   
    case CCLK:       //查询实时时间
      {
        str = strstr(Uart2.R_Buffer,"+CCLK:");
        if( str != NULL)
        {  
          GMT_to_BT((unsigned char*)str);
        }
        BC95.Start_Process = GETNCDP;
        BC95.Incident_Pend = TRUE;//标记挂起
        Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
      }
      break;   
    case  GETNCDP:                 //查询CDP服务器
      {
        if( strstr(Uart2.R_Buffer,"+NCDP:180.101.147.115,5683") != NULL)//获取到NCDP
        {        
          BC95.Start_Process = NSMI;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }
        else
        {        
          BC95.Start_Process = SETNCDP;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }
      }
      break;
    case  SETNCDP:                 //设置CDP服务器 
      {
        if(strstr(Uart2.R_Buffer,"OK") != NULL)
        {         
          BC95.Start_Process = GETNCDP;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }  
      }
      break;
    case NSMI:                 //设置发送消息指示
      {
        if(strstr(Uart2.R_Buffer,"OK") != NULL)
        {         
          BC95.Start_Process = NNMI;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }  
      }
      break;
    case  NNMI:                 //设置接收消息指示 
      {
        if(strstr(Uart2.R_Buffer,"OK") != NULL)
        {         
          BC95.Start_Process = NMGS;
          BC95.Incident_Pend = TRUE;//标记挂起
          Delete_Timer(BC95_Recv_Timeout_CallBack);//删除超时回调
        }  
      }
      break;
    case NMGS:        //发送消息
      {  
        str1 = strstr(Uart2.R_Buffer,"+NNMI:"); 
        //处理消息粘包
        while(str1 != NULL)
        { 
          str2 = strstr(str1+6,"+NNMI:"); 
          if(strnstr(str1,"+NNMI:4,AAAA0002",16) != NULL)     //上报历史数据的响应
          {
            BC95.Report_Bit = 0;
            BC95.Incident_Pend = TRUE;//标记挂起
            Delete_Timer(BC95_Recv_Timeout_CallBack);//删除接收超时回调
          }
          else if(strnstr(str1,",FFFE",16) != NULL)           //升级相关命令
          { 
            Delete_Timer(MCU_DeInit);//删除超时回调
            Upgrade_Recv_Process((unsigned char*)str1);
          }
          
          str1 = str2;
        }
      }
      break;
    case BC95_CONNECT_ERROR:
      //添加异常处理
      break;
    default:
      break;
    }
    
    memset(Uart2.R_Buffer,'\0',Uart2.Receive_Length);//清接收缓冲区
    Uart2.Receive_Length = 0;
    Uart2.Receive_Pend = FALSE;
  }
  
}
   
/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:        //
 Others:        //
*********************************************************************************/
void BC95_Data_Send(unsigned char *Data,unsigned short Len)
{    
  Uart2_Send((unsigned char*)Data,Len);
}

/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:        //
 Others:        //
*********************************************************************************/
void BC95_Recv_Timeout_CallBack(void)//启动超时重发
{
  if(BC95.Err_Conner.Connect != 0)//判断次数是否超时
  {
    BC95.Incident_Pend = TRUE;
    BC95.Err_Conner.Connect--;
  }
  else
  {   
    BC95.Incident_Pend = TRUE;
    BC95.Start_Process = BC95_CONNECT_ERROR;//启动错误
  }
}
/*********************************************************************************
 Function:      //
 Description:   //延时回调函数
 Input:         //
                //
 Output:        //
 Return:        //
 Others:        //
*********************************************************************************/
void BC95_Delay_CallBack(void)
{
  BC95.Incident_Pend = TRUE;//标记挂起
}
/*********************************************************************************
 Function:      //
 Description:   //上报历史数据
 Input:         //
                //
 Output:        //
 Return:        //
 Others:        //
*********************************************************************************/
unsigned char Report_History_Data(void)
{
  uint8_t data[40] = "AT+NMGS=12,080000000000000000000000\r\n";

   //获取时间   
  RTC_GetDate(RTC_Format_BIN, &RTC_DateStr);
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStr);
  
  //累积流量
  data[13] = Int_to_ASCLL(Cal.Water_Data.flow8[0]/0x10);
  data[14] = Int_to_ASCLL(Cal.Water_Data.flow8[0]%0x10);
  data[15] = Int_to_ASCLL(Cal.Water_Data.flow8[1]/0x10);
  data[16] = Int_to_ASCLL(Cal.Water_Data.flow8[1]%0x10);
  data[17] = Int_to_ASCLL(Cal.Water_Data.flow8[2]/0x10);
  data[18] = Int_to_ASCLL(Cal.Water_Data.flow8[2]%0x10);
  data[19] = Int_to_ASCLL(Cal.Water_Data.flow8[3]/0x10);
  data[20] = Int_to_ASCLL(Cal.Water_Data.flow8[3]%0x10);
  //年
  data[21] = Int_to_ASCLL((RTC_DateStr.RTC_Year+2000)/0x1000);
  data[22] = Int_to_ASCLL((RTC_DateStr.RTC_Year+2000)%0x1000/0x100);
  data[23] = Int_to_ASCLL((RTC_DateStr.RTC_Year+2000)%0x100/0x10);
  data[24] = Int_to_ASCLL((RTC_DateStr.RTC_Year+2000)%0x10); 
  //月
  data[25] = Int_to_ASCLL(RTC_DateStr.RTC_Month/0x10);
  data[26] = Int_to_ASCLL(RTC_DateStr.RTC_Month%0x10);
  //日
  data[27] = Int_to_ASCLL(RTC_DateStr.RTC_Date/0x10);
  data[28] = Int_to_ASCLL(RTC_DateStr.RTC_Date%0x10);
  //时
  data[29] = Int_to_ASCLL(RTC_TimeStr.RTC_Hours/0x10);
  data[30] = Int_to_ASCLL(RTC_TimeStr.RTC_Hours%0x10);
  //分
  data[31] = Int_to_ASCLL(RTC_TimeStr.RTC_Minutes/0x10);
  data[32] = Int_to_ASCLL(RTC_TimeStr.RTC_Minutes%0x10);
  //秒
  data[33] = Int_to_ASCLL(RTC_TimeStr.RTC_Seconds/0x10);
  data[34] = Int_to_ASCLL(RTC_TimeStr.RTC_Seconds%0x10);
  
  BC95_Data_Send(data,37);
  
  return 1;
}
/******************************************END********************************************************/