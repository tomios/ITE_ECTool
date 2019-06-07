#define  TOOLS_VER   "V0.4"

//*****************************************
// CCG5 PD FW Update Tool Version : 0.4
// 1. Optimize upgrade speed
//*****************************************

//*****************************************
// CCG5 PD FW Update Tool Version : 0.3
// 1. Notify EC, it will start update PD FW
//    66port command, 0x56 to disable EC access PD
//    66port command, 0x57 to enable EC access PD
//*****************************************

//*****************************************
// CCG5 PD FW Update Tool Version : 0.2
// 1. Add CCG5 FW Version Read Function
//*****************************************


//*****************************************
// CCG5 PD FW Update Tool Version : 0.1
// 1. Add CCG5 FW Update Function
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
//==================================================================================================

//========================================Type Define ==============================================
typedef unsigned char   BYTE;
typedef unsigned char   UINT8;
//typedef unsigned int    UINT16;
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
    Send_cmd_by_PM(0x81);     // 6C port write 0x81
    Send_data_by_PM(index);   // 68 port write data address
    Send_data_by_PM(data);    // 68 port write data
}
//--------------read EC RAM------------------------------------/
BYTE EC_ReadByte_PM(BYTE index)
{
    BYTE data;
    Send_cmd_by_PM(0x80);       // 6C port write 0x80
    Send_data_by_PM(index);     // 68 port write data address
    data = Read_data_from_PM(); // read 68 port, get data
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
#define  TOOLS_NAME  "CCG5 FW Update"
#define  ITE_IC      "CYPD5126"
#define  CopyRight   "(C)Copyright 2005-2020 ZXQ Telecom."
#define  TOOLS_AUTHOR "Morgen(zxqchongchi@gmail.com)"
#define  DEBUG       0
#define  ESC         0x1B

#define  PD_I2C_Start_RAM     0x9B0
#define  PD_I2C_Status_RAM    0x9B1
#define  PD_RegAddr01_RAM     0x9B2
#define  PD_RegAddr02_RAM     0x9B3
#define  PD_I2C_Count_RAM     0x9B4
#define  PD_SMB_DATA_RAM      0x9B5

#define  PD_I2C_Read      0x55
#define  PD_I2C_Write     0xAA
#define  PD_I2C_RW_OK     0xAA
#define  PD_I2C_RW_Fail   0x55
#define  PD_I2C_Free      0

BYTE   FW_Data[400][256];
UINT16 FW_Size;
    
FILE   *pPDFW_File;
BYTE    FW2_Addr_L;
BYTE    FW2_Addr_H;
//==================================================================================================
BYTE Clear_Device_INT(void)
{   
    // Control EC I2C read (INTR_REG)0x0006 PD Reg
    EC_RAM_WRITE(PD_RegAddr01_RAM, 0x06);
    EC_RAM_WRITE(PD_RegAddr02_RAM, 0x00);            // PD Reg is 0x0006
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x02);            // Read I2C data lenth is 2
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Read);     // Start EC I2C function
    
    _sleep(20);   // millisecond
    
    // Control EC I2C write (INTR_REG)0x0006 PD Reg
    EC_RAM_WRITE(PD_SMB_DATA_RAM, 0x06);
    EC_RAM_WRITE((PD_SMB_DATA_RAM+1), 0x00);          // PD Reg is 0x000A
    EC_RAM_WRITE((PD_SMB_DATA_RAM+2), 0x01);          // Data
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x03);             // Write I2C data lenth is 3
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Write);     // Start EC I2C function
    _sleep(20);   // millisecond
    
    return 0;
}


