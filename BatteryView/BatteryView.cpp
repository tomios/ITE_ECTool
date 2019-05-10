#define  TOOLS_VER   "V1.9"
//*****************************************
// BatteryTool Version : 1.9
//*****************************************
// 1. Add USB-C PD status

//*****************************************
// BatteryTool Version : 1.8
//*****************************************
// 1. Add to 16 temperature show


//*****************************************
// BatteryTool Version : 1.7
//*****************************************
// 3. Minsys Voltage display in HEX
// 4. Charger Status display in HEX


//*****************************************
// BatteryTool Version : 1.6
//*****************************************
// 1. Add Charger option to 4 group
// 2. InputCurrent display in HEX


//*****************************************
// BatteryTool Version : 1.5
//*****************************************
// 1. Add to 8 temperature show


//*****************************************
// BatteryTool Version : 1.4
// 1. Modify USB_C_PDO_WATT to Adapter WATT
// 2. Add OS_AC/OS_BAT/OS_BAT_Charge/OS_BAT_Discharge
//    OS_BAT_Alert1_Low/OS_BAT_Alert1_War
//*****************************************

//*****************************************
// BatteryTool Version : 1.3
// 1. Add OS battery OS_BAT_Remtime/OS_BAT_FCC
//    OS_BAT_RMC/OS_BAT_Current/OS_BAT_RSOC
//*****************************************

//*****************************************
// BatteryTool Version : 1.2
// 1. Add battery SerialNum and status info
//*****************************************

//*****************************************
// BatteryTool Version : 1.1
// 1. Add battery temperature info to log
//*****************************************

//*****************************************
// BatteryTool Version : 1.0
// 1. First Release
//    a. Read the battery info from EC and display it
//    b. The battery info address by modify the BatteryView.cfg file
//*****************************************

/* Copyright (C)Copyright 2005-2020 ZXQ Telecom. All rights reserved.

   Author: Morgen Zhu
   
   Description:These functions of this file are reference only in the Windows!
   It can read/write ITE-EC RAM by 
   ----PM-port(62/66)
   ----KBC-port(60/64)
   ----EC-port(2E/2F or 4E/4F)
   ----Decicated I/O Port(301/302/303)
*/


// Using VS2012 X86 cmd tool to compilation
// For windows-32/64bit

//=======================================Include file ==============================================
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <windows.h>
#include <time.h>
#include <conio.h>
#include <wincon.h>
#include <Powrprof.h>
#include <Winbase.h>

using namespace std;
#include "winio.h"
//#pragma comment(lib,"WinIo.lib")       // For 32bit
#pragma comment(lib,"WinIox64.lib")    // For 64bit

#pragma comment(lib, "Powrprof.lib")
//==================================================================================================

//========================================Type Define ==============================================
typedef unsigned char   BYTE;
typedef unsigned char   UINT8;
#define TRUE            1
#define FALSE           0
//==================================================================================================

//==========================The hardware port to read/write function================================
#define READ_PORT(port,data2)  GetPortVal(port, &data2, 1);
#define WRITE_PORT(port,data2) SetPortVal(port, data2, 1)
//==================================================================================================

//======================================== PM channel ==============================================
#define PM_STATUS_PORT66          0x66
#define PM_CMD_PORT66             0x66
#define PM_DATA_PORT62            0x62
#define PM_OBF                    0x01
#define PM_IBF                    0x02
//------------wait EC PM channel port66 output buffer full-----/
void Wait_PM_OBF (void)
{
    DWORD data;
    READ_PORT(PM_STATUS_PORT66, data);
    while(!(data& PM_OBF))
    {
        READ_PORT(PM_STATUS_PORT66, data);
    }
}

//------------wait EC PM channel port66 input buffer empty-----/
void Wait_PM_IBE (void)
{
    DWORD data;
    READ_PORT(PM_STATUS_PORT66, data);
    while(data& PM_IBF)
    {
        READ_PORT(PM_STATUS_PORT66, data);
    }
}

//------------send command by EC PM channel--------------------/
void Send_cmd_by_PM(BYTE Cmd)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_CMD_PORT66, Cmd);
    Wait_PM_IBE();
}

//------------send data by EC PM channel-----------------------/
void Send_data_by_PM(BYTE Data)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_DATA_PORT62, Data);
    Wait_PM_IBE();
}

//-------------read data from EC PM channel--------------------/
BYTE Read_data_from_PM(void)
{
    DWORD data;
    Wait_PM_OBF();
    READ_PORT(PM_DATA_PORT62, data);
    return(data);
}
//--------------write EC RAM-----------------------------------/
void EC_WriteByte_PM(BYTE index, BYTE data)
{
    Send_cmd_by_PM(0x81);
    Send_data_by_PM(index);
    Send_data_by_PM(data);
}
//--------------read EC RAM------------------------------------/
BYTE EC_ReadByte_PM(BYTE index)
{
    BYTE data;
    Send_cmd_by_PM(0x80);
    Send_data_by_PM(index);
    data = Read_data_from_PM();
    return data;
}
//==================================================================================================

