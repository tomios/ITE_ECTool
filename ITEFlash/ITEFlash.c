#define   TOOLVER   "V1.0"
//===========================================================================
// V1.0
// 1. 
//===========================================================================

/* Copyright (c) 2011-2099, ZXQ CO. LTD. All rights reserved.
   This application For ITE EC Flash function
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

EFI_FILE_PROTOCOL           *Root;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
EFI_FILE_PROTOCOL *FileHandle=0;
EFI_FILE_PROTOCOL *BKFileHandle=0;

#define STALL_MS  1000
#define Delay_TIME_MS  1000

/**************************TYPE define**************************************************/
#define Version      "2.4"
#define Vendor       "ITE"
/**************************TYPE define end**********************************************/

/**************************Variable define**********************************************/
UINT8 SPIFlashID[5];
UINT8 *str1;
UINT8 *str2;
UINT8 *str3;
UINT8 *str4;

UINT8  BackUp=0;     // Default is not backup
UINT8  FW_Size=128;  // Default is 128K
UINT8  ResetFlag=0;  // Default is not reset
UINT8  UpdateFlashFlag=0;  // Default is not reset

FILE   *pECFile;
FILE   *pBackFile;
/**************************Variable define end******************************************/

/***************************The Command of EC addition SPI Flash************************/
// commonly used
#define SPICmd_WRSR            0x01   // Write Status Register
#define SPICmd_PageProgram     0x02   // To Program Page, 1-256 bytes data to be programmed
                                      // into memory in a single operation
#define SPICmd_READ            0x03   // Read Data Bytes from Memory at Normal ReadMode
#define SPICmd_WRDI            0x04   // Write diaable
#define SPICmd_RDSR            0x05   // Read Status Register
#define SPICmd_WREN            0x06   // Write Enable
#define SPICmd_FastRead        0x0B   // Read Data Bytes from Memory at Fast Read Mode
#define SPICmd_ChipErase       0xC7   // Chip Erase
#define SPICmd_JEDEC_ID        0x9F   // JEDEC ID READ command 
                                      //(manufacturer and product ID of devices)
#define SPICmd_EWSR            0x50   // Enable write Status Register

//PMC SPI Flash
#define SPICmd_PMCDeviceID1    0xAB   // Read Manufacturer and Product ID
#define SPICmd_PMCDeviceID2    0x90   // Read Manufacturer and Device ID

//Winband SPI Flash
#define SPICmd_WBDeviceID      0x4B   // Read unique ID

// ITE eFlash cmd
#define SPICmd_1KSectorErase   0xD7    // Sector Erase, 1K bytes
#define SPICmd_AAIBytePro      0xAF
#define SPICmd_AAIWordPro      0xAD
/****************************************************************************************/

/***************************The Flash manufacturer ID************************************/
#define SSTID                   0xBF
#define WinbondID               0xEF  // Verification
#define AtmelID                 0x9F
#define STID                    0x20
#define SpansionID              0x01
#define MXICID                  0xC2
#define AMICID                  0x37
#define EONID                   0x1C
#define ESMTID                  0x8C
#define PMCID                   0x7F  // Verification
#define GDID                    0xC8  // Verification
#define ITEID                   0xFF  // Verification
/****************************************************************************************/
typedef struct SPIDeviceInfo
{
    UINT8 device_id;
    UINT8 *device_string;
    UINT8 *device_size_string;
}*pSPIDeviceInfo;

struct SPIFlashIDInfo
{
    UINT8 vendor_id;
    UINT8 *vendor_name;
    pSPIDeviceInfo device_info;
};

// GigaDevice Flash
struct SPIDeviceInfo GDFlash[]=
{
    {0x10, "GD25D05B", "64K"         },
    {0x11, "GD25D10B", "128K"        },
    {0x00, NULL, NULL   }
};