BYTE Check_Device_Response(void)
{
    BYTE WaitCount=0;
    BYTE PD_Cmd_TryCount=0;
    BYTE Response_Mode=0;
    
Check_Device_Re_Setp1:
    PD_Cmd_TryCount++;
    
    // Control EC I2C read (RESPONSE_REGISTER)0x007E PD Reg
    EC_RAM_WRITE(PD_RegAddr01_RAM, 0x7E);
    EC_RAM_WRITE(PD_RegAddr02_RAM, 0x00);            // PD Reg is 0x0000
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x02);            // Read I2C data lenth is 2
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Read);     // Start EC I2C function
    
    while(1)
    {
        _sleep(20);   // millisecond
        if(PD_I2C_RW_OK == EC_RAM_READ(PD_I2C_Status_RAM))
        {
            WaitCount=0;
            PD_Cmd_TryCount=0;
            break;
        }
        
        WaitCount++;
        if(WaitCount>3)
        {
            if(PD_Cmd_TryCount>3)
            {
                printf("Check Device Response Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
                return 1;
            }
            goto Check_Device_Re_Setp1;
        }
    }
    
    Response_Mode = EC_RAM_READ(PD_SMB_DATA_RAM);
    
    if(0x02 == (Response_Mode&0x7F)) // Response is 0x02, success
    {
        #if DEBUG
        printf("Check Device Response OK.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
        #endif
        return 0;
    }
    else
    {
        printf("Check Device Response Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
        printf("Current Response is [%#X]\n", Response_Mode);
        return 1;
    }
}


BYTE Read_FW2_Location(void)
{
    // Control EC I2C write 0x00 to (FIRMWARE_BINARY_LOCATION_REGISTER)0x0028 PD Reg
    EC_RAM_WRITE(PD_RegAddr01_RAM, 0x28);
    EC_RAM_WRITE(PD_RegAddr02_RAM, 0x00);            // PD Reg is 0x0028
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x04);            // Read I2C data lenth is 4
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Read);     // Start EC I2C function
    
    _sleep(1000);   // millisecond
    
    FW2_Addr_H = EC_RAM_READ(PD_SMB_DATA_RAM+2);
    FW2_Addr_L = EC_RAM_READ(PD_SMB_DATA_RAM+3);
    
    #if DEBUG
    printf("FW2 location is [%#X][%#X]\n",FW2_Addr_H, FW2_Addr_L);
    #endif
    
    return 0;
}