//================================KBC channel=======================================================
#define KBC_STATUS_PORT64         0x64
#define KBC_CMD_PORT64            0x64
#define KBC_DATA_PORT60           0x60
#define KBC_OBF                   0x01
#define KBC_IBF                   0x02
// wait EC KBC channel port64 output buffer full
void Wait_KBC_OBF (void)
{   
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(!(data& KBC_OBF))
    {
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

// wait EC KBC channel port64 output buffer empty
void Wait_KBC_OBE (void)
{
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(data& KBC_OBF)
    {
        READ_PORT(KBC_DATA_PORT60, data);
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

// wait EC KBC channel port64 input buffer empty
void Wait_KBC_IBE (void)
{
    DWORD data;
    READ_PORT(KBC_STATUS_PORT64, data);
    while(data& KBC_IBF)
    {
        READ_PORT(KBC_STATUS_PORT64, data);
    }
}

// send command by EC KBC channel
void Send_cmd_by_KBC (BYTE Cmd)
{
    Wait_KBC_OBE();
    Wait_KBC_IBE();
    WRITE_PORT(KBC_CMD_PORT64, Cmd);
    Wait_KBC_IBE();
}

// send data by EC KBC channel
void Send_data_by_KBC (BYTE Data)
{
    Wait_KBC_OBE();
    Wait_KBC_IBE();
    WRITE_PORT(KBC_DATA_PORT60, Data);
    Wait_KBC_IBE();
}

// read data from EC KBC channel
BYTE Read_data_from_KBC(void)
{
    DWORD data;
    Wait_KBC_OBF();
    READ_PORT(KBC_DATA_PORT60, data);
    return(data);
}
// Write EC RAM via KBC port(60/64)
void EC_WriteByte_KBC(BYTE index, BYTE data)
{
    Send_cmd_by_KBC(0x81);
    Send_data_by_KBC(index);
    Send_data_by_KBC(data);
}

// Read EC RAM via KBC port(60/64)
BYTE EC_ReadByte_KBC(BYTE index)
{
    Send_cmd_by_KBC(0x80);
    Send_data_by_KBC(index);
    return Read_data_from_KBC();
}
//==================================================================================================

//=======================================EC Direct Access interface=================================
//Port Config:
//  BADRSEL(0x200A) bit1-0  Addr    Data
//                  00      2Eh     2Fh
//                  01      4Eh     4Fh
//
//              01      4Eh     4Fh
//  ITE-EC Ram Read/Write Algorithm:
//  Addr    w   0x2E
//  Data    w   0x11
//  Addr    w   0x2F
//  Data    w   high byte
//  Addr    w   0x2E
//  Data    w   0x10
//  Addr    w   0x2F
//  Data    w   low byte
//  Addr    w   0x2E
//  Data    w   0x12
//  Addr    w   0x2F
//  Data    rw  value
UINT8 EC_ADDR_PORT = 0x4E;   // 0x2E or 0x4E
UINT8 EC_DATA_PORT = 0x4F;   // 0x2F or 0x4F
UINT8 High_Byte    = 0;
// Write EC RAM via EC port(2E/2F or 4E/4F)
void ECRamWrite_Direct(unsigned short iIndex, BYTE data)
{
    DWORD data1;
    data1 = data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, High_Byte); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, data1);
}

UINT8 ECRamRead_Direct(UINT8 iIndex)
{
    DWORD data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, High_Byte); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    READ_PORT(EC_DATA_PORT, data);
    return(data);
}


unsigned char ECRamReadExt_Direct(unsigned short iIndex)
{
    DWORD data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex>>8); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex&0xFF);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    READ_PORT(EC_DATA_PORT, data);
    return(data);
}

void ECRamWriteExt_Direct(unsigned short iIndex, BYTE data)
{
    DWORD data1;
    data1 = data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex>>8);    // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex&0xFF);  // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, data1);
}
//==================================================================================================

//=======================================Decicated I/O Port Operation===============================
// Need EC code configuration and need BIOS decode I/O
#define HIGH_BYTE_PORT    0x301
#define LOW_BYTE_PORT     (HIGH_BYTE_PORT+1)
#define DATA_BYTE_PORT    (HIGH_BYTE_PORT+2)  // Decicated I/O Port

BYTE EC_ReadByte_DeIO(BYTE iIndex)
{
    DWORD data;
    SetPortVal(HIGH_BYTE_PORT, High_Byte, 1);
    SetPortVal(LOW_BYTE_PORT, iIndex, 1);
    GetPortVal(DATA_BYTE_PORT, &data, 1);
    return data;
}

void EC_WriteByte_DeIO(BYTE iIndex, BYTE data)
{
    SetPortVal(HIGH_BYTE_PORT, High_Byte, 1);
    SetPortVal(LOW_BYTE_PORT, iIndex, 1);
    SetPortVal(DATA_BYTE_PORT, data, 1);
}
//==================================================================================================

//===============================Console control interface==========================================
#define EFI_BLACK                 0x00
#define EFI_BLUE                  0x01
#define EFI_GREEN                 0x02
#define EFI_RED                   0x04
#define EFI_BRIGHT                0x08

#define EFI_CYAN                  (EFI_BLUE | EFI_GREEN)
#define EFI_MAGENTA               (EFI_BLUE | EFI_RED)
#define EFI_BROWN                 (EFI_GREEN | EFI_RED)
#define EFI_LIGHTGRAY             (EFI_BLUE | EFI_GREEN | EFI_RED)
#define EFI_LIGHTBLUE             (EFI_BLUE | EFI_BRIGHT)
#define EFI_LIGHTGREEN            (EFI_GREEN | EFI_BRIGHT)
#define EFI_LIGHTCYAN             (EFI_CYAN | EFI_BRIGHT)
#define EFI_LIGHTRED              (EFI_RED | EFI_BRIGHT)
#define EFI_LIGHTMAGENTA          (EFI_MAGENTA | EFI_BRIGHT)
#define EFI_YELLOW                (EFI_BROWN | EFI_BRIGHT)
#define EFI_WHITE                 (EFI_BLUE | EFI_GREEN | EFI_RED | EFI_BRIGHT)

void SetTextColor(UINT8 TextColor, UINT8 BackColor)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, (TextColor|(BackColor<<4)));
}

void SetPosition_X_Y(UINT8 PositionX, UINT8 PositionY)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos={PositionX,PositionY};
    SetConsoleCursorPosition(hOut, pos);
}

void SetToolCursor()
{
    system("cls");
    system("color 07");
    
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO CursorInfo;  
    GetConsoleCursorInfo(handle, &CursorInfo);
    CursorInfo.bVisible = false;
    SetConsoleCursorInfo(handle, &CursorInfo);
}

void ClearToolCursor()
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);  
    CONSOLE_CURSOR_INFO CursorInfo;  
    GetConsoleCursorInfo(handle, &CursorInfo);
    CursorInfo.bVisible = true;
    SetConsoleCursorInfo(handle, &CursorInfo);
    
    SetTextColor(EFI_LIGHTGRAY, EFI_BLACK);
    system("cls");
}
//==================================================================================================

//  Example:
//  ECRamWrite_Direct(0x51,0x90);
//  EC_WriteByte_KBC(0x52,0x91);
//  EC_WriteByte_PM(0x53,0x93);
//  EC_WriteByte_DeIO(0x54,0x94);
//  printf("%d\n", ECRamRead_Direct(0x51));
//  printf("%d\n", EC_ReadByte_KBC(0x52));
//  printf("%d\n", EC_ReadByte_PM(0x53));
//  printf("%d\n", EC_ReadByte_DeIO(0x54));

// 0x301
//EC_ReadByte_DeIO
//EC_WriteByte_DeIO

//0x2E
//ECRamRead_Direct
//ECRamWrite_Direct
//ECRamReadExt_Direct
//ECRamWriteExt_Direct

//60/64
//EC_ReadByte_KBC
//EC_WriteByte_KBC

//62/66
//EC_ReadByte_PM
//EC_WriteByte_PM

#define  EC_RAM_WRITE  ECRamWriteExt_Direct
#define  EC_RAM_READ   ECRamReadExt_Direct

//==================================================================================================

//=======================================Tool info==================================================
#define  TOOLS_NAME  "Battery View"
#define  ITE_IC      "ITE8987"
#define  CopyRight   "(C)Copyright 2005-2020 ZXQ Telecom."
#define  DEBUG       0
#define  ESC         0x1B
//==================================================================================================

