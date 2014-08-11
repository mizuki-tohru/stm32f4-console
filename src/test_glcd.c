/*Unit Test Code  */
/*Editor Section  */
/*USE CUnit       */

#include <CUnit/CUnit.h>
#include <CUnit/Console.h>
#include <stdio.h>
#include <stdlib.h>

//#include "stm32f4xx.h"
//#include "platform_config.h"
//#include "hw_config.h"
//#include "glcd.h"
#include "editor.h"
#include "ubasic.h"
#include "tokenizer.h"
//#include "fontdata.h"

/* Defines -------------------------------------------------------------------*/
#define KEY_NUM    0x01
#define KEY_BS     0x08
#define KEY_TAB    0x09
#define KEY_ESC    0x1B
#define KEY_SHIFT  0x03
#define KEY_CTRL   0x04
#define KEY_SPACE  0x20
//#define KEY_RETURN 0x0d
#define KEY_RETURN 0x0a
#define KEY_CAPS   0x02
#define KEY_CURSOR 0x05
#define KEY_UP     0x11
#define KEY_DOWN   0x12
#define KEY_LEFT   0x12
#define KEY_RIGHT  0x13

char BASICBUF[MAXBASIC];
char LINEBUF[128];

#define MAX_BASICBUF MAXBASIC
char RUNBUF[MAXBASIC];

uint8_t glcd_buf[50*240];
uint8_t display_buf[50*240];
//uint8_t glcd_check[241];
uint8_t glcd_check[31];/*LCDへ転送するラインを示すフラグ*/
uint8_t glcd_x,glcd_y,flgUpdate;
/* 2013.07.21 */
uint16_t glcd_kanji_text_buf[50*16];

uint16_t editor_line[MAXLINE];
uint16_t nowline;
uint16_t viewtop;

int  cursor_ptr;
//int  end_ptr;
int  cursor_x;
int  cursor_y;
int  old_x;
int  old_y;
int  lineend;
int  line_end;
int  line_flg;
int end_ptr;

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


void glcd_TransFromBuf(void)
{
	int l;
	for (l = 1;l <= 240;l++) {
		if(glcd_check[(l>>3)]&(1<<(l&0x07))){
			glcd_check[(l>>3)] &= 0xff^(1<<(l&0x07));
		}
	}
}

//void test_glcd_getCursorPos(void)
//{
//	int i,retval;   /*012345678 90123 4 5678901 2*/
//	char testbuf[] = "10 PRINT \"HOGE\"\n20 END\n";
//	for(i=0;i<strlen(testbuf);i++){
//		BASICBUF[i] = testbuf[i];
//	}
//	BASICBUF[i+1] = 0;
//	end_ptr = i;
//	/*画面上にカーソルがあるとき、カーソルの文字列ポインタを渡す*/
//	retval = glcd_getCursorPos(0,0);
//	CU_ASSERT(retval == 0);/*アタマは当然0*/
//	retval = glcd_getCursorPos(7,0);
//	CU_ASSERT(retval == 7);
//	retval = glcd_getCursorPos(1,1);
//	CU_ASSERT(end_ptr == 23);
//	CU_ASSERT(retval == 17);/*2"0"の位置*/
//	retval = glcd_getCursorPos(20,1);
//	CU_ASSERT(retval == -1);
//	retval = glcd_getCursorPos(5,5);
//	CU_ASSERT(retval == -1);
//}