BYTE TypeC_Port_Disable(void)
{
    BYTE WaitCount=0;
    BYTE PD_Cmd_TryCount=0;
Disable_TypeC_Setp1:
    PD_Cmd_TryCount++;
    
    // Control EC I2C write 0x00 to (PDPORT_ENABLE)0x002C PD Reg
    EC_RAM_WRITE(PD_SMB_DATA_RAM, 0x2C);
    EC_RAM_WRITE((PD_SMB_DATA_RAM+1), 0x00);          // PD Reg is 0x002C
    EC_RAM_WRITE((PD_SMB_DATA_RAM+2), 0x00);          // Data
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x03);             // Write I2C data lenth is 3
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Write);     // Start EC I2C function
    
    while(1)
    {
        _sleep(20);   // millisecond
        if(PD_I2C_RW_OK == EC_RAM_READ(PD_I2C_Status_RAM))
        {
            WaitCount=0;
            PD_Cmd_TryCount=0;
            break;
        }
        
        WaitCount++;
        if(WaitCount>3)
        {
            if(PD_Cmd_TryCount>3)
            {
                printf("Disable TypeC Port Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
                return 1;
            }
            goto Disable_TypeC_Setp1;  // 
        }
    }
    
    #if DEBUG
    printf("Disable TypeC Port OK.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
    #endif
    
    return 0;
}

BYTE Jump_ALT_FW(void)
{
    BYTE WaitCount=0;
    BYTE PD_Cmd_TryCount=0;
    
Jump_ALT_Setp1:
    PD_Cmd_TryCount++;
    
    // Control EC I2C write 0x00 to (JUMP_TO_BOOT)0x0007 PD Reg
    EC_RAM_WRITE(PD_SMB_DATA_RAM, 0x07);
    EC_RAM_WRITE((PD_SMB_DATA_RAM+1), 0x00);          // PD Reg is 0x0007
    EC_RAM_WRITE((PD_SMB_DATA_RAM+2), 0x41);          // Data ‘A’, jump to FW1
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x03);             // Write I2C data lenth is 3
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Write);     // Start EC I2C function
    
    while(1)
    {
        _sleep(20);   // millisecond
        if(PD_I2C_RW_OK == EC_RAM_READ(PD_I2C_Status_RAM))
        {
            WaitCount=0;
            PD_Cmd_TryCount=0;
            break;
        }
        
        WaitCount++;
        if(WaitCount>3)
        {
            if(PD_Cmd_TryCount>3)
            {
                printf("Change to FW1 Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
                return 1;
            }
            goto Jump_ALT_Setp1;
        }
    }
    
    #if DEBUG
    printf("Change to FW1 OK.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
    #endif
    return 0;
}

BYTE Check_FW1_Activity(void)
{
    BYTE WaitCount=0;
    BYTE PD_Cmd_TryCount=0;
    BYTE Device_Mode=0;
    
FW1_Act_Setp1:
    PD_Cmd_TryCount++;
    
    // Control EC I2C read (DEVICE_MODE)0x0000 PD Reg
    EC_RAM_WRITE(PD_RegAddr01_RAM, 0x00);
    EC_RAM_WRITE(PD_RegAddr02_RAM, 0x00);            // PD Reg is 0x0000
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x02);            // Read I2C data lenth is 2
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Read);     // Start EC I2C function
    
    while(1)
    {
        _sleep(50);   // millisecond
        if(PD_I2C_RW_OK == EC_RAM_READ(PD_I2C_Status_RAM))
        {
            WaitCount=0;
            PD_Cmd_TryCount=0;
            break;
        }
        
        WaitCount++;
        if(WaitCount>3)
        {
            if(PD_Cmd_TryCount>3)
            {
                printf("Check Current FW1 action Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
                return 1;
            }
            goto FW1_Act_Setp1;
        }
    }
    
    Device_Mode = EC_RAM_READ(PD_SMB_DATA_RAM);
    
    if(0x01 == (Device_Mode&0x03))  // Current action FW is FW1
    {
        #if DEBUG
        printf("Check Current FW1 action OK.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
        #endif
        return 0;
    }
    else if(0x02 == (Device_Mode&0x03))
    {
        printf("Current action is  FW2\n");
        printf("Current Device mode is [%#X]\n", Device_Mode);
        return 1;
    }
    else
    {
        printf("Current action is  Boot Mode\n");
        printf("Current Device mode is [%#X]\n", Device_Mode);
        return 1;
    }
}

BYTE Enter_Flash_Mode(void)
{
    BYTE WaitCount=0;
    BYTE PD_Cmd_TryCount=0;
    
Flash_Mode_Setp1:
    PD_Cmd_TryCount++;
    
    // Control EC I2C write 0x00 to (ENTER_FLASHING_MODE)0x000A PD Reg
    EC_RAM_WRITE(PD_SMB_DATA_RAM, 0x0A);
    EC_RAM_WRITE((PD_SMB_DATA_RAM+1), 0x00);          // PD Reg is 0x000A
    EC_RAM_WRITE((PD_SMB_DATA_RAM+2), 0x50);          // Data ‘P’, Enter program
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x03);             // Write I2C data lenth is 3
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Write);     // Start EC I2C function
    
    while(1)
    {
        _sleep(50);   // millisecond
        if(PD_I2C_RW_OK == EC_RAM_READ(PD_I2C_Status_RAM))
        {
            WaitCount=0;
            PD_Cmd_TryCount=0;
            break;
        }
        
        WaitCount++;
        if(WaitCount>3)
        {
            if(PD_Cmd_TryCount>3)
            {
                printf("Enter Flash Mode Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
                return 1;
            }
            goto Flash_Mode_Setp1;
        }
    }
    
    #if DEBUG
    printf("Enter Flash Mode OK.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
    #endif
    
    // Read INT and Clear INT
    if(Clear_Device_INT())
    {
        return 1;
    }
    _sleep(50);   // millisecond
    
    
    // Check Device Response
    if(Check_Device_Response())
    {
        _sleep(500);   // millisecond
        #if DEBUG
        system("pause");
        #endif
        return 1;
    }
    
    return 0;
}

BYTE Write_256Byte(UINT16 PageNum, BYTE ROW_H, BYTE ROW_L)
{
    UINT16 i,j;
    for(i=0; i<8; i++)
    {
        for(j=0; j<32; j++)
        {
            EC_RAM_WRITE(PD_SMB_DATA_RAM+2+j, FW_Data[PageNum][i*32+j]);     // Start EC I2C function
            
            #if DEBUG
            if(15==j)
            {
                printf("\n");
            }
            printf("%02X ", FW_Data[PageNum][i*32+j]);
            #endif
        }
        #if DEBUG
        printf("\n");
        #endif
        
        EC_RAM_WRITE(PD_SMB_DATA_RAM, (i*32));
        EC_RAM_WRITE((PD_SMB_DATA_RAM+1), 0x02);        // Addr is 0200
        EC_RAM_WRITE(PD_I2C_Count_RAM, 34);             // Write I2C data lenth is 34
        EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Write);   // Start EC I2C function
        _sleep(15);   // millisecond
    }
    #if DEBUG
    printf("\n");
    #endif
    _sleep(20);   // millisecond
    // Control EC I2C write 0x46 to (FLASH_ROW_READ_WRITE)0x000C PD Reg, trigger to flash
    EC_RAM_WRITE(PD_SMB_DATA_RAM, 0x0C);
    EC_RAM_WRITE((PD_SMB_DATA_RAM+1), 0x00);          // PD Reg is 0x000C
    EC_RAM_WRITE((PD_SMB_DATA_RAM+2), 0x46);          // Data ‘F’, Enter program
    EC_RAM_WRITE((PD_SMB_DATA_RAM+3), 0x01);          // Data 0x01, Write
    EC_RAM_WRITE((PD_SMB_DATA_RAM+4), ROW_L);         // ROW number High Byte
    EC_RAM_WRITE((PD_SMB_DATA_RAM+5), ROW_H);         // ROW number Low Byte
    
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x06);             // Write I2C data lenth is 6
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Write);     // Start EC I2C function
    
    _sleep(60);   // millisecond
    
    #if DEBUG
    printf("Write ROW is [%#X][%#X]\n", ROW_H, ROW_L);
    #endif
   
    
    // Read INT and Clear INT
    if(Clear_Device_INT())
    {
        printf("Write ROW Clear INT Error\n");
        return 1;
    }

    // Check Device Response
    if(Check_Device_Response())
    {
        printf("Write ROW Response Error\n");
        _sleep(50);   // millisecond
        #if DEBUG
        system("pause");
        #endif
        return 1;
    }
    
    return 0;
}