// PMC Flash
struct SPIDeviceInfo PMCFlash[]=
{
    {0x20, "Pm25LD512C", "64K"            },
    {0x7B, "Pm25LV512A", "unknow size"    },
    {0x7C, "Pm25LV010A", "unknow size"    },
    {0x7D, "Pm25LV020",  "unknow size"    },
    {0x7E, "Pm25LV040",  "unknow size"    },
    {0x00, NULL, NULL   }
};

//Winbond Flash
struct SPIDeviceInfo WBFlash[]=
{
    {0x10, "W25X05CL", "8K"    },
    {0x11, "W25X10CL", "128K"  },
    {0x12, "W25X20CL", "256K"  },
    {0x14, "W25Q80DV", "1M"    },
    {0x15, "W25Q16",   "2M"    },
    {0x16, "W25Q32",   "4M"    },
    {0x00, NULL, NULL   }
};

// ITE e-flash
struct SPIDeviceInfo ITEFlash[]=
{
    {0xFE, "ITE e-flash", "128K"         },
    {0x00, NULL, NULL   }
};

struct SPIFlashIDInfo  aSPIFlashIDInfo[]=
{
    {   GDID,       "GigaDevice" , (pSPIDeviceInfo)&GDFlash},
    {   WinbondID,  "Winbond"    , (pSPIDeviceInfo)&WBFlash},
    {   PMCID,      "PMC"        , (pSPIDeviceInfo)&PMCFlash},
    {   ITEID,      "ITE"        , (pSPIDeviceInfo)&ITEFlash},
    {   0x00,       NULL,  NULL}
};
/****************************************************************************************/

/****************************************PM channel**************************************/
UINT8 PM_STATUS_PORT66          =0x66;
UINT8 PM_CMD_PORT66             =0x66;
UINT8 PM_DATA_PORT62            =0x62;
#define PM_OBF                    0x01
#define PM_IBF                    0x02
//------------wait EC PM channel port66 output buffer full-----/
void Wait_PM_OBF (void)
{
    while (!(IoRead8(PM_STATUS_PORT66) & PM_OBF));
}

//------------wait EC PM channel port66 input buffer empty-----/
void Wait_PM_IBE (void)
{
    while (IoRead8(PM_STATUS_PORT66) & PM_IBF);
}

//------------send command by EC PM channel--------------------/
void Send_cmd_by_PM(UINT8 Cmd)
{
    Wait_PM_IBE();
    IoWrite8(PM_CMD_PORT66, Cmd);
    Wait_PM_IBE();
}

//------------send data by EC PM channel-----------------------/
void Send_data_by_PM(UINT8 Data)
{
    Wait_PM_IBE();
    IoWrite8(PM_DATA_PORT62, Data);
    Wait_PM_IBE();
}

//-------------read data from EC PM channel--------------------/
UINT8 Read_data_from_PM(void)
{
    Wait_PM_OBF();
    return(IoRead8(PM_DATA_PORT62));
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
/****************************************PM channel end************************************/
/****************************************Flash operation-interface*************************/
#define  _EnterFollowMode   0x01
#define  _ExitFollowMode    0x05
#define  _SendCmd           0x02
#define  _SendByte          0x03
#define  _ReadByte          0x04

void FollowMode(UINT8 mode)
{
    Send_cmd_by_PM(mode);
}

void SendCmdToFlash(UINT8 cmd)
{
    Send_cmd_by_PM(_SendCmd);
    Send_cmd_by_PM(cmd);
}

void SendByteToFlash(UINT8 data)
{
    Send_cmd_by_PM(_SendByte);
    Send_cmd_by_PM(data);
}

UINT8 ReadByteFromFlash(void)
{
    Send_cmd_by_PM(_ReadByte);
    return(Read_data_from_PM());
}

void WaitFlashFree(void)
{
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_RDSR);
    while(0x00 != (ReadByteFromFlash()&0x01));
    FollowMode(_ExitFollowMode);
}

