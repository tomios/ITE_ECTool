#define  TOOLS_VER   "V0.8"
//*****************************************
// eFlashDebug Version : 0.8
// 1. modify Analysis_file()
//*****************************************

//*****************************************
// eFlashDebug Version : 0.6
// 1. Add eFlash Debug config file creat and modify
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
#define  TOOLS_NAME  "eFlashDebug"
#define  ITE_IC      "ITE8987"
#define  CopyRight   "(C)Copyright 2005-2020 ZXQ Telecom."
#define  TOOLS_AUTHOR "Morgen(zxqchongchi@gmail.com)"
#define  DEBUG       0
#define  ESC         0x1B
//==================================================================================================
typedef struct debugcode
{
    unsigned char CodeArray[4];
    unsigned char MessageInfo[128];
    unsigned int  MessageCount;
    struct debugcode *NextCode;
}DebugCodeStruct;

FILE *Binary_LogFile = NULL;
FILE *Analysis_LogFile = NULL;
FILE *CfgFile = NULL;
unsigned char ReadFlag=0;
unsigned int SetTime;

unsigned char eFlah_Block[4][1024];   // Link to 0x1E000
unsigned char eFlash_Current_Blcok;
unsigned char eFlash_Current_Page;
unsigned int  eFlash_Current_Index;
unsigned int  Message_num;
unsigned char Dump_Flag;
unsigned char Analysis_Flag;
DebugCodeStruct *pFirstCode;