BYTE Validate_FW()
{
    BYTE WaitCount=0;
    BYTE PD_Cmd_TryCount=0;
    
Validate_Setp1:
    PD_Cmd_TryCount++;
    
    // Control EC I2C write 0x02 to (VALIDATE_FW)0x000B PD Reg
    EC_RAM_WRITE(PD_SMB_DATA_RAM, 0x0B);
    EC_RAM_WRITE((PD_SMB_DATA_RAM+1), 0x00);          // PD Reg is 0x000B
    EC_RAM_WRITE((PD_SMB_DATA_RAM+2), 0x02);          // Data 0x02, FW2
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x03);             // Write I2C data lenth is 3
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Write);     // Start EC I2C function
    
    while(1)
    {
        _sleep(50);   // millisecond
        if(PD_I2C_RW_OK == EC_RAM_READ(PD_I2C_Status_RAM))
        {
            WaitCount=0;
            PD_Cmd_TryCount=0;
            break;
        }
        
        WaitCount++;
        if(WaitCount>3)
        {
            if(PD_Cmd_TryCount>3)
            {
                printf("VALIDATE_FW Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
                return 1;
            }
            goto Validate_Setp1;
        }
    }
    
    #if DEBUG
    printf("VALIDATE_FW OK.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
    #endif
    
    // Read INT and Clear INT
    if(Clear_Device_INT())
    {
        return 1;
    }
    _sleep(50);   // millisecond
    
    
    // Check Device Response
    if(Check_Device_Response())
    {
        _sleep(500);   // millisecond
        #if DEBUG
        system("pause");
        #endif
        return 1;
    }
    
    return 0;
}

BYTE Reset_CCG5()
{
    BYTE WaitCount=0;
    BYTE PD_Cmd_TryCount=0;
    BYTE Device_Mode=0;
    
Reset_CCG5_Setp1:
    PD_Cmd_TryCount++;
    
    // Control EC I2C write 0x02 to (RESET)0x0008 PD Reg
    EC_RAM_WRITE(PD_SMB_DATA_RAM, 0x08);
    EC_RAM_WRITE((PD_SMB_DATA_RAM+1), 0x00);          // PD Reg is 0x0008
    EC_RAM_WRITE((PD_SMB_DATA_RAM+2), 0x52);          // Data ‘R’, Enter program
    EC_RAM_WRITE((PD_SMB_DATA_RAM+3), 0x01);          // Data 0x01, Reset Device
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x04);             // Write I2C data lenth is 4
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Write);     // Start EC I2C function
    
    while(1)
    {
        _sleep(50);   // millisecond
        if(PD_I2C_RW_OK == EC_RAM_READ(PD_I2C_Status_RAM))
        {
            WaitCount=0;
            PD_Cmd_TryCount=0;
            break;
        }
        
        WaitCount++;
        if(WaitCount>3)
        {
            if(PD_Cmd_TryCount>3)
            {
                printf("Reset CCG5 Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
                return 1;
            }
            goto Reset_CCG5_Setp1;
        }
    }
    
    return 0;
}