void FlashWriteEnable(void)
{
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_WRSR);
    SendByteToFlash(0x00);

    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_WREN);

    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_RDSR);
    while(0x00 == (ReadByteFromFlash()&0x02));
    FollowMode(_ExitFollowMode);
}

void FlashWriteDisable(void)
{
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_WRDI);

    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_RDSR);
    while(0x00 != (ReadByteFromFlash()&0x02));
    FollowMode(_ExitFollowMode);
}

// For ITE e-flash
void FlashStatusRegWriteEnable(void)
{
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_WREN);

    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_EWSR);

}
/******************************************************************************************/
/******************************************************************************************/
void Read_SPIFlash_JEDEC_ID(void)
{
    UINT8 index;

    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_JEDEC_ID);
    for(index=0x00;index<3;index++)
    {
        SPIFlashID[index]=ReadByteFromFlash();
    }
    FollowMode(_ExitFollowMode);
}

void Show_FlashInfo(void)
{
    UINT8 i,j;
    struct SPIDeviceInfo *ptmp;
    printf("Flash ID          : %x %x %x\n", SPIFlashID[0], SPIFlashID[1], SPIFlashID[2]);

    for(i=0;aSPIFlashIDInfo[i].vendor_id!=0;i++)
    {
        if(aSPIFlashIDInfo[i].vendor_id == SPIFlashID[0])
        {
            ptmp=aSPIFlashIDInfo[i].device_info;
            printf("Flash Manufacture : %s\n", aSPIFlashIDInfo[i].vendor_name);
            for(j=0;(ptmp+j)->device_id != 0;j++)
            {
                if((ptmp+j)->device_id == SPIFlashID[2])
                {
                    printf("Flash Model       : %s\n", (ptmp+j)->device_string);
                    printf("Flash Size        : %s\n", (ptmp+j)->device_size_string);
                    return;
                }
            }
            printf("Unknow Flash Model\n");
            return;
        }
    }
    printf("Unknow Flash\n");
    return;
}

void Flash_BackUp(void)
{
    CHAR16 counter;
    UINTN  BufSize;
    UINT8 *strBK1;
    UINT8 *strBK2;
    UINT8 *strBK3;
    UINT8 *strBK4;

    Print(L"   Back up...       : ");
    
    if((pBackFile = fopen("ECbackup.bin", "wb")) == NULL)
    {
        Print(L"Creat backup file wrong!\n");
        return;
    }

    strBK1=(UINT8 *)malloc(0x8000);
    strBK2=(UINT8 *)malloc(0x8000);
    strBK3=(UINT8 *)malloc(0x8000);
    strBK4=(UINT8 *)malloc(0x8000);

    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_FastRead);
    SendByteToFlash(0x00);   // addr[24:15]
    SendByteToFlash(0x00);   // addr[8:14]
    SendByteToFlash(0x00);   // addr[0:7]
    SendByteToFlash(0x00);   // fast read dummy byte

    for(counter=0x0000;counter<0x8000;counter++)  //read 32k
    {
        strBK1[counter]=ReadByteFromFlash();
        if(0 == counter%0x800)
            Print(L"#");
    }

    for(counter=0x0000;counter<0x8000;counter++)  //read 32k
    {
        strBK2[counter]=ReadByteFromFlash();
        if(0 == counter%0x800)
            Print(L"#");
    }
    
    if(128==FW_Size)
    {
        for(counter=0x0000;counter<0x8000;counter++)  //read 32k
        {
            strBK3[counter]=ReadByteFromFlash();
            if(0 == counter%0x800)
                Print(L"#");
        }

        for(counter=0x0000;counter<0x8000;counter++)  //read 32k
        {
            strBK4[counter]=ReadByteFromFlash();
            if(0 == counter%0x800)
                Print(L"#");
        }
    }
        
    FollowMode(_ExitFollowMode);

    BufSize = 0x8000;
    //Status = BKFileHandle ->Write(BKFileHandle, &BufSize, strBK1);
    //Status = BKFileHandle ->Write(BKFileHandle, &BufSize, strBK2);
    fwrite(strBK1,1,0x8000,pBackFile);
    fwrite(strBK2,1,0x8000,pBackFile);
    
    if(128==FW_Size)
    {
        //Status = BKFileHandle ->Write(BKFileHandle, &BufSize, strBK3);
        //Status = BKFileHandle ->Write(BKFileHandle, &BufSize, strBK4);
        fwrite(strBK3,1,0x8000,pBackFile);
        fwrite(strBK4,1,0x8000,pBackFile);
    }

    Print(L"   -- BackUp OK. \n\n");

    free(strBK1);
    free(strBK2);
    free(strBK3);
    free(strBK4);
    fclose(pBackFile);
}

