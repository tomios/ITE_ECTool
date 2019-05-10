/*********************************************************/
/*  Name:       Hex2bin.c                                */
/*  Copyright:  NULL                                     */
/*  Author:     Chongchi                                 */
/*  Date: 2014-12-3 09:34                                */
/*  Description:                                         */
/*  compiler: Dev-C++ 5.6.3                              */
/*********************************************************/


#include<stdlib.h>
#include <stdio.h>

#define DEBUG 0

typedef  char uint8;
typedef  int  uint32;

FILE * InputFile;
FILE * OutputFile;
uint32 StartAdd = 0;
uint8 PadByte = 0xFF;
uint8 ChecksumType = 0;
uint8 InputFileName[127];
uint8 OutputFileName[127];

void hexstr_to_uint32(uint8 *str, uint32* data)
{
	uint32 i=0;
	uint32 temp=0;
	uint32 str_len;
	uint32 num_len;
	
	if(NULL == str)
	{
		return;
	}
	
	for(i=0; '\0'!=str[i]; i++);
	str_len = i;
	if(10 < str_len)  /* 0xFFFF FFFF 是最大值 */
	{
		return;
	}

	if(('X'==str[1]) || ('x'==str[1]))
	{
		num_len = str_len - 2;
		*data = 0;
		for(i=2; i<str_len; i++)
		{
			if('0'<=str[i] && '9'>=str[i])
			{
				temp=str[i] - '0';
			}
			if('A'<=str[i] && 'F'>=str[i])
			{
				temp=str[i] - 'A' + 10;
			}
			if('a'<=str[i] && 'f'>=str[i])
			{
				temp=str[i] - 'a' + 10;
			}
			num_len--;
			*data += temp<<(num_len*4);
		}
	}
	else
	{
		num_len = str_len;
		*data = 0;
		for(i=0; i<str_len; i++)
		{
			if('0'<=str[i] && '9'>=str[i])
			{
				temp=str[i] - '0';
			}
			if('A'<=str[i] && 'F'>=str[i])
			{
				temp=str[i] - 'A' + 10;
			}
			if('a'<=str[i] && 'f'>=str[i])
			{
				temp=str[i] - 'a' + 10;
			}
			num_len--;
			*data += temp<<(num_len*4);
		}
	}
}

void option_s(uint8 *op)
{
	if(NULL != op)
	{
		hexstr_to_uint32(op, &StartAdd);
		#if DEBUG
		printf("%#X\n", StartAdd);
		printf("%s\n", op);
		#endif
	}
}


void option_o(uint8 *op)
{
	uint32 i;
	if(NULL != op)
	{
		for(i=0; '\0'!=*(op+i); i++)
		{
			OutputFileName[i] = *(op+i);
		}
		OutputFileName[i] = '\0';
/*		strcpy(OutputFileName, op);  */
		#if DEBUG
		printf("%s\n", OutputFileName);
		#endif
	}
}

void option_i(uint8 *op)
{
	uint32 i;
	if(NULL != op)
	{
		for(i=0; '\0'!=*(op+i); i++)
		{
			InputFileName[i] = *(op+i);
		}
		InputFileName[i] = '\0';
/*		strcpy(OutputFileName, op);  */
		#if DEBUG
		printf("%s\n", InputFileName);
		#endif
	}
}

