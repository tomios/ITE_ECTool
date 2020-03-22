/* Copyright (C)Copyright 2005-2020 ZXQ Telecom. All rights reserved.

   Author: Morgen Zhu
   
   Description:These functions of this file are reference only in the Windows!
   It can read/write ITE-EC RAM by 
   ----EC-port(2E/2F or 4E/4F)

    Using VS2012 X86 cmd tool to compilation
    For windows-32/64bit
*/

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

//========================================Type Define ==============================================
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;
#define TRUE            1
#define FALSE           0

//=======================================Tool info==================================================
#define  TOOLS_NAME     "Battery View"
#define  CopyRight      "(C)Copyright 2005-2020 ZXQ Telecom."
#define  TOOL_DEBUG     0
#define  ESC            0x1B
#define  TOOLS_VER      "V3.2"

//==========================The hardware port to read/write function================================
#define READ_PORT(port,data2)  GetPortVal(port, &data2, 1);
#define WRITE_PORT(port,data2) SetPortVal(port, data2, 1)



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

// Write EC RAM via EC port(2E/2F or 4E/4F)
UINT8 EC_ADDR_PORT = 0x4E;   // 0x2E or 0x4E
UINT8 EC_DATA_PORT = 0x4F;   // 0x2F or 0x4F

UINT8 ITE_EC_RAM_Read_Direct(UINT16 iIndex)
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

void ITE_EC_RAM_Write_Direct(UINT16 iIndex, UINT8 data)
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


//=======================================Battery Info Type==========================================
#define  EC_RAM_WRITE  ITE_EC_RAM_Write_Direct
#define  EC_RAM_READ   ITE_EC_RAM_Read_Direct

#define  UI_BASE_X   70
#define  UI_BASE_Y   4


FILE *BAT_LogFile = NULL;
FILE *CfgFile = NULL;
unsigned int SetTime;
unsigned int All_Display_Count;

