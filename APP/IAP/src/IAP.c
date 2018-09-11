/*********************************************************************************
//概述：
//作者：随风飘跃     时间：       地点：
//CPU型号：         系统主频：
//版本号：V0.0        
*********************************************************************************/
/*********************************************************************************
文件包含区
*********************************************************************************/
#include "IAP.h"
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
struct Upgrade_Str Upgrade;
unsigned char APPValid;
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
 Function:      //
 Description:   //读EEPROM地址数据
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
uint32_t EEPROM_ReadWord(uint32_t Address)
{
 return(*(PointerAttr uint32_t *) (uint32_t)Address);       
}

/*********************************************************************************
 Function:      //
 Description:   ////重新初始化STM8的中断向量表  把它重新定义到BLD的中断向量中
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void STM8_Interrupt_Vector_Table_Redirection(void)
{
  disableInterrupts();   //关闭中断  
  uint8_t Index = 0;	
  uint32_t data = 0;
 
  FLASH_Unlock(FLASH_MemType_Program);
  for(Index = 1; Index < 0x20;Index++)
  {
    data = EEPROM_ReadWord(INTERRUPT_VECTOR_ADD+4*Index);
    FLASH_ProgramWord(0x8000+4*Index,data);
  }
  FLASH_Lock(FLASH_MemType_Program);
}
/*********************************************************************************
 Function:      //
 Description:   //跳转到引导程序运行
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void JumptoBLD(void)
{
  sim();               // disable interrupts，建议程序跳转前关中断，跳转到新程序后先清一次中断。
  asm("LDW X,  SP ");
  asm("LD  A,  $FF");
  asm("LD  XL, A  ");
  asm("LDW SP, X  ");
  asm("JPF $8000");
}
/*********************************************************************************
 Function:      //
 Description:   //运行BLD程序
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void Run_BLD(void)
{
  STM8_Interrupt_Vector_Table_Redirection();
  JumptoBLD();
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
void Upgrade_Recv_Process(unsigned char *str)
{
  unsigned short i = 0,j = 0;

  unsigned char MessageID = 0;
  unsigned short crc16 = 0;
  unsigned char APPVersion[11] = {0};
  
 memset(Upgrade.Buffer,'\0',Upgrade.Length);//清缓存
  Upgrade.Length = 0;
  
  i = 6;
  while(str[i] != ',')
  {
    if( (str[i] >= '0')&&(str[i] <= '9') )
    {
      Upgrade.Length = Upgrade.Length*10+ASCLL_to_Int(str[i]);
    }
    i++;
  }
 
  for(j = 0;j < Upgrade.Length;j++)
  {
    Upgrade.Buffer[j] = ASCLL_to_Int(str[i+1+2*j])*16+ASCLL_to_Int(str[i+2+2*j]);
  }

  crc16 = Upgrade.Buffer[4]*256+Upgrade.Buffer[5];
  Upgrade.Buffer[4] = 0;
  Upgrade.Buffer[5] = 0;
  
  //CRC16校验
  if(crc16 != CRC16((unsigned char*)Upgrade.Buffer,Upgrade.Length))
  {
    return;
  }    
   
  MessageID = Upgrade.Buffer[3];
  switch(MessageID)
  {
    case MESSAGE19:            //查询设备版本
      {
        if(Upgrade.Process >= MESSAGE19)
        {
          return;
        }
        Upgrade.Process = MESSAGE19;
        Upgrade.Incident_Pend = TRUE; 
        Upgrade.TimeoutCounter = UPGRADE_TIMEOUT_MAX;
        Delete_Timer(Upgrade_TimeOut_CallBack);//删除升级超时回调
      }
      break;
    case MESSAGE20:           //新版本通知
      { 
        Read_Version(APP_VERSION_ADD,APPVersion);
        if( 0 == memcmp(APPVersion,&Upgrade.Buffer[8],11) )     //已经是最新版本
        {
          Upgrade.ResultCode = 0x03;
        }
        else                                                    //可以升级
        {
          Upgrade.ResultCode = 0x00;
          memcpy(Upgrade.Version,&Upgrade.Buffer[8],11);
          Upgrade.PackageSize = Upgrade.Buffer[24]*256+Upgrade.Buffer[25];
          Upgrade.PackageTotalNum = Upgrade.Buffer[26]*256+Upgrade.Buffer[27]; 
          
          Upgrade.Process = MESSAGE21;
          Save_Upgrade_Info();     
          Save_Add_Flow(ADD_FLOW_ADD,&Cal.Water_Data);
        }
        
        Upgrade.Process = MESSAGE20;
        Upgrade.Incident_Pend = TRUE;
        Upgrade.TimeoutCounter = UPGRADE_TIMEOUT_MAX;
        Delete_Timer(Upgrade_TimeOut_CallBack);//删除升级超时回调
        
      } 
      break;
    case MESSAGE21:            //下发升级包
      {
      }
      break;
    case MESSAGE22:            //设备上报升级包下载状态，平台响应
      {
        
      }
      break;
    case MESSAGE23:            //平台命令升级
      {
     
      }
      break;
    case MESSAGE24:            //设备上报升级成功，平台响应
      {
        if(Upgrade.Buffer[8] == 0x00)     //平台处理成功
        {
          Upgrade.Process = IDLE;
          Save_Upgrade_Info();
          
          BC95.Incident_Pend = TRUE;
          Upgrade.Incident_Pend = FALSE;
          Upgrade.TimeoutCounter = UPGRADE_TIMEOUT_MAX;
          Delete_Timer(Upgrade_TimeOut_CallBack);//删除升级超时回调
        }
      }
      break;
    default:
      break;
  }
}
/*********************************************************************************
 Function:      //
 Description:   //升级发送进程
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void Upgrade_Send_Process(void)
{
  
  switch(Upgrade.Process)
  {
    case MESSAGE19:            //平台查询设备版本，设备响应
      { 
        SendUpgradeMessage19(); 
        Create_Timer(ONCE,5,
                     Upgrade_TimeOut_CallBack,0,PROCESS);//升级超时回调
      }
      break;
    case MESSAGE20:           //平台通知新版本，设备响应
      {
        SendUpgradeMessage20();  
        if(Upgrade.ResultCode == 0x03)  //已经是最新版本
        {
          Upgrade.Process = IDLE;
          Save_Upgrade_Info();
        
          Upgrade.Incident_Pend = FALSE;
          Create_Timer(ONCE,2,
                       BC95_Delay_CallBack,0,PROCESS);//延时2s
        }
        else if(Upgrade.ResultCode == 0x00)
        {
          Create_Timer(ONCE,2,
                     Run_BLD,0,PROCESS);//2s后运行BLD程序
        }
      } 
      break;
    case MESSAGE21:            //请求升级包
      {
      }
      break;
    case MESSAGE22:            //设备上报升级包下载状态
      {
        
      }
      break;
    case MESSAGE23:            //平台命令升级，设备响应
      {
     
      }
      break;
    case MESSAGE24:            //设备上报升级成功
      {
        SendUpgradeMessage24();
        Create_Timer(ONCE,5,
                     Upgrade_TimeOut_CallBack,0,PROCESS);//升级超时回调
      }
      break;
    default:
      break;
  }
}
/*********************************************************************************
 Function:      //
 Description:   //平台查询软件版本，设备响应报文
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void SendUpgradeMessage19(void)
{
  unsigned char i = 0;
  unsigned short crc16 = 0;
  unsigned char version[11] = {0};
  unsigned char buff[26] = {0};
  uint8_t data[64] = "AT+NMGS=25,00000000000000000000000000000000000000000000000000\r\n";
  

  Read_Version(APP_VERSION_ADD,version);
  
  buff[0] = 0xFF;
  buff[1] = 0xFE;
  buff[2] = 0x01;
  buff[3] = MESSAGE19;
  buff[4] = 0;
  buff[5] = 0;
  buff[6] = 0;
  buff[7] = 17;
  buff[8] = 0;
  memcpy(&buff[9],version,11);
  
  crc16 = CRC16(buff,25);
  buff[4] = (crc16>>8)&0xFF;
  buff[5] = crc16&0xFF;
  
  for(i = 0;i < 25;i++)    
  {
    data[11+2*i] = Int_to_ASCLL(buff[i]/0x10);
    data[12+2*i] = Int_to_ASCLL(buff[i]%0x10);
    
  }

  Uart2_Send(data,63);
}
/*********************************************************************************
 Function:      //
 Description:   //平台通知软件新版本，设备响应报文
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void SendUpgradeMessage20(void)
{
  unsigned char i = 0;
  unsigned short crc16 = 0;
  unsigned char buff[10] = {0};
  uint8_t data[64] = "AT+NMGS=9,000000000000000000\r\n";
 
  buff[0] = 0xFF;
  buff[1] = 0xFE;
  buff[2] = 0x01;
  buff[3] = MESSAGE20;
  buff[4] = 0;
  buff[5] = 0;
  buff[6] = 0;
  buff[7] = 1;
  buff[8] = Upgrade.ResultCode;
  
  crc16 = CRC16(buff,9);
  buff[4] = (crc16>>8)&0xFF;
  buff[5] = crc16&0xFF;
  
  for(i = 0;i < 9;i++)    
  {
    data[10+2*i] = Int_to_ASCLL(buff[i]/0x10);
    data[11+2*i] = Int_to_ASCLL(buff[i]%0x10);  
  }
  
  Uart2_Send(data,30);
}
/*********************************************************************************
 Function:      //
 Description:   //设备上报升级成功
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void SendUpgradeMessage24(void)
{
  unsigned char i = 0;
  unsigned short crc16 = 0;
  unsigned char version[11] = {0};
  unsigned char buff[26] = {0};
  uint8_t data[64] = "AT+NMGS=25,00000000000000000000000000000000000000000000000000\r\n";
  
  Read_Version(APP_VERSION_ADD,version);
    
  buff[0] = 0xFF;
  buff[1] = 0xFE;
  buff[2] = 0x01;
  buff[3] = MESSAGE24;
  buff[4] = 0;
  buff[5] = 0;
  buff[6] = 0;
  buff[7] = 17;
  buff[8] = 0x00;
  memcpy(&buff[9],version,11);
  
  crc16 = CRC16(buff,25);
  buff[4] = (crc16>>8)&0xFF;
  buff[5] = crc16&0xFF;
  
  for(i = 0;i < 25;i++)    
  {
    data[11+2*i] = Int_to_ASCLL(buff[i]/0x10);
    data[12+2*i] = Int_to_ASCLL(buff[i]%0x10);
  }
  
  BC95_Data_Send(data,63);
}
/*********************************************************************************
 Function:      //
 Description:   //升级超时回调函数

 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void Upgrade_TimeOut_CallBack(void)
{
  if(Upgrade.TimeoutCounter != 0)//判断次数是否超时
  {
    Upgrade.Incident_Pend = TRUE;
    Upgrade.TimeoutCounter--;
  }
  else
  {
    Upgrade.Process = IDLE;
    Save_Upgrade_Info();
  
    Upgrade.Incident_Pend = FALSE;
    BC95.Incident_Pend = TRUE;
  }
  
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
/**************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/

/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/

/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/

/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