void test_glcd_redraw(void)
{
	int i,ret_val;
	uint8_t testbuf[] = "10 PRINT \"HOGE\"\n20 END\n";
	uint16_t testline[] = {16,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//	uint16_t testline[] = {15,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//	for(i=0;i<strlen(testbuf);i++){
	for(i=0;i<sizeof(testbuf);i++){
		BASICBUF[i] = testbuf[i];
	}
	glcd_BufClear(1);
	end_ptr = 0;
	for(i=0;i<(sizeof(testline)/2);i++){
		editor_line[i] = testline[i];
		end_ptr += testline[i];
	}
	BASICBUF[i+1] = 0;
	
	viewtop = 0;
	
	glcd_redraw();
	/*'1'*/
	CU_ASSERT(glcd_buf[0+(50*0)] == 0xff);
	CU_ASSERT(glcd_buf[0+(50*1)] == 0xe7);
	CU_ASSERT(glcd_buf[0+(50*2)] == 0xe7);
	CU_ASSERT(glcd_buf[0+(50*3)] == 0xc7);
	CU_ASSERT(glcd_buf[0+(50*4)] == 0x97);
	CU_ASSERT(glcd_buf[0+(50*5)] == 0xe7);
	CU_ASSERT(glcd_buf[0+(50*6)] == 0xe7);
	CU_ASSERT(glcd_buf[0+(50*7)] == 0xe7);
	/*'0'*/
	CU_ASSERT(glcd_buf[1+(50*0)] == 0xff);
	CU_ASSERT(glcd_buf[1+(50*1)] == 0xe3);
	CU_ASSERT(glcd_buf[1+(50*2)] == 0xc9);
	CU_ASSERT(glcd_buf[1+(50*3)] == 0xc9);
	CU_ASSERT(glcd_buf[1+(50*4)] == 0x9c);
	CU_ASSERT(glcd_buf[1+(50*5)] == 0x9c);
	CU_ASSERT(glcd_buf[1+(50*6)] == 0x9c);
	CU_ASSERT(glcd_buf[1+(50*7)] == 0x9c);

	CU_ASSERT(glcd_check[0] == 0xff);

//	printf("test_glcd_redraw glcd_buf0= %x \n",glcd_buf[1+(50*0)]);
//	printf("test_glcd_redraw glcd_buf1= %x \n",glcd_buf[1+(50*1)]);
//	printf("test_glcd_redraw glcd_buf2= %x \n",glcd_buf[1+(50*2)]);
//	printf("test_glcd_redraw glcd_buf3= %x \n",glcd_buf[1+(50*3)]);
//	printf("test_glcd_redraw glcd_buf4= %x \n",glcd_buf[1+(50*4)]);
//	printf("test_glcd_redraw glcd_buf5= %x \n",glcd_buf[1+(50*5)]);
//	printf("test_glcd_redraw glcd_buf6= %x \n",glcd_buf[1+(50*6)]);
//	printf("test_glcd_redraw glcd_buf7= %x \n",glcd_buf[1+(50*7)]);

//	printf("test_glcd_redraw glcd_check0= %x \n",glcd_check[0]);

	CU_ASSERT(end_ptr == 23);
//	printf("test_glcd_redraw end_ptr= %d \n",end_ptr);
	glcd_makeimage("test_glcd_redraw_1.bmp");
}

void test_glcd_redraw2(void)
{
	int i;
	int flg;
	uint8_t testbuf1[] = "10 PRINT \"HOGE\"\n20 END";
	uint8_t testbuf2[] = "PRINT \"FUGA\"\n30 END\n";
	for(i=0;i<100;i++)BASICBUF[i] = 0;
	for(i=0;i<100;i++)LINEBUF[i] = 0;
	glcd_BufClear(1);
	end_ptr = 0;
	line_end = 0;
	line_ptr = 0;
	old_x = 0;
	old_y = 0;
	cursor_x = 0;
	cursor_y = 0;
	flg = 0;

	printf("\n");
	for(i=0;i<strlen(testbuf1);i++){
		glcd_PutEditor(testbuf1[i]);
	}
	CU_ASSERT(end_ptr == 16);
	glcd_PutEditor(0x08);	//KEY_BS
	glcd_PutEditor(0x08);	//KEY_BS
	glcd_PutEditor(0x08);	//KEY_BS
	CU_ASSERT(end_ptr == 16);
//	printf("test_glcd_PutEditor2 end_ptr = %d \n", end_ptr);
//	printf("test_glcd_PutEditor2 before X = %d Y = %d \n",cursor_x,cursor_y);
//	printf("test_glcd_PutEditor2 before old X = %d Y = %d \n",old_x,old_y);
	for(i=0;i<strlen(testbuf2);i++){
		glcd_PutEditor(testbuf2[i]);
//		printf(BASICBUF);
//		printf("\n");
	}
	CU_ASSERT(end_ptr == 39);
	glcd_makeimage("test_glcd_redraw2_1.bmp");
	glcd_redraw();
	glcd_makeimage("test_glcd_redraw2_2.bmp");
}

void test_glcd_seteditor_line(void)
{
	int i;
	uint8_t testbuf[] = "10 PRINT \"HOGE\"\n20 END\n";
	end_ptr = 0;

	for(i=0;i<sizeof(testbuf);i++){
		BASICBUF[i] = testbuf[i];
		end_ptr++;
	}
	glcd_BufClear(1);
	
	glcd_seteditor_line();
	CU_ASSERT(editor_line[0] == 16);
//	printf("test_glcd_seteditor_line editor_line= %d \n",editor_line[0]);
	CU_ASSERT(editor_line[1] == 7);
//	printf("test_glcd_seteditor_line editor_line= %d \n",editor_line[1]);
	CU_ASSERT(editor_line[2] == 0);
	glcd_makeimage("test_glcd_seteditor_line_1.bmp");
}

void test_glcd_DrawCursor(void)
{
	int i;
	int flg;
	uint8_t testbuf[] = "10 PRINT \"HOGE\"\n20 END\n";
	uint16_t testline[] = {16,7,0,0,0,0,0,0,0,0,0,0};
//	uint16_t testline[] = {15,6,0,0,0,0,0,0,0,0,0,0};
//	for(i=0;i<strlen(testbuf);i++){
//		BASICBUF[i] = testbuf[i];
//	}
	glcd_BufClear(1);
	end_ptr = 0;

//	for(i=0;i<strlen(testline);i++){
//	for(i=0;(testline[i] != 0);i++){
//		editor_line[i] = testline[i];
//	}
//	BASICBUF[i+1] = 0;
	old_x = 0;
	old_y = 0;
	cursor_x = 0;
	cursor_y = 0;
	flg = 0;
	printf("\n");
//	printf("test_glcd_DrawCursor strlen = %d \n",strlen(testbuf));

	for(i=0;i<strlen(testbuf)-1;i++){
		glcd_PutEditor(testbuf[i]);
//		if(i==20)glcd_makeimage("test_glcd_DrawCursor_10.bmp");
//		if(i==21)glcd_makeimage("test_glcd_DrawCursor_11.bmp");
//		if(i==22)glcd_makeimage("test_glcd_DrawCursor_12.bmp");
	}
	glcd_makeimage("test_glcd_DrawCursor_0a.bmp");
	printf("test_glcd_DrawCursor before X = %d Y = %d \n",cursor_x,cursor_y);
	printf("test_glcd_DrawCursor before old X = %d Y = %d \n",old_x,old_y);

//	glcd_PutEditor('\n');
#if 1
	LINEBUF[line_ptr] = '\n';
	line_end = cursor_x;
	LINEBUF[line_end+1] = 0x00;
	/*行番号があれば*/
	if((LINEBUF[0] >= '0')
	 &&(LINEBUF[0] <= '9')){
		insert_basic_line(LINEBUF,line_end+1);
		line_end = 0;
		line_ptr = 0;
		for(i=0;i<50;i++)LINEBUF[i] = 0;
	printf(BASICBUF);
	}else{/*行番号が無ければ*/
		for(i=line_end;i>=0;i--)LINEBUF[i+3] = LINEBUF[i];
		LINEBUF[0] = '1';
		LINEBUF[1] = '0';
		LINEBUF[2] = ' ';
		LINEBUF[3+line_end] = '\n';
		LINEBUF[4+line_end] = '2';
		LINEBUF[5+line_end] = '0';
		LINEBUF[6+line_end] = ' ';
		LINEBUF[7+line_end] = 'e';
		LINEBUF[8+line_end] = 'n';
		LINEBUF[9+line_end] = 'd';
		LINEBUF[10+line_end] = '\n';
		LINEBUF[11+line_end] = 0x00;
		line_end = 0;
		line_ptr = 0;
	}
	glcd_PutCharA('\n');
	printf(BASICBUF);
	glcd_TransFromBuf();
#endif
	glcd_makeimage("test_glcd_DrawCursor_0.bmp");
	printf("test_glcd_DrawCursor before X = %d Y = %d \n",cursor_x,cursor_y);
	printf("test_glcd_DrawCursor before old X = %d Y = %d \n",old_x,old_y);

	glcd_DrawCursor();
	CU_ASSERT(glcd_buf[0] == 0xff);
	CU_ASSERT(glcd_check[0] == 0x01);
//	printf("test_glcd_DrawCursor_buf0= %x \n",glcd_buf[0]);
//	printf("test_glcd_DrawCursor_check0= %x \n",glcd_check[0]);
	printf("test_glcd_DrawCursor after X = %d Y = %d \n",cursor_x,cursor_y);
	glcd_makeimage("test_glcd_DrawCursor_1.bmp");
}

void test_glcd_PutEditor(void)
{
	int i;
	for(i=0;i<100;i++)BASICBUF[i] = 0;
	for(i=0;i<10;i++)editor_line[i] = 0;
	old_x = 0;
	old_y = 0;
	cursor_x = 0;
	cursor_y = 0;
	end_ptr = 0;
	glcd_BufClear(1);

	glcd_PutEditor('1');
	CU_ASSERT(line_end == 1);
	CU_ASSERT(LINEBUF[0] == '1');
	CU_ASSERT(editor_line[0] == 1);
//	printf("test_glcd_PutEditor editor_line[0]= %d \n",editor_line[0]);

	glcd_PutEditor('0');
	CU_ASSERT(line_end == 2);
	CU_ASSERT(LINEBUF[1] == '0');
	CU_ASSERT(editor_line[0] == 2);

	glcd_PutEditor(' ');
	CU_ASSERT(line_end == 3);
	CU_ASSERT(LINEBUF[2] == ' ');
	CU_ASSERT(editor_line[0] == 3);

	glcd_PutEditor('P');
	CU_ASSERT(line_end == 4);
	CU_ASSERT(LINEBUF[3] == 'P');
	CU_ASSERT(editor_line[0] == 4);

	glcd_PutEditor('R');
	CU_ASSERT(line_end == 5);
	CU_ASSERT(LINEBUF[4] == 'R');
	CU_ASSERT(editor_line[0] == 5);

	glcd_PutEditor('I');
	CU_ASSERT(line_end == 6);
	CU_ASSERT(LINEBUF[5] == 'I');
	CU_ASSERT(editor_line[0] == 6);

	glcd_PutEditor('N');
	CU_ASSERT(line_end == 7);
	CU_ASSERT(LINEBUF[6] == 'N');
	CU_ASSERT(editor_line[0] == 7);

	glcd_PutEditor('T');
	CU_ASSERT(line_end == 8);
	CU_ASSERT(LINEBUF[7] == 'T');
	CU_ASSERT(editor_line[0] == 8);

	glcd_PutEditor(' ');
	CU_ASSERT(line_end == 9);
	CU_ASSERT(LINEBUF[8] == ' ');
	CU_ASSERT(editor_line[0] == 9);

	glcd_PutEditor('"');
	CU_ASSERT(line_end == 10);
	CU_ASSERT(LINEBUF[9] == '"');
	CU_ASSERT(editor_line[0] == 10);

	glcd_PutEditor('H');
	CU_ASSERT(line_end == 11);
	CU_ASSERT(LINEBUF[10] == 'H');
	CU_ASSERT(editor_line[0] == 11);

	glcd_PutEditor('O');
	CU_ASSERT(line_end == 12);
	CU_ASSERT(LINEBUF[11] == 'O');
	CU_ASSERT(editor_line[0] == 12);

	glcd_PutEditor('G');
	CU_ASSERT(line_end == 13);
	CU_ASSERT(LINEBUF[12] == 'G');
	CU_ASSERT(editor_line[0] == 13);

	glcd_PutEditor('E');
	CU_ASSERT(line_end == 14);
	CU_ASSERT(LINEBUF[13] == 'E');
	CU_ASSERT(editor_line[0] == 14);

	glcd_PutEditor('"');
	CU_ASSERT(line_end == 15);
	CU_ASSERT(LINEBUF[14] == '"');
	CU_ASSERT(editor_line[0] == 15);
	glcd_makeimage("test_glcd_PutEditor_1.bmp");

//	printf("test_glcd_PutEditor end_ptr = %d \n", end_ptr);

	glcd_PutEditor('\n');
	CU_ASSERT(line_end == 0);
	CU_ASSERT(BASICBUF[15] == '\n');
	CU_ASSERT(editor_line[0] == 16);
//	CU_ASSERT(BASICBUF[15] == 0x00); //こうなっている
//	CU_ASSERT(editor_line[0] == 15); //おかしい
	CU_ASSERT(editor_line[1] == 0);
//	printf("test_glcd_PutEditor end_ptr = %d \n", end_ptr);
	
//	printf(">");
//	printf(BASICBUF);
//	printf("<");
	glcd_makeimage("test_glcd_PutEditor_2.bmp");

	glcd_PutEditor('2');
	CU_ASSERT(line_end == 1);
	CU_ASSERT(LINEBUF[0] == '2');
	CU_ASSERT(editor_line[1] == 1);

	glcd_PutEditor('0');
	CU_ASSERT(line_end == 2);
	CU_ASSERT(LINEBUF[1] == '0');
	CU_ASSERT(editor_line[1] == 2);

	glcd_PutEditor(' ');
	CU_ASSERT(line_end == 3);
	CU_ASSERT(LINEBUF[2] == ' ');
	CU_ASSERT(editor_line[1] == 3);

	glcd_PutEditor('E');
	CU_ASSERT(line_end == 4);
	CU_ASSERT(LINEBUF[3] == 'E');
	CU_ASSERT(editor_line[1] == 4);

	glcd_PutEditor('N');
	CU_ASSERT(line_end == 5);
	CU_ASSERT(LINEBUF[4] == 'N');
	CU_ASSERT(editor_line[1] == 5);

	glcd_PutEditor('D');
	CU_ASSERT(line_end == 6);
	CU_ASSERT(LINEBUF[5] == 'D');
	CU_ASSERT(editor_line[1] == 6);
//	printf("test_glcd_PutEditor end_ptr = %d \n", end_ptr);

	glcd_PutEditor('\n');
	CU_ASSERT(line_end == 0);
	CU_ASSERT(end_ptr == 23);
	CU_ASSERT(BASICBUF[22] == '\n');
	CU_ASSERT(editor_line[1] == 7);
	CU_ASSERT(editor_line[2] == 0);
//	printf("test_glcd_PutEditor end_ptr = %d \n", end_ptr);
//	printf("test_glcd_PutEditor editor_line[0]= %d \n",editor_line[0]);
//	printf("test_glcd_PutEditor editor_line[1]= %d \n",editor_line[1]);
//	printf("test_glcd_PutEditor editor_line[2]= %d \n",editor_line[2]);
	glcd_makeimage("test_glcd_PutEditor_3.bmp");

	glcd_PutEditor(KEY_BS);
	CU_ASSERT(line_end == 0);
	CU_ASSERT(LINEBUF[0] ==0x00);
	CU_ASSERT(editor_line[1] == 7);
	printf("\n");
	printf("test_glcd_PutEditor line_end = %d \n", line_end);
	printf("test_glcd_PutEditor editor_line[1]= %d \n",editor_line[1]);

	glcd_PutEditor(KEY_SPACE);
	CU_ASSERT(line_end == 1);
	CU_ASSERT(LINEBUF[0] == KEY_SPACE);
	CU_ASSERT(editor_line[2] == 1);
}

void test_glcd_PutEditor2(void)
{
	int i;
	int flg;
	uint8_t testbuf1[] = "10 PRINT \"HOGE\"\n20 END";
	uint8_t testbuf2[] = "PRINT \"FUGA\"\n30 END\n";
	for(i=0;i<100;i++)BASICBUF[i] = 0;
	for(i=0;i<100;i++)LINEBUF[i] = 0;
	glcd_BufClear(1);
	end_ptr = 0;
	line_end = 0;
	line_ptr = 0;
	old_x = 0;
	old_y = 0;
	cursor_x = 0;
	cursor_y = 0;
	flg = 0;

	printf("\n");
	for(i=0;i<strlen(testbuf1);i++){
		glcd_PutEditor(testbuf1[i]);
	}
	CU_ASSERT(end_ptr == 16);
//	printf(LINEBUF);
//	printf("\n");
//	printf("test_glcd_PutEditor2 end_ptr = %d \n", end_ptr);
//	glcd_makeimage("test_glcd_PutEditor2_0.bmp");
	glcd_PutEditor(0x08);	//KEY_BS
//	printf("test_glcd_PutEditor2 end_ptr = %d \n", end_ptr);
//	glcd_makeimage("test_glcd_PutEditor2_1.bmp");
	glcd_PutEditor(0x08);	//KEY_BS
	glcd_PutEditor(0x08);	//KEY_BS
//	glcd_makeimage("test_glcd_PutEditor2_2.bmp");
	CU_ASSERT(end_ptr == 16);
//	printf("test_glcd_PutEditor2 end_ptr = %d \n", end_ptr);
//	printf("test_glcd_PutEditor2 before X = %d Y = %d \n",cursor_x,cursor_y);
//	printf("test_glcd_PutEditor2 before old X = %d Y = %d \n",old_x,old_y);
	for(i=0;i<strlen(testbuf2);i++){
		glcd_PutEditor(testbuf2[i]);
//		printf(BASICBUF);
//		printf("\n");
	}
	CU_ASSERT(end_ptr == 39);
//	glcd_makeimage("test_glcd_PutEditor2_3.bmp");
///	printf("test_glcd_PutEditor2 end_ptr = %d \n", end_ptr);
//	printf("test_glcd_PutEditor2 before X = %d Y = %d \n",cursor_x,cursor_y);
//	printf("test_glcd_PutEditor2 before old X = %d Y = %d \n",old_x,old_y);
//	printf(BASICBUF);
	
}

void test_glcd_PutEditor3(void)
{
	int i;
	int flg;
	uint8_t testbuf1[] = "HOGE\n";
	for(i=0;i<100;i++)BASICBUF[i] = 0;
	for(i=0;i<100;i++)LINEBUF[i] = 0;
	glcd_BufClear(1);
	end_ptr = 0;
	line_end = 0;
	line_ptr = 0;
	old_x = 0;
	old_y = 0;
	cursor_x = 0;
	cursor_y = 0;
	flg = 0;

	printf("\n");
	for(i=0;i<strlen(testbuf1);i++){
		glcd_PutEditor(testbuf1[i]);
	}
	glcd_PutEditor('\n');
	glcd_makeimage("test_glcd_PutEditor3_1.bmp");
	glcd_PutEditor('\n');
	glcd_makeimage("test_glcd_PutEditor3_2.bmp");
	glcd_PutEditor('\n');
	glcd_makeimage("test_glcd_PutEditor3_3.bmp");
	
//	printf(LINEBUF);
//	printf("\n");
//	printf("test_glcd_PutEditor2 end_ptr = %d \n", end_ptr);
//	glcd_makeimage("test_glcd_PutEditor2_0.bmp");
//	printf("test_glcd_PutEditor2 end_ptr = %d \n", end_ptr);
//	glcd_makeimage("test_glcd_PutEditor2_1.bmp");
//	glcd_makeimage("test_glcd_PutEditor2_2.bmp");
//	printf("test_glcd_PutEditor2 end_ptr = %d \n", end_ptr);
//	printf("test_glcd_PutEditor2 before X = %d Y = %d \n",cursor_x,cursor_y);
//	printf("test_glcd_PutEditor2 before old X = %d Y = %d \n",old_x,old_y);
//		printf(BASICBUF);
//		printf("\n");
//	glcd_makeimage("test_glcd_PutEditor2_3.bmp");
///	printf("test_glcd_PutEditor2 end_ptr = %d \n", end_ptr);
//	printf("test_glcd_PutEditor2 before X = %d Y = %d \n",cursor_x,cursor_y);
//	printf("test_glcd_PutEditor2 before old X = %d Y = %d \n",old_x,old_y);
//	printf(BASICBUF);
	
}

void test_glcd_PutEditor4(void)
{
	int i;
	int flg;
	uint8_t testbuf1[] = "files\n";
	for(i=0;i<100;i++)BASICBUF[i] = 0;
	for(i=0;i<50;i++)LINEBUF[i] = 0;
	glcd_BufClear(1);
	end_ptr = 0;
	line_end = 0;
	line_ptr = 0;
	old_x = 0;
	old_y = 0;
	cursor_x = 0;
	cursor_y = 0;
	flg = 0;

	printf("\n");
	for(i=0;i<strlen(testbuf1);i++){
		glcd_PutEditor(testbuf1[i]);
	}
	  glcd_PutsD(" ----A 2014/07/16 16:01 39  TEST.BAS\n");
	  glcd_PutsD(" ----A 2014/07/17 15:13 108  TEST2.BAS\n");
	  glcd_PutsD(" ----A 2014/08/01 14:35 472  TEST3.BAS\n");
	  glcd_PutsD(" ----A 2014/08/05 16:53 12542  test0.bmp\n");

	glcd_PutEditor('l');
	glcd_PutEditor('o');
	glcd_PutEditor('a');
	glcd_PutEditor('d');
	glcd_PutEditor(' ');
	glcd_PutEditor('"');
	glcd_PutEditor('T');
	glcd_PutEditor('E');
	glcd_PutEditor('S');
	glcd_PutEditor('T');
	glcd_makeimage("test_glcd_PutEditor4_1.bmp");
	glcd_PutEditor('3');
	glcd_makeimage("test_glcd_PutEditor4_2.bmp");
	glcd_PutEditor('.');
	glcd_makeimage("test_glcd_PutEditor4_3.bmp");
}

void test_glcd_PutCharA(void)
{
	int i;
	for(i=0;i<100;i++)BASICBUF[i] = 0;
	for(i=0;i<10;i++)editor_line[i] = 0;
	end_ptr = 0;
	old_x = 0;
	old_y = 0;
	cursor_x = 0;
	cursor_y = 0;
	glcd_BufClear(1);
	glcd_DrawCursor();

//	printf("test_glcd_PutCharA after X = %d Y = %d \n",cursor_x,cursor_y);
	glcd_PutCharA('1');
//	printf("test_glcd_PutCharA after X = %d Y = %d \n",cursor_x,cursor_y);

	/*'1'*/
	CU_ASSERT(glcd_buf[0+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[0+(50*1)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[0+(50*2)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[0+(50*3)] == 0x38^0xff);
	CU_ASSERT(glcd_buf[0+(50*4)] == 0x68^0xff);
	CU_ASSERT(glcd_buf[0+(50*5)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[0+(50*6)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[0+(50*7)] == 0x18^0xff);
//	printf("test_glcd_PutCharA glcd_buf0= %x \n",glcd_buf[0+(50*0)]);
//	printf("test_glcd_PutCharA glcd_buf1= %x \n",glcd_buf[0+(50*1)]);
//	printf("test_glcd_PutCharA glcd_buf2= %x \n",glcd_buf[0+(50*2)]);
//	printf("test_glcd_PutCharA glcd_buf3= %x \n",glcd_buf[0+(50*3)]);
//	printf("test_glcd_PutCharA glcd_buf4= %x \n",glcd_buf[0+(50*4)]);
//	printf("test_glcd_PutCharA glcd_buf5= %x \n",glcd_buf[0+(50*5)]);
//	printf("test_glcd_PutCharA glcd_buf6= %x \n",glcd_buf[0+(50*6)]);
//	printf("test_glcd_PutCharA glcd_buf7= %x \n",glcd_buf[0+(50*7)]);
//	glcd_makeimage("test_glcd_PutCharA_0.bmp");

	CU_ASSERT(cursor_x == 1);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 1);
//	printf("test_glcd_seteditor_line editor_line= %d \n",editor_line[0]);

	glcd_PutCharA('0');
	/*'0'*/
	CU_ASSERT(glcd_buf[1+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[1+(50*1)] == 0x1C^0xff);
	CU_ASSERT(glcd_buf[1+(50*2)] == 0x36^0xff);
	CU_ASSERT(glcd_buf[1+(50*3)] == 0x36^0xff);
	CU_ASSERT(glcd_buf[1+(50*4)] == 0x63^0xff);
	CU_ASSERT(glcd_buf[1+(50*5)] == 0x63^0xff);
	CU_ASSERT(glcd_buf[1+(50*6)] == 0x63^0xff);
	CU_ASSERT(glcd_buf[1+(50*7)] == 0x63^0xff);
	
	CU_ASSERT(cursor_x == 2);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 2);
//	printf("test_glcd_seteditor_line editor_line= %d \n",editor_line[0]);

	glcd_PutCharA(' ');
	/*'0'*/
	CU_ASSERT(glcd_buf[2+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[2+(50*1)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[2+(50*2)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[2+(50*3)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[2+(50*4)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[2+(50*5)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[2+(50*6)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[2+(50*7)] == 0x00^0xff);
	CU_ASSERT(cursor_x == 3);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 3);

	glcd_PutCharA('P');
	CU_ASSERT(cursor_x == 4);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 4);
	CU_ASSERT(glcd_buf[3+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[3+(50*1)] == 0xFC^0xff);
	CU_ASSERT(glcd_buf[3+(50*2)] == 0xC6^0xff);
	CU_ASSERT(glcd_buf[3+(50*3)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[3+(50*4)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[3+(50*5)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[3+(50*6)] == 0xC6^0xff);
	CU_ASSERT(glcd_buf[3+(50*7)] == 0xFC^0xff);

	glcd_PutCharA('R');
	CU_ASSERT(cursor_x == 5);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 5);
	CU_ASSERT(glcd_buf[4+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[4+(50*1)] == 0xFC^0xff);
	CU_ASSERT(glcd_buf[4+(50*2)] == 0xC6^0xff);
	CU_ASSERT(glcd_buf[4+(50*3)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[4+(50*4)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[4+(50*5)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[4+(50*6)] == 0xC6^0xff);
	CU_ASSERT(glcd_buf[4+(50*7)] == 0xFC^0xff);
	
	glcd_PutCharA('I');
	CU_ASSERT(glcd_buf[5+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[5+(50*1)] == 0x3C^0xff);
	CU_ASSERT(glcd_buf[5+(50*2)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[5+(50*3)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[5+(50*4)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[5+(50*5)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[5+(50*6)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[5+(50*7)] == 0x18^0xff);
	CU_ASSERT(cursor_x == 6);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 6);

	glcd_PutCharA('N');
	CU_ASSERT(glcd_buf[6+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[6+(50*1)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[6+(50*2)] == 0xE3^0xff);
	CU_ASSERT(glcd_buf[6+(50*3)] == 0xE3^0xff);
	CU_ASSERT(glcd_buf[6+(50*4)] == 0xD3^0xff);
	CU_ASSERT(glcd_buf[6+(50*5)] == 0xD3^0xff);
	CU_ASSERT(glcd_buf[6+(50*6)] == 0xDB^0xff);
	CU_ASSERT(glcd_buf[6+(50*7)] == 0xDB^0xff);
	CU_ASSERT(cursor_x == 7);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 7);

	glcd_PutCharA('T');
	CU_ASSERT(glcd_buf[7+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[7+(50*1)] == 0xFF^0xff);
	CU_ASSERT(glcd_buf[7+(50*2)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[7+(50*3)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[7+(50*4)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[7+(50*5)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[7+(50*6)] == 0x18^0xff);
	CU_ASSERT(glcd_buf[7+(50*7)] == 0x18^0xff);
	CU_ASSERT(cursor_x == 8);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 8);

	glcd_PutCharA(' ');
	CU_ASSERT(glcd_buf[8+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[8+(50*1)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[8+(50*2)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[8+(50*3)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[8+(50*4)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[8+(50*5)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[8+(50*6)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[8+(50*7)] == 0x00^0xff);
	CU_ASSERT(cursor_x == 9);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 9);

	glcd_PutCharA('"');
	CU_ASSERT(glcd_buf[9+(50*0)] == 0x76^0xff);
	CU_ASSERT(glcd_buf[9+(50*1)] == 0x36^0xff);
	CU_ASSERT(glcd_buf[9+(50*2)] == 0x36^0xff);
	CU_ASSERT(glcd_buf[9+(50*3)] == 0x6C^0xff);
	CU_ASSERT(glcd_buf[9+(50*4)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[9+(50*5)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[9+(50*6)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[9+(50*7)] == 0x00^0xff);
	CU_ASSERT(cursor_x == 10);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 10);

	glcd_PutCharA('H');
	CU_ASSERT(glcd_buf[10+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[10+(50*1)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[10+(50*2)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[10+(50*3)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[10+(50*4)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[10+(50*5)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[10+(50*6)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[10+(50*7)] == 0xFF^0xff);
//	printf("test_glcd_redraw glcd_buf0= %x \n",glcd_buf[10+(50*0)]);
//	printf("test_glcd_redraw glcd_buf1= %x \n",glcd_buf[10+(50*1)]);
//	printf("test_glcd_redraw glcd_buf2= %x \n",glcd_buf[10+(50*2)]);
//	printf("test_glcd_redraw glcd_buf3= %x \n",glcd_buf[10+(50*3)]);
//	printf("test_glcd_redraw glcd_buf4= %x \n",glcd_buf[10+(50*4)]);
//	printf("test_glcd_redraw glcd_buf5= %x \n",glcd_buf[10+(50*5)]);
//	printf("test_glcd_redraw glcd_buf6= %x \n",glcd_buf[10+(50*6)]);
//	printf("test_glcd_redraw glcd_buf7= %x \n",glcd_buf[10+(50*7)]);
	CU_ASSERT(cursor_x == 11);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 11);

	glcd_PutCharA('O');
	CU_ASSERT(glcd_buf[11+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[11+(50*1)] == 0x3C^0xff);
	CU_ASSERT(glcd_buf[11+(50*2)] == 0x66^0xff);
	CU_ASSERT(glcd_buf[11+(50*3)] == 0x66^0xff);
	CU_ASSERT(glcd_buf[11+(50*4)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[11+(50*5)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[11+(50*6)] == 0xC3^0xff);
	CU_ASSERT(glcd_buf[11+(50*7)] == 0xC3^0xff);
//	printf("test_glcd_redraw glcd_buf0= %x \n",glcd_buf[11+(50*0)]);
//	printf("test_glcd_redraw glcd_buf1= %x \n",glcd_buf[11+(50*1)]);
//	printf("test_glcd_redraw glcd_buf2= %x \n",glcd_buf[11+(50*2)]);
//	printf("test_glcd_redraw glcd_buf3= %x \n",glcd_buf[11+(50*3)]);
//	printf("test_glcd_redraw glcd_buf4= %x \n",glcd_buf[11+(50*4)]);
//	printf("test_glcd_redraw glcd_buf5= %x \n",glcd_buf[11+(50*5)]);
//	printf("test_glcd_redraw glcd_buf6= %x \n",glcd_buf[11+(50*6)]);
//	printf("test_glcd_redraw glcd_buf7= %x \n",glcd_buf[11+(50*7)]);
	CU_ASSERT(cursor_x == 12);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 12);

	glcd_PutCharA('G');
	CU_ASSERT(glcd_buf[12+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[12+(50*1)] == 0x1C^0xff);
	CU_ASSERT(glcd_buf[12+(50*2)] == 0x36^0xff);
	CU_ASSERT(glcd_buf[12+(50*3)] == 0x63^0xff);
	CU_ASSERT(glcd_buf[12+(50*4)] == 0x60^0xff);
	CU_ASSERT(glcd_buf[12+(50*5)] == 0xC0^0xff);
	CU_ASSERT(glcd_buf[12+(50*6)] == 0xC0^0xff);
	CU_ASSERT(glcd_buf[12+(50*7)] == 0xCF^0xff);
//	printf("test_glcd_redraw glcd_buf0= %x \n",glcd_buf[12+(50*0)]);
//	printf("test_glcd_redraw glcd_buf1= %x \n",glcd_buf[12+(50*1)]);
//	printf("test_glcd_redraw glcd_buf2= %x \n",glcd_buf[12+(50*2)]);
//	printf("test_glcd_redraw glcd_buf3= %x \n",glcd_buf[12+(50*3)]);
//	printf("test_glcd_redraw glcd_buf4= %x \n",glcd_buf[12+(50*4)]);
//	printf("test_glcd_redraw glcd_buf5= %x \n",glcd_buf[12+(50*5)]);
//	printf("test_glcd_redraw glcd_buf6= %x \n",glcd_buf[12+(50*6)]);
//	printf("test_glcd_redraw glcd_buf7= %x \n",glcd_buf[12+(50*7)]);
	CU_ASSERT(cursor_x == 13);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 13);

	glcd_PutCharA('E');
	CU_ASSERT(glcd_buf[13+(50*0)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[13+(50*1)] == 0xFF^0xff);
	CU_ASSERT(glcd_buf[13+(50*2)] == 0xC0^0xff);
	CU_ASSERT(glcd_buf[13+(50*3)] == 0xC0^0xff);
	CU_ASSERT(glcd_buf[13+(50*4)] == 0xC0^0xff);
	CU_ASSERT(glcd_buf[13+(50*5)] == 0xC0^0xff);
	CU_ASSERT(glcd_buf[13+(50*6)] == 0xC0^0xff);
	CU_ASSERT(glcd_buf[13+(50*7)] == 0xFE^0xff);
//	printf("test_glcd_redraw glcd_buf0= %x \n",glcd_buf[13+(50*0)]);
//	printf("test_glcd_redraw glcd_buf1= %x \n",glcd_buf[13+(50*1)]);
//	printf("test_glcd_redraw glcd_buf2= %x \n",glcd_buf[13+(50*2)]);
//	printf("test_glcd_redraw glcd_buf3= %x \n",glcd_buf[13+(50*3)]);
//	printf("test_glcd_redraw glcd_buf4= %x \n",glcd_buf[13+(50*4)]);
//	printf("test_glcd_redraw glcd_buf5= %x \n",glcd_buf[13+(50*5)]);
//	printf("test_glcd_redraw glcd_buf6= %x \n",glcd_buf[13+(50*6)]);
//	printf("test_glcd_redraw glcd_buf7= %x \n",glcd_buf[13+(50*7)]);
	CU_ASSERT(cursor_x == 14);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 14);
	printf("\n");
//	printf("test_glcd_PutCharA after X = %d Y = %d \n",cursor_x,cursor_y);
//	printf("test_glcd_PutCharA after old X = %d Y = %d \n",old_x,old_y);

	glcd_PutCharA('"');
	CU_ASSERT(glcd_buf[14+(50*0)] == 0x76^0xff);
	CU_ASSERT(glcd_buf[14+(50*1)] == 0x36^0xff);
	CU_ASSERT(glcd_buf[14+(50*2)] == 0x36^0xff);
	CU_ASSERT(glcd_buf[14+(50*3)] == 0x6C^0xff);
	CU_ASSERT(glcd_buf[14+(50*4)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[14+(50*5)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[14+(50*6)] == 0x00^0xff);
	CU_ASSERT(glcd_buf[14+(50*7)] == 0x00^0xff);
	CU_ASSERT(cursor_x == 15);
	CU_ASSERT(cursor_y == 0);
	CU_ASSERT(editor_line[0] == 15);
	glcd_makeimage("test_glcd_PutCharA_0.bmp");
	printf("test_glcd_PutCharA after X = %d Y = %d \n",cursor_x,cursor_y);
	printf("test_glcd_PutCharA after old X = %d Y = %d \n",old_x,old_y);

	glcd_PutCharA('\n');
	CU_ASSERT(cursor_x == 0);
	CU_ASSERT(cursor_y == 1);
	CU_ASSERT(editor_line[1] == 0);
	glcd_makeimage("test_glcd_PutCharA_1.bmp");
	printf("test_glcd_PutCharA after X = %d Y = %d \n",cursor_x,cursor_y);
	printf("test_glcd_PutCharA after old X = %d Y = %d \n",old_x,old_y);

	glcd_PutCharA('2');
	CU_ASSERT(cursor_x == 1);
	CU_ASSERT(cursor_y == 1);
	CU_ASSERT(editor_line[1] == 1);

	glcd_PutCharA('0');
	CU_ASSERT(cursor_x == 2);
	CU_ASSERT(cursor_y == 1);
	CU_ASSERT(editor_line[1] == 2);

	glcd_PutCharA(' ');
	CU_ASSERT(cursor_x == 3);
	CU_ASSERT(cursor_y == 1);
	CU_ASSERT(editor_line[1] == 3);

	glcd_PutCharA('E');
	CU_ASSERT(cursor_x == 4);
	CU_ASSERT(cursor_y == 1);
	CU_ASSERT(editor_line[1] == 4);

	glcd_PutCharA('N');
	CU_ASSERT(cursor_x == 5);
	CU_ASSERT(cursor_y == 1);
	CU_ASSERT(editor_line[1] == 5);

	glcd_PutCharA('D');
	CU_ASSERT(cursor_x == 6);
	CU_ASSERT(cursor_y == 1);
	CU_ASSERT(editor_line[1] == 6);

	glcd_PutCharA('\n');
	CU_ASSERT(cursor_x == 0);
	CU_ASSERT(cursor_y == 2);
	CU_ASSERT(editor_line[2] == 0);
	glcd_makeimage("test_glcd_PutCharA_2.bmp");
	
}

void test_glcd_PutsA(void)
{
	int i;
	for(i=0;i<100;i++)BASICBUF[i] = 0;
	for(i=0;i<10;i++)editor_line[i] = 0;
	end_ptr = 0;
	uint8_t testbuf[] = "10 PRINT \"HOGE\"\n20 END\n";
	glcd_BufClear(1);
	
	glcd_PutsA(testbuf);
	glcd_makeimage("test_glcd_PutsA_1.bmp");
//	CU_ASSERT(end_ptr == 22);
//	CU_ASSERT(BASICBUF[21] == '\n');
//	printf("test_glcd_PutEditor end_ptr = %d \n", end_ptr);
}

void test_glcd_list(void)
{
	int i,k,sflg,lflg,count;
	strcpy(BASICBUF,"10 let i\n20 let j\n30 let n\n40 let t\n50 for i = 0 to 239\n60 for j = 0 to 399\n70 if 120 > i then gosub 200 else gosub 300\n80 if 120 > j then gosub 400 else gosub 500\n90 if n * n + t * t > 9801 then gosub 600\n100 next j\n110 next i\n120 end\n200 n = 120 - i\n210 return\n300 n = i - 120\n310 return\n400 t = 120 - j\n410 return\n500 t = j  - 120\n510 return\n600 if n * n + t * t < 10404 then gosub 700\n610 return\n700 pset j i 1\n710 return\n720 end\n");
	strcpy(LINEBUF,"LIST\n");
//	for(i=0;i<MAX_BASICBUF;i++)RUNBUF[i] = 0;
	i = 0;
	sflg = 0;
	lflg = 0;
	count = 0;
	line_ptr = 0;
	line_end = 0;

//	char * prog = LINEBUF;
//	tokenizer_init(prog);

//	accept(TOKENIZER_LIST);
//	accept(TOKENIZER_CR);
#if 0
	while((BASICBUF[i] != 0)&&(i<MAX_BASICBUF)){
		if(i==0){
			if((BASICBUF[i] >= '0')&&(BASICBUF[i] <= '9')){
				sflg = 1;
			}else if(BASICBUF[i] == '\n'){
				lflg = 1;
			}
			if(sflg == 1){
				RUNBUF[count] = BASICBUF[i];
				count++;
			}
		}else{
			if(BASICBUF[i] == '\n'){
				lflg = 1;
			}
			if(lflg == 1){
				if((BASICBUF[i] >= '0')&&(BASICBUF[i] <= '9')){
					lflg = 0;
					sflg = 1;
				}else if(BASICBUF[i] == '\n'){
					lflg = 1;
					if(sflg == 1){
						RUNBUF[count] = BASICBUF[i];
						count++;
						sflg = 0;
					}
				}else{
					sflg = 0;
					lflg = 0;
				}
			}
			if(sflg == 1){
				RUNBUF[count] = BASICBUF[i];
				count++;
				if(BASICBUF[i] == '\n'){
					lflg = 1;
				}
			}
		}
		i++;
	}
#endif
	glcd_BufClear(1);
	glcd_posClear();
//	glcd_PutsD(RUNBUF);

//	glcd_PutsD(BASICBUF);
	while(BASICBUF[i]) {
		glcd_PutCharA(BASICBUF[i] & 0xff);
		count++;
//		printf(" %d",count);
		if(count == 262)glcd_makeimage("test_glcd_list_3.bmp");
		if(count == 263)glcd_makeimage("test_glcd_list_2.bmp");
		if(BASICBUF[i] == '\n'){
			for(k=0;k<50;k++)LINEBUF[k] = 0;
			line_ptr = 0;
			line_end = 0;
		}else{
			LINEBUF[line_ptr] =BASICBUF[i] & 0x0ff;
			line_ptr++;
			line_end++;
		}
		i++ ;
	}

//	printf(BASICBUF);
	glcd_makeimage("test_glcd_list_1.bmp");
}
//-----------------------------------------------------------------------------
int main() {
	CU_pSuite test_suite;
	int i;

	CU_initialize_registry();
	test_suite = CU_add_suite("GCLD_test", NULL, NULL);
//	CU_add_test(test_suite, "glcd_getCursorPos", test_glcd_getCursorPos);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_redraw", test_glcd_redraw);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_redraw2", test_glcd_redraw2);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_seteditor_line", test_glcd_seteditor_line);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_DrawCursor", test_glcd_DrawCursor);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_PutEditor", test_glcd_PutEditor);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_PutEditor2", test_glcd_PutEditor2);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_PutEditor3", test_glcd_PutEditor3);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_PutEditor4", test_glcd_PutEditor4);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_PutCharA", test_glcd_PutCharA);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_PutsA", test_glcd_PutsA);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_add_test(test_suite, "glcd_list", test_glcd_list);
	for (i = 0;i < (50*240);i++)display_buf[i] = 0xff;
	CU_console_run_tests();
	CU_cleanup_registry();
	return(0);
}
