/*********************************************************************************************************
//概述：
//作者：随风飘跃    时间：       地点：
//CPU型号：         系统主频：
//版本号：V0.0        
*********************************************************************************************************/
#ifndef __IAP_H__
#define __IAP_H__

/*********************************************************************************************************
文件包含区
*********************************************************************************************************/
#include "include.h"
/*********************************************************************************************************
宏定义区
*********************************************************************************************************/
#define UPGRADE_TIMEOUT_MAX     3
#define UPGRADE_TIMEROUT_TIME      10
/*********************************************************************************************************
数据类型定义
*********************************************************************************************************/
enum Upgrade_Process
{
  IDLE,
  MESSAGE19 = 19,
  MESSAGE20 = 20,
  MESSAGE21 = 21,
  MESSAGE22 = 22,
  MESSAGE23 = 23,
  MESSAGE24 = 24,
  FINISH,
  FAIL,
};
struct Upgrade_Str//BC95 总结构体
{
  bool Incident_Pend;//事件挂起标志
  unsigned char TimeoutCounter; //超时计数
  enum Upgrade_Process Process;         //进程
  unsigned char ResultCode;           //结果码
  unsigned char Version[11];         //升级版本
  unsigned short PackageTotalNum;      //升级包总数
  unsigned short PackageSize;           //升级包分片长度
  unsigned char PackageNum;             //升级包序号
  uint32_t ProgramAddr;                  //升级包写入地址
  unsigned char Buffer[512];            //升级包数据
  unsigned short Length;                //升级包长度
};
/*********************************************************************************************************
外部变量声明区
*********************************************************************************************************/
extern struct Upgrade_Str Upgrade;
extern unsigned char APPValid;
/*********************************************************************************************************
函数声明区
*********************************************************************************************************/
void Check_Run_APP(void);
void Run_APP(void);
void STM8_Interrupt_Vector_Table_Init(void);
uint32_t EEPROM_ReadWord(uint32_t Address);
uint32_t FLASH_ReadWord(uint32_t Address);
ErrorStatus FlashWrite (void *pbuff, unsigned short length);
void STM8_Interrupt_Vector_Table_Redirection(void);
void JumptoAPP(void);

void Upgrade_Recv_Process(unsigned char *str);
void Upgrade_Send_Process(void);
void SendUpgradeMessage19(void);
void SendUpgradeMessage20(void);
void SendUpgradeMessage21(void);
void SendUpgradeMessage22(void);
void SendUpgradeMessage23(void);
void Upgrade_TimeOut_CallBack(void);
void Upgrade_Delay_CallBack(void);
/********************************************************************************************************/
#endif

/******************************************END********************************************************/






