/*Unit Test Code */
/*Basic Section  */
/*USE CUnit      */

#include <CUnit/CUnit.h>
#include <CUnit/Console.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ubasic.h"
#include "tokenizer.h"

#define MAXBASIC 20000

//static char const *ptr, *nextptr;
//char *ptr, *nextptr;
//static int current_token = TOKENIZER_ERROR;
//int current_token = TOKENIZER_ERROR;
static int gosub_stack_ptr;
static int for_stack_ptr;
//static int ended;
static const char program[] = "10 cls \n20 print \"OK\"\n30 end\n";


char BASICBUF[MAXBASIC];
int end_ptr;
unsigned char dummykey[100];
int dummycount;
char LINEBUF[128];
int  line_end;
int  line_ptr;
uint8_t glcd_buf[50*240];
uint8_t glcd_check[31];/*LCD‚Ö“]‘—‚·‚éƒ‰ƒCƒ“‚ðŽ¦‚·ƒtƒ‰ƒO*/
uint8_t display_buf[50*240];

void glcd_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
	if(color &0x01){
	//	glcd_buf[(240*50)-((50*(239-y))+((399-x)>>3))-1] &= (0xff^(0x80 >> (x&0x07)));
	//	glcd_buf[49 + 50*y - ((399-x)>>3)] &= (0xff^(0x80 >> (x&0x07)));
		glcd_buf[(50*y)-((399-x)>>3)-1] &= (0xff^(0x80 >> (x&0x07)));
	}else{
	//	glcd_buf[(240*50)-((50*(239-y))+((399-x)>>3))-1] |= (0x80 >> (x&0x07));
	//	glcd_buf[49 + 50*y - ((399-x)>>3)] |= (0x80 >> (x&0x07));
		glcd_buf[(50*y)-((399-x)>>3)-1] |= (0x80 >> (x&0x07));
	}
//	glcd_check[240-y] = 1;
	glcd_check[(240-y)>>3] |= 1<<((240-y)&0x07);
}

void glcd_makeimage(char * filename)
{
	FILE * fp;
	int i,j;

	unsigned char BITMAP_HEADER[62];
	unsigned char BITMAP_DATA[(400*240)/8];
	unsigned char linebuf[50];
	unsigned char PADDING[2];

	BITMAP_HEADER[0] = 'B';
	BITMAP_HEADER[1] = 'M';
	//filesize 14+40+2+((400*240)/8)=12056
	BITMAP_HEADER[2] = 0x18;
	BITMAP_HEADER[3] = 0x2F;
	BITMAP_HEADER[4] = 0x00;
	BITMAP_HEADER[5] = 0x00;
	BITMAP_HEADER[6] = 0x00;
	BITMAP_HEADER[7] = 0x00;
	BITMAP_HEADER[8] = 0x00;
	BITMAP_HEADER[9] = 0x00;

	BITMAP_HEADER[10] = 0x3E;//offset
	BITMAP_HEADER[11] = 0x00;
	BITMAP_HEADER[12] = 0x00;
	BITMAP_HEADER[13] = 0x00;

	BITMAP_HEADER[14] = 0x28;
	BITMAP_HEADER[15] = 0x00;
	BITMAP_HEADER[16] = 0x00;
	BITMAP_HEADER[17] = 0x00;

	BITMAP_HEADER[18] = 0x90;
	BITMAP_HEADER[19] = 0x01;
	BITMAP_HEADER[20] = 0x00;
	BITMAP_HEADER[21] = 0x00;

	BITMAP_HEADER[22] = 0xF0;
	BITMAP_HEADER[23] = 0x00;
	BITMAP_HEADER[24] = 0x00;
	BITMAP_HEADER[25] = 0x00;

	BITMAP_HEADER[26] = 0x01;
	BITMAP_HEADER[27] = 0x00;

	BITMAP_HEADER[28] = 0x01;
	BITMAP_HEADER[29] = 0x00;

	BITMAP_HEADER[30] = 0x00;
	BITMAP_HEADER[31] = 0x00;
	BITMAP_HEADER[32] = 0x00;
	BITMAP_HEADER[33] = 0x00;

	BITMAP_HEADER[34] = 0xC0;
	BITMAP_HEADER[35] = 0x0E;
	BITMAP_HEADER[36] = 0x00;
	BITMAP_HEADER[37] = 0x00;

	BITMAP_HEADER[38] = 0xC0;
	BITMAP_HEADER[39] = 0x0E;
	BITMAP_HEADER[40] = 0x00;
	BITMAP_HEADER[41] = 0x00;

	BITMAP_HEADER[42] = 0xC0;
	BITMAP_HEADER[43] = 0x0E;
	BITMAP_HEADER[44] = 0x00;
	BITMAP_HEADER[45] = 0x00;

	BITMAP_HEADER[46] = 0x02;
	BITMAP_HEADER[47] = 0x00;
	BITMAP_HEADER[48] = 0x00;
	BITMAP_HEADER[49] = 0x00;

	BITMAP_HEADER[50] = 0x02;
	BITMAP_HEADER[51] = 0x00;
	BITMAP_HEADER[52] = 0x00;
	BITMAP_HEADER[53] = 0x00;
	//pallete
	BITMAP_HEADER[54] = 0xff;
	BITMAP_HEADER[55] = 0xff;
	BITMAP_HEADER[56] = 0xff;
	BITMAP_HEADER[57] = 0xff;
	BITMAP_HEADER[58] = 0x00;
	BITMAP_HEADER[59] = 0x00;
	BITMAP_HEADER[60] = 0x00;
	BITMAP_HEADER[61] = 0x00;

	PADDING[0] = 0x00;
	PADDING[1] = 0x00;
	if ((fp = fopen(filename, "wb")) == NULL) {
		printf("file open error!!\n");
	}else{
		fwrite(&BITMAP_HEADER,sizeof(BITMAP_HEADER),1,fp);
		for(i=0;i<240;i++){
			if(glcd_check[(i>>3)]&(1<<(i&0x07))){
				for(j=0;j<50;j++){
					linebuf[j] = glcd_buf[50*(239-i)+j] ^ 0xff;
					display_buf[50*(239-i)+j] = glcd_buf[50*(239-i)+j];
				}
			}else{
				for(j=0;j<50;j++){
					linebuf[j] = display_buf[50*(239-i)+j] ^ 0xff;
				}
			}
			fwrite(&linebuf,50,1,fp);
			fwrite(&PADDING,2,1,fp);
		}
		fclose(fp);
	}
}


void glcd_UnPutsA(char * buf)
{
	printf(buf);
}

void glcd_PutsA(char * buf)
{
	printf(buf);
}

void glcd_PutsD(char * buf)
{
	printf(buf);
}

void glcd_TransFromBuf(void)
{
	return;
}
void glcd_DrawCursor(void)
{
	return;
}

void glcd_BufClear(uint8_t flg)
{
	int i;
	if (flg)flg = 0xff;
	for (i = 0;i < (50*240);i++)glcd_buf[i] = flg;
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	for(i=0;i<31;i++)glcd_check[i] = 0xff;
	return;
}
FRESULT lsSDA(void)
{
	printf("lsSDA\n");
	return 0;
}
void cputs_p(int i,char * buf)
{
	printf(buf);
}
void glcd_PutsUint8A(uint32_t d)
{
	if (d < 10) {
	//	glcd_PutCharA(0x8000 | ('0'));
		LINEBUF[line_ptr] = '0' & 0x0ff;
		line_ptr++;
		line_end++;
	}
	int j = 10;
	if (d >= 1000000000) {
	//	glcd_PutCharA(0x8000 | ('0' + (d / 1000000000)));
		LINEBUF[line_ptr] = ('0' + (d / 1000000000)) & 0x0ff;
		line_ptr++;
		line_end++;
		d = d % 1000000000;
	}
	while (d >= j) {
		j *= 10;
	}
	j /= 10;
	while (j >= 1) {
	//	glcd_PutCharA(0x8000 | ('0' + (d / j)));
		LINEBUF[line_ptr] = ('0' + (d / j)) & 0x0ff;
		line_ptr++;
		line_end++;
		d = d % j;
		j /= 10;
	}
}