//=======================================Battery Info Type==========================================
#define  UI_BASE_X   25
#define  UI_BASE_Y   4

FILE *BAT_LogFile = NULL;
FILE *CfgFile = NULL;
unsigned int SetTime;

typedef struct BatteryInfoStruct
{
    char InfoName[128];
    char CfgItemName[128];    // Read cfg file item name
    char InfoValue[128];      // All info converted to character
    int  InfoAddr_L;          // Information address low
    int  InfoAddr_H;          // Information address high
    int  InfoInt;             // Information Value
    char Active;              // If EC code does not support this item, disable it
    char ActiveLog;           // if enable, it will be creat log
}EC_BatteryInfo;

typedef enum InfoNameEnum
{
    EC_Version=0,
    BAT_Mode,
    BAT_AC_State,
    BAT_FCCFlag,
    
    BAT_Current,
    BAT_Voltage,
    BAT_RMC,
    BAT_FCC,
    BAT_RSOC,
    BAT_ASOC,
    BAT_RealRSOC,
    BAT_DV,
    BAT_DC,
    BAT_Temp,
    BAT_CycleCount,
    BAT_CC,
    BAT_CV,
    BAT_Status_H,
    BAT_Status_L,
    
    CHARGE_Voltage,
    CHARGE_Current,
    CHARGE_OP0,
    CHARGE_OP1,
    CHARGE_OP2,
    CHARGE_OP3,
    PROHOT_OP0,
    PROHOT_OP1,
    MinSysVoltage,
    ChargerStatus,
    INPUT_Current,
    
    Adapter_WATT,
    USB_C_PDO_WATT,
    USB_C_PDO_C,
    USB_C_PDO_V,
    USB_C_Status,
    USB_C_Status2,
    
    BAT_ManuName,
    BAT_DeviceName,
    BAT_DevChemistry,
    BAT_ManuDate,
    BAT_SerialNum,
    
    Temp_Sensor1,
    Temp_Sensor2,
    Temp_Sensor3,
    Temp_Sensor4,
    Temp_Sensor5,
    Temp_Sensor6,
    Temp_Sensor7,
    Temp_Sensor8,
    Temp_Sensor9,
    Temp_Sensor10,
    Temp_Sensor11,
    Temp_Sensor12,
    Temp_Sensor13,
    Temp_Sensor14,
    Temp_Sensor15,
    Temp_Sensor16,
    
    OS_AC,
    OS_BAT,
    OS_BAT_Charge,
    OS_BAT_Discharge,
    OS_BAT_Remtime,
    OS_BAT_FCC,
    OS_BAT_RMC,
    OS_BAT_Current,
    OS_BAT_RSOC,
    OS_BAT_Alert1_Low,
    OS_BAT_Alert1_War,
    
    Acer_BAT_RMC,
    Acer_BAT_FCC,
    Acer_BAT_RSOC,
    INFONAMECOUNT         // count items and index it
}InfoNameEnum;