void Update_PD_FW(void)
{
    UINT16 i,j;
    UINT16 row_h, row_l;
    //===================================================================================
    // CCG5 Update flow
    
    // Read FW2 Location
    if(Read_FW2_Location())
    {
        return;
    }
    _sleep(200);   // millisecond
    #if DEBUG
    system("pause");
    #endif
    printf("Read FW location Pass\n");
    
    // Disable Type-C port
    if(TypeC_Port_Disable())
    {
        return;
    }
    _sleep(1000);   // millisecond
    #if DEBUG
    system("pause");
    #endif
    printf("Disable Type_C Port Pass\n");
    
    if(Check_FW1_Activity())
    {
        // Change to FW1
        if(Jump_ALT_FW())
        {
            return;
        }
        _sleep(1000);   // millisecond
        #if DEBUG
        system("pause");
        #endif
        printf("Jump FW1 Pass\n");
        
        // Check jump to FW1
        if(Check_FW1_Activity())
        {
            return;
        }
        _sleep(1000);   // millisecond
        #if DEBUG
        system("pause");
        #endif
        printf("Current action FW is FW1\n");
    }
    else
    {
        printf("Current action FW1, Set FW2 address is 0xC000\n");
        FW2_Addr_H = 0xC0;
        FW2_Addr_L = 0x00;
    }
    
    // Enter Flash Mode
    if(Enter_Flash_Mode())
    {
        return;
    }
    _sleep(1000);   // millisecond
    #if DEBUG
    system("pause");
    #endif
    printf("Enter Flash Mode Pass\n");
    
    row_h = 0;
    row_l = FW2_Addr_H;
    printf("[.....................................................................]\r[");
    for(i=0; i<FW_Size; i++)
    {
        if(Write_256Byte(i, row_h, row_l))
        {
            return;
        }
        row_l++;
        if(256==row_l)
        {
            row_l = 0;
            row_h++;
        }
        
        #if (!DEBUG)
        if(0==(i%5))
        {
            printf("@");
            printf("[%02d%%]\b\b\b\b\b", ((i*100)/FW_Size));
        }
        #endif
    }
    #if (!DEBUG)
    printf("[%02d%%]\b\b\b\b\b", 100);
    #endif
    
    //Validate FW
    if(Validate_FW())
    {
        return;
    }
    _sleep(1000);   // millisecond
    #if DEBUG
    system("pause");
    #endif
    
    // Reset PD
    if(Reset_CCG5())
    {
        return;
    }
}