DebugCodeStruct DebugCodeList[]=
{
    {{0x70, 0x01, 0x00, 0x00}, "EC Init OK"},
    {{0x70, 0x02, 0xEC, 0x00}, "EC code reset(0xEC)"},
    {{0x70, 0x02, 0xFD, 0x00}, "external watch-dog(0xFD)"},
    {{0x70, 0x02, 0xFE, 0x00}, "internal watch-dog(0xFE)"},
    {{0x70, 0x02, 0xFC, 0x00}, "EC VSTBY or WRST reset(0xFC)"},
    
    {{0x80, 0x01, 0xFF, 0xFF}, "Power On sequence Fail, Current state and Faile Step is:"},
    {{0x80, 0x02, 0x30, 0x00}, "S0->S3, System enter S3 by S3 auto off(0x30)"},
    {{0x80, 0x03, 0x01, 0x00}, "S0->S5, System shutdown by S3/S4/S5 Off(0x01)"},
    {{0x80, 0x03, 0x02, 0x00}, "S0->S5, System shutdown by S3/S4/S5 fail Off(0x02)"},
    {{0x80, 0x03, 0x03, 0x00}, "S0->S5, System shutdown by PSW 4 sec overwrite(0x03)"},
    {{0x80, 0x03, 0x04, 0x00}, "S0->S5, System shutdown by CPU too hot to shutdown(0x04)"},
    {{0x80, 0x03, 0x07, 0x00}, "S0->S5, System shutdown by Power on WDT(0x07)"},
    {{0x80, 0x03, 0x09, 0x00}, "S0->S5, System shutdown by Thermal Trip(0x09)"},
    {{0x80, 0x04, 0x00, 0x00}, "S0->S5, System shutdown by crisis recovery"},
    {{0x80, 0x05, 0x00, 0x00}, "EC Update OK and auto power on"},
    {{0x80, 0x06, 0x01, 0x00}, "TXE unLock power off"},
    {{0x80, 0x06, 0x02, 0x00}, "TXE unLock power on"},
    {{0x80, 0x07, 0x10, 0x00}, "Current system state is: S0"},
    {{0x80, 0x07, 0x33, 0x00}, "Current system state is: S3"},
    {{0x80, 0x07, 0x55, 0x00}, "Current system state is: S5"},
    {{0x80, 0x07, 0x50, 0x00}, "Current system state is: S5->S0"},
    {{0x80, 0x07, 0x30, 0x00}, "Current system state is: S3->S0"},
    {{0x80, 0x07, 0x03, 0x00}, "Current system state is: S0->S3"},
    {{0x80, 0x07, 0x05, 0x00}, "Current system state is: S0->S5"},
    {{0x80, 0x08, 0x00, 0x00}, "Cold boot time out, auto power on"},
    {{0x80, 0x09, 0x00, 0x00}, "S5, Press power button show logo"},
    {{0x80, 0x0A, 0x00, 0x00}, "S5, Press power button Skip show logo"},
    {{0x80, 0x0B, 0x00, 0x00}, "Press power button power on"},
    {{0x80, 0x0C, 0x00, 0x00}, "S3 state, battery low power, auto resume to S0"},
    {{0x80, 0x0D, 0x01, 0x00}, "S5, Press power button, AC out, power on"},
    {{0x80, 0x0D, 0x02, 0x00}, "S5, Press power button, AC in, power on"},
    {{0x80, 0x0D, 0x03, 0x00}, "S5, S3/S4 auto high, and power on"},
    {{0x80, 0x0E, 0x01, 0x00}, "S3, S3 auto high, and power on"},
    
    {{0x81, 0x01, 0x01, 0x00}, "Power button Press"},
    {{0x81, 0x01, 0x02, 0x00}, "Power button Release"},
    {{0x81, 0x02, 0x01, 0x00}, "AC in"},
    {{0x81, 0x02, 0x02, 0x00}, "AC out"},
    
    {{0x82, 0x01, 0xFF, 0xFF}, "ProcHot Trigger, T="},
    {{0x82, 0x02, 0xFF, 0xFF}, "ProcHot recovery, T="},
    {{0x82, 0x03, 0xFF, 0xFF}, "OverTemperature, Shutdown T="},
    
    {{0x83, 0xFF, 0xFF, 0xFF}, "Post code is:"},
    
    {{0x84, 0x01, 0x00, 0x00}, "wake up charge fail(0x01)"},
    {{0x84, 0x02, 0x00, 0x00}, "Precharge fail(0x02)"},
    {{0x84, 0x03, 0x00, 0x00}, "Normal charge fail(0x03)"},
    {{0x84, 0x04, 0x00, 0x00}, "Battery wake fail(0x04)"},
    {{0x84, 0x05, 0xFF, 0xFF}, "Battery OverVoltage(0x05)"},
    {{0x84, 0x06, 0x00, 0x00}, "Battery OverTemperature(0x06)"},
    {{0x84, 0x07, 0x00, 0x00}, "Battery not support(0x07)"},
    {{0x84, 0x08, 0x00, 0x00}, "Battery Authentication fail(0x08)"},
    {{0x84, 0x09, 0x00, 0x00}, "Battery enter Shipmode(0x09)"},
    {{0, 0, 0, 0}, NULL},
};

void ToolInit(void)
{
    int i,j;
    SetConsoleTitle(TOOLS_NAME);
    system("mode con cols=85 lines=30");
    
    eFlash_Current_Blcok = 0;
    eFlash_Current_Page = 0;
    eFlash_Current_Index = 0;
    Dump_Flag = 0;
    Analysis_Flag = 0;
    Message_num = 0;
}

void help(void)
{
    printf("============================================================\n");
    printf("=         eFlash Debug Dump Utility Version : %s         =\n",TOOLS_VER);
    printf("=        %s            =\n",CopyRight);
    printf("=                 All Rights Reserved.                     =\n");
    printf("=                             --%s                =\n", __DATE__);
    printf("=                                                          =\n");
    printf("=      If only Analysis file, please input file name       =\n");
    printf("=      example : eFlashDebug.exe /A  log.bin               =\n");
    printf("=      If only Dump EC Log from eFlash                     =\n");
    printf("=      example : eFlashDebug.exe /D                        =\n");
    printf("============================================================\n");
}


