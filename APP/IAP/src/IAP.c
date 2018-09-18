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
        Upgrade.Process = MESSAGE19;
        Save_Upgrade_Info();
        
        Run_BLD();
      }
      break;
    case MESSAGE24:            //设备上报升级成功，平台响应
      {
        if(Upgrade.Buffer[8] == 0x00)     //平台处理成功
        {
          Upgrade.Process = IDLE;
          Save_Upgrade_Info();         

          Delete_Timer(Upgrade_TimeOut_CallBack);//删除升级超时回调
          
          MCU_DeInit();
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
    case MESSAGE24:            //设备上报升级成功
      {
        SendUpgradeMessage24();
        Create_Timer(ONCE,UPGRADE_TIMEROUT_TIME,
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
//void SendUpgradeMessage19(void)
//{
//  unsigned char i = 0;
//  unsigned short crc16 = 0;
//  unsigned char version[11] = {0};
//  unsigned char buff[26] = {0};
//  uint8_t data[64] = "AT+NMGS=25,00000000000000000000000000000000000000000000000000\r\n";
//  
//
//  Read_Version(APP_VERSION_ADD,version);
//  
//  buff[0] = 0xFF;
//  buff[1] = 0xFE;
//  buff[2] = 0x01;
//  buff[3] = MESSAGE19;
//  buff[4] = 0;
//  buff[5] = 0;
//  buff[6] = 0;
//  buff[7] = 17;
//  buff[8] = 0;
//  memcpy(&buff[9],version,11);
//  
//  crc16 = CRC16(buff,25);
//  buff[4] = (crc16>>8)&0xFF;
//  buff[5] = crc16&0xFF;
//  
//  for(i = 0;i < 25;i++)    
//  {
//    data[11+2*i] = Int_to_ASCLL(buff[i]/0x10);
//    data[12+2*i] = Int_to_ASCLL(buff[i]%0x10); 
//  }
//
//  Uart2_Send(data,63);
//}
/*********************************************************************************
 Function:      //
 Description:   //平台通知软件新版本，设备响应报文
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
//void SendUpgradeMessage20(void)
//{
//  unsigned char i = 0;
//  unsigned short crc16 = 0;
//  unsigned char buff[10] = {0};
//  uint8_t data[64] = "AT+NMGS=9,000000000000000000\r\n";
// 
//  buff[0] = 0xFF;
//  buff[1] = 0xFE;
//  buff[2] = 0x01;
//  buff[3] = MESSAGE20;
//  buff[4] = 0;
//  buff[5] = 0;
//  buff[6] = 0;
//  buff[7] = 1;
//  buff[8] = Upgrade.ResultCode;
//  
//  crc16 = CRC16(buff,9);
//  buff[4] = (crc16>>8)&0xFF;
//  buff[5] = crc16&0xFF;
//  
//  for(i = 0;i < 9;i++)    
//  {
//    data[10+2*i] = Int_to_ASCLL(buff[i]/0x10);
//    data[11+2*i] = Int_to_ASCLL(buff[i]%0x10);  
//  }
//  
//  Uart2_Send(data,30);
//}
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
    MCU_DeInit();
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