// For ITE e-flash
void Block_1K_Erase(UINT8 addr2,UINT8 addr1,UINT8 addr0)
{
    FlashStatusRegWriteEnable();
    FlashWriteEnable();

    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_1KSectorErase);
    SendByteToFlash(addr2);
    SendByteToFlash(addr1);
    SendByteToFlash(addr0);
    WaitFlashFree();
}

// For ITE 128K e-flash
void ITE_eFlash_Erase(void)
{
    UINT16 i,j;
    Print(L"   Eraseing...      : ");

    for(i=0; i<0x02; i++)           // 2*64K
    {
        for(j=0; j<0x100; j+=0x04)  // 64K
        {
            Block_1K_Erase((UINT8)i, (UINT8)j, 0x00);
            if(0 == j%0x8)
                Print(L"#");
        }
    }

    Print(L"   -- Erase OK. \n\n");
}

// For ITE 128K e-flash
UINT8 ITE_eFlash_Erase_Verify(void)
{
    CHAR16 counter;
    UINT8 Dat;
    UINT8 i;
    Print(L"   Erase Verify...  : ");

    FlashWriteDisable();
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_FastRead);
    SendByteToFlash(0x00);   // addr[24:15]
    SendByteToFlash(0x00);   // addr[8:14]
    SendByteToFlash(0x00);   // addr[0:7]
    SendByteToFlash(0x00);   // fast read dummy byte

    for(i=0; i<4; i++)
    {
        for(counter=0x0000;counter<0x8000;counter++)  // 32K
        {
            Dat=ReadByteFromFlash();
            if(Dat!=0xFF) //verify Byte is all 0xFF, otherwise  error
            {
                WaitFlashFree();
                Print(L" Dat is: %d \n",Dat);
                Print(L" Counter is: %d \n",counter);
                Print(L" Block is: %d \n",i);
                Print(L" -- Verify Fail. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
                Print(L"#");
        }
    }
    
    WaitFlashFree();
    Print(L"   -- Verify OK. \n\n");
    return(TRUE);
}

// If SPI flash support chip erase command
// send this command to erase all flash
void Erase_Flash(void)
{
    CHAR16 counter;

    Print(L"   Eraseing...      : ");

    FlashWriteEnable();
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_ChipErase);
    FollowMode(_ExitFollowMode);

    for(counter=0x00;counter<0x20;counter++)
    {
        Print(L"#");
    }
    
    if(128==FW_Size)
    {
        for(counter=0x00;counter<0x20;counter++)
        {
            Print(L"#");
        }
    }
    Print(L"   -- Erase OK. \n\n");
}