BYTE Read_PDFW_ToBuffer(char *FileName)
{
    UINT16 len1;
    UINT16 i;
    FILE   *debuglog;
    #if DEBUG
    printf("File Name : %s\n",FileName);
    #endif
    
    if((pPDFW_File = fopen((const char*)FileName, "rb")) == NULL)
    {
        printf("open PD FW file wrong!\n");
        return(FALSE);
    }
    
    debuglog = fopen("Debug.log","w");
    if(debuglog == NULL)
    {
        printf("Creat debuglog file Fail\n\n");
        return(FALSE);
    }
    
    FW_Size = 0;
    while (!feof(pPDFW_File))
    {
        len1=fread(FW_Data[FW_Size], 1, 0x100, pPDFW_File);

        #if DEBUG
        for(i=0; i<256; i++)
        {
            if(0==(i%16))
            {
                fprintf(debuglog, "\n");
            }
            fprintf(debuglog, "%02X ", FW_Data[FW_Size][i]);
        }
        fprintf(debuglog, "\n%02X\n",FW_Size);
        #endif
        
        FW_Size++;
    }
    FW_Size--;
    
    #if DEBUG
    printf("FWSize=[%d]K\n", FW_Size);
    #endif

    fclose(pPDFW_File);
    fclose(debuglog);
    
    return(TRUE);
}

BYTE Read_Device_Mode(void)
{
    BYTE WaitCount=0;
    BYTE PD_Cmd_TryCount=0;
    BYTE Ver_Byte1=0;
    
Read_FW_Mode_Setp1:
    PD_Cmd_TryCount++;
    
    // Control EC I2C read (DEVICE_MODE)0x0000 PD Reg
    EC_RAM_WRITE(PD_RegAddr01_RAM, 0x00);
    EC_RAM_WRITE(PD_RegAddr02_RAM, 0x00);            // PD Reg is 0x0020
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x03);            // Read I2C data lenth is 3
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Read);     // Start EC I2C function
    
    while(1)
    {
        _sleep(50);   // millisecond
        if(PD_I2C_RW_OK == EC_RAM_READ(PD_I2C_Status_RAM))
        {
            WaitCount=0;
            PD_Cmd_TryCount=0;
            break;
        }
        
        WaitCount++;
        if(WaitCount>3)
        {
            if(PD_Cmd_TryCount>3)
            {
                printf("Read FW Mode Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
                return 1;
            }
            goto Read_FW_Mode_Setp1;
        }
    }
    
    Ver_Byte1 = EC_RAM_READ(PD_SMB_DATA_RAM);
    
    printf("Device Mode: [%#X]", Ver_Byte1);
    if(0x80==(Ver_Byte1&0x80))
    {
        printf("[HPIv2]");
    }
    else
    {
        printf("[HPIv1]");
    }
    
    if(0==(Ver_Byte1&0x30))
    {
        printf("[128K]");
    }
    else if(0x10==(Ver_Byte1&0x30))
    {
        printf("[256K]");
    }
    else
    {
        printf("[Other]");
    }
    
    if(0==(Ver_Byte1&0x0C))
    {
        printf("[1-Port]");
    }
    else if(0x04==(Ver_Byte1&0x0C))
    {
        printf("[2-Port]");
    }
    else
    {
        printf("[Other]");
    }
    
    if(0==(Ver_Byte1&0x03))
    {
        printf("[BootMode]\n");
    }
    else if(1==(Ver_Byte1&0x03))
    {
        printf("[FW1]\n");
    }
    else
    {
        printf("[FW2]\n");
    }

    return 0;
}

