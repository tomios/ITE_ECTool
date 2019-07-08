#define   TOOLVER   "V2.0"
//===========================================================================
// V2.0
// 1. Modify for support IT-557x 6K RAM
//===========================================================================


//===========================================================================
// V1.0
// 1. First release, Has the following functions
//    a. View EC RAM
//    b. View Logic Device config
//    c. Save all EC register
//===========================================================================

/* Copyright (c) 2011-2099, ZXQ CO. LTD. All rights reserved.

   Author: NULL
   
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
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/IoLib.h>              // IO access
#include <Protocol/SimpleFileSystem.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/EfiShell.h>
#include <Library/ShellLib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Guid/GlobalVariable.h>
#include <Guid/ConsoleOutDevice.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/ShellLibHiiGuid.h>

#include <Library/BaseLib.h>
#include <Library/ShellCommandLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>


extern EFI_BOOT_SERVICES         *gBS;
extern EFI_SYSTEM_TABLE          *gST;
extern EFI_RUNTIME_SERVICES      *gRT;
//==================================================================================================

//========================================Type Define ==============================================
typedef unsigned char   BYTE;
typedef unsigned char   UINT8;


//==================================================================================================

//==========================The hardware port to read/write function================================
#define READ_PORT  IoRead8
#define WRITE_PORT IoWrite8
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
    while (!(READ_PORT(PM_STATUS_PORT66) & PM_OBF));
}

//------------wait EC PM channel port66 input buffer empty-----/
void Wait_PM_IBE (void)
{
    while (READ_PORT(PM_STATUS_PORT66) & PM_IBF);
}

//------------send command by EC PM channel--------------------/
void Send_cmd_by_PM(UINT8 Cmd)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_CMD_PORT66, Cmd);
    Wait_PM_IBE();
}

//------------send data by EC PM channel-----------------------/
void Send_data_by_PM(UINT8 Data)
{
    Wait_PM_IBE();
    WRITE_PORT(PM_DATA_PORT62, Data);
    Wait_PM_IBE();
}

//-------------read data from EC PM channel--------------------/
UINT8 Read_data_from_PM(void)
{
    Wait_PM_OBF();
    return(READ_PORT(PM_DATA_PORT62));
}

// Write EC RAM via PM port(62/66)
void ECRAMWriteByte_PM(UINT8 index, UINT8 data)
{
    Send_cmd_by_PM(0x81);
    Send_data_by_PM(index);
    Send_data_by_PM(data);
}

// Read EC RAM via PM port(62/66)
UINT8 ECRAMReadByte_PM(UINT8 index)
{
    Send_cmd_by_PM(0x80);
    Send_data_by_PM(index);
    return Read_data_from_PM();
}
//==================================================================================================


//=======================================EC Direct Access interface=================================
//Port Config:
//	BADRSEL(0x200A) bit1-0	Addr	Data
//					00		2Eh		2Fh
//					01		4Eh		4Fh
//
//				01		4Eh		4Fh
//	ITE-EC Ram Read/Write Algorithm:
//	Addr 	w	0x2E
//	Data 	w	0x11
//	Addr	w	0x2F
//	Data	w	high byte
//	Addr	w	0x2E
//	Data	w	0x10
//	Addr	w	0x2F
//	Data	w	low byte
//	Addr	w	0x2E
//	Data	w	0x12
//	Addr	w	0x2F
//	Data	rw	value
UINT8 EC_ADDR_PORT = 0x4E;   // 0x2E or 0x4E
UINT8 EC_DATA_PORT = 0x4F;   // 0x2F or 0x4F
UINT8 High_Byte    = 0;
// Write EC RAM via EC port(2E/2F or 4E/4F)
void ECRAMWriteByte(UINT8 index, UINT8 data)
{
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, High_Byte); //High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, index);     //Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, data);      //Low byte
}

// Read EC RAM via EC port(2E/2F or 4E/4F)
UINT8 ECRAMReadByte(UINT8 index)
{
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, High_Byte); //High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, index);        //Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    return READ_PORT(EC_DATA_PORT);         //Low byte
}
//==================================================================================================

//=======================================SIO Access interface=======================================
UINT8  CurrentLDN       =0x01;

UINT8 ReadSIOReg(UINT8 offset)
{
    WRITE_PORT(EC_ADDR_PORT, offset);
    return READ_PORT(EC_DATA_PORT);
}

void WriteSIOReg(UINT8 offset, UINT8 data1)
{
    WRITE_PORT(EC_ADDR_PORT, 0x07);
    WRITE_PORT(EC_DATA_PORT, CurrentLDN);

    WRITE_PORT(EC_ADDR_PORT, offset);
    WRITE_PORT(EC_DATA_PORT, data1);
}

void SelectLDN()
{
    WRITE_PORT(EC_ADDR_PORT, 0x07);
    WRITE_PORT(EC_DATA_PORT, CurrentLDN);
}
//==================================================================================================

//=======================================Decicated I/O Port Operation===============================
// Need EC code configuration and need BIOS decode I/O
#define HIGH_BYTE_PORT    0xA01
#define LOW_BYTE_PORT     HIGH_BYTE_PORT+1
#define DATA_BYTE_PORT    HIGH_BYTE_PORT+2

UINT8 ECRAMReadByte_DeIO(UINT8 index)
{
    WRITE_PORT(HIGH_BYTE_PORT, High_Byte);
    WRITE_PORT(LOW_BYTE_PORT, index);
    return READ_PORT(DATA_BYTE_PORT);
}

void ECRAMWriteByte_DeIO(UINT8 index, UINT8 data)
{
    WRITE_PORT(HIGH_BYTE_PORT, High_Byte);
    WRITE_PORT(LOW_BYTE_PORT, index);
    WRITE_PORT(DATA_BYTE_PORT, data);
}
//==================================================================================================

//===============================Console control interface==========================================
void SetTextColor(UINT8 TextColor, UINT8 BackColor)
{
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (TextColor, BackColor));
}

void SetPosition_X_Y(UINT8 PositionX, UINT8 PositionY)
{
    gST->ConOut->SetCursorPosition(gST->ConOut,
                                   PositionX,
                                   PositionY);
}
//==================================================================================================

//  Example:
//	ECRamWrite_Direct(0x51,0x90);
//	EC_WriteByte_KBC(0x52,0x91);
//	EC_WriteByte_PM(0x53,0x93);
//	EC_WriteByte_DeIO(0x54,0x94);
//	printf("%d\n", ECRamRead_Direct(0x51));
//	printf("%d\n", EC_ReadByte_KBC(0x52));
//	printf("%d\n", EC_ReadByte_PM(0x53));
//	printf("%d\n", EC_ReadByte_DeIO(0x54));

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

#define EC_RAM_WRITE     ECRAMWriteByte
#define EC_RAM_READ      ECRAMReadByte
#define BYTE  char
#define WORD  int
//==================================================================================================

#define STALL_MS       1000
#define KEY_NULL       0x0000
#define KEY_UP         0x0001
#define KEY_DOWN       0x0002
#define KEY_RIGHT      0x0003
#define KEY_LEFT       0x0004
#define KEY_F1         0x000B
#define KEY_F2         0x000C
#define KEY_F3         0x000D
#define KEY_F4         0x000E
#define KEY_F5         0x000F
#define KEY_F6         0x0010
#define KEY_F7         0x0011
#define KEY_F9         0x0013
#define KEY_F10        0x0014
#define KEY_F11        0x0015
#define KEY_F12        0x0016

#define KEY_TAB         0x0009
#define KEY_ENTER       0x000D
#define KEY_ESC         0x0017

#define KEY_0           0x0030
#define KEY_1           0x0031
#define KEY_2           0x0032
#define KEY_3           0x0033
#define KEY_4           0x0034
#define KEY_5           0x0035
#define KEY_6           0x0036
#define KEY_7           0x0037
#define KEY_8           0x0038
#define KEY_9           0x0039
#define KEY_A           0x0061
#define KEY_B           0x0062
#define KEY_C           0x0063
#define KEY_D           0x0064
#define KEY_E           0x0065
#define KEY_F           0x0066
#define KEY_a           0x0041
#define KEY_b           0x0042
#define KEY_c           0x0043
#define KEY_d           0x0044
#define KEY_e           0x0045
#define KEY_f           0x0046

#define POLLING_TIME_MS 50

UINT8  PageData[257]    ={0xAA};
UINT8  WriteRAMByte     =0;
UINT8  CurrentData_X    =0x07;
UINT8  CurrentData_Y    =0x07;
UINT8  MenuOpen         =0;
UINT8  CurrentMenu      =0x03;
UINT8  CurrentItem      =0x07;

UINT8  EC_CHIP_ID1      =0;
UINT8  EC_CHIP_ID2      =0;
UINT8  EC_CHIP_Ver      =0;

UINT8  ExitToolFlag     =0;
UINT8  LogSaveFlag      =0;
UINT8  WriteRAMFlag     =0;
UINT8  PortCfgFlag      =0;
UINT8  WritePortFlag    =0;
UINTN OldMode, NewMode;

EFI_TIME   CurrentTime;


typedef struct Item
{
    char    *Addr_Name;
    char    *Item_Name;
    UINT16  Addr_Num;
    UINT8   Item_X;
    UINT8   Item_Y;
}ItemSTRUCT,*pItemSTRUCT;

typedef struct Menu_1
{
    char    *Menu_Name;
    UINT8   Menu_X;
    UINT8   Menu_Y;
    pItemSTRUCT Item;
}*pMenu1STRUCT;


//==================================================================================================
#define GUI_X  1           // The position_X of the menu
#define GUI_Y  1           // The position_Y of the menu

#define Item1_X (GUI_X+1)  // The position_X of the item 1 in the menu
ItemSTRUCT Item1_Array[]=
{
    {NULL,NULL,                                16},
    {"0x1000", "SMFI",    0x1000, Item1_X, GUI_Y+ 1},
    {"0x1100", "INTC",    0x1100, Item1_X, GUI_Y+ 2},
    {"0x1200", "EC2I",    0x1200, Item1_X, GUI_Y+ 3},
    {"0x1300", "KBC",     0x1300, Item1_X, GUI_Y+ 4},
    {"0x1400", "SWUC",    0x1400, Item1_X, GUI_Y+ 5},
    {"0x1500", "PMC",     0x1500, Item1_X, GUI_Y+ 6},
    {"0x1600", "GPIO",    0x1600, Item1_X, GUI_Y+ 7},
    {"0x1700", "PS/2",    0x1700, Item1_X, GUI_Y+ 8},
    {"0x1800", "PWM",     0x1800, Item1_X, GUI_Y+ 9},
    {"0x1900", "ADC",     0x1900, Item1_X, GUI_Y+10},
    {"0x1A00", "DAC",     0x1A00, Item1_X, GUI_Y+11},
    {"0x1B00", "WUC",     0x1B00, Item1_X, GUI_Y+12},
    {"0x1C00", "SMBus",   0x1C00, Item1_X, GUI_Y+13},
    {"0x1D00", "KB Scan", 0x1D00, Item1_X, GUI_Y+14},
    {"0x1E00", "ECPM",    0x1E00, Item1_X, GUI_Y+15},
    {"0x1F00", "ETWD",    0x1F00, Item1_X, GUI_Y+16},
    {NULL,NULL}
};

#define Item2_X (GUI_X+14)
ItemSTRUCT Item2_Array[]=
{
    {NULL,NULL,                                17},
    {"0x2000", "GCTRL",    0x2000, Item2_X, GUI_Y+ 1},
    {"0x2100", "EGPC",     0x2100, Item2_X, GUI_Y+ 2},
    {"0x2200", "Batt.RAM", 0x2200, Item2_X, GUI_Y+ 3},
    {"0x2300", "CIR",      0x2300, Item2_X, GUI_Y+ 4},
    {"0x2400", "TMKBC",    0x2400, Item2_X, GUI_Y+ 5},
    {"0x2500", "DBGR",     0x2500, Item2_X, GUI_Y+ 6},
    {"0x2600", "SPI",      0x2600, Item2_X, GUI_Y+ 7},
    {"0x2700", "UART1",    0x2700, Item2_X, GUI_Y+ 8},
    {"0x2800", "UART2",    0x2800, Item2_X, GUI_Y+ 9},
    {"0x2900", "TMR",      0x2900, Item2_X, GUI_Y+10},
    {"0x2A00", "HWM(SIO)", 0x2A00, Item2_X, GUI_Y+11},
    {"0x2B00", "Unknow",   0x2B00, Item2_X, GUI_Y+12},
    {"0x2C00", "GPIO(SIO)",0x2C00, Item2_X, GUI_Y+13},
    {"0x2D00", "E-Flash",  0x2D00, Item2_X, GUI_Y+14},
    {"0x2E00", "CEC",      0x2E00, Item2_X, GUI_Y+15},
    {"0x2F00", "USBSW",    0x2F00, Item2_X, GUI_Y+16},
    {"0x3000", "PECI",     0x3000, Item2_X, GUI_Y+17},
    {NULL,NULL}
};

#define Item3_X (GUI_X+27)
ItemSTRUCT Item3_Array[]=
{
    {NULL,NULL,                                26}, // This is item count
    {"0x8000", "uC SFR",   0x8000, Item3_X, GUI_Y+ 1},
    {"0xC000", "UC RAM",   0xC000, Item3_X, GUI_Y+ 2},
    {"0x0000", "SRAM-0",    0x0000, Item3_X, GUI_Y+ 3},
    {"0x0100", "SRAM-1",    0x0100, Item3_X, GUI_Y+ 4},
    {"0x0200", "SRAM-2",    0x0200, Item3_X, GUI_Y+ 5},
    {"0x0300", "SRAM-3",    0x0300, Item3_X, GUI_Y+ 6},
    {"0x0400", "SRAM-4",    0x0400, Item3_X, GUI_Y+ 7},
    {"0x0500", "SRAM-5",    0x0500, Item3_X, GUI_Y+ 8},
    {"0x0600", "SRAM-6",    0x0600, Item3_X, GUI_Y+ 9},
    {"0x0700", "SRAM-7",    0x0700, Item3_X, GUI_Y+10},
    {"0x0800", "SRAM-8",    0x0800, Item3_X, GUI_Y+11},
    {"0x0900", "SRAM-9",    0x0900, Item3_X, GUI_Y+12},
    {"0x0A00", "SRAM-A",    0x0A00, Item3_X, GUI_Y+13},
    {"0x0B00", "SRAM-B",    0x0B00, Item3_X, GUI_Y+14},
    {"0x0C00", "SRAM-C",    0x0C00, Item3_X, GUI_Y+15},
    {"0x0D00", "SRAM-D",    0x0D00, Item3_X, GUI_Y+16},
    {"0x0E00", "SRAM-E",    0x0E00, Item3_X, GUI_Y+17},
    {"0x0F00", "SRAM-F",    0x0F00, Item3_X, GUI_Y+18},
    {"0xD000", "SRAM-G",    0x0F00, Item3_X, GUI_Y+19},
    {"0xD100", "SRAM-H",    0x0F00, Item3_X, GUI_Y+20},
    {"0xD200", "SRAM-I",    0x0F00, Item3_X, GUI_Y+21},
    {"0xD300", "SRAM-J",    0x0F00, Item3_X, GUI_Y+22},
    {"0xD400", "SRAM-K",    0x0F00, Item3_X, GUI_Y+23},
    {"0xD500", "SRAM-L",    0x0F00, Item3_X, GUI_Y+24},
    {"0xD600", "SRAM-M",    0x0F00, Item3_X, GUI_Y+25},
    {"0xD700", "SRAMN",    0x0F00, Item3_X, GUI_Y+26},
    {NULL,NULL}
};

#define Item4_X (GUI_X+40)
ItemSTRUCT Item4_Array[]=
{
    {NULL,NULL,                                15},
    {"01H", "UART1",     0x01, Item4_X, GUI_Y+ 1},
    {"02H", "UART2",     0x02, Item4_X, GUI_Y+ 2},
    {"04H", "SWUC",      0x04, Item4_X, GUI_Y+ 3},
    {"05H", "MOUSE",     0x05, Item4_X, GUI_Y+ 4},
    {"06H", "KBC",       0x06, Item4_X, GUI_Y+ 5},
    {"0AH", "CIR",       0x0A, Item4_X, GUI_Y+ 6},
    {"0FH", "SFMI",      0x0F, Item4_X, GUI_Y+ 7},
    {"10H", "RTCT",      0x10, Item4_X, GUI_Y+ 8},
    {"11H", "PMC1",      0x11, Item4_X, GUI_Y+ 9},
    {"12H", "PMC2",      0x12, Item4_X, GUI_Y+10},
    {"13H", "SSPI",      0x13, Item4_X, GUI_Y+11},
    {"14H", "PECI",      0x14, Item4_X, GUI_Y+12},
    {"17H", "PMC3",      0x17, Item4_X, GUI_Y+13},
    {"18H", "PMC4",      0x18, Item4_X, GUI_Y+14},
    {"19H", "PMC5",      0x19, Item4_X, GUI_Y+15},
    {NULL,NULL}
};

struct Menu_1 Menu1_Array[]=
{
    {NULL,                   6, 0, NULL},
    {"EC REGA   ", GUI_X   , GUI_Y, (pItemSTRUCT)&Item1_Array},
    {"EC REGB   ", GUI_X+13, GUI_Y, (pItemSTRUCT)&Item2_Array},
    {"EC RAM(F2)", GUI_X+26, GUI_Y, (pItemSTRUCT)&Item3_Array},
    {"LPC REG   ", GUI_X+39, GUI_Y, (pItemSTRUCT)&Item4_Array},
    {"LogREG(F4)", GUI_X+52, GUI_Y, NULL},
    {"Exit(ESC) ", GUI_X+65, GUI_Y, NULL},
    {NULL, 0, 0, NULL}
};
//==================================================================================================

#define ECRAM_X   (GUI_X+4)
#define ECRAM_Y   (GUI_Y+5)
void DisplayECRAM(void)
{
    UINT16 i;
    UINT16 j;
    UINT16 k;
    
    for(i=0; i<16; i++)
    {
        for(j=0,k=0; j<16; j++,k+=3)
        {
            if(0==PageData[i*16+j])
            {
                SetTextColor(EFI_LIGHTGRAY, EFI_BLACK);
            }
            else
            {
                SetTextColor(EFI_YELLOW, EFI_BLACK);
            }
            SetPosition_X_Y((UINT8)(k+ECRAM_X), (UINT8)(i+ECRAM_Y));
            printf("%02X \n", PageData[i*16+j]);
            
            if(0!=PortCfgFlag)
            {
                continue;
            }
            if((CurrentData_Y==i) && (CurrentData_X==j))
            {
                if(0==WriteRAMFlag)
                {
                    SetTextColor(EFI_LIGHTGRAY, EFI_GREEN);
                    SetPosition_X_Y(CurrentData_X*3+ECRAM_X, CurrentData_Y+ECRAM_Y);
                    printf("%02X\n", PageData[i*16+j]);
                }
                else
                {
                    SetTextColor(EFI_LIGHTRED, EFI_BLACK);
                    SetPosition_X_Y(CurrentData_X*3+ECRAM_X, CurrentData_Y+ECRAM_Y);
                    printf("%02X\n", WriteRAMByte);
                }
            }
        }
    }
}

#define RAMSTR_X   (GUI_X+56)
#define RAMSTR_Y   (GUI_Y+5)
void DisplayRAMStr(void)
{
    UINT16 i;
    UINT16 j;
    UINT16 k;

    
    for(i=0; i<16; i++)
    {
        for(j=0,k=0; j<16; j++,k++)
        {
            SetTextColor(EFI_LIGHTGRAY, EFI_BLACK);
            SetPosition_X_Y((UINT8)(RAMSTR_X+k), (UINT8)(RAMSTR_Y+i));
            if(((PageData[i*16+j])<0x20) || (((PageData[i*16+j])<0xA0) && ((PageData[i*16+j])>0x7E)))
            {
                printf(".\n");
                
                if(0!=PortCfgFlag)
                {
                    continue;
                }
            
                if((CurrentData_Y==i) && (CurrentData_X==j))
                {
                    SetPosition_X_Y((UINT8)(RAMSTR_X+k), (UINT8)(RAMSTR_Y+i));
                    SetTextColor(EFI_LIGHTGRAY, EFI_GREEN);
                    printf(".\n");
                }
            }
            else
            {
                printf("%c\n", PageData[i*16+j]);
                
                if(0!=PortCfgFlag)
                {
                    continue;
                }
                
                if((CurrentData_Y==i) && (CurrentData_X==j))
                {
                    SetPosition_X_Y((UINT8)(RAMSTR_X+k), (UINT8)(RAMSTR_Y+i));
                    SetTextColor(EFI_LIGHTGRAY, EFI_GREEN);
                    printf("%c\n", PageData[i*16+j]);
                }
            }
        }
        printf("\n");
    }
}

#define CHIP_ID_X   (GUI_X+1)
#define CHIP_ID_Y   (GUI_Y+23)
void DisplayECChipID(void)
{
    High_Byte = 0x20;
    EC_CHIP_ID1 = EC_RAM_READ(0x00);
    EC_CHIP_ID2 = EC_RAM_READ(0x01);
    EC_CHIP_Ver = EC_RAM_READ(0x02);
    
    // Clear blank area
    SetTextColor(EFI_LIGHTCYAN, EFI_BLACK);
    SetPosition_X_Y(CHIP_ID_X, CHIP_ID_Y-2);
    printf("                                                  \n");
    printf("                                                  \n");
    printf("                                                  \n");
    printf("                                                  \n");
    printf("                                                  \n");
    printf("                                                  \n");
    printf("                                                  \n");
    printf("                                                  \n");
    
    // Display EC Chip ID
    SetTextColor(EFI_LIGHTCYAN, EFI_BLACK);
    SetPosition_X_Y(CHIP_ID_X, CHIP_ID_Y);
    printf("CHIP ID:%02X%02X-%02X\n", EC_CHIP_ID1, EC_CHIP_ID2, EC_CHIP_Ver);
    
    // Dispaly name
    SetPosition_X_Y(CHIP_ID_X+17, CHIP_ID_Y);
    printf("Morgen@ITE-EC\n");
}

void DisplayGUI(void)
{
    UINT8 i;
    
    // Display Menu
    SetTextColor(EFI_LIGHTMAGENTA, EFI_BLACK);
    for(i=1; NULL!=Menu1_Array[i].Menu_Name; i++)
    {
        SetPosition_X_Y(Menu1_Array[i].Menu_X, Menu1_Array[i].Menu_Y);
        printf(" %s\n", Menu1_Array[i].Menu_Name);
    }
    
    // Display X index
    SetTextColor(EFI_LIGHTGREEN, EFI_BLACK);
    SetPosition_X_Y(ECRAM_X-1, ECRAM_Y-1);
    printf(" 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F       \n");
    
    // Display Y index
    for(i=0; i<16; i++)
    {
        SetPosition_X_Y(ECRAM_X-4, ECRAM_Y+i);
        printf(" %X0 \n", i);
        
        // Clear display gap when menu open
        SetPosition_X_Y(GUI_X+51, GUI_Y+3+i);
        printf("   \n", i);
    }

    // Clear blank area when menu open
    SetTextColor(EFI_LIGHTMAGENTA, EFI_BLACK);
    SetPosition_X_Y(GUI_X, GUI_Y+1);
    printf("                                                                  \n");
    printf("                                                                  \n");
    printf("                                                                  \n");
    
    // Display RAM Data
    DisplayECRAM();
    
    // Display Current data index
    SetTextColor(EFI_LIGHTMAGENTA, EFI_BLACK);
    SetPosition_X_Y(ECRAM_X-4, ECRAM_Y-1);
    printf(" %X%X\n", CurrentData_Y, CurrentData_X);
    
    // Display RAM String
    DisplayRAMStr();
    
    DisplayECChipID();
    
    // Display Port Addr and data
    SetTextColor(EFI_LIGHTCYAN, EFI_BLACK);
    SetPosition_X_Y(CHIP_ID_X, CHIP_ID_Y+1);
    printf("PORT:%X %X\n", EC_ADDR_PORT, EC_DATA_PORT);
    
    // Display Current Register Address and Name
    if((NULL!=Menu1_Array[CurrentMenu].Item))
    {
        SetTextColor(EFI_LIGHTCYAN, EFI_BLACK);
        SetPosition_X_Y(ECRAM_X, ECRAM_Y-2);
        printf("Address : %#X\n", (Menu1_Array[CurrentMenu].Item)[CurrentItem].Addr_Num);
        SetPosition_X_Y(ECRAM_X, ECRAM_Y-3);
        printf("Name    : %s\n", (Menu1_Array[CurrentMenu].Item)[CurrentItem].Item_Name);
    }
}

void DisplayMenu(void)
{
    UINT16 i;
    pItemSTRUCT temp;
    
    if(1==MenuOpen)
    {
        DisplayGUI();
        MenuOpen++;
    }
    
    // Display Item
    temp = Menu1_Array[CurrentMenu].Item;

    SetTextColor(EFI_LIGHTCYAN, EFI_BLUE);
    SetPosition_X_Y(Menu1_Array[CurrentMenu].Menu_X, Menu1_Array[CurrentMenu].Menu_Y);
    printf(" %s\n", Menu1_Array[CurrentMenu].Menu_Name);
    
    if(NULL!=temp)
    {
        for(i=1; NULL!=temp[i].Addr_Name; i++) // must start 1 !!!
        {
            if(i==CurrentItem)
            {
                SetTextColor(EFI_LIGHTRED, EFI_BLUE);
            }
            else
            {
                SetTextColor(EFI_LIGHTCYAN, EFI_BLUE);
            }
            
            SetPosition_X_Y(temp[i].Item_X, temp[i].Item_Y);
            printf("%s  %-9s\n", temp[i].Addr_Name, temp[i].Item_Name);
        }
    }
}

void PortConfigure(UINT16 key)
{
    if(0==WritePortFlag)
    {
        if((key>=KEY_0)&&(key<=KEY_9))
        {
            EC_ADDR_PORT = (UINT8)((key-0x30)<<4);
            WritePortFlag = 1;
        }
        else if((key>=KEY_A)&&(key<=KEY_F))
        {
            EC_ADDR_PORT = (UINT8)((key-KEY_A+0x0A)<<4);
            WritePortFlag = 1;
        }
        else if((key>=KEY_a)&&(key<=KEY_f))
        {
            EC_ADDR_PORT = (UINT8)((key-KEY_a+0x0A)<<4);
            WritePortFlag = 1;
        }
    }
    else if(1==WritePortFlag)
    {
        if((key>=KEY_0)&&(key<=KEY_9))
        {
            EC_ADDR_PORT = (UINT8)(EC_ADDR_PORT | (key-0x30));
            WritePortFlag = 2;
        }
        else if((key>=KEY_A)&&(key<=KEY_F))
        {
            EC_ADDR_PORT = (UINT8)(EC_ADDR_PORT | (key-KEY_A+0x0A));
            WritePortFlag = 2;
        }
        else if((key>=KEY_a)&&(key<=KEY_f))
        {
            EC_ADDR_PORT = (UINT8)(EC_ADDR_PORT | (key-KEY_a+0x0A));
            WritePortFlag = 2;
        }
    }
    
    // Display Port Addr and data
    SetTextColor(EFI_LIGHTRED, EFI_LIGHTGRAY);
    SetPosition_X_Y(CHIP_ID_X, CHIP_ID_Y+1);
    printf("PORT:%02X %02X\n", EC_ADDR_PORT, EC_DATA_PORT);
    
    if(2==WritePortFlag)
    {
        PortCfgFlag=0;
        WritePortFlag =0;
        EC_DATA_PORT = EC_ADDR_PORT+1;
        DisplayGUI();
    }
    
}

void HandleScanCode(UINT16 key)
{
    pItemSTRUCT temp;
    
    if(0!=PortCfgFlag)
    {
        return;
    }
    
    // Clear write RAM flag 
    if(0!=WriteRAMFlag)
    {
        return;
    }
    
    if(0==MenuOpen)
    {
        switch(key)
        {
            case KEY_UP:
                (CurrentData_Y>0)?(CurrentData_Y--):(CurrentData_Y=15);
                break;
            case KEY_DOWN:
                (CurrentData_Y<15)?(CurrentData_Y++):(CurrentData_Y=0);
                break;
            case KEY_LEFT:
                (CurrentData_X>0)?(CurrentData_X--):(CurrentData_X=15);
                break;
            case KEY_RIGHT:
                (CurrentData_X<15)?(CurrentData_X++):(CurrentData_X=0);
                break;
            case KEY_F2:
                MenuOpen =1;
                break;
            case KEY_F4:
                LogSaveFlag =1;
                break;
            default:break;
        }
        
        // Display Current data index
        SetTextColor(EFI_LIGHTMAGENTA, EFI_BLACK);
        SetPosition_X_Y(ECRAM_X-4, ECRAM_Y-1);
        printf(" %X%X\n", CurrentData_Y, CurrentData_X);
    }
    else
    {
        temp = Menu1_Array[CurrentMenu].Item;
        switch(key)
        {
            case KEY_UP:
                (CurrentItem>1)?(CurrentItem--):(CurrentItem=(UINT8)temp[0].Addr_Num);
                break;
            case KEY_DOWN:
                (CurrentItem<temp[0].Addr_Num)?(CurrentItem++):(CurrentItem=1);
                break;
            case KEY_LEFT:
                (CurrentMenu>1)?(CurrentMenu--):(CurrentMenu=Menu1_Array[0].Menu_X);
                MenuOpen =1;
                break;
            case KEY_RIGHT:
                (CurrentMenu<Menu1_Array[0].Menu_X)?(CurrentMenu++):(CurrentMenu=1);
                MenuOpen=1;
                break;
            default:break;
        }
    }
}

void HandleUnicodeChar(UINT16 key)
{
    if(0!=PortCfgFlag)
    {
        return;
    }
    
    switch(key)
    {
        case KEY_ENTER:
            if(0!=MenuOpen)
            {
                MenuOpen =0;
                DisplayGUI();
                if(Menu1_Array[0].Menu_X==CurrentMenu)
                {
                    ExitToolFlag=1;
                }
            
                if(5==CurrentMenu)   // LogREG(F4)
                {
                    LogSaveFlag=1;
                }
            }
            break;
        case KEY_TAB:
            if((0==MenuOpen) && (0==WriteRAMFlag))
            {
                PortCfgFlag=1;
            }
        default:break;
    }
    
    if(0==WriteRAMFlag)
    {
        if((key>=KEY_0)&&(key<=KEY_9))
        {
            WriteRAMByte = (UINT8)((key-0x30)<<4);
            WriteRAMFlag = 1;
        }
        else if((key>=KEY_A)&&(key<=KEY_F))
        {
            WriteRAMByte = (UINT8)((key-KEY_A+0xA)<<4);
            WriteRAMFlag = 1;
        }
        else if((key>=KEY_a)&&(key<=KEY_f))
        {
            WriteRAMByte = (UINT8)((key-KEY_a+0xA)<<4);
            WriteRAMFlag = 1;
        }
    }
    else
    {
        if((key>=KEY_0)&&(key<=KEY_9))
        {
            WriteRAMByte = (UINT8)(WriteRAMByte | (key-0x30));
            WriteRAMFlag = 2;
        }
        else if((key>=KEY_A)&&(key<=KEY_F))
        {
            WriteRAMByte = (UINT8)(WriteRAMByte | (key-KEY_A+0xA));
            WriteRAMFlag = 2;
        }
        else if((key>=KEY_a)&&(key<=KEY_f))
        {
            WriteRAMByte = (UINT8)(WriteRAMByte | (key-KEY_a+0xA));
            WriteRAMFlag = 2;
        }
    }
}

void UpdateECRAM(void)
{
    UINT16 i;
    pItemSTRUCT temp;
    
    temp = Menu1_Array[CurrentMenu].Item;
    
    if(NULL==temp)
    {
        return;
    }
    
    if(4 == CurrentMenu)  // Item4_Array, for logic Device register
    {
        CurrentLDN = (UINT8)(temp[CurrentItem].Addr_Num);
        SelectLDN();
        
        for(i=0;i<256;i++)
        {
            PageData[i]=ReadSIOReg((UINT8)i);
        }
    }
    else
    {
        High_Byte = (UINT8)(temp[CurrentItem].Addr_Num>>8);
        for(i=0;i<256;i++)
        {
            PageData[i]=EC_RAM_READ((UINT8)i);
        }
    }
}


void SaveAllREG(void)
{
    UINT16 i,j,k,l;
    pItemSTRUCT temp;
    BYTE LogFileName[64]={0};
    UINT8 OneByte;
    
    FILE *LogFileH;
    
    // Display prompt information
    SetTextColor(EFI_LIGHTMAGENTA, EFI_BLACK);
    SetPosition_X_Y(ECRAM_X+8, ECRAM_Y+6);
    printf("                         \n");
    SetPosition_X_Y(ECRAM_X+8, ECRAM_Y+7);
    printf("  Save All Register...   \n");
    SetPosition_X_Y(ECRAM_X+8, ECRAM_Y+8);
    printf("                         \n");
    
    // Get file name by time
    gRT->GetTime(&CurrentTime, NULL);
    sprintf(LogFileName, "ECAllreg%04d%02d%02d%02d%02d%02d.log",
            CurrentTime.Year, CurrentTime.Month, CurrentTime.Day, 
            CurrentTime.Hour, CurrentTime.Minute, CurrentTime.Second);

    //printf("\nLog  :  %s\n", LogFileName);
    
    if((LogFileH = fopen(LogFileName, "w+")) == NULL)
    {
        printf("ceart file wrong!\n");
        return ;
    }

    fprintf(LogFileH, "************************************************************\n");
    fprintf(LogFileH, "**        ITE-EC RAM Log Windows Utility Version : %s   **\n",TOOLVER);
    fprintf(LogFileH, "**                                                        **\n");
    fprintf(LogFileH, "**                  Create by Morgen                      **\n");
    fprintf(LogFileH, "************************************************************\n");

    fprintf(LogFileH, "==============================================================\r\n");
    fprintf(LogFileH, "=====================EC Register information==================\r\n");
    fprintf(LogFileH, "==============================================================\r\n");
        
    for(i=1; (NULL!=Menu1_Array[i].Item); i++)
    {
        temp = Menu1_Array[i].Item;
        
        if(4 == i)  // Item4_Array, for logic Device register
        {
            fprintf(LogFileH, "==============================================================\r\n");
            fprintf(LogFileH, "==================Logic Device configuration==================\r\n");
            fprintf(LogFileH, "==============================================================\r\n");
        }
        
        for(j=1; (NULL!=temp[j].Addr_Name); j++)
        {
            
            
            fprintf(LogFileH, "\r\n------------------------ %04X -------------------------\r\n",
                                temp[j].Addr_Num);
            fprintf(LogFileH, "     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n\r\n");
            
            if(4 == i)  // Item4_Array, for logic Device register
            {
                CurrentLDN = (UINT8)(temp[j].Addr_Num);
                SelectLDN();
                for(k=0; k<16; k++)
                {
                    fprintf(LogFileH, "%X0   ", k);
                    for(l=0; l<16; l++)
                    {
                        OneByte=ReadSIOReg((UINT8)(k*16+l));
                    
                        fprintf(LogFileH, "%02X ", OneByte);
                    }
                    fprintf(LogFileH, "\n");
                }
            }
            else
            {
                High_Byte = (UINT8)(temp[j].Addr_Num>>8);
                for(k=0; k<16; k++)
                {
                    fprintf(LogFileH, "%X0   ", k);
                    for(l=0; l<16; l++)
                    {
                        OneByte=EC_RAM_READ((UINT8)(k*16+l));
                    
                        fprintf(LogFileH, "%02X ", OneByte);
                    }
                    fprintf(LogFileH, "\r\n");
                }
            }
        }
    }
    
    fclose(LogFileH);
}


// Init Screen
void SetToolScreen()
{
    UINTN MAXMODE;
    UINTN i;
    UINTN Col[10]={0};
    UINTN Row[10]={0};
    

    MAXMODE = gST->ConOut->Mode->MaxMode;
    //printf("MAXMODE=%d\n", MAXMODE);
    NewMode = OldMode = gST->ConOut->Mode->Mode;
    //printf("OldMode = %d\n\n", OldMode);

    for(i=0; i<MAXMODE; i++)
    {
        gST->ConOut->QueryMode(gST->ConOut, i, &Col[i], &Row[i]);
        //printf("Mode %ld: Col-%ld Row-%ld\n", i, Col[i], Row[i]);
    }

    for(i=0; i<MAXMODE; i++)
    {
        if((Col[i]==100) && (Row[i]==31))
        {
            NewMode = i;
            if(NewMode != MAXMODE)
            {
                // Set Console mode
                gST->ConOut->SetMode(gST->ConOut, NewMode);
            }
            break;
        }
    }
    
    // Set Cursor
    gST->ConOut->ClearScreen (gST->ConOut);
    gST->ConOut->EnableCursor (gST->ConOut, FALSE);
}

void ClearToolScreen()
{
    gST->ConOut->SetMode(gST->ConOut, OldMode);
    gST->ConOut->EnableCursor (gST->ConOut, TRUE);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK));
    gST->ConOut->ClearScreen (gST->ConOut);
}

void Config_Chip()
{
    UINT8 ConfigData;
    UINT8 i;
    // ITE IT-557x chip is DLM architecture for EC  RAM and It's support 6K/8K RAM.
    // If used RAM less  than 4K, you can access EC RAM form 0x000--0xFFF by 4E/4F IO port
    // If used RAM more than 4K, RAM address change to 0xC000
    // If you want to access EC RAM by 4E/4F IO port, you must set as follow register first
    // REG_1060[BIT7]
    if(0x55==EC_CHIP_ID1)
    {
        High_Byte = 0x20;
        ConfigData = EC_RAM_READ(0x60);
        ConfigData = ConfigData | 0x80;
        EC_RAM_WRITE(0x60, ConfigData);
        
        // Modify Item3 count to 26
        Item3_Array[0].Addr_Num = 26;
        
        // Modify Item address
        for(i=19; NULL!=Item3_Array[i].Addr_Name; i++) // EC RAM 0xD000
        {
            Item3_Array[i].Addr_Num = 0xD000 + (i-19)*0x100;
        }
        Item3_Array[2].Addr_Num = 0xE000;
    }
    else
    {
        // Modify Item3 count to 18
        Item3_Array[0].Addr_Num = 18;
    }
}
/*
//--------------------------------------------------------------------------------------------------
BYTE PcIoReadB(DWORD Address)
{
    return (BYTE)inp((DWORD)Address);
}


DWORD PcIoWriteB(DWORD Address, BYTE Data)
{
    return outp((DWORD)Address, Data);
}

BYTE ReadWhichLdn(void)
{
    PcIoWriteB(Sio_index, 0x07);
    return PcIoReadB(Sio_index+1);
}

BYTE ReadSioReg(BYTE Ldn, BYTE offset)
{
    PcIoWriteB(Sio_index, offset);
    return PcIoReadB(Sio_index+1);
}

void WriteSioReg(BYTE Ldn, BYTE offset, BYTE data)
{
    PcIoWriteB(Sio_index, 0x07);
    PcIoWriteB((Sio_index+1), Ldn);

    PcIoWriteB(Sio_index, offset);
    PcIoWriteB((Sio_index+1), data);
}
//--------------------------------------------------------------------------------------------------
*/