// The follow infomation address reference batteryview.cfg
EC_BatteryInfo BAT1_Info[] =
{
    {"EC_Version          :", "N/A", "N/A", 0, 0, 0, TRUE, TRUE},
    {"BAT_Mode            :", "N/A", "N/A", 0, 0, 0, TRUE, TRUE},
    {"BAT_AC_State        :", "N/A", "N/A", 0, 0, 0, TRUE, TRUE},
    {"BAT_FCCFlag         :", "N/A", "N/A", 0, 0, 0, TRUE, TRUE},
    
    {"BAT_Current         :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_Voltage         :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_RMC             :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_FCC             :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_RSOC            :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_ASOC            :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"RMC/FCC             :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_DV              :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_DC              :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_Temp            :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_CycleCount      :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_CC              :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_CV              :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_Status_H        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_Status_L        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    
    {"CHARGE_Voltage      :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"CHARGE_Current      :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"CHARGE_OP0          :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"CHARGE_OP1          :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"CHARGE_OP2          :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"CHARGE_OP3          :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"PROHOT_OP0          :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"PROHOT_OP1          :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"MinSysVoltage       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"ChargerStatus       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"INPUT_Current       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    
    {"Adapter_watt        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"USB_C_Watt          :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"USB_C_PDO_C         :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"USB_C_PDO_V         :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"USB_C_Status        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"USB_C_Status2       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},

    {"BAT_ManuName        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_DeviceName      :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_DevChemistry    :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_ManuDate        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"BAT_SerialNum       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    
    {"Temp_Sensor1        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor2        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor3        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor4        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor5        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor6        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor7        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor8        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor9        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor10       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor11       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor12       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor13       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor14       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor15       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Temp_Sensor16       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},

    {"OS_AC               :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT              :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT_Charge       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT_Discharge    :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT_Remtime      :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT_FCC          :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT_RMC          :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT_Current      :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT_RSOC         :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT_Alert1_Low   :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"OS_BAT_Alert1_War   :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    
    {"Acer_BAT_RMC        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Acer_BAT_FCC        :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    {"Acer_BAT_RSOC       :", "N/A", "N/A", 0, 0, 0, FALSE, FALSE},
    
    {0, 0, 0, 0, 0, 0}   // end
};
//==================================================================================================

void ReadCfgFile(void)
{
    char StrLine[1024];
    char StrNum[16];
    int  InfoIndex=0;
    int  HexNum;
    char *str;
    char *pStrLine;
    int i=0;
    int j=0;
    
    if((CfgFile = fopen("BatteryView.cfg","r")) == NULL)
    {
        printf("BatteryView.cfg not exist\n");
        return ;
    }
    
    while (!feof(CfgFile))
    {   
        // Read one line data
        fgets(StrLine,1024,CfgFile);
        //printf("%s", StrLine);
        
        pStrLine = StrLine;
        if(('$'==StrLine[0]) && (('1'==StrLine[1])))
        {
            //InfoIndex = (StrLine[3]-'0')*10 + (StrLine[4]-'0');
            
            
            while(('#' != (*pStrLine++)));
            HexNum = (int)strtol(pStrLine, &str, 16);
            //printf("%#X ",HexNum);
            BAT1_Info[InfoIndex].InfoAddr_L = HexNum;
            
            pStrLine++;
            while(('#' != (*pStrLine++)));
            
            HexNum = (int)strtol(pStrLine, &str, 16);
            //printf("%#X ",HexNum);
            BAT1_Info[InfoIndex].InfoAddr_H = HexNum;
            
            pStrLine++;
            while(('#' != (*pStrLine++)));
            HexNum = (int)strtol(pStrLine, &str, 10);
            //printf("%#X\n",HexNum);
            if(0x00==HexNum)
            {
                BAT1_Info[InfoIndex].Active = 0;
                BAT1_Info[InfoIndex].ActiveLog = 0;
            }
            else if(0x01==HexNum)
            {
                BAT1_Info[InfoIndex].Active = 1;
                BAT1_Info[InfoIndex].ActiveLog = 0;
            }
            else if(0x03==HexNum)
            {
                BAT1_Info[InfoIndex].Active = 1;
                BAT1_Info[InfoIndex].ActiveLog = 1;
            }
            
            pStrLine++;
            while(('#' != (*pStrLine++)));
            j=0;
            while(('#' != (*pStrLine)))
            {
                BAT1_Info[InfoIndex].CfgItemName[j] = *pStrLine;
                j++;
                pStrLine++;
            }
            
            InfoIndex++;
        }
        else if(('$'==StrLine[0]) && (('0'==StrLine[1])))
        {
            if('0' == StrLine[3])
            {
                while(('#' != (*pStrLine++)));
                HexNum = (int)strtol(pStrLine, &str, 10);
                //printf("Log file : %d\n",HexNum);
                BAT_LogFile = (FILE*)HexNum;
            }
            if('1' == StrLine[3])
            {
                while(('#' != (*pStrLine++)));
                HexNum = (int)strtol(pStrLine, &str, 16);
                //printf("IO : %#X\n",HexNum);
                EC_ADDR_PORT = HexNum;
                EC_DATA_PORT = HexNum+1;
            }
            if('2' == StrLine[3])
            {
                while(('#' != (*pStrLine++)));
                HexNum = (int)strtol(pStrLine, &str, 10);
                //printf("Time : %d\n",HexNum);
                SetTime = HexNum;
            }
        }
    }
    #if 0
    printf("\n\n\n");
    for(i=0; i<INFONAMECOUNT; i++)
    {
        printf("%d %#X %#X %#X %#X %s\n", i, BAT1_Info[i].InfoAddr_L, BAT1_Info[i].InfoAddr_H, BAT1_Info[i].Active,
                                    BAT1_Info[i].ActiveLog, BAT1_Info[i].CfgItemName);
    }
    #endif
    
    fclose(CfgFile);
}

void ToolInit(void)
{
    int i,j;
    SetConsoleTitle(TOOLS_NAME);
    system("mode con cols=95 lines=60");
    
    printf("Battery Tool %s (For ITE %s EC code)\n",TOOLS_VER, ITE_IC);
    printf("%s All rights reserved.\n",CopyRight);
    
    SetTextColor(EFI_LIGHTMAGENTA, EFI_BLACK);
    printf("<ESC> to exit!");
    SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
    for(i=0,j=0;i<INFONAMECOUNT;i++)
    {
        if(BAT1_Info[i].Active)
        {
            SetPosition_X_Y(0, UI_BASE_Y+j);
            printf(BAT1_Info[i].CfgItemName);
            j++;
        }
    }
}

void InitBatteryInfo(void)
{
    unsigned int tmpvalue;
    unsigned char index;
    unsigned char ch;
#if DEBUG
    SetPosition_X_Y(1,UI_BASE_Y-1);
    SetTextColor(EFI_CYAN, EFI_BLACK);
    printf("Debug mode");
#else
    if(BAT1_Info[EC_Version].Active)
    {
        sprintf(BAT1_Info[EC_Version].InfoValue, "%02d.%02d.%02d.%02d",
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_L),
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_H),
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_H+1),
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_H+2));  // The EC version is 4Byte
    }
    if(BAT1_Info[BAT_Mode].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Mode].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Mode].InfoAddr_L);
        BAT1_Info[BAT_Mode].InfoInt = tmpvalue;
        
        sprintf(BAT1_Info[BAT_Mode].InfoValue, "%04X [%-5s]",
                BAT1_Info[BAT_Mode].InfoInt, ((tmpvalue&0x8000)?"mWh":"mAh")); // mAh or 10mWh
    }
    if(BAT1_Info[BAT_CycleCount].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_CycleCount].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_CycleCount].InfoAddr_L);
        BAT1_Info[BAT_CycleCount].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_CycleCount].InfoValue, "%-8d",
                BAT1_Info[BAT_CycleCount].InfoInt);
    }
    if(BAT1_Info[BAT_DV].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_DV].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_DV].InfoAddr_L);
        BAT1_Info[BAT_DV].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_DV].InfoValue, "%-8d mV",
                BAT1_Info[BAT_DV].InfoInt);
    }
    if(BAT1_Info[BAT_DC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_DC].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_DC].InfoAddr_L);
        if(BAT1_Info[BAT_Mode].InfoInt&0x8000) // mAh or 10mWh
        {
            tmpvalue = tmpvalue*10;
        }
        BAT1_Info[BAT_DC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_DC].InfoValue, "%-8d %s",
                BAT1_Info[BAT_DC].InfoInt,
                ((BAT1_Info[BAT_Mode].InfoInt&0x8000)?"mWh":"mAh"));
    }
    
    if(BAT1_Info[BAT_ManuName].Active)
    {
        index = 0;
        while(1)
        {
            ch = EC_RAM_READ(BAT1_Info[BAT_ManuName].InfoAddr_L+index);
            (BAT1_Info[BAT_ManuName].InfoValue)[index] = ch;
            index++;
            if(0==ch)
            {
                break;
            }
        }
    }
    
    if(BAT1_Info[BAT_DeviceName].Active)
    {
        index = 0;
        while(1)
        {
            ch = EC_RAM_READ(BAT1_Info[BAT_DeviceName].InfoAddr_L+index);
            (BAT1_Info[BAT_DeviceName].InfoValue)[index] = ch;
            index++;
            if(0==ch)
            {
                break;
            }
        }
    }
    
    if(BAT1_Info[BAT_DevChemistry].Active)
    {
        index = 0;
        while(1)
        {
            ch = EC_RAM_READ(BAT1_Info[BAT_DevChemistry].InfoAddr_L+index);
            (BAT1_Info[BAT_DevChemistry].InfoValue)[index] = ch;
            index++;
            if(0==ch)
            {
                break;
            }
        }
    }
    
    if(BAT1_Info[BAT_ManuDate].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_ManuDate].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_ManuDate].InfoAddr_L);
        BAT1_Info[BAT_ManuDate].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_ManuDate].InfoValue, "%d-%d-%d",
                (((tmpvalue>>9)&0x7F)+1980),
                ((tmpvalue>>5)&0x0F),
                tmpvalue&0x1F
                );
    }
    
    if(BAT1_Info[BAT_SerialNum].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_SerialNum].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_SerialNum].InfoAddr_L);
        BAT1_Info[BAT_ManuDate].InfoInt = tmpvalue;
        
        sprintf(BAT1_Info[BAT_SerialNum].InfoValue, "%-d [%04X]", tmpvalue, tmpvalue);
    }
    
#endif
}