typedef struct BatteryInfoStruct
{
    char InfoName[128];
    char CfgItemName[128];     // Read cfg file item name
    char InfoValue[128];       // All info converted to character
    int  InfoAddr_L;           // Information address low
    int  InfoAddr_H;           // Information address high
    int  InfoInt;              // Information Value
    UINT8 LogAndDisplay;       // Set 0:Not display, No log
                               // Set 1:display, No log
                               // Set 3:display, log
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
    
    Dec_Data_1,
    Dec_Data_2,
    Dec_Data_3,
    Dec_Data_4,
    Hex_Data_1,
    Hex_Data_2,
    Hex_Data_3,
    Hex_Data_4,
    
    CHARGE_Voltage,
    CHARGE_Current,
    CHARGE_OP0,
    CHARGE_OP1,
    CHARGE_OP2,
    CHARGE_OP3,
    CHARGE_OP4,
    CHARGE_OP5,
    CHARGE_OP6,
    PROHOT_OP0,
    PROHOT_OP1,
    MinSysVoltage,
    ChargerStatus,
    INPUT_Current,
    
    Adapter_WATT,
    PORT_A_WATT,
    PORT_A_PDO_C,
    PORT_A_PDO_V,
    PORT_B_WATT,
    PORT_B_PDO_C,
    PORT_B_PDO_V,
    PORT_A_Status,
    PORT_B_Status,
    
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
    {"EC_Version          :", "N/A", "N/A", 0, 0, 0, TRUE},
    {"BAT_Mode            :", "N/A", "N/A", 0, 0, 0, TRUE},
    {"BAT_AC_State        :", "N/A", "N/A", 0, 0, 0, TRUE},
    {"BAT_FCCFlag         :", "N/A", "N/A", 0, 0, 0, TRUE},
    
    {"BAT_Current         :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_Voltage         :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_RMC             :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_FCC             :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_RSOC            :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_ASOC            :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"RMC/FCC             :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_DV              :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_DC              :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_Temp            :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_CycleCount      :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_CC              :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_CV              :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_Status_H        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_Status_L        :", "N/A", "N/A", 0, 0, 0, FALSE},
    
    {"Dec_Data_1          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Dec_Data_2          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Dec_Data_3          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Dec_Data_4          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Hex_Data_1          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Hex_Data_2          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Hex_Data_3          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Hex_Data_4          :", "N/A", "N/A", 0, 0, 0, FALSE},
    
    {"CHARGE_Voltage      :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"CHARGE_Current      :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"CHARGE_OP0          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"CHARGE_OP1          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"CHARGE_OP2          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"CHARGE_OP3          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"CHARGE_OP4          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"CHARGE_OP5          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"CHARGE_OP6          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PROHOT_OP0          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PROHOT_OP1          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"MinSysVoltage       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"ChargerStatus       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"INPUT_Current       :", "N/A", "N/A", 0, 0, 0, FALSE},
    
    {"Adapter_watt        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PORT_A_WATT         :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PORT_A_PDO_C        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PORT_A_PDO_V        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PORT_B_WATT         :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PORT_B_PDO_C        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PORT_B_PDO_V        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PORT_A_Status       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"PORT_B_Status       :", "N/A", "N/A", 0, 0, 0, FALSE},

    {"BAT_ManuName        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_DeviceName      :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_DevChemistry    :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_ManuDate        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"BAT_SerialNum       :", "N/A", "N/A", 0, 0, 0, FALSE},
    
    {"Temp_Sensor1        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor2        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor3        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor4        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor5        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor6        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor7        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor8        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor9        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor10       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor11       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor12       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor13       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor14       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor15       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Temp_Sensor16       :", "N/A", "N/A", 0, 0, 0, FALSE},

    {"OS_AC               :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT              :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT_Charge       :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT_Discharge    :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT_Remtime      :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT_FCC          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT_RMC          :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT_Current      :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT_RSOC         :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT_Alert1_Low   :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"OS_BAT_Alert1_War   :", "N/A", "N/A", 0, 0, 0, FALSE},
    
    {"Acer_BAT_RMC        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Acer_BAT_FCC        :", "N/A", "N/A", 0, 0, 0, FALSE},
    {"Acer_BAT_RSOC       :", "N/A", "N/A", 0, 0, 0, FALSE},

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
    
    All_Display_Count=0;
    
    while (!feof(CfgFile))
    {   
        // Read one line data
        fgets(StrLine,1024,CfgFile);
        //printf("%s", StrLine);
        
        pStrLine = StrLine;
        if(('$'==StrLine[0]) && (('1'==StrLine[1])))
        {
            //==================================================
            // Read EC RAM address
            while(('#' != (*pStrLine++)));
            HexNum = (int)strtol(pStrLine, &str, 16);
            //printf("%#X ",HexNum);
            BAT1_Info[InfoIndex].InfoAddr_L = HexNum;
            
            pStrLine++;
            while(('#' != (*pStrLine++)));
            
            HexNum = (int)strtol(pStrLine, &str, 16);
            //printf("%#X ",HexNum);
            BAT1_Info[InfoIndex].InfoAddr_H = HexNum;
            
            //==================================================
            // Read item display and log config
            pStrLine++;
            while(('#' != (*pStrLine++)));
            HexNum = (int)strtol(pStrLine, &str, 10);
            //printf("%#X\n",HexNum);
            BAT1_Info[InfoIndex].LogAndDisplay = HexNum;
            if(0<HexNum)
            {
                All_Display_Count++;
            }
            //==================================================
            // Read item display name
            pStrLine++;
            while(('#' != (*pStrLine++)));
            j=0;
            while(('#' != (*pStrLine)))
            {
                BAT1_Info[InfoIndex].CfgItemName[j] = *pStrLine;
                j++;
                pStrLine++;
            }
            //printf("%#s\n",BAT1_Info[InfoIndex].CfgItemName);
            
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
        printf("Item_%02d %#06X %#06X %#04X %s\n", i,
                BAT1_Info[i].InfoAddr_L, BAT1_Info[i].InfoAddr_H, 
                BAT1_Info[i].LogAndDisplay, BAT1_Info[i].CfgItemName);
    }
    #endif
    
    fclose(CfgFile);
}

void ToolInit(void)
{
    int i,j,k;
    int diplay_x;
    int diplay_y;
    unsigned char EC_CHIP_ID1;
    unsigned char EC_CHIP_ID2;
    unsigned char EC_CHIP_Ver;
    
    // ITE IT-557x chip is DLM architecture for EC  RAM and It's support 6K/8K RAM.
    // If used RAM less  than 4K, you can access EC RAM form 0x000--0xFFF by 4E/4F IO port
    // If used RAM more than 4K, RAM address change to 0xC000
    // If you want to access EC RAM by 4E/4F IO port, you must set as follow register first
    // REG_1060[BIT7]
    EC_CHIP_ID1 = EC_RAM_READ(0x2000);
    EC_CHIP_ID2 = EC_RAM_READ(0x2001);
    if(0x55==EC_CHIP_ID1)
    {
        EC_CHIP_Ver = EC_RAM_READ(0x1060);
        EC_CHIP_Ver = EC_CHIP_Ver | 0x80;
        EC_RAM_WRITE(0x1060, EC_CHIP_Ver);
    }
    
    
    SetConsoleTitle(TOOLS_NAME);
    system("mode con cols=150 lines=80");
    
    printf("Battery Tool %s\n", TOOLS_VER);
    printf("Current EC Chip is IT-%X%X\n",EC_CHIP_ID1,EC_CHIP_ID2);
    //printf("%s All rights reserved.\n",CopyRight);
    
    SetTextColor(EFI_LIGHTMAGENTA, EFI_BLACK);
    printf("<ESC> to exit!");
    SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
    
    
    // Print Item
    diplay_x = 0;
    diplay_y = UI_BASE_Y;
    for(i=0,j=0;i<INFONAMECOUNT;i++)
    {
        if(0x01&BAT1_Info[i].LogAndDisplay)
        {
            SetPosition_X_Y(diplay_x, diplay_y+j);
            printf(BAT1_Info[i].CfgItemName);
            j++;
            if(j>(All_Display_Count>>1))
            {
                j=0;
                diplay_x = UI_BASE_X;
                diplay_y = UI_BASE_Y;
            }
        }
    }
    SetPosition_X_Y(diplay_x, diplay_y+j+2);
}

void Polling_Battery_Static_Data(void)
{
    unsigned int tmpvalue;
    unsigned char index;
    unsigned char ch;

    if(0x01&BAT1_Info[EC_Version].LogAndDisplay)
    {
        sprintf(BAT1_Info[EC_Version].InfoValue, "%02d.%02d.%02d.%02d",
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_L),
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_H),
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_H+1),
                EC_RAM_READ(BAT1_Info[EC_Version].InfoAddr_H+2));  // The EC version is 4Byte
    }
    if(0x01&BAT1_Info[BAT_Mode].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Mode].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Mode].InfoAddr_L);
        BAT1_Info[BAT_Mode].InfoInt = tmpvalue;
        
        sprintf(BAT1_Info[BAT_Mode].InfoValue, "%04X [%-5s]",
                BAT1_Info[BAT_Mode].InfoInt, ((tmpvalue&0x8000)?"mWh":"mAh")); // mAh or 10mWh
    }
    if(0x01&BAT1_Info[BAT_CycleCount].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_CycleCount].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_CycleCount].InfoAddr_L);
        BAT1_Info[BAT_CycleCount].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_CycleCount].InfoValue, "%-8d",
                BAT1_Info[BAT_CycleCount].InfoInt);
    }
    if(0x01&BAT1_Info[BAT_DV].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_DV].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_DV].InfoAddr_L);
        BAT1_Info[BAT_DV].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_DV].InfoValue, "%-8d mV",
                BAT1_Info[BAT_DV].InfoInt);
    }
    if(0x01&BAT1_Info[BAT_DC].LogAndDisplay)
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
    
    if(0x01&BAT1_Info[BAT_ManuName].LogAndDisplay)
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
    
    if(0x01&BAT1_Info[BAT_DeviceName].LogAndDisplay)
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
    
    if(0x01&BAT1_Info[BAT_DevChemistry].LogAndDisplay)
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
    
    if(0x01&BAT1_Info[BAT_ManuDate].LogAndDisplay)
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
    
    if(0x01&BAT1_Info[BAT_SerialNum].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_SerialNum].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_SerialNum].InfoAddr_L);
        BAT1_Info[BAT_SerialNum].InfoInt = tmpvalue;
        
        sprintf(BAT1_Info[BAT_SerialNum].InfoValue, "%-d [%04X]", tmpvalue, tmpvalue);
    }
}