//==================================================================================================
// This is for read log data form IT8987 eFlash
// Read step:
//      1. Send 0xAA to EC flag, EC will read 256Byte eFlash data to EC RAM
//      2. EC modify flag 0xAA to 0x55 when it read complete
//      3. This tool read flag is 0x55, and then read 256Byte from EC RAM
//      4. Repeat the above steps 1-3
//==================================================================================================
void Read_eFlashData(void)
{
    char tmp[64];
    unsigned char eFlah_PageData[256];
    
    printf("\nStart Dump eFlash\n");
    printf("  ");
    
    //creat log file
    time_t t = time(0);
    strftime( tmp, sizeof(tmp), "%Y-%m-%d[%X]",localtime(&t) );
    tmp[13] = '.';
    tmp[16] = '.';
    strcat(tmp,"log.bin");
    Binary_LogFile = fopen(tmp,"wb");
    
    printf("[................................]\r[");
    
    // read data
    for(eFlash_Current_Blcok=0; eFlash_Current_Blcok<4; eFlash_Current_Blcok++)
    {
        EC_RAM_WRITE(0x983, eFlash_Current_Blcok);
        for(eFlash_Current_Page=0; eFlash_Current_Page<4; eFlash_Current_Page++)
        {
            EC_RAM_WRITE(0x984, eFlash_Current_Page);
            EC_RAM_WRITE(0x985, 0xAA); // Flag EC to start read eFlash
            
            // wait EC read eFlash
            while(1)
            {
                _sleep(SetTime);   // millisecond
                if(0x55==EC_RAM_READ(0x985))
                {
                    break;
                }
            }
            
            // read eFlash data from EC RAM
            for(eFlash_Current_Index=0; eFlash_Current_Index<256; eFlash_Current_Index++)
            {
                eFlah_PageData[eFlash_Current_Index] = EC_RAM_READ(0xC00+eFlash_Current_Index);
            }
            
            fwrite(eFlah_PageData,1,256,Binary_LogFile);
            
            printf("@");
            printf("@");
            
            printf("[%02d%%]\b\b\b\b\b", ((eFlash_Current_Blcok*4 + eFlash_Current_Page + 1)*100/16));
        }
    }
    
    EC_RAM_WRITE(0x985, 0x00); // Flag EC to stop read eFlash
    
    printf("\nDump eFlash Log OK!\n\n");
    
    fclose(Binary_LogFile);
}