void PollBatteryInfo(void)
{
    unsigned int tmpvalue;
    float tmpvalue1;

#if DEBUG
    sprintf(BAT1_Info[EC_Version].InfoValue, "%02X.%02X.%02X.%02X", 1,1,1,1);
    sprintf(BAT1_Info[BAT_Mode].InfoValue, "%-8s", "mAh");
    sprintf(BAT1_Info[BAT_AC_State].InfoValue, "%-8s", "AC IN");
    sprintf(BAT1_Info[BAT_FCCFlag].InfoValue, "%-8s", "YES");
    sprintf(BAT1_Info[BAT_Current].InfoValue, "%-8d mA", -1500);
    sprintf(BAT1_Info[BAT_Voltage].InfoValue, "%-8d mV", 1000);
    sprintf(BAT1_Info[BAT_RMC].InfoValue, "%-8d", 2500);
    sprintf(BAT1_Info[BAT_FCC].InfoValue, "%-8d", 3500);
    sprintf(BAT1_Info[BAT_RSOC].InfoValue, "%-8d %%", 88);
    sprintf(BAT1_Info[BAT_RealRSOC].InfoValue, "%-8.2f %%", (2700*1.0/2800)*100);
    sprintf(BAT1_Info[BAT_DV].InfoValue, "%-8d mV", 4500);
    sprintf(BAT1_Info[BAT_DC].InfoValue, "%-8d", 5500);
    sprintf(BAT1_Info[BAT_Temp].InfoValue, "%-8.1f C", 2000*0.1-273.15);
    sprintf(BAT1_Info[BAT_CycleCount].InfoValue, "%-8d", 10);
    sprintf(BAT1_Info[BAT_CC].InfoValue, "%-8d mA", 1500);
    sprintf(BAT1_Info[BAT_CV].InfoValue, "%-8d mV", 1000);
    sprintf(BAT1_Info[CHARGE_Voltage].InfoValue, "%-8d mV", 1000);
    sprintf(BAT1_Info[CHARGE_Current].InfoValue, "%-8d mA", 1800);
    sprintf(BAT1_Info[INPUT_Current].InfoValue, "%-8d mA", 2500);
#else
    if(BAT1_Info[BAT_AC_State].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_AC_State].InfoAddr_L)&0x01;
        BAT1_Info[BAT_AC_State].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_AC_State].InfoValue, "%-8s", (tmpvalue?"AC IN":"AC OUT"));
    }
    if(BAT1_Info[BAT_FCCFlag].Active)
    {
        BAT1_Info[BAT_FCCFlag].InfoInt = (BAT1_Info[BAT_Status_L].InfoInt&0x0020)>>5;
        sprintf(BAT1_Info[BAT_FCCFlag].InfoValue, "%-8s", (BAT1_Info[BAT_FCCFlag].InfoInt?"Full":"Not Full"));
    }
    if(BAT1_Info[BAT_Current].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Current].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Current].InfoAddr_L);

        if(tmpvalue>0x8000)
        {
            tmpvalue ^=0xFFFF;
            tmpvalue+=1;
            tmpvalue = -tmpvalue;
        }

        BAT1_Info[BAT_Current].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_Current].InfoValue, "%-8d mA",
                BAT1_Info[BAT_Current].InfoInt);
    }
    if(BAT1_Info[BAT_Voltage].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Voltage].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Voltage].InfoAddr_L);
        BAT1_Info[BAT_Voltage].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_Voltage].InfoValue, "%-8d mV",
                BAT1_Info[BAT_Voltage].InfoInt);
    }
    if(BAT1_Info[BAT_RMC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_RMC].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_RMC].InfoAddr_L);
        if(BAT1_Info[BAT_Mode].InfoInt&0x8000)
        {
            tmpvalue = tmpvalue*10;
        }
        BAT1_Info[BAT_RMC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_RMC].InfoValue, "%-8d %s",
                BAT1_Info[BAT_RMC].InfoInt,
                ((BAT1_Info[BAT_Mode].InfoInt&0x8000)?"mWh":"mAh"));
    }
    if(BAT1_Info[BAT_FCC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_FCC].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_FCC].InfoAddr_L);
        if(BAT1_Info[BAT_Mode].InfoInt&0x8000)
        {
            tmpvalue = tmpvalue*10;
        }
        BAT1_Info[BAT_FCC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_FCC].InfoValue, "%-8d %s",
                BAT1_Info[BAT_FCC].InfoInt,
                ((BAT1_Info[BAT_Mode].InfoInt&0x8000)?"mWh":"mAh"));
    }
    if(BAT1_Info[BAT_RSOC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_RSOC].InfoAddr_L);
        BAT1_Info[BAT_RSOC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_RSOC].InfoValue, "%-8d %%",
                BAT1_Info[BAT_RSOC].InfoInt);
    }
    if(BAT1_Info[BAT_ASOC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_ASOC].InfoAddr_L);
        BAT1_Info[BAT_ASOC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_ASOC].InfoValue, "%-8d %%",
                BAT1_Info[BAT_ASOC].InfoInt);
    }
    if(BAT1_Info[BAT_RealRSOC].Active)
    {
        if(BAT1_Info[BAT_FCC].InfoInt)
        {
            tmpvalue1 = (BAT1_Info[BAT_RMC].InfoInt*1.0 / BAT1_Info[BAT_FCC].InfoInt)*100;
        }
        else
        {
            tmpvalue1=0;
        }
        sprintf(BAT1_Info[BAT_RealRSOC].InfoValue, "%-8.2f %%", tmpvalue1);
        BAT1_Info[BAT_RealRSOC].InfoInt = (int)(tmpvalue1+0.5);
    }
    if(BAT1_Info[BAT_Temp].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Temp].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Temp].InfoAddr_L);
        tmpvalue1 = tmpvalue*0.1-273.15;
        BAT1_Info[BAT_Temp].InfoInt = tmpvalue1;
        sprintf(BAT1_Info[BAT_Temp].InfoValue, "%-8.1f C", tmpvalue1);
    }
    if(BAT1_Info[CHARGE_Current].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_Current].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_Current].InfoAddr_L);
        BAT1_Info[CHARGE_Current].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_Current].InfoValue, "%-8d mA",
        BAT1_Info[CHARGE_Current].InfoInt);
    }
    if(BAT1_Info[CHARGE_Voltage].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_Voltage].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_Voltage].InfoAddr_L);
        BAT1_Info[CHARGE_Voltage].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_Voltage].InfoValue, "%-8d mV",
        BAT1_Info[CHARGE_Voltage].InfoInt);
    }
    if(BAT1_Info[INPUT_Current].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[INPUT_Current].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[INPUT_Current].InfoAddr_L);
        BAT1_Info[INPUT_Current].InfoInt = tmpvalue;
        sprintf(BAT1_Info[INPUT_Current].InfoValue, "%-8d mA [%-#04X]",
        BAT1_Info[INPUT_Current].InfoInt, BAT1_Info[INPUT_Current].InfoInt);
    }
    if(BAT1_Info[CHARGE_OP0].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP0].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP0].InfoAddr_L);
        BAT1_Info[CHARGE_OP0].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP0].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP0].InfoInt);
    }
    if(BAT1_Info[CHARGE_OP1].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP1].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP1].InfoAddr_L);
        BAT1_Info[CHARGE_OP1].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP1].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP1].InfoInt);
    }
    if(BAT1_Info[CHARGE_OP2].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP2].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP2].InfoAddr_L);
        BAT1_Info[CHARGE_OP2].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP2].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP2].InfoInt);
    }
    if(BAT1_Info[CHARGE_OP3].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP3].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP3].InfoAddr_L);
        BAT1_Info[CHARGE_OP3].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP3].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP3].InfoInt);
    }
    if(BAT1_Info[PROHOT_OP0].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PROHOT_OP0].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[PROHOT_OP0].InfoAddr_L);
        BAT1_Info[PROHOT_OP0].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PROHOT_OP0].InfoValue, "%-#08X",
        BAT1_Info[PROHOT_OP0].InfoInt);
    }
    if(BAT1_Info[PROHOT_OP1].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PROHOT_OP1].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[PROHOT_OP1].InfoAddr_L);
        BAT1_Info[PROHOT_OP1].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PROHOT_OP1].InfoValue, "%-#08X",
        BAT1_Info[PROHOT_OP1].InfoInt);
    }
    if(BAT1_Info[MinSysVoltage].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[MinSysVoltage].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[MinSysVoltage].InfoAddr_L);
        BAT1_Info[MinSysVoltage].InfoInt = tmpvalue;
        sprintf(BAT1_Info[MinSysVoltage].InfoValue, "%-#08X",
        BAT1_Info[MinSysVoltage].InfoInt);
    }
    if(BAT1_Info[ChargerStatus].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[ChargerStatus].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[ChargerStatus].InfoAddr_L);
        BAT1_Info[ChargerStatus].InfoInt = tmpvalue;
        sprintf(BAT1_Info[ChargerStatus].InfoValue, "%-#08X",
        BAT1_Info[ChargerStatus].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor1].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor1].InfoAddr_L);
        BAT1_Info[Temp_Sensor1].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor1].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor1].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor2].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor2].InfoAddr_L);
        BAT1_Info[Temp_Sensor2].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor2].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor2].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor3].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor3].InfoAddr_L);
        BAT1_Info[Temp_Sensor3].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor3].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor3].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor4].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor4].InfoAddr_L);
        BAT1_Info[Temp_Sensor4].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor4].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor4].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor5].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor5].InfoAddr_L);
        BAT1_Info[Temp_Sensor5].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor5].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor5].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor6].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor6].InfoAddr_L);
        BAT1_Info[Temp_Sensor6].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor6].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor6].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor7].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor7].InfoAddr_L);
        BAT1_Info[Temp_Sensor7].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor7].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor7].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor8].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor8].InfoAddr_L);
        BAT1_Info[Temp_Sensor8].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor8].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor8].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor9].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor9].InfoAddr_L);
        BAT1_Info[Temp_Sensor9].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor9].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor9].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor10].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor10].InfoAddr_L);
        BAT1_Info[Temp_Sensor10].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor10].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor10].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor11].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor11].InfoAddr_L);
        BAT1_Info[Temp_Sensor11].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor11].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor11].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor12].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor12].InfoAddr_L);
        BAT1_Info[Temp_Sensor12].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor12].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor12].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor13].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor13].InfoAddr_L);
        BAT1_Info[Temp_Sensor13].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor13].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor13].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor14].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor14].InfoAddr_L);
        BAT1_Info[Temp_Sensor14].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor14].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor14].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor15].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor15].InfoAddr_L);
        BAT1_Info[Temp_Sensor15].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor15].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor15].InfoInt);
    }
    if(BAT1_Info[Temp_Sensor16].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor16].InfoAddr_L);
        BAT1_Info[Temp_Sensor16].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor16].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor16].InfoInt);
    }
    if(BAT1_Info[BAT_CC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_CC].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_CC].InfoAddr_L);
        BAT1_Info[BAT_CC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_CC].InfoValue, "%-8d mA",BAT1_Info[BAT_CC].InfoInt);
    }
    if(BAT1_Info[BAT_CV].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_CV].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_CV].InfoAddr_L);
        BAT1_Info[BAT_CV].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_CV].InfoValue, "%-8d mV",BAT1_Info[BAT_CV].InfoInt);
    }
    if(BAT1_Info[BAT_Status_H].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Status_H].InfoAddr_L);
        BAT1_Info[BAT_Status_H].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_Status_H].InfoValue, "%-#08X", tmpvalue);
        //sprintf(BAT1_Info[BAT_Status_H].InfoValue, "OCA:%d | TCA:%d | RS | OTA:%d | TDA:%d | RS | RCA:%d | RTA:%d",
        //                                          (tmpvalue&0x8000)?1:0, (tmpvalue&0x4000)?1:0,
        //                                          (tmpvalue&0x1000)?1:0, (tmpvalue&0x0800)?1:0, 
        //                                          (tmpvalue&0x0200)?1:0, (tmpvalue&0x0100)?1:0);
    }
    if(BAT1_Info[BAT_Status_L].Active)
    {
    
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Status_L].InfoAddr_L);
        BAT1_Info[BAT_Status_L].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_Status_L].InfoValue, "%-#08X", tmpvalue);
        //sprintf(BAT1_Info[BAT_Status_L].InfoValue, "INI:%d | DSG:%d | FC :%d | FD :%d | EC:%02X   | [%#04X]",
        //                                          (tmpvalue&0x0080)?1:0, (tmpvalue&0x0040)?1:0,
        //                                          (tmpvalue&0x0020)?1:0, (tmpvalue&0x0010)?1:0,
        //                                          tmpvalue&0x000F, tmpvalue);
    }
    if(BAT1_Info[Acer_BAT_RMC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Acer_BAT_RMC].InfoAddr_H)<<8
                 | EC_RAM_READ(BAT1_Info[Acer_BAT_RMC].InfoAddr_L);
        if(BAT1_Info[BAT_Mode].InfoInt&0x8000)
        {
            tmpvalue = tmpvalue*10;
        }
        BAT1_Info[Acer_BAT_RMC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Acer_BAT_RMC].InfoValue, "%-8d %s",
                BAT1_Info[Acer_BAT_RMC].InfoInt,
                ((BAT1_Info[BAT_Mode].InfoInt&0x8000)?"mWh":"mAh"));
    }
    if(BAT1_Info[Acer_BAT_FCC].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Acer_BAT_FCC].InfoAddr_H)<<8
                 | EC_RAM_READ(BAT1_Info[Acer_BAT_FCC].InfoAddr_L);
        if(BAT1_Info[BAT_Mode].InfoInt&0x8000)
        {
            tmpvalue = tmpvalue*10;
        }
        BAT1_Info[Acer_BAT_FCC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Acer_BAT_FCC].InfoValue, "%-8d %s",
                BAT1_Info[Acer_BAT_FCC].InfoInt,
                ((BAT1_Info[BAT_Mode].InfoInt&0x8000)?"mWh":"mAh"));
    }
    if(BAT1_Info[Acer_BAT_RSOC].Active)
    {
        if(BAT1_Info[Acer_BAT_FCC].InfoInt)
        {
            tmpvalue1 = (BAT1_Info[Acer_BAT_RMC].InfoInt*1.0 / BAT1_Info[Acer_BAT_FCC].InfoInt)*100;
        }
        else
        {
            tmpvalue1=0;
        }
        sprintf(BAT1_Info[Acer_BAT_RSOC].InfoValue, "%-8.2f %%", tmpvalue1);
        BAT1_Info[Acer_BAT_RSOC].InfoInt = (int)(tmpvalue1+0.5);
    }
    if(BAT1_Info[Adapter_WATT].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Adapter_WATT].InfoAddr_L);
        BAT1_Info[Adapter_WATT].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Adapter_WATT].InfoValue, "%-8d W",BAT1_Info[Adapter_WATT].InfoInt);
    }
    if(BAT1_Info[USB_C_PDO_WATT].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[USB_C_PDO_WATT].InfoAddr_L);
        BAT1_Info[USB_C_PDO_WATT].InfoInt = tmpvalue;
        sprintf(BAT1_Info[USB_C_PDO_WATT].InfoValue, "%-8d W",BAT1_Info[USB_C_PDO_WATT].InfoInt);
    }
    if(BAT1_Info[USB_C_PDO_C].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[USB_C_PDO_C].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[USB_C_PDO_C].InfoAddr_L);
        BAT1_Info[USB_C_PDO_C].InfoInt = tmpvalue/10;
        sprintf(BAT1_Info[USB_C_PDO_C].InfoValue, "%-8d A",BAT1_Info[USB_C_PDO_C].InfoInt);
    }
    if(BAT1_Info[USB_C_PDO_V].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[USB_C_PDO_V].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[USB_C_PDO_V].InfoAddr_L);
        BAT1_Info[USB_C_PDO_V].InfoInt = tmpvalue/10;
        sprintf(BAT1_Info[USB_C_PDO_V].InfoValue, "%-8d V",BAT1_Info[USB_C_PDO_V].InfoInt);
    }
    if(BAT1_Info[USB_C_Status].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[USB_C_Status].InfoAddr_L);
        BAT1_Info[USB_C_Status].InfoInt = tmpvalue;
        //sprintf(BAT1_Info[USB_C_Status].InfoValue, "%-#08X",BAT1_Info[USB_C_Status].InfoInt);
        if(tmpvalue&0x01)
        {
            sprintf(BAT1_Info[USB_C_Status].InfoValue, "[%-s][%-s][%-s][%-s][%-s][%-s]",
                                    (tmpvalue&0x01)?"Connect   ":"Connect   ",
                                    (tmpvalue&0x02)?"cc1":((tmpvalue&0x04)?"cc2":"   "),
                                    (tmpvalue&0x08)?"DFP":"UFP",
                                    (tmpvalue&0x10)?"Source":"Sink  ",
                                    (tmpvalue&0x20)?"Watt_Low":"Watt_Hi ",
                                    (tmpvalue&0x40)?"PDO_OK":"      "
                                    );
        }
        else
        {
            sprintf(BAT1_Info[USB_C_Status].InfoValue, "[%-s][%-s][%-s][%-s][%-s][%-s]",
                                    "Disconnect",
                                    "   ",
                                    "   ",
                                    "      ",
                                    "        ",
                                    "      "
                                    );
        }
    }
    if(BAT1_Info[USB_C_Status2].Active)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[USB_C_Status2].InfoAddr_L);
        BAT1_Info[USB_C_Status2].InfoInt = tmpvalue;
        if(tmpvalue&0x01)
        {
            sprintf(BAT1_Info[USB_C_Status2].InfoValue, "[%-s]", "DC_ADP   ");
        }
        else if(tmpvalue&0x02)
        {
            sprintf(BAT1_Info[USB_C_Status2].InfoValue, "[%-s]", "PD_ADP   ");
        }
        else if(tmpvalue&0x04)
        {
            sprintf(BAT1_Info[USB_C_Status2].InfoValue, "[%-s]", "TYPEC_ADP");
        }
        else
        {
            sprintf(BAT1_Info[USB_C_Status2].InfoValue, "[%-s]", "         ");
        }
    }