void Polling_Battery_Dynamic_Data(void)
{
    unsigned int tmpvalue;
    float tmpvalue1;

    if(0x01&BAT1_Info[BAT_AC_State].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_AC_State].InfoAddr_L)&0x01;
        BAT1_Info[BAT_AC_State].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_AC_State].InfoValue, "%-8s", (tmpvalue?"AC IN":"AC OUT"));
    }
    if(0x01&BAT1_Info[BAT_FCCFlag].LogAndDisplay)
    {
        BAT1_Info[BAT_FCCFlag].InfoInt = EC_RAM_READ(BAT1_Info[BAT_FCCFlag].InfoAddr_L)&0x20;
        sprintf(BAT1_Info[BAT_FCCFlag].InfoValue, "%-8s", (BAT1_Info[BAT_FCCFlag].InfoInt?"Full":"Not Full"));
    }
    if(0x01&BAT1_Info[BAT_Current].LogAndDisplay)
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
    if(0x01&BAT1_Info[BAT_Voltage].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Voltage].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Voltage].InfoAddr_L);
        BAT1_Info[BAT_Voltage].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_Voltage].InfoValue, "%-8d mV",
                BAT1_Info[BAT_Voltage].InfoInt);
    }
    if(0x01&BAT1_Info[BAT_RMC].LogAndDisplay)
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
    if(0x01&BAT1_Info[BAT_FCC].LogAndDisplay)
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
    if(0x01&BAT1_Info[BAT_RSOC].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_RSOC].InfoAddr_L);
        BAT1_Info[BAT_RSOC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_RSOC].InfoValue, "%-8d %%",
                BAT1_Info[BAT_RSOC].InfoInt);
    }
    if(0x01&BAT1_Info[BAT_ASOC].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_ASOC].InfoAddr_L);
        BAT1_Info[BAT_ASOC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_ASOC].InfoValue, "%-8d %%",
                BAT1_Info[BAT_ASOC].InfoInt);
    }
    if(0x01&BAT1_Info[BAT_RealRSOC].LogAndDisplay)
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
    if(0x01&BAT1_Info[BAT_CycleCount].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_CycleCount].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_CycleCount].InfoAddr_L);
        BAT1_Info[BAT_CycleCount].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_CycleCount].InfoValue, "%-8d",
                BAT1_Info[BAT_CycleCount].InfoInt);
    }
    if(0x01&BAT1_Info[BAT_Temp].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Temp].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_Temp].InfoAddr_L);
        tmpvalue1 = tmpvalue*0.1-273.15;
        BAT1_Info[BAT_Temp].InfoInt = tmpvalue1;
        sprintf(BAT1_Info[BAT_Temp].InfoValue, "%-8.1f C", tmpvalue1);
    }
    if(0x01&BAT1_Info[CHARGE_Current].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_Current].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_Current].InfoAddr_L);
        BAT1_Info[CHARGE_Current].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_Current].InfoValue, "%-8d mA",
        BAT1_Info[CHARGE_Current].InfoInt);
    }
    if(0x01&BAT1_Info[CHARGE_Voltage].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_Voltage].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_Voltage].InfoAddr_L);
        BAT1_Info[CHARGE_Voltage].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_Voltage].InfoValue, "%-8d mV",
        BAT1_Info[CHARGE_Voltage].InfoInt);
    }
    if(0x01&BAT1_Info[INPUT_Current].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[INPUT_Current].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[INPUT_Current].InfoAddr_L);
        BAT1_Info[INPUT_Current].InfoInt = tmpvalue;
        sprintf(BAT1_Info[INPUT_Current].InfoValue, "%-8d mA [%-#04X]",
        BAT1_Info[INPUT_Current].InfoInt, BAT1_Info[INPUT_Current].InfoInt);
    }
    if(0x01&BAT1_Info[CHARGE_OP0].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP0].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP0].InfoAddr_L);
        BAT1_Info[CHARGE_OP0].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP0].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP0].InfoInt);
    }
    if(0x01&BAT1_Info[CHARGE_OP1].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP1].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP1].InfoAddr_L);
        BAT1_Info[CHARGE_OP1].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP1].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP1].InfoInt);
    }
    if(0x01&BAT1_Info[CHARGE_OP2].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP2].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP2].InfoAddr_L);
        BAT1_Info[CHARGE_OP2].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP2].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP2].InfoInt);
    }
    if(0x01&BAT1_Info[CHARGE_OP3].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP3].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP3].InfoAddr_L);
        BAT1_Info[CHARGE_OP3].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP3].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP3].InfoInt);
    }
    if(0x01&BAT1_Info[CHARGE_OP4].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP4].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP4].InfoAddr_L);
        BAT1_Info[CHARGE_OP4].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP4].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP4].InfoInt);
    }
    if(0x01&BAT1_Info[CHARGE_OP5].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP5].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP5].InfoAddr_L);
        BAT1_Info[CHARGE_OP5].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP5].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP5].InfoInt);
    }
    if(0x01&BAT1_Info[CHARGE_OP6].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[CHARGE_OP6].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[CHARGE_OP6].InfoAddr_L);
        BAT1_Info[CHARGE_OP6].InfoInt = tmpvalue;
        sprintf(BAT1_Info[CHARGE_OP6].InfoValue, "%-#08X",
        BAT1_Info[CHARGE_OP6].InfoInt);
    }
    if(0x01&BAT1_Info[PROHOT_OP0].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PROHOT_OP0].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[PROHOT_OP0].InfoAddr_L);
        BAT1_Info[PROHOT_OP0].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PROHOT_OP0].InfoValue, "%-#08X",
        BAT1_Info[PROHOT_OP0].InfoInt);
    }
    if(0x01&BAT1_Info[PROHOT_OP1].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PROHOT_OP1].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[PROHOT_OP1].InfoAddr_L);
        BAT1_Info[PROHOT_OP1].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PROHOT_OP1].InfoValue, "%-#08X",
        BAT1_Info[PROHOT_OP1].InfoInt);
    }
    if(0x01&BAT1_Info[MinSysVoltage].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[MinSysVoltage].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[MinSysVoltage].InfoAddr_L);
        BAT1_Info[MinSysVoltage].InfoInt = tmpvalue;
        sprintf(BAT1_Info[MinSysVoltage].InfoValue, "%-#08X",
        BAT1_Info[MinSysVoltage].InfoInt);
    }
    if(0x01&BAT1_Info[ChargerStatus].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[ChargerStatus].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[ChargerStatus].InfoAddr_L);
        BAT1_Info[ChargerStatus].InfoInt = tmpvalue;
        sprintf(BAT1_Info[ChargerStatus].InfoValue, "%-#08X",
        BAT1_Info[ChargerStatus].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor1].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor1].InfoAddr_L);
        BAT1_Info[Temp_Sensor1].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor1].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor1].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor2].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor2].InfoAddr_L);
        BAT1_Info[Temp_Sensor2].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor2].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor2].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor3].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor3].InfoAddr_L);
        BAT1_Info[Temp_Sensor3].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor3].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor3].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor4].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor4].InfoAddr_L);
        BAT1_Info[Temp_Sensor4].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor4].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor4].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor5].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor5].InfoAddr_L);
        BAT1_Info[Temp_Sensor5].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor5].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor5].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor6].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor6].InfoAddr_L);
        BAT1_Info[Temp_Sensor6].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor6].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor6].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor7].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor7].InfoAddr_L);
        BAT1_Info[Temp_Sensor7].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor7].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor7].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor8].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor8].InfoAddr_L);
        BAT1_Info[Temp_Sensor8].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor8].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor8].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor9].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor9].InfoAddr_L);
        BAT1_Info[Temp_Sensor9].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor9].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor9].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor10].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor10].InfoAddr_L);
        BAT1_Info[Temp_Sensor10].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor10].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor10].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor11].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor11].InfoAddr_L);
        BAT1_Info[Temp_Sensor11].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor11].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor11].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor12].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor12].InfoAddr_L);
        BAT1_Info[Temp_Sensor12].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor12].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor12].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor13].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor13].InfoAddr_L);
        BAT1_Info[Temp_Sensor13].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor13].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor13].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor14].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor14].InfoAddr_L);
        BAT1_Info[Temp_Sensor14].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor14].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor14].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor15].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor15].InfoAddr_L);
        BAT1_Info[Temp_Sensor15].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor15].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor15].InfoInt);
    }
    if(0x01&BAT1_Info[Temp_Sensor16].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Temp_Sensor16].InfoAddr_L);
        BAT1_Info[Temp_Sensor16].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Temp_Sensor16].InfoValue, "%-8d C",BAT1_Info[Temp_Sensor16].InfoInt);
    }
    if(0x01&BAT1_Info[BAT_CC].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_CC].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_CC].InfoAddr_L);
        BAT1_Info[BAT_CC].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_CC].InfoValue, "%-8d mA",BAT1_Info[BAT_CC].InfoInt);
    }
    if(0x01&BAT1_Info[BAT_CV].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_CV].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[BAT_CV].InfoAddr_L);
        BAT1_Info[BAT_CV].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_CV].InfoValue, "%-8d mV",BAT1_Info[BAT_CV].InfoInt);
    }
    if(0x01&BAT1_Info[BAT_Status_H].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Status_H].InfoAddr_L);
        BAT1_Info[BAT_Status_H].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_Status_H].InfoValue, "%-#08X", tmpvalue);
        //sprintf(BAT1_Info[BAT_Status_H].InfoValue, "OCA:%d | TCA:%d | RS | OTA:%d | TDA:%d | RS | RCA:%d | RTA:%d",
        //                                          (tmpvalue&0x8000)?1:0, (tmpvalue&0x4000)?1:0,
        //                                          (tmpvalue&0x1000)?1:0, (tmpvalue&0x0800)?1:0, 
        //                                          (tmpvalue&0x0200)?1:0, (tmpvalue&0x0100)?1:0);
    }
    if(0x01&BAT1_Info[BAT_Status_L].LogAndDisplay)
    {
    
        tmpvalue = EC_RAM_READ(BAT1_Info[BAT_Status_L].InfoAddr_L);
        BAT1_Info[BAT_Status_L].InfoInt = tmpvalue;
        sprintf(BAT1_Info[BAT_Status_L].InfoValue, "%-#08X", tmpvalue);
        //sprintf(BAT1_Info[BAT_Status_L].InfoValue, "INI:%d | DSG:%d | FC :%d | FD :%d | EC:%02X   | [%#04X]",
        //                                          (tmpvalue&0x0080)?1:0, (tmpvalue&0x0040)?1:0,
        //                                          (tmpvalue&0x0020)?1:0, (tmpvalue&0x0010)?1:0,
        //                                          tmpvalue&0x000F, tmpvalue);
    }
    if(0x01&BAT1_Info[Acer_BAT_RMC].LogAndDisplay)
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
    if(0x01&BAT1_Info[Acer_BAT_FCC].LogAndDisplay)
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
    if(0x01&BAT1_Info[Acer_BAT_RSOC].LogAndDisplay)
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
    if(0x01&BAT1_Info[Adapter_WATT].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Adapter_WATT].InfoAddr_L);
        BAT1_Info[Adapter_WATT].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Adapter_WATT].InfoValue, "%-8d W",BAT1_Info[Adapter_WATT].InfoInt);
    }
    if(0x01&BAT1_Info[PORT_A_WATT].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PORT_A_WATT].InfoAddr_L);
        BAT1_Info[PORT_A_WATT].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PORT_A_WATT].InfoValue, "%-8d W",BAT1_Info[PORT_A_WATT].InfoInt);
    }
    if(0x01&BAT1_Info[PORT_A_PDO_C].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PORT_A_PDO_C].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[PORT_A_PDO_C].InfoAddr_L);
        BAT1_Info[PORT_A_PDO_C].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PORT_A_PDO_C].InfoValue, "%-8d mA",BAT1_Info[PORT_A_PDO_C].InfoInt);
    }
    if(0x01&BAT1_Info[PORT_A_PDO_V].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PORT_A_PDO_V].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[PORT_A_PDO_V].InfoAddr_L);
        BAT1_Info[PORT_A_PDO_V].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PORT_A_PDO_V].InfoValue, "%-8d mV",BAT1_Info[PORT_A_PDO_V].InfoInt);
    }
    if(0x01&BAT1_Info[PORT_B_WATT].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PORT_B_WATT].InfoAddr_L);
        BAT1_Info[PORT_B_WATT].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PORT_B_WATT].InfoValue, "%-8d W",BAT1_Info[PORT_B_WATT].InfoInt);
    }
    if(0x01&BAT1_Info[PORT_B_PDO_C].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PORT_B_PDO_C].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[PORT_B_PDO_C].InfoAddr_L);
        BAT1_Info[PORT_B_PDO_C].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PORT_B_PDO_C].InfoValue, "%-8d mA",BAT1_Info[PORT_B_PDO_C].InfoInt);
    }
    if(0x01&BAT1_Info[PORT_B_PDO_V].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PORT_B_PDO_V].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[PORT_B_PDO_V].InfoAddr_L);
        BAT1_Info[PORT_B_PDO_V].InfoInt = tmpvalue;
        sprintf(BAT1_Info[PORT_B_PDO_V].InfoValue, "%-8d mV",BAT1_Info[PORT_B_PDO_V].InfoInt);
    }
    if(0x01&BAT1_Info[PORT_A_Status].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PORT_A_Status].InfoAddr_L);
        BAT1_Info[PORT_A_Status].InfoInt = tmpvalue;
        //sprintf(BAT1_Info[PORT_A_Status].InfoValue, "%-#08X",BAT1_Info[PORT_A_Status].InfoInt);
        if(tmpvalue&0x01)
        {
            sprintf(BAT1_Info[PORT_A_Status].InfoValue, "[%-s][%-s][%-s][%-s]",
                                    (tmpvalue&0x01)?"Connect   ":"Connect   ",
                                    (tmpvalue&0x02)?"cc1":"cc2",
                                    (tmpvalue&0x04)?"Source":"Sink  ",
                                    (tmpvalue&0x08)?"DFP":"UFP");
        }
        else
        {
            sprintf(BAT1_Info[PORT_A_Status].InfoValue, "[%-s][%-s][%-s][%-s]",
                                    "Disconnect",
                                    "   ",
                                    "      ",
                                    "   ");
        }
    }
    if(0x01&BAT1_Info[PORT_B_Status].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[PORT_B_Status].InfoAddr_L);
        BAT1_Info[PORT_B_Status].InfoInt = tmpvalue;
        //sprintf(BAT1_Info[PORT_B_Status].InfoValue, "%-#08X",BAT1_Info[PORT_B_Status].InfoInt);
        if(tmpvalue&0x01)
        {
            sprintf(BAT1_Info[PORT_B_Status].InfoValue, "[%-s][%-s][%-s][%-s]",
                                    (tmpvalue&0x01)?"Connect   ":"Connect   ",
                                    (tmpvalue&0x02)?"cc1":"cc2",
                                    (tmpvalue&0x04)?"Source":"Sink  ",
                                    (tmpvalue&0x08)?"DFP":"UFP");
        }
        else
        {
            sprintf(BAT1_Info[PORT_B_Status].InfoValue, "[%-s][%-s][%-s][%-s]",
                                    "Disconnect",
                                    "   ",
                                    "      ",
                                    "   ");
        }
    }
    if(0x01&BAT1_Info[Dec_Data_1].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Dec_Data_1].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[Dec_Data_1].InfoAddr_L);
        BAT1_Info[Dec_Data_1].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Dec_Data_1].InfoValue, "%-8d",BAT1_Info[Dec_Data_1].InfoInt);
    }
    if(0x01&BAT1_Info[Dec_Data_2].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Dec_Data_2].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[Dec_Data_2].InfoAddr_L);
        BAT1_Info[Dec_Data_2].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Dec_Data_2].InfoValue, "%-8d",BAT1_Info[Dec_Data_2].InfoInt);
    }
    if(0x01&BAT1_Info[Dec_Data_3].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Dec_Data_3].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[Dec_Data_3].InfoAddr_L);
        BAT1_Info[Dec_Data_3].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Dec_Data_3].InfoValue, "%-8d",BAT1_Info[Dec_Data_3].InfoInt);
    }
    if(0x01&BAT1_Info[Dec_Data_4].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Dec_Data_4].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[Dec_Data_4].InfoAddr_L);
        BAT1_Info[Dec_Data_4].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Dec_Data_4].InfoValue, "%-8d",BAT1_Info[Dec_Data_4].InfoInt);
    }
    if(0x01&BAT1_Info[Hex_Data_1].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Hex_Data_1].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[Hex_Data_1].InfoAddr_L);
        BAT1_Info[Hex_Data_1].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Hex_Data_1].InfoValue, "%-#08X", BAT1_Info[Hex_Data_1].InfoInt);
    }
    if(0x01&BAT1_Info[Hex_Data_2].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Hex_Data_2].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[Hex_Data_2].InfoAddr_L);
        BAT1_Info[Hex_Data_2].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Hex_Data_2].InfoValue, "%-#08X", BAT1_Info[Hex_Data_2].InfoInt);
    }
    if(0x01&BAT1_Info[Hex_Data_3].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Hex_Data_3].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[Hex_Data_3].InfoAddr_L);
        BAT1_Info[Hex_Data_3].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Hex_Data_3].InfoValue, "%-#08X", BAT1_Info[Hex_Data_3].InfoInt);
    }
    if(0x01&BAT1_Info[Hex_Data_4].LogAndDisplay)
    {
        tmpvalue = EC_RAM_READ(BAT1_Info[Hex_Data_4].InfoAddr_H)<<8
                | EC_RAM_READ(BAT1_Info[Hex_Data_4].InfoAddr_L);
        BAT1_Info[Hex_Data_4].InfoInt = tmpvalue;
        sprintf(BAT1_Info[Hex_Data_4].InfoValue, "%-#08X", BAT1_Info[Hex_Data_4].InfoInt);
    }
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
void Polling_OS_Battery_Data(void)
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
            BAT1_Info[OS_BAT_Remtime].InfoInt = remtime;
            sprintf(BAT1_Info[OS_BAT_Remtime].InfoValue, "%-6d min", remtime);
        }
        else
        {
            BAT1_Info[OS_BAT_Remtime].InfoInt = 0;
            sprintf(BAT1_Info[OS_BAT_Remtime].InfoValue, "%-18s", "N/A");
        }
        
        BAT1_Info[OS_BAT_FCC].InfoInt = Battery_state.MaxCapacity;
        sprintf(BAT1_Info[OS_BAT_FCC].InfoValue, "%-6d mWh", Battery_state.MaxCapacity);
        
        BAT1_Info[OS_BAT_RMC].InfoInt = Battery_state.RemainingCapacity;
        sprintf(BAT1_Info[OS_BAT_RMC].InfoValue, "%-6d mWh", Battery_state.RemainingCapacity);
        
        BAT1_Info[OS_BAT_Current].InfoInt = Battery_state.Rate;
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