//==================================================================================================
void Analysis_file(char *Filename)
{
    int i,j,k,l;
    unsigned char block_status[4];
    unsigned char temp_block_num;
    unsigned char a1, a2, a3, a4;
    DebugCodeStruct *tempCode;
    DebugCodeStruct *indexCode;
    char AnalysisFileName[32]={0};
    char *postfix=".txt";
    
    printf("Binary log file name is : %s \n",Filename);
    printf("\n Start Analysis binary log file\n");
    printf("  ");
    
    Binary_LogFile = fopen(Filename,"rb"); // Binary file read, the open method must be "rb"
    if(NULL==Binary_LogFile)
    {
        printf("\n Open Log file error\n");
        return;
    }
    
    strcat(AnalysisFileName, Filename);
    strcat(AnalysisFileName, postfix);
    //printf("File name: %s\n", AnalysisFileName);
    Analysis_LogFile = fopen(AnalysisFileName,"w");
    
    if(NULL==Analysis_LogFile)
    {
        fclose(Binary_LogFile);
        printf("\n Creat Log file error\n");
        return;
    }
    
    // read 4K data from binary file
    for(i=0; i<4; i++)
    {
        j= fread(eFlah_Block[i], 1, 1024, Binary_LogFile);
        //printf("Size = %d\n", j);
    }
    fclose(Binary_LogFile);

    //================================================
    /*
    for(i=0; i<4; i++)
    {
        for(j=0; j<1024; j++)
        {
            if(0==(j%16))
            {
                printf("\n");
            }
            
            if(0==(j%256))
            {
                printf("\n\n");
            }
            
            printf("%#04X ", eFlah_Block[i][j]);
            
        }
        printf("\n");
        printf("\n");
    }
    */
    //================================================
//=================================================================================

    for(eFlash_Current_Blcok=0; eFlash_Current_Blcok<4; eFlash_Current_Blcok++)
    {
        for(i=0; i<4; i++)
        {
            for(j=8; j<256; j+=4)
            {
                a1 = eFlah_Block[eFlash_Current_Blcok][i*256+j];
                a2 = eFlah_Block[eFlash_Current_Blcok][i*256+j+1];
                a3 = eFlah_Block[eFlash_Current_Blcok][i*256+j+2];
                a4 = eFlah_Block[eFlash_Current_Blcok][i*256+j+3];
                
                if((0xFF==a1) && (0xFF==a2) && (0xFF==a3) && (0xFF==a4))
                {
                    if(3==eFlash_Current_Blcok)
                    {
                        eFlash_Current_Blcok=0;
                    }
                    else if(eFlash_Current_Blcok<3)
                    {
                        temp_block_num = eFlash_Current_Blcok+1;
                        a1 = eFlah_Block[temp_block_num][8];
                        a2 = eFlah_Block[temp_block_num][9];
                        a3 = eFlah_Block[temp_block_num][10];
                        a4 = eFlah_Block[temp_block_num][11];
                        if((0xFF==a1) && (0xFF==a2) && (0xFF==a3) && (0xFF==a4))
                        {
                            eFlash_Current_Blcok=0;
                        }
                        else if((0==a1) && (0==a2) && (0==a3) && (0==a4))
                        {
                            eFlash_Current_Blcok=0;
                        }
                        else
                        {
                            eFlash_Current_Blcok = temp_block_num;
                        }
                    }
                    //printf("Blcok-page-index: %X = %X = %X\n", eFlash_Current_Blcok, i, j);
                    goto CheckEnd;
                }
            }
        }
    }

CheckEnd:
    //printf("Blcok-page-index: %X = %X = %X\n", eFlash_Current_Blcok, i, j);
    //system("pause");
//===========================================================================    

    fprintf(Analysis_LogFile, "eFlash Debug Dump Utility Version : %s    \n\n",TOOLS_VER);
    
    eFlash_Current_Page = 0;
    eFlash_Current_Index = 8;
    while(1)
    {
        // Get 4Byte debug code
        a1 = eFlah_Block[eFlash_Current_Blcok][eFlash_Current_Page*256+eFlash_Current_Index];
        a2 = eFlah_Block[eFlash_Current_Blcok][eFlash_Current_Page*256+eFlash_Current_Index+1];
        a3 = eFlah_Block[eFlash_Current_Blcok][eFlash_Current_Page*256+eFlash_Current_Index+2];
        a4 = eFlah_Block[eFlash_Current_Blcok][eFlash_Current_Page*256+eFlash_Current_Index+3];
        
        // This is for debug
        //printf("block:%X#, page:%#X, index:%#X,  Data: %#X-%#X-%#X-%#X\n", eFlash_Current_Blcok,eFlash_Current_Page, eFlash_Current_Index, a1,a2,a3,a4);
        
        // If current debug code are all 0xFF, stop to decode
        if((0xFF==a1) && (0xFF==a2) && (0xFF==a3) && (0xFF==a4))
        {
            break;
        }
        
        // Print massage number to file
        Message_num++;
        fprintf(Analysis_LogFile, "[Message num is %04d] :: ", Message_num);
        
        // Print Message to file
        //====================================
        indexCode = pFirstCode;
        j=0;
        while(NULL != (indexCode->NextCode))
        {
            tempCode = indexCode;
            
            // Four Byte match
            if(((tempCode->CodeArray[0])==a1) &&
               ((tempCode->CodeArray[1])==a2) &&
               ((tempCode->CodeArray[2])==a3) &&
               ((tempCode->CodeArray[3])==a4))
            {
                fprintf(Analysis_LogFile, "%s\n", tempCode->MessageInfo);
                j=0x55;
                break;
            }
            // First byte is debug code, 3 Byte is data
            else if(((tempCode->CodeArray[0])==a1) &&
                    ((tempCode->CodeArray[1])==0xFF) &&
                    ((tempCode->CodeArray[2])==0xFF) &&
                    ((tempCode->CodeArray[3])==0xFF))
            {
                fprintf(Analysis_LogFile, "%s %#02X %#02X %#02X\n", tempCode->MessageInfo, a2, a3, a4);
                j=0x55;
                break;
            }
            // The first 2 byte is debug code, and then the last 2 Byte is data
            else if(((tempCode->CodeArray[0])==a1) &&
                    ((tempCode->CodeArray[1])==a2) &&
                    ((tempCode->CodeArray[2])==0xFF) &&
                    ((tempCode->CodeArray[3])==0xFF))
            {
                fprintf(Analysis_LogFile, "%s %#02X %#02X\n", tempCode->MessageInfo, a3, a4);
                j=0x55;
                break;
            }
            
            indexCode = indexCode->NextCode;
        }
        if(0==j)
        {
            fprintf(Analysis_LogFile, "eFlash Debug code:%#04X, %#04X, %#04X, %#04X \
            <<<<<<<<<<<<<<<<<<<<Not support eFlash code, Block-page-index is:%X-%X-%X>>>>>>>>>>>>>>>>>>>>>>\n", \
            a1, a2, a3, a4,eFlash_Current_Blcok,eFlash_Current_Page,eFlash_Current_Index);
        }
        //====================================
        
        eFlash_Current_Index += 4;
        if(eFlash_Current_Index>=256)
        {
            eFlash_Current_Index=8;
            eFlash_Current_Page++;
            if(4==eFlash_Current_Page)
            {
                eFlash_Current_Page=0;
                eFlash_Current_Blcok++;
                if(4==eFlash_Current_Blcok)
                {
                    eFlash_Current_Blcok=0;
                }
            }
        }
    }
    
    
    SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
    printf("\n Analysis binary log file OK\n");
    fprintf(Analysis_LogFile,"\n Analysis binary log file OK\n");
    SetTextColor(EFI_WHITE, EFI_BLACK);
    fflush(Analysis_LogFile);
    
Analysis_End:
    fclose(Analysis_LogFile);
}