UINT8 Erase_Flash_Verify()
{
    CHAR16 counter;
    UINT8 Dat;
    Print(L"   Erase Verify...  : ");

    FlashWriteDisable();
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_FastRead);
    SendByteToFlash(0x00);   // addr[24:15]
    SendByteToFlash(0x00);   // addr[8:14]
    SendByteToFlash(0x00);   // addr[0:7]
    SendByteToFlash(0x00);   // fast read dummy byte

    for(counter=0x0000;counter<0x8000;counter++)
    {
        Dat=ReadByteFromFlash();
        if(Dat!=0xFF) //verify Byte is all 0xFF, otherwise  error
        {
            WaitFlashFree();
            Print(L" Dat1 is: %d \n",Dat);
            Print(L" counter1 is: %d \n",counter);
            Print(L" -- Verify Fail. \n");
            return(FALSE);
        }
        if(0 == counter%0x800)
            Print(L"#");
    }

    for(counter=0x0000;counter<0x8000;counter++)
    {
        Dat=ReadByteFromFlash();
        if(Dat!=0xFF) //verify Byte is all 0xFF, otherwise  error
        {
            WaitFlashFree();
            Print(L" Dat2 is: %d \n",Dat);
            Print(L" counter2 is: %d \n",counter);
            Print(L" -- Verify Fail. \n");
            return(FALSE);
        }
        if(0 == counter%0x800)
            Print(L"#");
    }
    
    if(128==FW_Size)
    {
        for(counter=0x0000;counter<0x8000;counter++)
        {
            Dat=ReadByteFromFlash();
            if(Dat!=0xFF) //verify Byte is all 0xFF, otherwise  error
            {
                WaitFlashFree();
                Print(L" Dat3 is: %d \n",Dat);
                Print(L" counter3 is: %d \n",counter);
                Print(L" -- Verify Fail. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
               Print(L"#");
        }

        for(counter=0x0000;counter<0x8000;counter++)
        {
            Dat=ReadByteFromFlash();
            if(Dat!=0xFF) //verify Byte is all 0xFF, otherwise  error
            {
                WaitFlashFree();
                Print(L" Dat4 is: %d \n",Dat);
                Print(L" counter4 is: %d \n",counter);
                Print(L" -- Verify Fail. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
                Print(L"#");
        }
    }

    WaitFlashFree();
    Print(L"   -- Verify OK. \n\n");
    return(TRUE);
}

// Read EC file to memory
// fopen() and fread() Must be within the same function
UINT8 Read_ECFile_ToBuf(UINT8 *FileName)
{
    UINTN  len1;
    
    if((pECFile = fopen(FileName, "r")) == NULL)
    {
        printf("open EC file wrong!\n");
        return(FALSE);
    }
    
    len1=fread(str1, 1, 0x8000, pECFile);
    printf("Read Byte: %X\n", len1);
    if(0x8000 != len1)
    {
        printf("Read EC File error 1\n");
        return(FALSE);
    }
    
    len1=fread(str2, 1, 0x8000, pECFile);
    printf("Read Byte: %X\n", len1);
    if(0x8000 != len1)
    {
        printf("Read EC File error 2\n");
        return(FALSE);
    }
    
    if(128==FW_Size)
    {
        len1=fread(str3,1,0x8000,pECFile);
        printf("Read Byte: %X\n", len1);
        if(0x8000 != len1)
        {
            printf("Read EC File error 3\n");
            return(FALSE);
        }
        
        len1=fread(str4,1,0x8000,pECFile);
        printf("Read Byte: %X\n", len1);
        if(0x8000 != len1)
        {
            printf("Read EC File error 4\n");
            return(FALSE);
        }
    }
    fclose(pECFile);
    
    return(TRUE);
}

// Only for 128KB ITE eFlash program
// e-Flash program:
// 1. send start address
// 2. send word program command(ITE e-flash only support this command)
// 3. send tow byte
// 4. wait flsh free
// 5. got to setp 1
void ITE_eFlash_Program(void)
{
    UINTN i;
    Print(L"   Programing...    : ");

    FlashStatusRegWriteEnable();
    FlashWriteEnable();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_AAIWordPro);
    SendByteToFlash(0x00);
    SendByteToFlash(0x00);
    SendByteToFlash(0x00);
    SendByteToFlash(str1[0]);
    SendByteToFlash(str1[1]);
    WaitFlashFree();
    
    Print(L"#");
    for(i=2; i<0x8000; i+=2) // First tow byte already to to flash
    {
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_AAIWordPro);
        SendByteToFlash(str1[i]);
        SendByteToFlash(str1[i+1]);
        WaitFlashFree();
        if(0 == i%0x800)
            Print(L"#");
    }
    
    for(i=0; i<0x8000; i+=2)
    {
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_AAIWordPro);
        SendByteToFlash(str2[i]);
        SendByteToFlash(str2[i+1]);
        WaitFlashFree();
        if(0 == i%0x800)
            Print(L"#");
    }
    
    for(i=0; i<0x8000; i+=2)
    {
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_AAIWordPro);
        SendByteToFlash(str3[i]);
        SendByteToFlash(str3[i+1]);
        WaitFlashFree();
        if(0 == i%0x800)
            Print(L"#");
    }
    
    for(i=0; i<0x8000; i+=2)
    {
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_AAIWordPro);
        SendByteToFlash(str4[i]);
        SendByteToFlash(str4[i+1]);
        WaitFlashFree();
        if(0 == i%0x800)
            Print(L"#");
    }

    FlashWriteDisable();
    Print(L"   -- Programing OK. \n\n");
}