char * ltodeci(long l,char * buf,unsigned char i)
{
	int j,k,m;
	unsigned long x,y;
	if(l < 0){
	//	buf[0] = '-';
		y = -1*l;
	//	m = 1;
		m = 0;
	}else{
		m = 0;
		y = l;
	}
	for(j = m;j < i;j++) {
		x = y;
		for (k = 0;k < (i - j - 1);k++) x = x/10;
		buf[j] = "0123456789"[(x % 10)];
	}
	buf[j] = 0;
	return buf;
}
void glcd_posClear(void)
{
	return;
}

#if 0
typedef enum {
	FR_OK = 0,				/* (0) Succeeded */
	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,				/* (2) Assertion failed */
	FR_NOT_READY,			/* (3) The physical drive cannot work */
	FR_NO_FILE,				/* (4) Could not find the file */
	FR_NO_PATH,				/* (5) Could not find the path */
	FR_INVALID_NAME,		/* (6) The path name format is invalid */
	FR_DENIED,				/* (7) Access denied due to prohibited access or directory full */
	FR_EXIST,				/* (8) Access denied due to prohibited access */
	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,			/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */
	FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any parameter error */
	FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */
	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > _FS_SHARE */
	FR_INVALID_PARAMETER	/* (19) Given parameter is invalid */
} FRESULT;
#endif

FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs)
{
	/* Get number of free clusters on the drive */
	return FR_OK;
}
FRESULT f_opendir (DIR* dj, const TCHAR* path)
{
	/* Open an existing directory */
	return FR_OK;
}
FRESULT f_readdir (DIR* dj, FILINFO* fno)
{
	/* Read a directory item */
	return FR_OK;
}
FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode)
{
	/* Open or create a file */
	return FR_OK;
}
FRESULT f_close (FIL* fp)
{
	/* Open or create a file */
	return FR_OK;
}
FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br)
{
	/* Read data from a file */
	return FR_OK;
}
FRESULT f_write (FIL* fp, const void* buff, UINT btr, UINT* bw)
{
	/* Read data from a file */
	return FR_OK;
}
FRESULT lsSD (void)
{
	return FR_OK;
}

unsigned char kgetc (void)
{
	unsigned char ret;
	ret = dummykey[dummycount];
	dummycount++;
	return ret;
}

void _delay_ms(uint32_t mSec)
{ 
	return;
}

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;
typedef struct
{
  uint32_t SR;     /*!< ADC status register,                         Address offset: 0x00 */
  uint32_t CR1;    /*!< ADC control register 1,                      Address offset: 0x04 */      
  uint32_t CR2;    /*!< ADC control register 2,                      Address offset: 0x08 */
  uint32_t SMPR1;  /*!< ADC sample time register 1,                  Address offset: 0x0C */
  uint32_t SMPR2;  /*!< ADC sample time register 2,                  Address offset: 0x10 */
  uint32_t JOFR1;  /*!< ADC injected channel data offset register 1, Address offset: 0x14 */
  uint32_t JOFR2;  /*!< ADC injected channel data offset register 2, Address offset: 0x18 */
  uint32_t JOFR3;  /*!< ADC injected channel data offset register 3, Address offset: 0x1C */
  uint32_t JOFR4;  /*!< ADC injected channel data offset register 4, Address offset: 0x20 */
  uint32_t HTR;    /*!< ADC watchdog higher threshold register,      Address offset: 0x24 */
  uint32_t LTR;    /*!< ADC watchdog lower threshold register,       Address offset: 0x28 */
  uint32_t SQR1;   /*!< ADC regular sequence register 1,             Address offset: 0x2C */
  uint32_t SQR2;   /*!< ADC regular sequence register 2,             Address offset: 0x30 */
  uint32_t SQR3;   /*!< ADC regular sequence register 3,             Address offset: 0x34 */
  uint32_t JSQR;   /*!< ADC injected sequence register,              Address offset: 0x38*/
  uint32_t JDR1;   /*!< ADC injected data register 1,                Address offset: 0x3C */
  uint32_t JDR2;   /*!< ADC injected data register 2,                Address offset: 0x40 */
  uint32_t JDR3;   /*!< ADC injected data register 3,                Address offset: 0x44 */
  uint32_t JDR4;   /*!< ADC injected data register 4,                Address offset: 0x48 */
  uint32_t DR;     /*!< ADC regular data register,                   Address offset: 0x4C */
} ADC_TypeDef;

FlagStatus ADC_GetFlagStatus(ADC_TypeDef* ADCx, uint8_t ADC_FLAG)
{
  return  SET;
}

uint16_t ADC_GetConversionValue(ADC_TypeDef* ADCx)
{
  return (uint16_t) 0x11; /*Dummy Value*/
}

void ADC_SoftwareStartConv(ADC_TypeDef* ADCx)
{
	return;
}
/*------------------------------------------------------------------------*/
void test_singlechar(void)
{
	int ret;
//	char *ptr = malloc(100);
//	strcpy(ptr,"\n,;+-&|*/%()<>=A");
//	const char ptr[] = "\n,;+-&|*/%()<>=A";
//	const char ptr[] = "hoge";
//	static char ptr[] = "hoge";

	char * ptr2;
	ptr = (char *)malloc(100);
	ptr2 = ptr;
	strcpy(ptr,"\n,;+-&|*/%()<>=A");

	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_CR);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_COMMA);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_SEMICOLON);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_PLUS);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_MINUS);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_AND);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_OR);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_ASTR);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_SLASH);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_MOD);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_LEFTPAREN);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_RIGHTPAREN);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_LT);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_GT);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == TOKENIZER_EQ);
	ptr++;
	ret = singlechar();
	CU_ASSERT(ret == 0);
	free(ptr2);
}
void test_get_next_token(void)
{
	char * ptr2;
	ptr = (char *)malloc(100);
	ptr2 = ptr;
	strcpy(ptr,"10 cls \n20 print \"OK\"\n30 end\n");
//	char ptr[] = "10 cls \n20 print \"OK\"\n30 end\n";
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_NUMBER);
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_CLS);
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_CR);
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_NUMBER);
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_PRINT);
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_STRING);
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_CR);
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_NUMBER);
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_END);
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	CU_ASSERT(current_token == TOKENIZER_CR);
	free(ptr2);
}

void test_tokenizer_init(void)
{
//	const char prog[] = "10 cls \n20 print \"OK\"\n30 end\n";
	char prog[] = "10 cls \n20 print \"OK\"\n30 end\n";
	tokenizer_init(prog);
//	printf("test_tokenizer_init = %d \n",current_token);
//	printf("TOKENIZER_NUMBER = %d \n",TOKENIZER_NUMBER);
//	printf("TOKENIZER_CR = %d \n",TOKENIZER_CR);
	CU_ASSERT(current_token == TOKENIZER_NUMBER);
}

void test_tokenizer_token(void)
{
	int ret;
	current_token = TOKENIZER_CR;
	ret = tokenizer_token();
	CU_ASSERT(ret == TOKENIZER_CR);
	current_token = TOKENIZER_NUMBER;
	ret = tokenizer_token();
	CU_ASSERT(ret == TOKENIZER_NUMBER);
}

void test_tokenizer_next(void)
{
	char prog[] = "10 cls \n20 print \"OK\"\n30 end\n";
	tokenizer_init(prog);
//	tokenizer_next();
//	printf("test_tokenizer_next = %d \n",current_token);
	CU_ASSERT(current_token == TOKENIZER_NUMBER);
	tokenizer_next();
//	printf("test_tokenizer_next = %d \n",current_token);
	CU_ASSERT(current_token == TOKENIZER_CLS);
	tokenizer_next();
//	printf("test_tokenizer_next = %d \n",current_token);
	CU_ASSERT(current_token == TOKENIZER_CR);
	tokenizer_next();
	CU_ASSERT(current_token == TOKENIZER_NUMBER);
	tokenizer_next();
	CU_ASSERT(current_token == TOKENIZER_PRINT);
	tokenizer_next();
	CU_ASSERT(current_token == TOKENIZER_STRING);
	tokenizer_next();
	CU_ASSERT(current_token == TOKENIZER_CR);
	tokenizer_next();
	CU_ASSERT(current_token == TOKENIZER_NUMBER);
	tokenizer_next();
	CU_ASSERT(current_token == TOKENIZER_END);
	tokenizer_next();
	CU_ASSERT(current_token == TOKENIZER_CR);
	tokenizer_next();
	CU_ASSERT(current_token == TOKENIZER_ENDOFINPUT);
}