int main(int Argc, char **Argv)
{
    EFI_INPUT_KEY   Key;
    EFI_STATUS      Status;
    pItemSTRUCT temp;
    UINT16 i;
    
    Key.ScanCode=KEY_NULL;

    // Step 1
    SetToolScreen();
    
    // Step 2
    DisplayGUI();
    DisplayECChipID();
    Config_Chip();
    
    // step3
    while (KEY_ESC!=Key.ScanCode)
    {
        // Get Key event
        Status= gST -> ConIn -> ReadKeyStroke(gST->ConIn,&Key);
        if (Status == EFI_SUCCESS)
        {
            // Debug
            //SetPosition_X_Y(GUI_X+30, CHIP_ID_Y-2);
            //printf("Scancode[%03X]  UnicodeChar[%03X]\n",Key.ScanCode,Key.UnicodeChar);
            
            // F1-F4 to menu
            if(0!=Key.ScanCode)
            {
                HandleScanCode(Key.ScanCode);
                
                if(0!=MenuOpen)
                {
                    DisplayMenu();
                }
            }
            // 0-F to modify the RAM
            else if(0!=Key.UnicodeChar)
            {
                HandleUnicodeChar(Key.UnicodeChar);
                
                if(1==ExitToolFlag)
                {
                    break;
                }
                
                if(2==WriteRAMFlag)
                {
                    if(4 == CurrentMenu)  // Item4_Array, for logic Device register
                    {
                        if((0==CurrentData_Y) && (0x07==CurrentData_X)) // LDN modify
                        {
                            temp = Menu1_Array[CurrentMenu].Item;
                            for(i=1; (NULL!=temp[i].Addr_Name); i++)
                            {
                                if(WriteRAMByte == temp[i].Addr_Num)
                                {
                                    CurrentItem  = (UINT8)i;
                                    DisplayGUI();
                                }
                            }
                        }
                        else
                        {
                            WriteSIOReg((CurrentData_X+CurrentData_Y*16), WriteRAMByte);
                        }
                    }
                    else
                    {
                        EC_RAM_WRITE((CurrentData_X+CurrentData_Y*16), WriteRAMByte);
                    }
                
                    WriteRAMFlag = 0;
                    WriteRAMByte = 0;
                }
            }
            if(0!=PortCfgFlag)
            {
                PortConfigure(Key.UnicodeChar);
                continue;
            }
        }
        else
        {
            if(0==MenuOpen)
            {
                // Update RAM
                if(0==WritePortFlag)
                {
                    UpdateECRAM();
                }
                
                // Display RAM Data
                DisplayECRAM();
    
                // Display RAM String
                DisplayRAMStr();
            }
            
            if(1==LogSaveFlag)
            {
                LogSaveFlag=0;
                SaveAllREG();
                
            }
        }
        
        gBS->Stall(STALL_MS*POLLING_TIME_MS);
        
        // Debug
        //SetPosition_X_Y(GUI_X+5, CHIP_ID_Y-1);
        //printf("P:%X  W:%X WPF:%X\n", PortCfgFlag, WriteRAMFlag, WritePortFlag);
        
        
        // Get and Display Time
        gRT->GetTime(&CurrentTime, NULL);
        SetPosition_X_Y(CHIP_ID_X+50, CHIP_ID_Y);
        SetTextColor(EFI_LIGHTCYAN, EFI_BLACK);
        printf("%04d-%02d-%02d %02d:%02d:%02d\n",
                CurrentTime.Year, CurrentTime.Month, CurrentTime.Day,
                CurrentTime.Hour, CurrentTime.Minute, CurrentTime.Second);
    }

    ClearToolScreen();
    printf("\r\nThank you !!!\r\n");
    
    return EFI_SUCCESS;
}



















