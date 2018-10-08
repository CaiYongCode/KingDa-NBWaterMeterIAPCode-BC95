/*********************************************************************************************************
//概述：
//作者：随风飘跃    时间：       地点：
//CPU型号：         系统主频：
//版本号：V0.0        
*********************************************************************************************************/
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
 Description:   //向ROM写数据
 Input:         //
                //
 Output:        //
 Return:	//
 Others:        //
*********************************************************************************/
void WriteRom (unsigned short addr, void *pbuff, unsigned char length)
{
 unsigned char itemp;
 unsigned char cnt;
 unsigned char *p; 
 unsigned short itemp16;
 do
 {FLASH_Unlock(FLASH_MemType_Data); }//开启EEPROM编程
 while (FLASH_GetFlagStatus(FLASH_FLAG_DUL) == RESET); 
 itemp = 0;
 p = pbuff;
 cnt =0 ;
 while (itemp < length)
 {
   FLASH_ProgramByte(addr ,p[itemp]);
   itemp16 = 0xffff;
   while(FLASH_GetFlagStatus(FLASH_FLAG_EOP) == RESET)
   {
    itemp16 --;
    if(itemp16 <= 1)
    {break;}
   }
   itemp16 = FLASH_ReadByte(addr);
   if(p[itemp] != itemp16)
   {
    if(++cnt > 50)
    {
     FLASH_Lock(FLASH_MemType_Data);
    }
    continue ; 
   }
   else
   {cnt = 0;itemp ++;addr ++;}
 }
 FLASH_Lock(FLASH_MemType_Data);
}
/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:	//
 Others:        //
*********************************************************************************/
void Read_Version(unsigned short addr,unsigned char *version)
{
  unsigned char i = 0;
  
  for(i = 0;i < 11;i++)
  {
    version[i] = *((const unsigned char *)(addr+i));
  }
}
/*********************************************************************************
 Function:      //
 Description:   //
 Input:         //
                //
 Output:        //
 Return:	//
 Others:        //
*********************************************************************************/
void Save_Version(unsigned short addr,unsigned char *version)
{
  WriteRom (addr,version,11);
}
   /*********************************************************************************
 Function:      //
 Description:   //读取水表参数
 Input:         //
                //
 Output:        //
 Return:	//
 Others:        //
*********************************************************************************/
void Read_Meter_Parameter(void)
{
  //读取上报频率
  MeterParameter.ReportFrequency = *((const unsigned short *)(REPORT_FREQUENCY_ADDR));
  if(MeterParameter.ReportFrequency < 5) //默认上报频率24小时
  {
    MeterParameter.ReportFrequency = 1440;
  } 
}
/*********************************************************************************
 Function:      //
 Description:   //读取累积水量
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void Read_ACUM_Flow(unsigned short addr,union flow_union *Flow)       
{
  union flow_union *flow;
  flow = (union flow_union *)(addr);
  Flow->flow32 = flow->flow32;
}
/*********************************************************************************
 Function:      //
 Description:   //存储累积水量
 Input:         //
                //
 Output:        //
 Return:      	//
 Others:        //
*********************************************************************************/
void Save_Add_Flow(u16 addr,union flow_union *Flow)       
{
    WriteRom (addr,Flow->flow8,4);      
}

/*********************************************************************************
 Function:      //
 Description:   //读取APP程序有效标志
 Input:         //
                //
 Output:        //
 Return:	//
 Others:        //
*********************************************************************************/
void Read_APP_Valid(void)
{  
  APPValid = *((const unsigned char *)(APP_VALID_ADD));
}
/*********************************************************************************
 Function:      //
 Description:   //存储APP程序有效标志
 Input:         //
                //
 Output:        //
 Return:	//
 Others:        //
*********************************************************************************/
void Save_APP_Valid(void)
{  
  WriteRom (APP_VALID_ADD,&APPValid,1);
}
/*********************************************************************************
 Function:      //
 Description:   //读取升级信息
 Input:         //
                //
 Output:        //
 Return:	//
 Others:        //
*********************************************************************************/
void Read_Upgrade_Info(void)
{  
//  unsigned char i = 0; 
  Upgrade.Process =(enum Upgrade_Process)(*((const unsigned char *)(Upgrade_Process_ADD)));
  if(Upgrade.Process != IDLE)
  {
    Upgrade.TimeoutCounter = UPGRADE_TIMEOUT_MAX;
  }
  
//  for(i = 0;i < 11;i++)
//  {
//    Upgrade.Version[i] = *((const unsigned char *)(UPGRADE_VERSION_ADD+i));
//  }
//  Upgrade.PackageSize = *((const unsigned short *)(UPGRADE_PACKAGE_SINGLE_SIZE));
//  Upgrade.PackageTotalNum = *((const unsigned short *)(UPGRADE_PACKAGE_TOTAL_NUMBER));
}
/*********************************************************************************
 Function:      //
 Description:   //存储升级信息
 Input:         //
                //
 Output:        //
 Return:	//
 Others:        //
*********************************************************************************/
void Save_Upgrade_Info(void)
{
  //升级进行空闲，则清除升级信息
//  if(Upgrade.Process == IDLE)
//  {
//    memset(Upgrade.Version,'\0',11);
//    Upgrade.PackageSize = 0;
//    Upgrade.PackageTotalNum = 0;
//  }
  WriteRom (Upgrade_Process_ADD,&(Upgrade.Process),1);
//  WriteRom (UPGRADE_VERSION_ADD,Upgrade.Version,11);
//  WriteRom (UPGRADE_PACKAGE_SINGLE_SIZE,&(Upgrade.PackageSize),2);
//  WriteRom (UPGRADE_PACKAGE_TOTAL_NUMBER,&(Upgrade.PackageTotalNum),2);
}
/******************************************END********************************************************/