void test_tokenizer_num(void)
{
	ptr = "10";
	int ret;
	ret = tokenizer_num();
	CU_ASSERT(ret == 10);
	ptr = "1000";
	ret = tokenizer_num();
	CU_ASSERT(ret == 1000);
	ptr = "10A0";
	ret = tokenizer_num();
	CU_ASSERT(ret == 10);
	ptr = "FF";
	ret = tokenizer_num();
//	CU_ASSERT(ret == -1);
	CU_ASSERT(ret == 0);
//	printf("test_tokenizer_num ret = %d \n",ret);
	ptr = "-100";
	ret = tokenizer_num();
	CU_ASSERT(ret == -100);
//	printf("test_tokenizer_num ret = %d \n",ret);
}

void test_tokenizer_string(void)
{
	char * ptr2;
	ptr = (char *)malloc(100);
	ptr2 = ptr;
	strcpy(ptr,"\"Hoge\"");
	char buf[20];
	int ret;
	current_token = TOKENIZER_STRING;

//	char ptr[] = "Hoge";
//	printf(ptr);
	tokenizer_string(buf,20);
	ret = strcmp(buf,"Hoge");
	CU_ASSERT(ret == 0);
//	printf(buf);

	strcpy(ptr,"\"Hoge Hoge\"");
//	ptr[] = "\"Hoge Hoge\"";
//	printf(ptr);
	current_token = TOKENIZER_STRING;
	tokenizer_string(buf,20);
	ret = strcmp(buf,"Hoge Hoge");
	CU_ASSERT(ret == 0);
//	printf(buf);

	free(ptr2);
}

//void test_tokenizer_error_print(void)
//{
//	tokenizer_error_print();
//}

void test_tokenizer_finished(void)
{
	char * ptr2;
	ptr = (char *)malloc(100);
	ptr2 = ptr;
	strcpy(ptr,"Hoge");
	int ret;
	current_token = TOKENIZER_CR;
//	char ptr[] = "Hoge";
	ret = tokenizer_finished();
	CU_ASSERT(ret == 0);
	ptr++;
	ptr++;
	ptr++;
	ret = tokenizer_finished();
	CU_ASSERT(ret == 0);
	ptr++;
	ret = tokenizer_finished();
	CU_ASSERT(ret == 1);
	strcpy(ptr,"Hoge");
//	ptr[] = "Hoge";
	ret = tokenizer_finished();
	CU_ASSERT(ret == 0);
	current_token = TOKENIZER_ENDOFINPUT;
	ret = tokenizer_finished();
	CU_ASSERT(ret == 1);
	free(ptr2);
}

void test_tokenizer_variable_num(void)
{
	char * ptr2;
	ptr = (char *)malloc(100);
	ptr2 = ptr;
	strcpy(ptr,"abcABCX");
	long ret;
//	char ptr = "abcABCX";
	ret = tokenizer_variable_num();
	CU_ASSERT(ret == 0);
	ptr++;
	ret = tokenizer_variable_num();
	CU_ASSERT(ret == 1);
	ptr++;
	ret = tokenizer_variable_num();
	CU_ASSERT(ret == 2);
	ptr++;
	ret = tokenizer_variable_num();
	CU_ASSERT(ret == 0+26);
	ptr++;
	ret = tokenizer_variable_num();
	CU_ASSERT(ret == 1+26);
	ptr++;
	ret = tokenizer_variable_num();
	CU_ASSERT(ret == 2+26);
	ptr++;
	ret = tokenizer_variable_num();
	CU_ASSERT(ret == -1);
	free(ptr2);
}

void test_tokenizer_num_or_variable_num(void)
{
	long ret;
	char * ptr2;
	ptr = (char *)malloc(100);
	ptr2 = ptr;
	strcpy(ptr,"abjA[0A[1A[2B[0B[1B[2E[0E[1E[2E[3IJ");
//	ptr[] = "abjA[0A[1A[2B[0B[1B[2E[0E[1E[2E[3IJ";

	variables[0] = 100;
	variables[1] = 200;
//	variables[10] = 219;
	variables[9] = 219;
	variables_big[0] = 3000;
	variables_big[1] = 4000;
	arrays[0][0] = 99;
	arrays[0][1] = 98;
	arrays[0][2] = 97;
	arrays[1][0] = 89;
	arrays[1][1] = 88;
	arrays[1][2] = 87;
	strings[0][0] = 'h';
	strings[0][1] = 'o';
	strings[0][2] = 'g';
	strings[0][3] = 'e';
	strings[0][4] = 0;
	
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 100);
//	printf("test_tokenizer_num_or_variable_num ret = %d \n",ret);
	ptr++;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 200);
//	printf("test_tokenizer_num_or_variable_num ret = %d \n",ret);
	ptr++;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 219);
	ptr++;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 99);
//	printf("test_tokenizer_num_or_variable_num ret = %d \n",ret);
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 98);
//	printf("test_tokenizer_num_or_variable_num ret = %d \n",ret);
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 97);
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 89);
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 88);
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 87);
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 'h');
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 'o');
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 'g');
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 'e');
	ptr+=3;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 3000);
	ptr++;
	ret = tokenizer_num_or_variable_num();
	CU_ASSERT(ret == 4000);
	free(ptr2);
}

void test_tokenizer_bignum_or_variable_bignum(void)
{
	long ret;
	ptr = "1000I20000";
	variables_big[0] = 3000;
	ret = tokenizer_bignum_or_variable_bignum();
	CU_ASSERT(ret == 1000);
	ptr+=4;
	ret = tokenizer_bignum_or_variable_bignum();
	CU_ASSERT(ret == 3000);
	ptr++;
	ret = tokenizer_bignum_or_variable_bignum();
	CU_ASSERT(ret == 20000);
}

void test_ubasic_init(void)
{
//	const char * prog = "10 cls \n20 print \"OK\"\n30 end\n";
	char * prog = "10 cls \n20 print \"OK\"\n30 end\n";
	ubasic_init(prog);
	CU_ASSERT(current_token == TOKENIZER_NUMBER);
	CU_ASSERT(for_stack_ptr == 0);
	CU_ASSERT(gosub_stack_ptr == 0);
	CU_ASSERT(ended == 0);
	CU_ASSERT(ErrLine == 0);
}

void test_accept(void)
{
//	const char * prog = "10 cls \n20 print \"OK\"\n30 end\n";
	char * prog = "10 cls \n20 print \"OK\"\n30 end\n";
	ended = 0;
	tokenizer_init(prog);
//	printf("test_accept token = %d \n",current_token);
	accept(TOKENIZER_NUMBER);
	CU_ASSERT(ended == 0);
	tokenizer_next();
//	printf("test_accept token = %d \n",current_token);
	accept(TOKENIZER_CR);
	CU_ASSERT(ended == 0);
	tokenizer_next();
//	printf("test_accept token = %d \n",current_token);
	accept(TOKENIZER_PRINT);
	CU_ASSERT(ended == 0);
	tokenizer_next();
//	printf("test_accept token = %d \n",current_token);
	accept(TOKENIZER_CR);
	CU_ASSERT(ended == 0);

}

void test_comment_accept(void)
{
	int i;
	char * prog = "10 cls \n20\n";
	tokenizer_init(prog);
	comment_accept();
	accept(TOKENIZER_CR);
//	tokenizer_next();
//	accept(TOKENIZER_NUMBER);
	i = tokenizer_num();
//	i = tokenizer_token();
//	printf("test_comment_accept %d \n",i);
	CU_ASSERT(i == 20);
}

void test_varfactor(void)
{
	variables[0] = 100;
	int ret;
	char * prog = "a\n";
	tokenizer_init(prog);
	ret = varfactor();
//	printf("test_varfactor %d \n",ret);
	CU_ASSERT(ret == 100);
}