void CreatCfgFile(void)
{
    unsigned int i;
    
    CfgFile = fopen("eFlashDebug.cfg","w");
    if(CfgFile == NULL)
    {
        printf("Creat default eFlashDebug.cfg file Fail\n\n");
        return;
    }
    
    fprintf(CfgFile, "[ToolVersion=%s]\n", TOOLS_VER);
    fprintf(CfgFile, "#************************************************************\n");
    fprintf(CfgFile, "# This eFlash Debug config file\n");
    fprintf(CfgFile, "# eFlash Debug code start with $# \n");
    fprintf(CfgFile, "# You can add DebugCode and Message according to the rules\n");
    fprintf(CfgFile, "#\n");
    fprintf(CfgFile, "# Author : %s\n", TOOLS_AUTHOR);
    fprintf(CfgFile, "#************************************************************\n\n\n");
    
    for(i=0; 0!=DebugCodeList[i].CodeArray[0]; i++)
    {
        fprintf(CfgFile, "$#");
        fprintf(CfgFile, "%02X,%02X,%02X,%02X",
                                DebugCodeList[i].CodeArray[0],
                                DebugCodeList[i].CodeArray[1],
                                DebugCodeList[i].CodeArray[2],
                                DebugCodeList[i].CodeArray[3]);
        fprintf(CfgFile, "  #");
        fprintf(CfgFile, "%s#\n", DebugCodeList[i].MessageInfo);
    }
    
    fclose(CfgFile);
    printf("Creat default eFlashDebug.cfg file OK\n\n");
}