// SPI program 
void Program_Flash(void)
{
    CHAR16 counter;
    CHAR16 counter2;
    UINT8 counter3;

    Print(L"   Programing...    : ");

    for(counter2=0x0000;counter2<0x80;counter2++)   // 32Kb    // 128 block
    {
        FlashWriteEnable();
        WaitFlashFree();
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_PageProgram);
        SendByteToFlash(0x00);
        SendByteToFlash(0x00+(UINT8)counter2);
        SendByteToFlash(0x00);
        for(counter=0x0000;counter<0x100;counter++)   // block 256Byte
        {
            SendByteToFlash(str1[counter+(counter2<<8)]);
        }
        WaitFlashFree();
        if(0 == counter2%0x8)
            Print(L"#");
    }

    for(counter2=0x0000;counter2<0x80;counter2++)   // 32Kb
    {
        FlashWriteEnable();
        WaitFlashFree();
        FollowMode(_EnterFollowMode);
        SendCmdToFlash(SPICmd_PageProgram);
        SendByteToFlash(0x00);
        SendByteToFlash(0x00+(UINT8)(counter2+0x80));
        SendByteToFlash(0x00);
        for(counter=0x0000;counter<0x100;counter++)
        {
            SendByteToFlash(str2[counter+(counter2<<8)]);
        }
        WaitFlashFree();
        if(0 == counter2%0x8)
            Print(L"#");
    }

    if(128==FW_Size)
    {
        counter3 = 1;
        for(counter2=0x0000;counter2<0x80;counter2++)   // 32Kb    // 128 block
        {
            FlashWriteEnable();
            WaitFlashFree();
            FollowMode(_EnterFollowMode);
            SendCmdToFlash(SPICmd_PageProgram);
            SendByteToFlash(counter3);
            SendByteToFlash(0x00+(UINT8)counter2);
            SendByteToFlash(0x00);
            for(counter=0x0000;counter<0x100;counter++)   // block 256Byte
            {
                SendByteToFlash(str3[counter+(counter2<<8)]);
            }
            WaitFlashFree();
            if(0 == counter2%0x8)
                Print(L"#");
        }

        for(counter2=0x0000;counter2<0x80;counter2++)   // 32Kb
        {
            FlashWriteEnable();
            WaitFlashFree();
            FollowMode(_EnterFollowMode);
            SendCmdToFlash(SPICmd_PageProgram);
            SendByteToFlash(counter3);
            SendByteToFlash(0x00+(UINT8)(counter2+0x80));
            SendByteToFlash(0x00);
            for(counter=0x0000;counter<0x100;counter++)
            {
                SendByteToFlash(str4[counter+(counter2<<8)]);
            }
            WaitFlashFree();
            if(0 == counter2%0x8)
                Print(L"#");
        }
    }
    
    FlashWriteDisable();
    Print(L"   -- Programing OK. \n\n");
}