void test_factor(void)
{
	variables[1] = 55;
	int ret;
	char * prog = "77\n";
	tokenizer_init(prog);
	ret = factor();
	CU_ASSERT(ret == 77);
	prog = "b\n";
	tokenizer_init(prog);
	ret = factor();
//	printf("test_factor %d \n",ret);
	CU_ASSERT(ret == 55);
	prog = "(b+5)\n";
	tokenizer_init(prog);
	ret = factor();
	CU_ASSERT(ret == 60);
	prog = "(b*2)\n";
	tokenizer_init(prog);
	ret = factor();
	CU_ASSERT(ret == 110);
}

void test_term(void)
{
	variables[1] = 44;
	char * prog = "b/2\n";
	int ret;
	tokenizer_init(prog);
	ret = term();
	CU_ASSERT(ret == 22);
}

void test_expr(void)
{
	variables[1] = 22;
	int ret;
	char * prog = "b/2+3\n";
	tokenizer_init(prog);
	ret = expr();
	CU_ASSERT(ret == 14);
}

void test_relation(void)
{
	variables[1] = 66;
	int ret;
	char * prog = "b/2+3 > 37\n";
	tokenizer_init(prog);
	ret = relation();
	CU_ASSERT(ret == 0);
	prog = "b/2+3 > 35\n";
	tokenizer_init(prog);
	ret = relation();
	CU_ASSERT(ret == 1);
}