#endif 
}

/*
// https://msdn.microsoft.com
typedef struct {
  BOOLEAN AcOnLine;
  BOOLEAN BatteryPresent;
  BOOLEAN Charging;
  BOOLEAN Discharging;
  BOOLEAN Spare1[4];    //Reserved
  DWORD   MaxCapacity;
  DWORD   RemainingCapacity;
  DWORD   Rate;
  DWORD   EstimatedTime;
  DWORD   DefaultAlert1;
  DWORD   DefaultAlert2;
} SYSTEM_BATTERY_STATE, *PSYSTEM_BATTERY_STATE;
*/
void PollOSBatteryInfo(void)
{
    int remtime;
    SYSTEM_BATTERY_STATE Battery_state;
    SYSTEM_POWER_STATUS  SysPowerState;
    
    GetSystemPowerStatus(&SysPowerState);
    CallNtPowerInformation(SystemBatteryState, NULL, 0, &Battery_state, sizeof(Battery_state));
    
    if (Battery_state.BatteryPresent)
    {
        sprintf(BAT1_Info[OS_AC].InfoValue, "%-18s", (Battery_state.AcOnLine)?("Adapter IN"):("Adapter OUT"));
        sprintf(BAT1_Info[OS_BAT].InfoValue, "%-18s", (Battery_state.BatteryPresent)?("Battery IN"):("Battery OUT"));
        sprintf(BAT1_Info[OS_BAT_Charge].InfoValue, "%-18s", (Battery_state.Charging)?("YES"):("NO"));
        sprintf(BAT1_Info[OS_BAT_Discharge].InfoValue, "%-18s", (Battery_state.Discharging)?("YES"):("NO"));

        remtime = Battery_state.EstimatedTime/60;
        if(remtime < 999)
        {
            sprintf(BAT1_Info[OS_BAT_Remtime].InfoValue, "%-6d min", remtime);
        }
        else
        {
            sprintf(BAT1_Info[OS_BAT_Remtime].InfoValue, "%-18s", "N/A");
        }
        
        BAT1_Info[OS_BAT_FCC].InfoInt = Battery_state.MaxCapacity;
        sprintf(BAT1_Info[OS_BAT_FCC].InfoValue, "%-6d mWh", Battery_state.MaxCapacity);
        
        BAT1_Info[OS_BAT_RMC].InfoInt = Battery_state.RemainingCapacity;
        sprintf(BAT1_Info[OS_BAT_RMC].InfoValue, "%-6d mWh", Battery_state.RemainingCapacity);
        
        sprintf(BAT1_Info[OS_BAT_Current].InfoValue, "%-6d mW", Battery_state.Rate);
        
        BAT1_Info[OS_BAT_RSOC].InfoInt = SysPowerState.BatteryLifePercent;
        sprintf(BAT1_Info[OS_BAT_RSOC].InfoValue, "%-6d %%", SysPowerState.BatteryLifePercent);
        
        sprintf(BAT1_Info[OS_BAT_Alert1_Low].InfoValue, "%-6d mWh", Battery_state.DefaultAlert1);
        sprintf(BAT1_Info[OS_BAT_Alert1_War].InfoValue, "%-6d mWh", Battery_state.DefaultAlert2);
    }
    else
    {
        sprintf(BAT1_Info[OS_AC].InfoValue, "%-18s", (Battery_state.AcOnLine)?("Adapter IN"):("Adapter OUT"));
        sprintf(BAT1_Info[OS_BAT].InfoValue, "%-18s", ("Battery OUT"));
        sprintf(BAT1_Info[OS_BAT_Charge].InfoValue, "%-18s", ("NO"));
        sprintf(BAT1_Info[OS_BAT_Discharge].InfoValue, "%-18s", ("NO"));
        sprintf(BAT1_Info[OS_BAT_Remtime].InfoValue, "%-6d min", 0);
        sprintf(BAT1_Info[OS_BAT_FCC].InfoValue, "%-6d mWh", 0);
        sprintf(BAT1_Info[OS_BAT_RMC].InfoValue, "%-6d mWh", 0);
        sprintf(BAT1_Info[OS_BAT_Current].InfoValue, "%-6d mW", 0);
        sprintf(BAT1_Info[OS_BAT_RSOC].InfoValue, "%-6d %%", 0);
        sprintf(BAT1_Info[OS_BAT_Alert1_Low].InfoValue, "%-6d mWh", 0);
        sprintf(BAT1_Info[OS_BAT_Alert1_War].InfoValue, "%-6d mWh", 0);
    }
}