uint8 hex_to_bin(void)
{
	uint8  StartCode;
	uint8  ByteCountStr[4];
	uint32 ByteCount;
	uint8  AddressStr[6];
	uint32 Address;
	uint8  RecordTypeStr[4];
	uint32 RecordType;
	uint8  DataStr[256];
	uint32 Data;
	uint8  CheckSumStr[4];
	uint32 CheckSum;
	uint8  WriteDataStr[4];
	uint32 WriteData;
	WriteDataStr[2] = '\0';
	
	uint32 j;
	
	while(1)//读取每一条记录
	{
		fread(&StartCode, 1, 1, InputFile);
		#if DEBUG
		printf("start code--%c\n", StartCode);
		#endif
		if(':' != StartCode)  //起始符段
		{
			printf("start code error\n");
			return 1;
		}
		
		fread(&ByteCountStr, 2, 1, InputFile);  //数据区长度段
		ByteCountStr[2]='\0';
		hexstr_to_uint32(ByteCountStr, &ByteCount);
		#if DEBUG
		printf("ByteCountStr--%s\n", ByteCountStr);
		printf("ByteCount--%d\n", ByteCount);
		#endif
		
		fread(&AddressStr, 4, 1, InputFile);   //起始地址段
		AddressStr[4]='\0';
		hexstr_to_uint32(AddressStr, &Address);
		#if DEBUG
		printf("AddressStr--%s\n", AddressStr);
		printf("Address--%d\n", Address);
		#endif
		
		if(Address < StartAdd)   //对于小于起始地址的数据不予记录
		{
			fread(&RecordTypeStr, 2, 1, InputFile);      //类型段
			fread(&DataStr, ByteCount*2, 1, InputFile);  //数据段
			fread(&CheckSumStr, 4, 1, InputFile);        //校验和段+回车换行符。越过4个字节
			if(0==ByteCount || 1==RecordType)
			{
				break;
			}
			continue;
		}
		
		fread(&RecordTypeStr, 2, 1, InputFile);  //类型段
		RecordTypeStr[2]='\0';
		hexstr_to_uint32(RecordTypeStr, &RecordType);
		#if DEBUG
		printf("RecordTypeStr--%s\n", RecordTypeStr);
		printf("RecordType--%d\n", RecordType);
		#endif
		
		fread(&DataStr, ByteCount*2, 1, InputFile);  //数据段
		DataStr[ByteCount*2]='\0';
		#if DEBUG
		printf("DataStr--%s\n", DataStr);
		#endif
		
		fseek(OutputFile, Address-StartAdd, 0);
		for(j=0; j<ByteCount*2; j+=2)
		{
			WriteDataStr[0] = DataStr[j];
			WriteDataStr[1] = DataStr[j+1];
			hexstr_to_uint32(WriteDataStr, &WriteData);
			fwrite(&WriteData, 1, 1, OutputFile);
		}
		
		fread(&CheckSumStr, 2, 1, InputFile);    //校验和段
		CheckSumStr[2]='\0';
		hexstr_to_uint32(CheckSumStr, &CheckSum);
		#if DEBUG
		printf("CheckSumStr--%s\n", CheckSumStr);
		printf("CheckSum--%#X\n", CheckSum);
		#endif
		
		fread(&CheckSumStr, 2, 1, InputFile);    //回车换行符
		CheckSumStr[2]='\0';
		#if DEBUG
		printf("CheckSumStr[0]--%d\n", CheckSumStr[0]);
		printf("CheckSumStr[1]--%d\n", CheckSumStr[1]);
		#endif
		
		if(0==ByteCount || 1==RecordType)
		{
			break;
		}	
	}
	return 0;
}

void usage(void)
{
	printf("\n usage: hex2bin [option parameter]\n");
	printf(" Option:\n");
	printf("    [-s address]     Starting address in hex\n");
	printf("    <-i inputfile>   Input file name\n");
	printf("    <-o outputfile>  Output file name\n");
}


void main( int argc, char *argv[] )
{
	uint32 i;
	uint32 flag_in=0;
	uint32 flag_out=0;
	
	for(i=0; i<argc; i++)
	{
		if('-' == (*(*(argv+i)+0)))    /* option */
		{
			switch((*(*(argv+i)+1)))
			{
				case 's':++i; option_s((*(argv+i))); break;
				case 'o':++i; option_o((*(argv+i))); flag_out=1; break;
				case 'i':++i; option_i((*(argv+i))); flag_in=1;  break;
			}
		}
	}
	if(0 == flag_in)
	{
		printf("no input file\n");
		usage();
		goto end3;
	}
	if(0 == flag_out)
	{
		printf("no output file\n");
		usage();
		goto end3;
	}
	
	if(NULL == (InputFile=fopen(InputFileName,"rb")))
	{
		printf("Input file %s error.\n",InputFileName);
		usage();
		goto end2;
	}
	
	if(NULL == (OutputFile=fopen(OutputFileName,"wb")))
	{
		printf("Output file %s error.\n",OutputFileName);
		usage();
		goto end1;
	}
	
	if(0 != hex_to_bin() )
	{
		printf("convert hex to bin error\n");
	}
	
end1:
	fclose(OutputFile);
end2:
	fclose(InputFile);
end3:
	printf("press any key to continue...");
	system("pause>nul");
}