void Display_BatteryInfo(void)
{
    int i,j;
    int diplay_x = 16;
    int diplay_y = UI_BASE_Y;
    
    for(i=0,j=0;i<INFONAMECOUNT;i++)
    {
        if(0x01&BAT1_Info[i].LogAndDisplay)
        {
            SetPosition_X_Y(diplay_x, diplay_y+j);
            SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
            printf(":  %s\n", BAT1_Info[i].InfoValue);
            j++;
            if(j>(All_Display_Count>>1))
            {
                j=0;
                diplay_x = UI_BASE_X+20;
                diplay_y = UI_BASE_Y;
            }
        }
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
        if(0x02&BAT1_Info[i].LogAndDisplay)
        {
            fprintf(BAT_LogFile, "%-20d", BAT1_Info[i].InfoInt);
        }
    }
    fprintf(BAT_LogFile, "\n");
    fflush(BAT_LogFile);
}

void display(void)
{
    char i;
    SetPosition_X_Y(0, (All_Display_Count>>1)+5);
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
    Polling_Battery_Static_Data();
    Polling_Battery_Dynamic_Data();
    Polling_OS_Battery_Data();

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
        
        fprintf(BAT_LogFile, "Battery Tool %s\n",TOOLS_VER);

        fprintf(BAT_LogFile, "EC current version is : %s\n", BAT1_Info[EC_Version].InfoValue);

        fprintf(BAT_LogFile, "This log file is the %s information of the battery pack\n", 
                            (BAT1_Info[BAT_AC_State].InfoInt)?("Charge"):("discharge"));

        fprintf(BAT_LogFile, "Set the data polling time  is %d(s)\n" ,SetTime/100);

        fprintf(BAT_LogFile, "The battery Capacity units is %s\n\n", 
                            (BAT1_Info[BAT_Mode].InfoInt&0x8000)?("mWh"):("mAh"));

        fprintf(BAT_LogFile, "%-22s", "Date&Time");
        
        for(i=0;i<INFONAMECOUNT;i++)
        {
            if(0x02&BAT1_Info[i].LogAndDisplay)
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
        Polling_Battery_Dynamic_Data();
        Polling_OS_Battery_Data();
        
        if(BAT_LogFile)
        {
            PrintLogFile();
        }
        
        Display_BatteryInfo();
        
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