void PrintLogFile(void)
{
    char tmp[64];
    int i;
    
    time_t t = time(0);
    strftime( tmp, sizeof(tmp), "%Y/%m/%d/%X",localtime(&t) );
    fprintf(BAT_LogFile, "%-22s" ,tmp);

    for(i=0;i<INFONAMECOUNT;i++)
    {
        if(BAT1_Info[i].ActiveLog)
        {
            fprintf(BAT_LogFile, "%-20d", BAT1_Info[i].InfoInt);
        }
    }
    fprintf(BAT_LogFile, "\n");
    fflush(BAT_LogFile);
}

void PrintBatteryInfo(void)
{
    int i,j;
    for(i=0,j=0;i<INFONAMECOUNT;i++)
    {
        if(BAT1_Info[i].Active)
        {
            SetPosition_X_Y(UI_BASE_X, UI_BASE_Y+j);
            SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
            printf("%s", BAT1_Info[i].InfoValue);
            j++;
        }
    }
}

void display(void)
{
    char i;
    //SetPosition_X_Y(UI_BASE_X, UI_BASE_Y+37);
    printf("\n                                          \r");
    for(i=0;i<10;i++)
    {
        printf("%c",'#');
        //Sleep(SetTime);
        _sleep(SetTime);   // millisecond
    }
}