UINT8 Program_Flash_Verify(void)
{
    CHAR16 counter;

    Print(L"   Program Verify...: ");

    FlashWriteDisable();
    WaitFlashFree();
    FollowMode(_EnterFollowMode);
    SendCmdToFlash(SPICmd_FastRead);
    SendByteToFlash(0x00);   // addr[24:15]
    SendByteToFlash(0x00);   // addr[8:14]
    SendByteToFlash(0x00);   // addr[0:7]
    SendByteToFlash(0x00);   // fast read dummy byte

    for(counter=0x0000;counter<0x8000;counter++)
    {
        if(ReadByteFromFlash()!=str1[counter]) //verify Byte
        {
            WaitFlashFree();
            Print(L" -- Verify Fail. \n");
            return(FALSE);
        }
        if(0 == counter%0x800)
            Print(L"#");
    }

    for(counter=0x0000;counter<0x8000;counter++)
    {
        if(ReadByteFromFlash()!=str2[counter]) //verify Byte is all 0xFF, otherwise  error
        {
            WaitFlashFree();
            Print(L" -- Verify Fail. \n");
            return(FALSE);
        }
        if(0 == counter%0x800)
            Print(L"#");
    }

    if(128==FW_Size)
    {
        for(counter=0x0000;counter<0x8000;counter++)
        {
            if(ReadByteFromFlash()!=str3[counter]) //verify Byte
            {
                WaitFlashFree();
                Print(L" -- Verify Fail. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
                Print(L"#");
        }

        for(counter=0x0000;counter<0x8000;counter++)
        {
            if(ReadByteFromFlash()!=str4[counter]) //verify Byte is all 0xFF, otherwise  error
            {
                WaitFlashFree();
                Print(L" -- Verify Fail. \n");
                return(FALSE);
            }
            if(0 == counter%0x800)
                Print(L"#");
        }
    }
    
    WaitFlashFree();
    Print(L"   -- Verify OK. \n");
    return(TRUE);
}

void Show_Version(void)
{
  printf("*************************************************************\n");
  printf("**            EC Flash Utility Version : %s               **\n",Version);
  printf("**                                                         **\n");
  printf("**      (C)Copyright %s Telecom Technology Co.,Ltd     **\n",Vendor);
  printf("**                 All Rights Reserved.                    **\n");
  printf("**                                                         **\n");
  printf("**                 Modified by Morgen                      **\n");
  printf("*************************************************************\n");
}


int main(int Argc, char **Argv)
{    
    UINT8  i;

    if (Argc == 1)
    {
        printf("=======================================================\n");
        printf("=         ITE EC Flash Utility Version : %s          =\n",Version);
        printf("**      (C)Copyright %s Telecom Technology Co.,Ltd     **\n",Vendor);
        printf("=                 All Rights Reserved.                =\n");
        printf("=                             --%s           =\n", __DATE__);
        printf("=                                                     =\n");
        printf("=                                                     =\n");
        printf("=  FTEFI [/128] ... [/R] xxxx.bin                     =\n");
        printf("=   /B      Back up flash data                        =\n");
        printf("=   /F      Update Flash                              =\n");
        printf("=   /64     Size of FW is 64K                         =\n");
        printf("=   /128    Size of FW is 128K                        =\n");
        printf("=   /R      Reset EC after update                     =\n");
        printf("=   /686C   update e-flash by port 686C(Default 6266) =\n");
        printf("=======================================================\n");
        goto ArgcError;
    }
    
    BackUp=0;     // Default is not backup
    FW_Size=64;   // Default is 64K
    ResetFlag=0;  // Default is not reset
    UpdateFlashFlag = 0;
    PM_STATUS_PORT66 =0x66;
    PM_CMD_PORT66    =0x66;
    PM_DATA_PORT62   =0x62; // Default port is 6266
    for(i=1; i<Argc; i++)
    {
        if(!strcmp("/B",Argv[i]) || !strcmp("/b",Argv[i]))
        {
            BackUp=1;  // Back up the chip FW when update FW
        }
        
        if(!strcmp("/R",Argv[i]) || !strcmp("/r",Argv[i]))
        {
            ResetFlag=1;  // Flag reset ec after update
        }
        
        if(!strcmp("/F",Argv[i]) || !strcmp("/f",Argv[i]))
        {
            UpdateFlashFlag=1;  // Need Update Flash
        }
        
        if(!strcmp("/64",Argv[i]))
        {
            FW_Size=64;
        }
        
        if(!strcmp("/128",Argv[i]))
        {
            FW_Size=128;
        }
        
        if(!strcmp("/686C",Argv[i]) || !strcmp("/686c",Argv[i]))
        {
            PM_STATUS_PORT66 =0x6C;
            PM_CMD_PORT66    =0x6C;
            PM_DATA_PORT62   =0x68; // Set port is 686C
        }
    }

    // load EC file
    // malloc 32K *n memory
    str1=(UINT8 *)malloc(0x8000);
    str2=(UINT8 *)malloc(0x8000);
    str3=(UINT8 *)malloc(0x8000);
    str4=(UINT8 *)malloc(0x8000);
    
    if(FALSE == Read_ECFile_ToBuf(Argv[i-1]))
    {
        printf("Read EC file Fail\n");
        goto ReadFileError;
    }

    // Clear Screen
    gST->ConOut->ClearScreen(gST->ConOut);
  
    // Command for EC enter flash function
    Send_cmd_by_PM(0xDC);    // port 66H   call EC function: ITE_Flash_Utility()

    // ACK, 62port, EC will send data 0x33 to Host
    // from Host Interface PM Channel 1 Data Out Port
    while(0x33 != Read_data_from_PM());

    // Read Flash JEDEC ID
    Read_SPIFlash_JEDEC_ID();

    // show Flash Info
    Show_Version();      // display tools version
    Show_FlashInfo();    // display vendor/device/size info
    Print(L"EC File           : %a \n\n", Argv[i-1]);

     
    // back EC Flash Data
    if(BackUp)
    {
        Flash_BackUp();       // read flash(64k) data to back
    }
    
    // Don't Update Flash
    if(0==UpdateFlashFlag)
    {
        goto end;
    }

    // ITE e-flash
    if(0xFF == SPIFlashID[0])
    {
        // flash erase
        ITE_eFlash_Erase();
        
        if(FALSE == ITE_eFlash_Erase_Verify())
        {
            printf("For ITE e-Flash\n");
            printf("Verify Erase fail,please choose other ways of erase and program\n");
            goto end;
        }
        
        // flash program
        ITE_eFlash_Program();
    }
    // SPI flash
    else
    {
        // SPI flash erase
        Erase_Flash();
        
        // flash erase verify
        if(FALSE ==Erase_Flash_Verify())
        {
            printf("For SPI Flash\n");
            printf("Verify Erase fail,please choose other ways of erase and program\n");
            goto end;
        }
        
        // program flash
        Program_Flash();
    }

    // program verify
    if(FALSE == Program_Flash_Verify())
    {
        printf("Verify program fail,please choose other ways of erase and program\n");
        goto end;
    }

end:
FollowMode(_ExitFollowMode);
    if(ResetFlag)
    {
        printf("Please wait, EC will reset\n");
        gBS->Stall(STALL_MS*Delay_TIME_MS);
        Send_cmd_by_PM(0xFE);           // exit FlashECCode and reset EC
    }
    else
    {
        printf("Please wait, EC will restart\n");
        gBS->Stall(STALL_MS*Delay_TIME_MS);
        Send_cmd_by_PM(0xFC);           // exit FlashECCode
    }

ReadFileError:
    free(str1);
    free(str2);
    free(str3);
    free(str4);
    
ArgcError:
    return EFI_UNSUPPORTED;
    
}