BYTE Read_FW_Ver(void)
{
    BYTE WaitCount=0;
    BYTE PD_Cmd_TryCount=0;
    BYTE Ver_Byte1=0;
    BYTE Ver_Byte2=0;
    BYTE Ver_Byte3=0;
    BYTE Ver_Byte4=0;
    BYTE Ver_Byte5=0;
    BYTE Ver_Byte6=0;
    BYTE Ver_Byte7=0;
    BYTE Ver_Byte8=0;
    
Read_FW_Ver_Setp1:
    PD_Cmd_TryCount++;
    
    // Control EC I2C read (FW2_VERSION)0x0020 PD Reg, to read FW2 version
    EC_RAM_WRITE(PD_RegAddr01_RAM, 0x20);
    EC_RAM_WRITE(PD_RegAddr02_RAM, 0x00);            // PD Reg is 0x0020
    EC_RAM_WRITE(PD_I2C_Count_RAM, 0x08);            // Read I2C data lenth is 8
    EC_RAM_WRITE(PD_I2C_Start_RAM, PD_I2C_Read);     // Start EC I2C function
    
    while(1)
    {
        _sleep(50);   // millisecond
        if(PD_I2C_RW_OK == EC_RAM_READ(PD_I2C_Status_RAM))
        {
            WaitCount=0;
            PD_Cmd_TryCount=0;
            break;
        }
        
        WaitCount++;
        if(WaitCount>3)
        {
            if(PD_Cmd_TryCount>3)
            {
                printf("Read FW2 Version Fail.WaitCount=[%d], TryCount=[%d]\n",WaitCount, PD_Cmd_TryCount);
                return 1;
            }
            goto Read_FW_Ver_Setp1;
        }
    }
    
    Ver_Byte1 = EC_RAM_READ(PD_SMB_DATA_RAM);
    Ver_Byte2 = EC_RAM_READ(PD_SMB_DATA_RAM+1);
    Ver_Byte3 = EC_RAM_READ(PD_SMB_DATA_RAM+2);
    Ver_Byte4 = EC_RAM_READ(PD_SMB_DATA_RAM+3);
    Ver_Byte5 = EC_RAM_READ(PD_SMB_DATA_RAM+4);
    Ver_Byte6 = EC_RAM_READ(PD_SMB_DATA_RAM+5);
    Ver_Byte7 = EC_RAM_READ(PD_SMB_DATA_RAM+6);
    Ver_Byte8 = EC_RAM_READ(PD_SMB_DATA_RAM+7);
    
    printf("PD_FW_Ver: %02X%02X%02X%02X%02X%02X%02X%02X", 
                       Ver_Byte1,Ver_Byte2,Ver_Byte3,Ver_Byte4,Ver_Byte5,Ver_Byte6,Ver_Byte7,Ver_Byte8);
    
    return 0;
}


void help(void)
{
    printf("============================================================\n");
    printf("=         %s FW Update Utility Version : %s        =\n",ITE_IC,TOOLS_VER);
    printf("=        %s            =\n",CopyRight);
    printf("=                 All Rights Reserved.                     =\n");
    printf("=                             --%s                =\n", __DATE__);
    printf("=                                                          =\n");
    printf("=      [/R_FW]            Read Current PD FW binary        =\n");
    printf("=      [/R_VER]           Read Current PD FW Version       =\n");
    printf("=      [/W_FW  PDFW.bin]  Update PD FW                     =\n");
    printf("============================================================\n");
}

//==================================================================================================
UINT16 main(UINT16 argc, char *argv[])
{
    BYTE IOInitOK=0;
    BYTE PD_Action=0;
    BYTE PD_Update_Flag=0;
    
    if (argc == 1)
    {
        goto ArgcError;
    }
    
    if(!strcmp("/R_FW",argv[1]))
    {
        PD_Action=1;
    }
    if(!strcmp("/R_VER",argv[1]))
    {
        PD_Action=2;
    }
    if(!strcmp("/W_FW",argv[1]))
    {
        PD_Action=3;
    }
    
    system("cls");
    
    printf("============================================================\n");
    printf("=         %s FW Update Utility Version : %s        =\n",ITE_IC,TOOLS_VER);
    printf("=        %s            =\n",CopyRight);
    printf("=                 All Rights Reserved.                     =\n");
    printf("=                             --%s                =\n", __DATE__);
    printf("=                                                          =\n");
    printf("============================================================\n");
    
    // Init IO port
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
    
    Send_cmd_by_PM(0x56);  //EC must stop PD register access after receive this command
    
    if(3==PD_Action)
    {
        if(TRUE == Read_PDFW_ToBuffer(argv[2]))
        {
            Update_PD_FW();
        }
    }
    else if(2==PD_Action)
    {
        Read_Device_Mode();
        Read_FW_Ver();
    }
    else
    {
        printf("Not support\n");
    }
    
    Send_cmd_by_PM(0x57);  //EC can access PD register after receive this command
    
    
    goto end;
    
ArgcError:
    ShutdownWinIo();
    help();
    return 1;

IOError:
    ShutdownWinIo();
    return 1;
    
end:
ShutdownWinIo();
    return 0;
}