int main(int argc, char *argv[])
{
    char IOInitOK=0;
    char ch=0;
    int i;

    IOInitOK = InitializeWinIo();
    if(IOInitOK)
    {
        SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
        printf("WinIo OK\n");
    }
    else
    {
        SetTextColor(EFI_LIGHTRED, EFI_BLACK);
        printf("Error during initialization of WinIo\n");
        goto IOError;
    }
    
    SetTextColor(EFI_WHITE, EFI_BLACK);
    system("cls");
    
    // Read battery.cfg file to init battery info address
    ReadCfgFile();

    if(NULL == CfgFile)
    {
        goto end;
    }

    ToolInit();
    InitBatteryInfo();
    PollBatteryInfo();
    PollOSBatteryInfo();

//---------------------------------------Creat log file---------------------------------------------
    if(BAT_LogFile)
    {
        time_t t = time(0);
        char tmp[64];
        strftime( tmp, sizeof(tmp), "%Y-%m-%d[%X]",localtime(&t) );
        tmp[13] = '.';
        tmp[16] = '.';
        strcat(tmp,"log.txt");
        BAT_LogFile = fopen(tmp,"w");
        
        fprintf(BAT_LogFile, "Battery Tool %s (For ITE %s EC code)\n",TOOLS_VER, ITE_IC);

        fprintf(BAT_LogFile, "EC current version is : %s\n", BAT1_Info[EC_Version].InfoValue);

        fprintf(BAT_LogFile, "This log file is the %s information of the battery pack\n", 
                            (BAT1_Info[BAT_AC_State].InfoInt)?("Charge"):("discharge"));

        fprintf(BAT_LogFile, "Set the data polling time  is %d(s)\n" ,SetTime/100);

        fprintf(BAT_LogFile, "The battery Capacity units is %s\n\n", 
                            (BAT1_Info[BAT_Mode].InfoInt&0x8000)?("mWh"):("mAh"));

        fprintf(BAT_LogFile, "%-22s", "Date&Time");
        
        for(i=0;i<INFONAMECOUNT;i++)
        {
            if(BAT1_Info[i].ActiveLog)
            {
                fprintf(BAT_LogFile, "%-20.18s", BAT1_Info[i].CfgItemName);
            }
        }
        fprintf(BAT_LogFile, "\n");
        fflush(BAT_LogFile);
    }
//--------------------------------------------------------------------------------------------------
    
    while(ESC!=ch)
    {
        PollBatteryInfo();
        PollOSBatteryInfo();
        
        if(BAT_LogFile)
        {
            PrintLogFile();
        }
        
        PrintBatteryInfo();
        
        display();
        
        if(kbhit())
        {
            ch=getch();
            if(ESC!=ch)
            {
                ch=0;
            }
        }
    }

    if(BAT_LogFile)
    {
        fclose(BAT_LogFile);
    }
    
    goto end;

IOError:
    ShutdownWinIo();
    return 1;
    
end:
ShutdownWinIo();
    return 0;
}