void ReadCfgFile(void)
{
    char StrLine[1024];
    int  HexNum1, HexNum2, HexNum3, HexNum4;
    char *StrNum1, *StrNum2, *StrNum3, *StrNum4;
    DebugCodeStruct *tempCode;
    DebugCodeStruct *indexCode;
    char *str;
    char *pStrLine;
    int i, j;
    
    if((CfgFile = fopen("eFlashDebug.cfg","r")) == NULL)
    {
        printf("eFlashDebug.cfg not exist\n\n");
        
        // Craet eFlashDebug config file
        CreatCfgFile();
        return ;
    }
    
    pFirstCode = (DebugCodeStruct*)malloc(sizeof(DebugCodeStruct));
    pFirstCode->MessageCount=0;
    indexCode = pFirstCode;
    indexCode->NextCode = NULL;
    
    j=1;
    while (!feof(CfgFile))
    {   
        // Read one line data
        fgets(StrLine,1024,CfgFile);
        //printf("%s", StrLine);
        
        pStrLine = StrLine;
        
        if(('$'==StrLine[0]) && (('#'==StrLine[1])))
        {
            tempCode = indexCode;
            
            StrNum1 = pStrLine+2;
            StrNum2 = pStrLine+5;
            StrNum3 = pStrLine+8;
            StrNum4 = pStrLine+11;
            
            HexNum1 = (int)strtol(StrNum1, &str, 16);
            HexNum2 = (int)strtol(StrNum2, &str, 16);
            HexNum3 = (int)strtol(StrNum3, &str, 16);
            HexNum4 = (int)strtol(StrNum4, &str, 16);
            
            tempCode->CodeArray[0] = HexNum1;
            tempCode->CodeArray[1] = HexNum2;
            tempCode->CodeArray[2] = HexNum3;
            tempCode->CodeArray[3] = HexNum4;
            tempCode->MessageCount=j;
            j = j+1;

            pStrLine = StrNum4;
            while(('#' != (*pStrLine++)));
            i = 0;
            while(('#' != (*pStrLine)))
            {
               tempCode->MessageInfo[i] = *pStrLine;
               i++;
               pStrLine++;
            }
            tempCode->MessageInfo[i] = 0;
                        
            
            /*printf("Message-%d, eFlash Debug code: %#X, %#X, %#X, %#X === %s\n",
                                                            tempCode->MessageCount,
                                                            tempCode->CodeArray[0],
                                                            tempCode->CodeArray[1],
                                                            tempCode->CodeArray[2],
                                                            tempCode->CodeArray[3],
                                                            tempCode->MessageInfo);*/
                                                            
            tempCode = (DebugCodeStruct*)malloc(sizeof(DebugCodeStruct));
            tempCode->NextCode = NULL;
            indexCode->NextCode = tempCode;
            indexCode = indexCode->NextCode;
        }
    }
    ReadFlag = 0x55;
}

void FreeMemory(void)
{
    DebugCodeStruct *tempCode;
    DebugCodeStruct *indexCode;
    
    indexCode = pFirstCode;

    if(NULL != indexCode)
    {
        while(NULL != (indexCode->NextCode))
        {
            tempCode = indexCode;
            indexCode = indexCode->NextCode;
       
            free(tempCode);
        }
    }
}

//==================================================================================================
int main(int argc, char *argv[])
{
    char IOInitOK=0;
    
    ToolInit();
    if (argc == 1)
    {
        goto ArgcError;
    }
    
    if(!strcmp("/D",argv[1]) || !strcmp("/d",argv[1]))
    {
        Dump_Flag=1;
    }
    
    if(!strcmp("/A",argv[1]) || !strcmp("/a",argv[1]))
    {
        if(3!=argc)
        {
            goto ArgcError;
        }
        Analysis_Flag=1;
    }
    
    printf("=======================================================\n");
    printf("=         eFlash Debug Dump Utility Version : %s    =\n",TOOLS_VER);
    printf("=======================================================\n");
    
    if(1==Dump_Flag)
    {
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
        
        //Read data form EC eFlash
        Read_eFlashData();
    }
    
    if(1==Analysis_Flag)
    {
        // Read eFlash.cfg file
        ReadCfgFile();
        
        if(0x55==ReadFlag)
        {
            // Analysis of the binary log file
            Analysis_file(argv[2]);
        
            // free memory
            FreeMemory();
        }
    }
    
    if((0==Dump_Flag) && (0==Analysis_Flag))
    {
        goto ArgcError;
    }
    
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