void test_jump_linenum(void)
{
	int ret;
//	char * prog = "100 \n200\n";
	strcpy(BASICBUF,"100 \n200 \n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	jump_linenum(200);
	ret = tokenizer_num();
	CU_ASSERT(ret == 200);
}

void test_goto_statement(void)
{
	int ret;
//	const char * prog = "goto 100\n";
	strcpy(BASICBUF,"10 GOTO 100\n100 \n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	accept(TOKENIZER_NUMBER);
	goto_statement();
//	printf("\ntest_goto_statement %d \n",current_token);
//	ret = tokenizer_num();
//	printf("\ntest_goto_statement %d \n",ret);
//	CU_ASSERT(ret == 100);
	
	strcpy(BASICBUF,"100 PRINT \"TEST\"\n200 GOTO 100\n300 END\n");
	prog = BASICBUF;
	tokenizer_init(prog);
	accept(TOKENIZER_NUMBER);
//	accept(TOKENIZER_PRINT);
//	printf("test_goto_statement %d \n",current_token);
	print_statement();
	accept(TOKENIZER_NUMBER);
	
	goto_statement();
//	printf("test_goto_statement %d \n",current_token);

	ret = tokenizer_num();
	CU_ASSERT(ret == 100);

	accept(TOKENIZER_NUMBER);
//	printf("test_goto_statement %d \n",current_token);
//	accept(TOKENIZER_NUMBER);
//	printf("test_goto_statement %d \n",current_token);
//	accept(TOKENIZER_PRINT);
	print_statement();
#if 0
	accept(TOKENIZER_PRINT);
	do {
		if(tokenizer_token() == TOKENIZER_STRING) {
			tokenizer_string(strings[MAX_STRINGNUM], sizeof(strings[MAX_STRINGNUM]));
			glcd_PutsA(strings[MAX_STRINGNUM]);
	printf(strings[MAX_STRINGNUM]);
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_COMMA) {
			glcd_PutsA(" ");
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_SEMICOLON) {
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_VARIABLE ||
				  tokenizer_token() == TOKENIZER_NUMBER) {
	printf("\ntest_Print_statement %d \n",expr());
		} else {
			break;
		}
	} while(tokenizer_token() != TOKENIZER_CR &&
			tokenizer_token() != TOKENIZER_ENDOFINPUT);
	tokenizer_next();
//	accept(TOKENIZER_NUMBER);
#endif
//	printf(" after \n");
//	printf(ptr);
//	printf("test_goto_statement %d \n",current_token);

	accept(TOKENIZER_NUMBER);
	goto_statement();
	ret = tokenizer_num();
	CU_ASSERT(ret == 100);

	accept(TOKENIZER_NUMBER);
//	printf("test_goto_statement 1 %d \n",current_token);
//	accept(TOKENIZER_NUMBER);
	print_statement();
#if 0
	accept(TOKENIZER_PRINT);
	do {
		if(tokenizer_token() == TOKENIZER_STRING) {
			tokenizer_string(strings[MAX_STRINGNUM], sizeof(strings[MAX_STRINGNUM]));
			glcd_PutsA(strings[MAX_STRINGNUM]);
	printf(strings[MAX_STRINGNUM]);
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_COMMA) {
			glcd_PutsA(" ");
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_SEMICOLON) {
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_VARIABLE ||
				  tokenizer_token() == TOKENIZER_NUMBER) {
	printf("\ntest_Print_statement %d \n",expr());
		} else {
			break;
		}
	} while(tokenizer_token() != TOKENIZER_CR &&
			tokenizer_token() != TOKENIZER_ENDOFINPUT);
	tokenizer_next();
//	accept(TOKENIZER_NUMBER);
#endif
//	printf("test_goto_statement 2 %d \n",current_token);
	accept(TOKENIZER_NUMBER);
	goto_statement();
	ret = tokenizer_num();
	CU_ASSERT(ret == 100);

	accept(TOKENIZER_NUMBER);
//	printf("test_goto_statement 3 %d \n",current_token);
//	accept(TOKENIZER_NUMBER);
	print_statement();
//	printf("test_goto_statement 4 %d \n",current_token);
	accept(TOKENIZER_NUMBER);
	goto_statement();
//	accept(TOKENIZER_NUMBER);
	ret = tokenizer_num();
	CU_ASSERT(ret == 100);
}

void test_print_statement(void)
{
	strcpy(BASICBUF,"10 PRINT \"HOGE\n\"\n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	accept(TOKENIZER_NUMBER);
	print_statement();
}

void test_if_statement(void)
{
	int r;
	int token;
	int linenum;
	int r2;
	int op;

	strcpy(BASICBUF,"100 IF a = 1 THEN GOTO 300\n200 PRINT \"200\"\n300 PRINT \"300\"\n");
//	strcpy(BASICBUF,"100 IF 1 THEN GOTO 300\n200 PRINT \"200\"\n300 PRINT \"300\"\n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	variables[0] = 100;
//	variables[0] = 1;

	accept(TOKENIZER_NUMBER);
//	printf("\ntest_if_statement %d \n",current_token);

	if_statement();

#if 0
	printf("test_if_statement if=8 %d \n",current_token);
	accept(TOKENIZER_IF);

//	r = relation();
	r = expr();
	printf("test_if_statement r = %d \n",r);
	op = tokenizer_token();
	while(op == TOKENIZER_LT ||
		  op == TOKENIZER_GT ||
		  op == TOKENIZER_EQ) {
		tokenizer_next();
		r2 = expr();
	printf("test_if_statement op = %d r2 = %d\n",op,r2);
		switch(op) {
			case TOKENIZER_LT:
				r = r < r2;
				break;
			case TOKENIZER_GT:
				r = r > r2;
				break;
			case TOKENIZER_EQ:
				r = r == r2;
				break;
		}
		op = tokenizer_token();
	}
	
	printf("test_if_statement r = %d \n",r);
	accept(TOKENIZER_THEN);
	if(r) {
		printf("Go statement %d \n",current_token);
		statement();
#if 0
		token = tokenizer_token();
		switch(token) {
			case TOKENIZER_GOTO:
//				goto_statement();

	accept(TOKENIZER_GOTO);
	linenum = tokenizer_num();
	printf("GOTO linenum %d \n",linenum);
	jump_linenum(linenum);

//	ptr = BASICBUF;
//	nextptr = ptr;
//	while(tokenizer_num() != linenum) {
//		do {
//			do {
//	printf("test_if_statement %d \n",current_token);
//				tokenizer_next();
//			} while(tokenizer_token() != TOKENIZER_CR &&
//					tokenizer_token() != TOKENIZER_ENDOFINPUT);
//			if(tokenizer_token() == TOKENIZER_CR) {
//				tokenizer_next();
//			}
//		} while(tokenizer_token() != TOKENIZER_NUMBER);
//	}
//	printf("test_if_statement last %d \n",current_token);
	accept(TOKENIZER_NUMBER);

				break;
			default:
				printf("Error token = %d \n",token);
				break;
		}
#endif
	} else {
		do {
			printf("test_if_statement next_token%d \n",current_token);
			tokenizer_next();
		} while(tokenizer_token() != TOKENIZER_ELSE &&
		tokenizer_token() != TOKENIZER_CR &&
		tokenizer_token() != TOKENIZER_ENDOFINPUT);
		if(tokenizer_token() == TOKENIZER_ELSE) {
			tokenizer_next();
			statement();
		} else if(tokenizer_token() == TOKENIZER_CR) {
			tokenizer_next();
			printf("test_if_statement last_token%d \n",current_token);
		}
	}
#endif

	accept(TOKENIZER_NUMBER); //
//	printf("test_if_statement %d \n",current_token);
	print_statement();
	
	prog = BASICBUF;
	tokenizer_init(prog);
	variables[0] = 1;

	accept(TOKENIZER_NUMBER);
//	printf("\ntest_if_statement %d \n",current_token);
	if_statement();
	accept(TOKENIZER_NUMBER);
//	printf("test_if_statement %d \n",current_token);
	print_statement();

}

void test_if_statement_2(void)
{
	int r;
	int token;
	int linenum;
	int r2;
	int op;

	strcpy(BASICBUF,"100 if 120 > a then gosub 200 else gosub 300\n200 PRINT \"200\"\n300 PRINT \"300\"\n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	variables[0] = 130;

//	printf("\ntest_if_statement %d \n",current_token);
	accept(TOKENIZER_NUMBER);

	if_statement();
#if 0
	printf("test_if_statement if=8 %d \n",current_token);
	accept(TOKENIZER_IF);

//	r = relation();
	r = expr();
	printf("test_if_statement r = %d \n",r);
	op = tokenizer_token();
	while(op == TOKENIZER_LT ||
		  op == TOKENIZER_GT ||
		  op == TOKENIZER_EQ) {
		tokenizer_next();
		r2 = expr();
	printf("test_if_statement op = %d r2 = %d\n",op,r2);
		switch(op) {
			case TOKENIZER_LT:
				r = r < r2;
				break;
			case TOKENIZER_GT:
				r = r > r2;
				break;
			case TOKENIZER_EQ:
				r = r == r2;
				break;
		}
		op = tokenizer_token();
	}
	
	printf("test_if_statement r = %d \n",r);
	accept(TOKENIZER_THEN);
	if(r) {
		printf("Go statement %d \n",current_token);
		statement();
#if 0
		token = tokenizer_token();
		switch(token) {
			case TOKENIZER_GOTO:
//				goto_statement();

	accept(TOKENIZER_GOTO);
	linenum = tokenizer_num();
	printf("GOTO linenum %d \n",linenum);
	jump_linenum(linenum);

//	ptr = BASICBUF;
//	nextptr = ptr;
//	while(tokenizer_num() != linenum) {
//		do {
//			do {
//	printf("test_if_statement %d \n",current_token);
//				tokenizer_next();
//			} while(tokenizer_token() != TOKENIZER_CR &&
//					tokenizer_token() != TOKENIZER_ENDOFINPUT);
//			if(tokenizer_token() == TOKENIZER_CR) {
//				tokenizer_next();
//			}
//		} while(tokenizer_token() != TOKENIZER_NUMBER);
//	}
//	printf("test_if_statement last %d \n",current_token);
	accept(TOKENIZER_NUMBER);

				break;
			default:
				printf("Error token = %d \n",token);
				break;
		}
#endif
	} else {
		do {
			printf("test_if_statement next_token%d \n",current_token);
			tokenizer_next();
		} while(tokenizer_token() != TOKENIZER_ELSE &&
		tokenizer_token() != TOKENIZER_CR &&
		tokenizer_token() != TOKENIZER_ENDOFINPUT);
		if(tokenizer_token() == TOKENIZER_ELSE) {
			tokenizer_next();
			statement();
		} else if(tokenizer_token() == TOKENIZER_CR) {
			tokenizer_next();
			printf("test_if_statement last_token%d \n",current_token);
		}
	}
#endif
//	printf("test_if_statement %d \n",current_token);
	accept(TOKENIZER_NUMBER); //
	print_statement();
}

void test_let_statement(void)
{
	long var,var2;
	long ix;
	char * c;
	int i;
	int Line;
	int token;
	
	strcpy(BASICBUF,"100 LET a\n110 a = 2\n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	accept(TOKENIZER_NUMBER);
	accept(TOKENIZER_LET);
//	printf("test_let_statement token = %d \n",current_token);
	accept(TOKENIZER_VARIABLE);
//	printf("test_let_statement token = %d \n",current_token);
	accept(TOKENIZER_CR);
//	printf("test_let_statement token = %d \n",current_token);
	accept(TOKENIZER_NUMBER);
//	printf("test_let_statement token = %d \n",current_token);
	let_statement();
	
#if 0
	ix = 0;
	var = tokenizer_variable_num();
	if((var >= MAX_VARNUM)&&(var <(MAX_VARNUM+MAX_ARRYNUM))){
		accept(TOKENIZER_VARIABLE);
		printf("test_let_statement token array? = %d \n",current_token);
		ix = tokenizer_num();
		accept(TOKENIZER_ARRAY);
	}else{
		printf("test_let_statement token var? = %d \n",current_token);
		accept(TOKENIZER_VARIABLE);
	}
//	if(*nextptr == '='){
//	if('=' == (* (char *))(tokenizer_nextptr())){
//	c = tokenizer_nextptr();
//	printf(nextptr);
	printf(ptr);
//	if(c[0] == '='){
	if(ptr[0] == '='){
		accept(TOKENIZER_EQ);
		printf("test_let_statement token eq? = %d \n",current_token);
		if(var < MAX_VARNUM){
		printf("test_let_statement var = %d \n",var);
			var2 = expr();
		printf("test_let_statement var2 = %d \n",var2);
			ubasic_set_variable(var, var2);
			
//	if(var >= 0 && var <= MAX_VARNUM) {
//		variables[var] = var2;
//	}else if((var > MAX_VARNUM) && (var <= (MAX_VARNUM+MAX_ARRYNUM))) {
//		if(arrays[(var-MAX_VARNUM)][0] < MAX_ARRYLEN){
//			arrays[(var-MAX_VARNUM)][(arrays[(var-MAX_VARNUM)][0])] = var2;
//			arrays[(var-MAX_VARNUM)][0]++;
//		}
//	}else if(((var > (MAX_VARNUM+MAX_ARRYNUM)))
//			&& (var <= (MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM))) {
//		i = strlen(strings[(var-MAX_VARNUM-MAX_ARRYNUM)]);
//		if(i < (MAX_STRINGLEN-1)){
//			strings[(var-MAX_VARNUM-MAX_ARRYNUM)][i] = var2;
//			strings[(var-MAX_VARNUM-MAX_ARRYNUM)][i+1] = 0;
//		}
//	}else if(((var > MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM))
//			&& (var <= (MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM+MAX_BIGVARNUM))) {
//		variables_big[(var-MAX_VARNUM-MAX_ARRYNUM-MAX_STRINGNUM)] = var2;
//	}
			
			
		}else if(var < (MAX_VARNUM+MAX_ARRYNUM)){
			if(ix<MAX_ARRYLEN){
				ubasic_set_array(var,ix, expr());
			}
		}else if(var < (MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM)){
			ubasic_set_variable(var, expr());
		}else if(var < (MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM+MAX_BIGVARNUM)){
			ubasic_set_variable(var, expr());
		}
	}
	accept(TOKENIZER_CR);
#endif	
	printf("var a =  %d \n",variables[0]);
	
	variables[0] = 3;
	strcpy(BASICBUF,"100 LET a\n110 a = 2\n");
	prog = BASICBUF;
	tokenizer_init(prog);
	ubasic_run();
	ubasic_run();
#if 0
	if(tokenizer_finished()) {
		ended = 1;
		printf("uBASIC program finished\n");
		return;
	}
//	line_statement();
	Line = tokenizer_num();
	printf("Line number %d\n", Line);
	printf("test_let_statement token %d \n",current_token);
	accept(TOKENIZER_NUMBER);
//	statement();
	token = tokenizer_token();

	printf("test_let_statement noe_token %d \n",token);
	printf("test_let_statement current_token %d \n",current_token);
	switch(token) {
	printf("var a =  %d \n",variables[0]);
		case TOKENIZER_LET:
	printf("test_let_statement token LET? %d \n",current_token);
			accept(TOKENIZER_LET);
	printf("test_let_statement token VAR? %d \n",current_token);
			accept(TOKENIZER_VARIABLE);
	printf("test_let_statement token CR? %d \n",current_token);
			accept(TOKENIZER_CR);
			break;
		case TOKENIZER_VARIABLE:
			let_statement();
			break;
		default:
			break;
	}
#endif
	printf("var a =  %d \n",variables[0]);
}

void test_gosub_statement(void)
{
	strcpy(BASICBUF,"100 GOSUB 400\n200 PRINT \"END\"\n300 END\n400 PRINT \"400\"\n500 RETURN");
	char * prog = BASICBUF;
	tokenizer_init(prog);

	accept(TOKENIZER_NUMBER);
	printf("test_gosub_statement %d \n",current_token);
	CU_ASSERT(current_token == 15);
	gosub_statement();
	accept(TOKENIZER_NUMBER); //
	printf("test_gosub_statement %d \n",current_token);
	CU_ASSERT(current_token == 7);
	print_statement();
	accept(TOKENIZER_NUMBER); //
	printf("test_gosub_statement %d \n",current_token);
	CU_ASSERT(current_token == 16);

	return_statement();
	
	accept(TOKENIZER_NUMBER);
	printf("test_gosub_statement %d \n",current_token);
	CU_ASSERT(current_token == 7);
	print_statement();
	accept(TOKENIZER_NUMBER);
	printf("test_gosub_statement %d \n",current_token);
	CU_ASSERT(current_token == 18);
	end_statement();

}

void test_return_statement(void)
{
	return_statement();
}

void test_for_statement(void)
{
	strcpy(BASICBUF,"100 FOR a = 0 TO 10\n110 PRINT \"HOGE\\n\"\n120 NEXT a\n130 END\n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 1 %d \n",current_token);
	for_statement();
//	printf("test_for_statement a = %d \n",variables[0]); 	//0
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 2 %d \n",current_token);
	print_statement();		//----------------------------------1
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 3 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//1
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 4 %d \n",current_token);
	print_statement();		//----------------------------------2
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 5 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//2
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 6 %d \n",current_token);
	print_statement();		//-----------------------------------3
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 7 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//3
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 8 %d \n",current_token);
	print_statement();		//------------------------------------4
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 9 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//4
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 10 %d \n",current_token);
	print_statement();		//-------------------------------------5
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 11 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//5
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 12 %d \n",current_token);
	print_statement();		//------------------------------------6
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 13 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//6
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 14 %d \n",current_token);
	print_statement();		//--------------------------------------7
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 15 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//7
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 16 %d \n",current_token);
	print_statement();		//-------------------------------------8
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 17 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//8
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 18 %d \n",current_token);
	print_statement();		//--------------------------------------9
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 19 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//9
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 20 %d \n",current_token);
	print_statement();		//---------------------------------------10
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 21 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//10
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 22 %d \n",current_token);
	print_statement();		//----------------------------------------11
	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 23 %d \n",current_token);
	next_statement();
//	printf("test_for_statement a = %d \n",variables[0]);	//11

	accept(TOKENIZER_NUMBER);
//	printf("test_for_statement 21 %d \n",current_token);
	end_statement();
	
}

void test_pset_statement(void)
{
	strcpy(BASICBUF,"100 PSET 100 100 1\n200 PSET a 100  1\n300 END\n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	variables[0] = 101;
	glcd_BufClear(1);

	accept(TOKENIZER_NUMBER);
	printf("\ntest_pset_statement %d \n",current_token);
	CU_ASSERT(current_token == 34);
	pset_statement();
	glcd_makeimage("test_pset_statement_0.bmp");
	accept(TOKENIZER_NUMBER);
	printf("test_pset_statement %d \n",current_token);
	CU_ASSERT(current_token == 34);
	pset_statement();
	glcd_makeimage("test_pset_statement_1.bmp");
}

void test_end_statement(void)
{
	end_statement();
}

void test_cls_statement(void)
{
	cls_statement();
}

void test_rem_statement(void)
{
	strcpy(BASICBUF,"100 PRINT \"1\"\nREM 200 PRINT \"2\"\n300 PRINT \"3\"\n400 END\n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	glcd_BufClear(1);

	accept(TOKENIZER_NUMBER); //
	printf("\ntest_gosub_statement %d \n",current_token);
	CU_ASSERT(current_token == 7);
	print_statement();
	rem_statement();
	accept(TOKENIZER_NUMBER); //
	printf("test_gosub_statement %d \n",current_token);
	CU_ASSERT(current_token == 7);
	print_statement();
}

void test_list_statement(void)
{
	strcpy(BASICBUF,"100 FOR a = 0 TO 10\n110 PRINT \"HOGE\\n\"\n120 NEXT a\n130 END\n");
	
	strcpy(LINEBUF,"LIST\n");
	char * prog = LINEBUF;
	tokenizer_init(prog);
	list_statement();
//	glcd_makeimage("test_list_statement_0.bmp");
}

void test_load_statement(void)
{
	load_statement();
}

void test_files_statement(void)
{
	files_statement();
}

void test_peek_statement(void)
{
	strcpy(BASICBUF,"100 a = PEEK b\n110 END\n");
	char * prog = BASICBUF;
	variables[1] = 101;
	tokenizer_init(prog);
	glcd_BufClear(1);
	
	accept(TOKENIZER_NUMBER); //
//	printf("\ntest_peek_statement %d \n",current_token);
	CU_ASSERT(current_token == 4);
	peek_statement();
	accept(TOKENIZER_NUMBER); //
//	printf("\ntest_peek_statement %d \n",current_token);
	CU_ASSERT(current_token == 18);
	
}

void test_poke_statement(void)
{
	strcpy(BASICBUF,"100 POKE b 100 \n110 END\n");
	char * prog = BASICBUF;
	variables[1] = 101;
	tokenizer_init(prog);
	glcd_BufClear(1);
	
	accept(TOKENIZER_NUMBER); //
	CU_ASSERT(current_token == 40);
	poke_statement();
	accept(TOKENIZER_NUMBER); //
	CU_ASSERT(current_token == 18);
}

void test_wait_statement(void)
{
	wait_statement();
}

void test_input_statement(void)
{
	int i;
	strcpy(BASICBUF,"100 INPUT E\n110 END\n");
	strcpy(dummykey,"Hoge100\n200");
	for(i=0;i<50;i++)strings[0][i] = 0;
	char * prog = BASICBUF;
	tokenizer_init(prog);
	glcd_BufClear(1);
	dummycount = 0;

	accept(TOKENIZER_NUMBER); //
	CU_ASSERT(current_token == 42);
	input_statement();
	printf("\n");
	printf(strings[0]);
	accept(TOKENIZER_NUMBER); //
	CU_ASSERT(current_token == 18);
}

void test_inp_statement(void)
{
	int i;
	strcpy(BASICBUF,"100 INP a\n110 INP a\n120 INP a\n130 INP a\n140 END\n");
	strcpy(dummykey,"Hoge100\n200");
	variables[0] = 0;
	char * prog = BASICBUF;
	tokenizer_init(prog);
	glcd_BufClear(1);
	dummycount = 0;

	printf("\n");
	accept(TOKENIZER_NUMBER); //
	CU_ASSERT(current_token == 43);
	inp_statement();
	printf("\ntest_inp_statement %d ",variables[0]);
	accept(TOKENIZER_NUMBER); //
	CU_ASSERT(current_token == 43);
	inp_statement();
	printf(" %d ",variables[0]);
	accept(TOKENIZER_NUMBER); //
	CU_ASSERT(current_token == 43);
	inp_statement();
	printf(" %d ",variables[0]);
	accept(TOKENIZER_NUMBER); //
	CU_ASSERT(current_token == 43);
	inp_statement();
	printf(" %d \n",variables[0]);
	accept(TOKENIZER_NUMBER); //
	CU_ASSERT(current_token == 18);
}

void test_inr_statement(void)
{
	inr_statement();
}

void test_ina_statement(void)
{
	ina_statement();
}

void test_run_statement(void)
{
	run_statement();
}

void test_statement(void)
{
	statement();
}

void test_line_statement(void)
{
	line_statement();
}

void test_ubasic_run(void)
{
	int token;

//	strcpy(BASICBUF,"100 LET a\r\n110 a = 100\r\n120 IF a = 1 THEN GOTO 200\r\n130 PRINT \"130\"\r\n140 END\r\n200 PRINT \"200\"\r\n210 END\r\n");
	strcpy(BASICBUF,"100 LET a\r\n110 a = 100\r\n120 IF a = 100 THEN GOTO 200\r\n130 PRINT \"130\"\r\n140 END\r\n200 PRINT \"200\"\r\n210 END\r\n");
//	strcpy(BASICBUF,"100 LET a\n110 a = 100\n120 IF a = 100 THEN GOTO 200\n130 PRINT \"130\"\n140 END\n200 PRINT \"200\"\n210 END\n");
	char * prog = BASICBUF;
	tokenizer_init(prog);
	ended = 0;
	ubasic_run();
//	printf("Line number %d\n", ErrLine);
//	printf("ended %d\n", ended);
	ubasic_run();
//	printf("Line number %d\n", ErrLine);
//	printf("ended %d\n", ended);
	ubasic_run();
#if 0
	if(tokenizer_finished()) {
		ended = 1;
		printf("uBASIC program finished\n");
		return;
	}
//	line_statement();
	printf("test_VAR_statement token %d \n",current_token);
	accept(TOKENIZER_NUMBER);
//	statement();
	token = tokenizer_token();

	printf("test_IF_statement now_token %d \n",token);
	printf("test_IF_statement current_token %d \n",current_token);
	switch(token) {
	printf("var a =  %d \n",variables[0]);
		case TOKENIZER_IF:
	printf("test_IF_statement token IF? %d \n",current_token);
			if_statement();
			break;
		case TOKENIZER_GOTO:
	printf("test_IF_statement token GOTO? %d \n",current_token);
			goto_statement();
			break;
		case TOKENIZER_LET:
	printf("test_let_statement token LET? %d \n",current_token);
			accept(TOKENIZER_LET);
	printf("test_let_statement token VAR? %d \n",current_token);
			accept(TOKENIZER_VARIABLE);
	printf("test_let_statement token CR? %d \n",current_token);
			accept(TOKENIZER_CR);
			break;
		case TOKENIZER_VARIABLE:
	printf("test_let_statement token VARIABLE? %d \n",current_token);
			let_statement();
			break;
		default:
			break;
	}
#endif
	
//	printf("Line number %d\n", ErrLine);
//	printf("ended %d\n", ended);
	ubasic_run();
#if 0
	if(tokenizer_finished()) {
		ended = 1;
		printf("uBASIC program finished\n");
		return;
	}
//	line_statement();
//	ErrLine = tokenizer_num();
//	printf("Line number %d\n", ErrLine);
	printf("test_IF_statement token %d \n",current_token);
	accept(TOKENIZER_NUMBER);
//	statement();
	token = tokenizer_token();

	printf("test_IF_statement now_token %d \n",token);
	printf("test_IF_statement current_token %d \n",current_token);
	switch(token) {
	printf("var a =  %d \n",variables[0]);
		case TOKENIZER_PRINT:
	printf("test_IF_statement token PRINT? %d \n",current_token);
			print_statement();
			break;
		case TOKENIZER_IF:
	printf("test_IF_statement token IF? %d \n",current_token);
			if_statement();
			break;
		case TOKENIZER_GOTO:
	printf("test_IF_statement token GOTO? %d \n",current_token);
			goto_statement();
			break;
		case TOKENIZER_LET:
	printf("test_let_statement token LET? %d \n",current_token);
			accept(TOKENIZER_LET);
	printf("test_let_statement token VAR? %d \n",current_token);
			accept(TOKENIZER_VARIABLE);
	printf("test_let_statement token CR? %d \n",current_token);
			accept(TOKENIZER_CR);
			break;
		case TOKENIZER_VARIABLE:
			let_statement();
			break;
		default:
			break;
	}
#endif
//	printf("Line number %d\n", ErrLine);
//	printf("ended %d\n", ended);
	ubasic_run();
//	printf("Line number %d\n", ErrLine);
//	printf("ended %d\n", ended);
}

void test_ubasic_run2(void)
{
	int token;
	long count;
	strcpy(BASICBUF,"10 let i\n20 let j\n30 let n\n40 let t\n50 for i = 0 to 239\n60 for j = 0 to 399\n70 if 120 > i then gosub 200 else gosub 300\n80 if 120 > j then gosub 400 else gosub 500\n90 if n * n + t * t > 9801 then gosub 600\n100 next j\n110 next i\n120 end\n200 n = 120 - i\n210 return\n300 n = i - 120\n310 return\n400 t = 120 - j\n410 return\n500 t = j  - 120\n510 return\n600 if n * n + t * t < 10404 then gosub 700\n610 return\n700 pset j i 1\n710 return\n720 end\n");
	char * prog = BASICBUF;
	ended = 0;
	count = 0;
	
	glcd_BufClear(1);
//	glcd_clearEditor();
//	glcd_DrawCursor_first();
//	tokenizer_init(prog);
	ubasic_init(prog);
	printf("\n count:");
	while(ended == 0){
		ubasic_run();
		count++;
//		printf(" %d", ErrLine);
//		printf(" %d", count);
		if(ended){
			glcd_makeimage("test_ubasic_run2_0.bmp");
//			printf("ended Line number %d\n", ErrLine);
		}
	}
}

void test_save_statement(void)
{
	int token;
	strcpy(BASICBUF,"10 save \"hoge2.bmp\"\n20 end\n");
	char * prog = BASICBUF;
	ended = 0;
	
	glcd_BufClear(1);
	ubasic_init(prog);
	while(ended == 0){
		ubasic_run();
	}
}

void test_ubasic_finished(void)
{
	int ret;
	ret = ubasic_finished();
}

void test_ubasic_set_variable(void)
{
	variables[0] = 0;
	variables[25] = 0;
	arrays[0][0] = 0;
	arrays[0][1] = 0;
	arrays[3][0] = 0;
	arrays[3][1] = 0;
	strcpy(strings[0],"");
	variables_big[0] = 0;
	variables_big[5] = 0;
//	printf("\n");

	ubasic_set_variable('a'-'a',100);
	CU_ASSERT(variables[0] == 100);
	ubasic_set_variable('z'-'a',200);
	CU_ASSERT(variables[25] == 200);
	ubasic_set_variable(26+'A'-'A',150);
//	printf("test_ubasic_set_variable arrays[0] = %d %d %d\n", arrays[0][0], arrays[0][1], arrays[0][2]);
	CU_ASSERT(arrays[0][0] == 1);
	CU_ASSERT(arrays[0][1] == 150);
	ubasic_set_variable(26+'A'-'A',160);
//	printf("test_ubasic_set_variable arrays[0] = %d %d %d\n", arrays[0][0], arrays[0][1], arrays[0][2]);
	CU_ASSERT(arrays[0][0] == 2);
	CU_ASSERT(arrays[0][2] == 160);
	ubasic_set_variable(26+'D'-'A',170);
//	printf("test_ubasic_set_variable arrays[3] = %d %d %d\n", arrays[3][0], arrays[3][1], arrays[3][2]);
	CU_ASSERT(arrays[3][0] == 1);
	CU_ASSERT(arrays[3][1] == 170);
	ubasic_set_variable(26+'D'-'A',180);
//	printf("test_ubasic_set_variable arrays[3] = %d %d %d\n", arrays[3][0], arrays[3][1], arrays[3][2]);
	CU_ASSERT(arrays[3][0] == 2);
	CU_ASSERT(arrays[3][2] == 180);
	ubasic_set_variable(26+'E'-'A','H');
	CU_ASSERT(strings[0][0] == 'H');
	CU_ASSERT(strings[0][1] == 0);
	ubasic_set_variable(26+'E'-'A','o');
	CU_ASSERT(strings[0][1] == 'o');
	CU_ASSERT(strings[0][2] == 0);
	ubasic_set_variable(26+'E'-'A','g');
	CU_ASSERT(strings[0][2] == 'g');
	CU_ASSERT(strings[0][3] == 0);
	ubasic_set_variable(26+'E'-'A','e');
	CU_ASSERT(strings[0][3] == 'e');
	CU_ASSERT(strings[0][4] == 0);
	ubasic_set_variable(26+'I'-'A',100000);
	CU_ASSERT(variables_big[0] == 100000);
	ubasic_set_variable(26+'I'-'A',100010);
	CU_ASSERT(variables_big[0] == 100010);
	ubasic_set_variable(26+'N'-'A',200000);
	CU_ASSERT(variables_big[5] == 200000);
	ubasic_set_variable(26+'N'-'A',220010);
	CU_ASSERT(variables_big[5] == 220010);
	
}

void test_ubasic_set_variable_big(void)
{
	variables_big[0] = 0;
	variables_big[1] = 0;
	variables_big[2] = 0;
	
	ubasic_set_variable_big(26+'I'-'A',123456);
	CU_ASSERT(variables_big[0] == 123456);
	ubasic_set_variable_big(26+'J'-'A',456789);
	CU_ASSERT(variables_big[1] == 456789);
	ubasic_set_variable_big(26+'K'-'A',10101010);
	CU_ASSERT(variables_big[2] == 10101010);
//	printf("test_ubasic_set_variable arrays[0] = %d %d %d\n", arrays[0][0], arrays[0][1], arrays[0][2]);
}

void test_ubasic_set_array(void)
{
	ubasic_set_array(26+'A'-'A', 0, 155);
//	printf("test_ubasic_set_variable arrays[0] = %d %d %d\n", arrays[0][0], arrays[0][1], arrays[0][2]);
	CU_ASSERT(arrays[0][0] == 155);
	ubasic_set_array(26+'A'-'A', 1, 156);
	ubasic_set_array(26+'A'-'A', 0, 157);
	CU_ASSERT(arrays[0][0] == 157);
	CU_ASSERT(arrays[0][1] == 156);
}

void test_ubasic_clear_array(void)
{
	arrays[0][0] = 122;
	arrays[0][1] = 123;
	ubasic_clear_array(26+'A'-'A');
	CU_ASSERT(arrays[0][0] == 0);
	CU_ASSERT(arrays[0][1] == 0);
}

void test_ubasic_set_string(void)
{
	strcpy(strings[0],"");
	ubasic_set_string(26+'E'-'A', 'F');
	ubasic_set_string(26+'E'-'A', 'u');
	ubasic_set_string(26+'E'-'A', 'g');
	ubasic_set_string(26+'E'-'A', 'a');
	CU_ASSERT(strings[0][0] == 'F');
	CU_ASSERT(strings[0][1] == 'u');
	CU_ASSERT(strings[0][2] == 'g');
	CU_ASSERT(strings[0][3] == 'a');
	CU_ASSERT(strings[0][4] == 0);

}

void test_ubasic_clear_strig(void)
{
	strcpy(strings[1],"Puge");
	ubasic_clear_strig(26+'F'-'A');
	CU_ASSERT(strings[1][0] == 0);
	CU_ASSERT(strings[1][1] == 0);
	CU_ASSERT(strings[1][2] == 0);
	CU_ASSERT(strings[1][3] == 0);
	CU_ASSERT(strings[1][4] == 0);
}

void test_ubasic_get_variable(void)
{
	int ret;
	variables[0] = 555;
	
	ret = ubasic_get_variable('a'-'a');
	CU_ASSERT(ret == 555);
}

int main() {
	
	CU_pSuite test_suite;

	CU_initialize_registry();
	test_suite = CU_add_suite("BASIC_test", NULL, NULL);
	CU_add_test(test_suite, "singlechar", test_singlechar);
	CU_add_test(test_suite, "get_next_token", test_get_next_token);
	CU_add_test(test_suite, "tokenizer_init", test_tokenizer_init);
	CU_add_test(test_suite, "tokenizer_token", test_tokenizer_token);
	CU_add_test(test_suite, "tokenizer_next", test_tokenizer_next);
	CU_add_test(test_suite, "tokenizer_num", test_tokenizer_num);
	CU_add_test(test_suite, "tokenizer_string", test_tokenizer_string);
//	CU_add_test(test_suite, "tokenizer_error_print", test_tokenizer_error_print);
	CU_add_test(test_suite, "tokenizer_finished", test_tokenizer_finished);
	CU_add_test(test_suite, "tokenizer_variable_num", test_tokenizer_variable_num);
	CU_add_test(test_suite, "tokenizer_num_or_variable_num", test_tokenizer_num_or_variable_num);
	CU_add_test(test_suite, "tokenizer_bignum_or_variable_bignum", test_tokenizer_bignum_or_variable_bignum);
	CU_add_test(test_suite, "ubasic_init", test_ubasic_init);
	CU_add_test(test_suite, "accept", test_accept);
	CU_add_test(test_suite, "comment_accept", test_comment_accept);
	CU_add_test(test_suite, "varfactor", test_varfactor);
	CU_add_test(test_suite, "factor", test_factor);
	CU_add_test(test_suite, "term", test_term);
	CU_add_test(test_suite, "expr", test_expr);
	CU_add_test(test_suite, "relation", test_relation);
	CU_add_test(test_suite, "jump_linenum", test_jump_linenum);
	CU_add_test(test_suite, "goto_statement", test_goto_statement);
	CU_add_test(test_suite, "print_statement", test_print_statement);
	CU_add_test(test_suite, "if_statement", test_if_statement);
	CU_add_test(test_suite, "if_statement 2", test_if_statement_2);
	CU_add_test(test_suite, "let_statement", test_let_statement);
	CU_add_test(test_suite, "gosub_statement", test_gosub_statement);
//	CU_add_test(test_suite, "return_statement", test_return_statement);
	CU_add_test(test_suite, "for_statement", test_for_statement);
	CU_add_test(test_suite, "pset_statement", test_pset_statement);
	CU_add_test(test_suite, "rem_statement", test_rem_statement);
	CU_add_test(test_suite, "list_statement", test_list_statement);
	CU_add_test(test_suite, "peek_statement", test_peek_statement);
	CU_add_test(test_suite, "poke_statement", test_poke_statement);
	CU_add_test(test_suite, "input_statement", test_input_statement);
	CU_add_test(test_suite, "inp_statement", test_inp_statement);
	CU_add_test(test_suite, "ubasic_run", test_ubasic_run);
	CU_add_test(test_suite, "ubasic_run2", test_ubasic_run2);
	CU_add_test(test_suite, "save_statement", test_save_statement);
#if 0
	CU_add_test(test_suite, "end_statement", test_end_statement);
	CU_add_test(test_suite, "cls_statement", test_cls_statement);
	CU_add_test(test_suite, "files_statement", test_files_statement);
	CU_add_test(test_suite, "wait_statement", test_wait_statement);
	CU_add_test(test_suite, "inr_statement", test_inr_statement);
	CU_add_test(test_suite, "ina_statement", test_ina_statement);
	CU_add_test(test_suite, "run_statement", test_run_statement);
	CU_add_test(test_suite, "statement", test_statement);
	CU_add_test(test_suite, "line_statement", test_line_statement);
	CU_add_test(test_suite, "ubasic_finished", test_ubasic_finished);
#endif
	CU_add_test(test_suite, "ubasic_set_variable", test_ubasic_set_variable);
	CU_add_test(test_suite, "ubasic_set_variable_big", test_ubasic_set_variable_big);
	CU_add_test(test_suite, "ubasic_set_array", test_ubasic_set_array);
	CU_add_test(test_suite, "ubasic_clear_array", test_ubasic_clear_array);
	CU_add_test(test_suite, "ubasic_set_string", test_ubasic_set_string);
	CU_add_test(test_suite, "ubasic_clear_strig", test_ubasic_clear_strig);
	CU_add_test(test_suite, "ubasic_get_variable", test_ubasic_get_variable);
	CU_console_run_tests();
	CU_cleanup_registry();
	return(0);
}

