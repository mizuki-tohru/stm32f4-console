/*
 * Copyright (c) 2006, Adam Dunkels
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#define DEBUG 0

#include "ubasic.h"
#include "tokenizer.h"
#include "stm32f4xx_dcmi.h"
#include "stm32f4xx.h"
#include "glcd.h"
#include "hw_config.h"
#include "editor.h"

#if TEST
#include "stm32f4xx_adc.h"
#endif

#include <stdio.h> /* printf() */
#include <stdlib.h> /* exit() */
#include <string.h>

FATFS *Bfs;  /* Pointer to file system object */
DIR  Bdir;
FIL BfileR;

static char * program_ptr;

#define MAX_GOSUB_STACK_DEPTH 10
static int gosub_stack[MAX_GOSUB_STACK_DEPTH];
static int gosub_stack_ptr;

struct for_state {
  int line_after_for;
  int for_variable;
  int to;
};

unsigned char bmpheader[] = {'B','M',0x18,0x2F,0x00,0x00,0x00,0x00,0x00,0x00,
	0x3E,0x00,0x00,0x00,0x28,0x00,0x00,0x00,0x90,0x01,0x00,0x00,0xF0,0x00,0x00,0x00,
	0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0xC0,0x0E,0x00,0x00,0xC0,0x0E,0x00,0x00,
	0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00};

#define MAX_FOR_STACK_DEPTH 4
static struct for_state for_stack[MAX_FOR_STACK_DEPTH];
static int for_stack_ptr;

int ended;

#define MAX_BASICBUF MAXBASIC
/*---------------------------------------------------------------------------*/

char upper(char c)
{
  if ((c >= 'a') && (c <= 'z'))
    return c - 'a' + 'A';
  return c;
}

/*---------------------------------------------------------------------------*/
int isEx(char *filename,char * ex)
{
  int p1,p2;
  p1 = 0;
  while ((filename[p1] != 0) && (filename[p1] != '.')) {
    p1++;
  }
  if (filename[p1] == 0)
    return 0;
  p1++;
  if (filename[p1] == 0)
    return 0;
  p2 = 0;
  while ((filename[p1] != 0) && (ex[p2] != 0)) {
    if (upper(filename[p1]) != upper(ex[p2]))
      return 0;
    p1++;
    p2++;
  }
  if ((filename[p1] == 0) && (ex[p2] == 0))
    return 1;
  return 0;
}

/*---------------------------------------------------------------------------*/
//void ubasic_init(const char *program)
void ubasic_init(char *program)
{
	program_ptr = program;
	for_stack_ptr = gosub_stack_ptr = 0;
	tokenizer_init(program_ptr);
	ended = 0;
	ErrLine = 0;
}
/*---------------------------------------------------------------------------*/
//static void accept(int token)
void accept(int token)
{
	if(token != tokenizer_token()) {
//		buf[0] = (token/10) + '0';
//		buf[1] = (token%10) + '0';
//		buf[2] = ' ';
//		buf[3] = (current_token/10) + '0';
//		buf[4] = (current_token%10) + '0';
//		buf[5] = '\n';
//		buf[6] = 0x00;
//		cputs_p(6,buf);
		tokenizer_error_print();
		ended = 1;
		glcd_DrawCursor();
	//	exit(1);
	}
	tokenizer_next();
}
/*---------------------------------------------------------------------------*/
//static void comment_accept(void)
void comment_accept(void)
{
	int op;
	op = tokenizer_token();
	while((TOKENIZER_CR != op)&&(TOKENIZER_ENDOFINPUT != op)){
		tokenizer_next();
		op = tokenizer_token();
	}
}
/*---------------------------------------------------------------------------*/
//static int varfactor(void)
int varfactor(void)
{
	long r;
	r = ubasic_get_variable(tokenizer_variable_num());
	accept(TOKENIZER_VARIABLE);
	return r;
}
/*---------------------------------------------------------------------------*/
//static int factor(void)
int factor(void)
{
	int r;
	switch(tokenizer_token()) {
		case TOKENIZER_NUMBER:
			r = tokenizer_num();
			accept(TOKENIZER_NUMBER);
			break;
		case TOKENIZER_LEFTPAREN:
			accept(TOKENIZER_LEFTPAREN);
			r = expr();
			accept(TOKENIZER_RIGHTPAREN);
			break;
		default:
			r = varfactor();
			break;
	}
	return r;
}
/*---------------------------------------------------------------------------*/
//static int term(void)
int term(void)
{
	int f1, f2;
	int op;
	f1 = factor();
	op = tokenizer_token();
	while(op == TOKENIZER_ASTR ||
		  op == TOKENIZER_SLASH ||
		  op == TOKENIZER_MOD) {
		tokenizer_next();
		f2 = factor();
		switch(op) {
			case TOKENIZER_ASTR:
				f1 = f1 * f2;
				break;
			case TOKENIZER_SLASH:
				f1 = f1 / f2;
				break;
			case TOKENIZER_MOD:
				f1 = f1 % f2;
				break;
		}
		op = tokenizer_token();
	}
	return f1;
}
/*---------------------------------------------------------------------------*/
//static int expr(void)
int expr(void)
{
	int t1, t2;
	int op;
	t1 = term();
	op = tokenizer_token();
	while(op == TOKENIZER_PLUS ||
		  op == TOKENIZER_MINUS ||
		  op == TOKENIZER_AND ||
		  op == TOKENIZER_OR) {
		tokenizer_next();
		t2 = term();
		switch(op) {
			case TOKENIZER_PLUS:
				t1 = t1 + t2;
				break;
			case TOKENIZER_MINUS:
				t1 = t1 - t2;
				break;
			case TOKENIZER_AND:
				t1 = t1 & t2;
				break;
			case TOKENIZER_OR:
				t1 = t1 | t2;
				break;
		}
		op = tokenizer_token();
	}
	return t1;
}
/*---------------------------------------------------------------------------*/
//static int relation(void)
int relation(void)
{
	int r1, r2;
	int op;
	r1 = expr();
	op = tokenizer_token();
	while(op == TOKENIZER_LT ||
		  op == TOKENIZER_GT ||
		  op == TOKENIZER_EQ) {
		tokenizer_next();
		r2 = expr();
		switch(op) {
			case TOKENIZER_LT:
				r1 = r1 < r2;
				break;
			case TOKENIZER_GT:
				r1 = r1 > r2;
				break;
			case TOKENIZER_EQ:
				r1 = r1 == r2;
				break;
		}
		op = tokenizer_token();
	}
	return r1;
}
/*---------------------------------------------------------------------------*/
//static void jump_linenum(int linenum)
void jump_linenum(int linenum)
{
	ptr = BASICBUF;
	nextptr = ptr;

	tokenizer_init(ptr);
	while(tokenizer_num() != linenum) {
		do {
			do {
				tokenizer_next();
			} while(tokenizer_token() != TOKENIZER_CR &&
					tokenizer_token() != TOKENIZER_ENDOFINPUT);
			if(tokenizer_token() == TOKENIZER_CR) {
				tokenizer_next();
			}
			if(tokenizer_token() == TOKENIZER_ENDOFINPUT){
				ended = 1;
				return;
			}
		} while(tokenizer_token() != TOKENIZER_NUMBER);
	}
}
/*---------------------------------------------------------------------------*/
//static void goto_statement(void)
void goto_statement(void)
{
	accept(TOKENIZER_GOTO);
	jump_linenum(tokenizer_num());
}
/*---------------------------------------------------------------------------*/
//static void print_statement(void)
void print_statement(void)
{
	char buf[20];
	accept(TOKENIZER_PRINT);
	do {
		if(tokenizer_token() == TOKENIZER_STRING) {
			tokenizer_string(strings[MAX_STRINGNUM], sizeof(strings[MAX_STRINGNUM]));
			glcd_PutsA(strings[MAX_STRINGNUM]);
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_COMMA) {
			glcd_PutsA(" ");
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_SEMICOLON) {
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_VARIABLE ||
				  tokenizer_token() == TOKENIZER_NUMBER) {
			glcd_PutsA(ltodeci(expr(),buf,6));
		} else {
			break;
		}
	} while(tokenizer_token() != TOKENIZER_CR &&
			tokenizer_token() != TOKENIZER_ENDOFINPUT);
	glcd_PutsA("\n");
//	cputs_p(6,"Print");
	tokenizer_next();
}
/*---------------------------------------------------------------------------*/
//static void if_statement(void)
void if_statement(void)
{
	int r;
	accept(TOKENIZER_IF);
	r = relation();
	accept(TOKENIZER_THEN);
	if(r) {
		statement();
	} else {
		do {
			tokenizer_next();
		} while(tokenizer_token() != TOKENIZER_ELSE &&
		tokenizer_token() != TOKENIZER_CR &&
		tokenizer_token() != TOKENIZER_ENDOFINPUT);
		if(tokenizer_token() == TOKENIZER_ELSE) {
			tokenizer_next();
			statement();
		} else if(tokenizer_token() == TOKENIZER_CR) {
			tokenizer_next();
		}
	}
}
/*---------------------------------------------------------------------------*/
//static void let_statement(void)
void let_statement(void)
{
	long var;
	long ix;
	ix = 0;
	var = tokenizer_variable_num();
	if((var >= MAX_VARNUM)&&(var <(MAX_VARNUM+MAX_ARRYNUM))){
		accept(TOKENIZER_VARIABLE);
		ix = tokenizer_num();
		accept(TOKENIZER_ARRAY);
	}else{
		accept(TOKENIZER_VARIABLE);
	}
	if(ptr[0] == '='){
		accept(TOKENIZER_EQ);
		if(var < MAX_VARNUM){
			ubasic_set_variable(var, expr());
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
}
/*---------------------------------------------------------------------------*/
//static void gosub_statement(void)
void gosub_statement(void)
{
	int linenum,nextnum;
	char *tmpptr, *tmpnextptr;
	
	accept(TOKENIZER_GOSUB);
	linenum = tokenizer_num();
	accept(TOKENIZER_NUMBER);
	tmpptr = ptr;
	tmpnextptr = nextptr;
	while(tokenizer_token() != TOKENIZER_CR &&
		tokenizer_token() != TOKENIZER_ENDOFINPUT){
		tokenizer_next();
	}
	accept(TOKENIZER_CR);
	nextnum = tokenizer_num();
	ptr = tmpptr;
	nextptr = tmpnextptr;
	if(gosub_stack_ptr < MAX_GOSUB_STACK_DEPTH) {
		gosub_stack[gosub_stack_ptr] = nextnum;
		gosub_stack_ptr++;
		jump_linenum(linenum);
	}else{
	}
}
/*---------------------------------------------------------------------------*/
//static void return_statement(void)
void return_statement(void)
{
	accept(TOKENIZER_RETURN);
	if(gosub_stack_ptr > 0) {
		gosub_stack_ptr--;
		jump_linenum(gosub_stack[gosub_stack_ptr]);
	}else{
	}
}
/*---------------------------------------------------------------------------*/
//static void next_statement(void)
void next_statement(void)
{
	long var;
	accept(TOKENIZER_NEXT);
	var = tokenizer_variable_num();
	accept(TOKENIZER_VARIABLE);
	if(for_stack_ptr > 0 && var == for_stack[for_stack_ptr - 1].for_variable) {
		ubasic_set_variable(var, ubasic_get_variable(var) + 1);
		if(ubasic_get_variable(var) <= for_stack[for_stack_ptr - 1].to) {
			jump_linenum(for_stack[for_stack_ptr - 1].line_after_for);
		}else{
			for_stack_ptr--;
			accept(TOKENIZER_CR);
		}
	}else{
		accept(TOKENIZER_CR);
	}
}
/*---------------------------------------------------------------------------*/
//static void for_statement(void)
void for_statement(void)
{
	long for_variable, to;
	accept(TOKENIZER_FOR);
	for_variable = tokenizer_variable_num();
	accept(TOKENIZER_VARIABLE);
	accept(TOKENIZER_EQ);
	ubasic_set_variable(for_variable, expr());
	accept(TOKENIZER_TO);
	to = expr();
	accept(TOKENIZER_CR);

	if(for_stack_ptr < MAX_FOR_STACK_DEPTH) {
		for_stack[for_stack_ptr].line_after_for = tokenizer_num();
		for_stack[for_stack_ptr].for_variable = for_variable;
		for_stack[for_stack_ptr].to = to;
		for_stack_ptr++;
	}else{
	}
}
/*---------------------------------------------------------------------------*/
//static void pset_statement(void)
void pset_statement(void)
{
	uint8_t pset_x,pset_y,pset_c;
	accept(TOKENIZER_PSET);
	pset_x = tokenizer_num_or_variable_num();
	if(current_token == TOKENIZER_VARIABLE){
		accept(TOKENIZER_VARIABLE);
	}else{
		accept(TOKENIZER_NUMBER);
	}
	pset_y = tokenizer_num_or_variable_num();
	if(current_token == TOKENIZER_VARIABLE){
		accept(TOKENIZER_VARIABLE);
	}else{
		accept(TOKENIZER_NUMBER);
	}
	pset_c = tokenizer_num_or_variable_num();
	if(current_token == TOKENIZER_VARIABLE){
		accept(TOKENIZER_VARIABLE);
	}else{
		accept(TOKENIZER_NUMBER);
	}
	accept(TOKENIZER_CR);
	if((pset_x >= 0)&&(pset_x<400)){
		if((pset_y >= 0)&&(pset_y<240)){
			glcd_SetPixel(pset_x, pset_y, pset_c);
		}
	}
}
/*---------------------------------------------------------------------------*/
//static void end_statement(void)
void end_statement(void)
{
	accept(TOKENIZER_END);
	ended = 1;
	glcd_DrawCursor();
}
/*---------------------------------------------------------------------------*/
//static void cls_statement(void)
void cls_statement(void)
{
	int i;
	for(i=0;i<50;i++)LINEBUF[i] = 0;
	accept(TOKENIZER_CLS);
	accept(TOKENIZER_CR);
	glcd_BufClear(1);
	glcd_posClear();
}
/*---------------------------------------------------------------------------*/
void refresh_statement(void)
{
	accept(TOKENIZER_REFRESH);
	accept(TOKENIZER_CR);
	glcd_TransFromBuf();
}
/*---------------------------------------------------------------------------*/
//static void rem_statement(void)
void rem_statement(void)
{
	accept(TOKENIZER_REM);
	comment_accept();
	accept(TOKENIZER_CR);
}
/*---------------------------------------------------------------------------*/
//static void list_statement(void)
void list_statement(void)
{
	accept(TOKENIZER_LIST);
	accept(TOKENIZER_CR);

	glcd_BufClear(1);
	glcd_posClear();
	glcd_PutsD(BASICBUF);
}
/*---------------------------------------------------------------------------*/
//static void load_statement(void)
void load_statement(void)
{
	BYTE res;
	long p2;
	FILINFO Finfo;
	char filename[20];
	UINT BytesRead;
	char tmp[62];
	int i;

	accept(TOKENIZER_LOAD);
	tokenizer_string(filename, 20);
	accept(TOKENIZER_STRING);
	accept(TOKENIZER_CR);
	res = f_getfree("", (DWORD*)&p2, &Bfs);// testcode
	res = f_opendir(& Bdir, "");
	if (res) {
		glcd_PutsD("Error: NO SD Card!!\n");
	} else {
		for(;;) {
			res = f_readdir(& Bdir, &Finfo);
			if ((res != FR_OK) || !Finfo.fname[0]){
				glcd_PutsD("Error: File not Found\n");
				break;
			}
			if(strncmp(&(Finfo.fname[0]), filename, strlen(filename)) == 0) {
				if (isEx(&(Finfo.fname[0]),"BAS")){
					for(i=0;i<MAX_BASICBUF;i++)BASICBUF[i] = 0;
					if (f_open(&BfileR, filename , FA_READ) == FR_OK) {
						f_read (&BfileR, BASICBUF, 256, &BytesRead);
						f_close(&BfileR);
					}else{
						glcd_PutsD("Error: Can not Read file!!\n");
					}
					ErrLine = 0;
					break;
				}
				if (isEx(&(Finfo.fname[0]),"TXT")){
					for(i=0;i<MAX_STRINGLEN;i++)strings[3][i] = 0;
					if (f_open(&BfileR, filename , FA_READ) == FR_OK) {
						f_read (&BfileR, strings[3], 4096, &BytesRead);
						f_close(&BfileR);
					}else{
						glcd_PutsD("Error: Can not Read file!!\n");
					}
					break;
				}
				if (isEx(&(Finfo.fname[0]),"DAT")){
					for(i=0;i<MAX_ARRYLEN;i++)arrays[3][i] = 0;
					if (f_open(&BfileR, filename , FA_READ) == FR_OK) {
						f_read (&BfileR, arrays[3], 4096, &BytesRead);
						f_close(&BfileR);
					}else{
						glcd_PutsD("Error: Can not Read file!!\n");
					}
					break;
				}
				if (isEx(&(Finfo.fname[0]),"BMP")){
					for (i = 0;i < (50*240);i++)glcd_buf[i] = 0x00;
					for(i=0;i<31;i++)glcd_check[i] = 0xff;
					if (f_open(&BfileR, filename , FA_READ) == FR_OK) {
						f_read (&BfileR, tmp, 62, &BytesRead);
						for(i=0;i<240;i++){
							f_read (&BfileR, glcd_buf+(50*(240-i)), 50, &BytesRead);
							f_read (&BfileR, tmp, 2, &BytesRead);
						}
						f_close(&BfileR);
						glcd_posClear();
						glcd_TransFromBuf();
					}else{
						glcd_PutsD("Error: Can not Read file!!\n");
					}
					break;
				}
			}
		}
	}
}
/*---------------------------------------------------------------------------*/
//static void save_statement(void)
void save_statement(void)
{
	BYTE res;
	long p2;
	FILINFO Finfo;
	char filename[20];
	unsigned char tmp[2];
	UINT BytesRead;
	UINT BytesWrite;
	int i;
	tmp[0] = 0x00;
	tmp[1] = 0x01;

	accept(TOKENIZER_LOAD);
	tokenizer_string(filename, 20);
	accept(TOKENIZER_STRING);
	accept(TOKENIZER_CR);
	res = f_getfree("", (DWORD*)&p2, &Bfs);// testcode
	res = f_opendir(& Bdir, "");
	if (res) {
		glcd_PutsD("Error: NO SD Card!!\n");
	} else {
		for(;;) {
			res = f_readdir(& Bdir, &Finfo);
			if ((res != FR_OK) || !Finfo.fname[0]){
				glcd_PutsD("Error: File not Found\n");
				break;
			}
			if(strlen(filename)!=0){
				if (isEx(filename,"BAS")){
					if(f_open(&BfileR,filename,FA_CREATE_ALWAYS | FA_WRITE) == FR_OK){
						f_write(&BfileR,BASICBUF,strlen(BASICBUF),&BytesWrite);
						f_close(&BfileR);
					}else{
						glcd_PutsD("Error: Can not Write file!!\n");
					}
					break;
				}
				if (isEx(filename,"TXT")){
					if(f_open(&BfileR,filename,FA_CREATE_ALWAYS | FA_WRITE) == FR_OK){
						f_write(&BfileR,strings[3],strlen(strings[3]),&BytesWrite);
						f_close(&BfileR);
					}else{
						glcd_PutsD("Error: Can not Write file!!\n");
					}
					break;
				}
				if (isEx(filename,"DAT")){
					if(f_open(&BfileR,filename,FA_CREATE_ALWAYS | FA_WRITE) == FR_OK){
						f_write(&BfileR,arrays[3],4096,&BytesWrite);
						f_close(&BfileR);
					}else{
						glcd_PutsD("Error: Can not Write file!!\n");
					}
					break;
				}
				if (isEx(filename,"BMP")){
					if(f_open(&BfileR,filename,FA_CREATE_ALWAYS | FA_WRITE) == FR_OK){
						f_write(&BfileR, bmpheader, 62, &BytesWrite);
						for(i=0;i<240;i++){
							f_write(&BfileR, glcd_buf+(50*(240-i)), 50, &BytesWrite);
							f_write(&BfileR, tmp, 2, &BytesWrite);
						}
						f_write(&BfileR,arrays[3],4096,&BytesWrite);
						f_close(&BfileR);
					}else{
						glcd_PutsD("Error: Can not Write file!!\n");
					}
					break;
				}
			}
		}
	}
}

/*---------------------------------------------------------------------------*/
//static void files_statement(void)
void files_statement(void)
{
	int res;
	long p2;
	accept(TOKENIZER_FILES);
	accept(TOKENIZER_CR);

//	cputs_p(6,"FILES:");
	
	if (lsSDA() == FR_OK) {
//		res = f_getfree("", (DWORD*)&p2, &Bfs);// testcode
//		res = f_opendir(& Bdir, "");
		res = 0;
	}else{
		res = 1;
	}
}
/*---------------------------------------------------------------------------*/
//static void peek_statement(void)
void peek_statement(void)
{
	long var;
	long addr;
	var = tokenizer_variable_num();
	accept(TOKENIZER_VARIABLE);
	accept(TOKENIZER_EQ);
	accept(TOKENIZER_PEEK);
	addr = tokenizer_bignum_or_variable_bignum();
	if(current_token == TOKENIZER_VARIABLE){
		accept(TOKENIZER_VARIABLE);
	}else{
		accept(TOKENIZER_NUMBER);
	}
#if TEST
#else
	ubasic_set_variable(var, *((uint32_t *)addr));
#endif
	accept(TOKENIZER_CR);
}
/*---------------------------------------------------------------------------*/
//static void poke_statement(void)
void poke_statement(void)
{
	int data;
	long addr;
	accept(TOKENIZER_POKE);
	addr = tokenizer_bignum_or_variable_bignum();
	if(current_token == TOKENIZER_VARIABLE){
		accept(TOKENIZER_VARIABLE);
	}else{
		accept(TOKENIZER_NUMBER);
	}
	data = tokenizer_num_or_variable_num();
	if(current_token == TOKENIZER_VARIABLE){
		accept(TOKENIZER_VARIABLE);
	}else{
		accept(TOKENIZER_NUMBER);
	}
#if TEST
#else
	*((uint32_t *)(addr)) = (uint32_t)data;
#endif
	accept(TOKENIZER_CR);
}
/*---------------------------------------------------------------------------*/
//static void	wait_statement(void)
void wait_statement(void)
{
	int wait;

	accept(TOKENIZER_POKE);
	wait = tokenizer_num_or_variable_num();
	accept(TOKENIZER_VARIABLE);
	_delay_ms(wait);
	accept(TOKENIZER_CR);
}
/*---------------------------------------------------------------------------*/
//static void input_statement(void)
void input_statement(void)
{
	long var;
	char c;
	accept(TOKENIZER_INPUT);
	var = tokenizer_variable_num();
	accept(TOKENIZER_VARIABLE);
	ubasic_clear_strig(var);
	while(c != '\n'){
		if((c = kgetc()) != 0){
			ubasic_set_string(var, c);
		}
	}
	accept(TOKENIZER_CR);
}
/*---------------------------------------------------------------------------*/
//static void inp_statement(void)
void inp_statement(void)
{
	long var;
	char c;
	int flg = 0;
	accept(TOKENIZER_INP);
	var = tokenizer_variable_num();
	accept(TOKENIZER_VARIABLE);
	while(flg == 0){
		if((c = kgetc()) != 0){
			ubasic_set_variable(var, c);
			flg = 1;
		}
	}
	accept(TOKENIZER_CR);
}
/*---------------------------------------------------------------------------*/
//static void inr_statement(void)
void inr_statement(void)
{
	long var;
	accept(TOKENIZER_INR);
	var = tokenizer_variable_num();
	accept(TOKENIZER_VARIABLE);
	ubasic_set_variable(var, kgetc());
	accept(TOKENIZER_CR);

}
/*---------------------------------------------------------------------------*/
//static void ina_statement(void)
void ina_statement(void)
{
	long var;
	int ch;
	var = tokenizer_variable_num();
	accept(TOKENIZER_VARIABLE);
	accept(TOKENIZER_EQ);
	accept(TOKENIZER_INA);
	ch = tokenizer_num_or_variable_num();
	accept(TOKENIZER_VARIABLE);
	ubasic_set_variable(var, ubasic_get_adc(ch));
	accept(TOKENIZER_CR);
}
/*---------------------------------------------------------------------------*/
//static void run_statement(void)
void run_statement(void)
{
	accept(TOKENIZER_RUN);
	accept(TOKENIZER_CR);
	program_ptr = BASICBUF;
	for_stack_ptr = gosub_stack_ptr = 0;
	tokenizer_init(program_ptr);
	ended = 0;
}
/*---------------------------------------------------------------------------*/
//static void statement(void)
void statement(void)
{
	int token;
	token = tokenizer_token();

	switch(token) {
		case TOKENIZER_PRINT:
			print_statement();
			break;
		case TOKENIZER_IF:
			if_statement();
			break;
		case TOKENIZER_GOTO:
			goto_statement();
			break;
		case TOKENIZER_GOSUB:
			gosub_statement();
			break;
		case TOKENIZER_RETURN:
			return_statement();
			break;
		case TOKENIZER_FOR:
			for_statement();
			break;
		case TOKENIZER_NEXT:
			next_statement();
			break;
		case TOKENIZER_END:
			end_statement();
			break;
		case TOKENIZER_PSET:
			pset_statement();
			break;
		case TOKENIZER_CLS:
			cls_statement();
			break;
		case TOKENIZER_REFRESH:
			refresh_statement();
			break;
		case TOKENIZER_LIST:
			list_statement();
			break;
		case TOKENIZER_LOAD:
			load_statement();
			break;
		case TOKENIZER_SAVE:
			save_statement();
			break;
		case TOKENIZER_FILES:
			files_statement();
			break;
		case TOKENIZER_PEEK:
			peek_statement();
			break;
		case TOKENIZER_POKE:
			poke_statement();
			break;
		case TOKENIZER_WAIT:
			wait_statement();
			break;
		case TOKENIZER_INPUT:
			input_statement();
			break;
		case TOKENIZER_INP:
			inp_statement();
			break;
		case TOKENIZER_INR:
			inr_statement();
			break;
		case TOKENIZER_INA:
			ina_statement();
			break;
		case TOKENIZER_REM:
			rem_statement();
			break;
		case TOKENIZER_RUN:
			run_statement();
			break;
		case TOKENIZER_ERROR:
			tokenizer_error_print();
			ended = 1;
			glcd_DrawCursor();
			break;
		case TOKENIZER_LET:
			accept(TOKENIZER_LET);
			accept(TOKENIZER_VARIABLE);
			accept(TOKENIZER_CR);
			break;
			/* Fall through. */
		case TOKENIZER_VARIABLE:
			let_statement();
			break;
		default:
			exit(1);
	}
}
/*---------------------------------------------------------------------------*/
//static void line_statement(void)
void line_statement(void)
{
	ErrLine = tokenizer_num();
	/*    current_linenum = tokenizer_num();*/
	accept(TOKENIZER_NUMBER);
	statement();
	return;
}
/*---------------------------------------------------------------------------*/
void ubasic_run(void)
{
	if(tokenizer_finished()) {
		ended = 1;
		glcd_TransFromBuf();
		return;
	}
	line_statement();
}
/*---------------------------------------------------------------------------*/
int ubasic_finished(void)
{
	return ended || tokenizer_finished();
}
/*---------------------------------------------------------------------------*/
void ubasic_set_variable(int varnum, long value)
{
	int i;
	if(varnum >= 0 && varnum < MAX_VARNUM) {
		variables[varnum] = value;
	}else if((varnum >= MAX_VARNUM) && (varnum < (MAX_VARNUM+MAX_ARRYNUM))) {
		if(arrays[(varnum-MAX_VARNUM)][0] < MAX_ARRYLEN){
			arrays[(varnum-MAX_VARNUM)][(arrays[(varnum-MAX_VARNUM)][0])+1] = value;
			arrays[(varnum-MAX_VARNUM)][0]++;
		}
	}else if(((varnum >= (MAX_VARNUM+MAX_ARRYNUM)))
			&& (varnum < (MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM))) {
		i = strlen(strings[(varnum-MAX_VARNUM-MAX_ARRYNUM)]);
		if(i < (MAX_STRINGLEN-1)){
			strings[(varnum-MAX_VARNUM-MAX_ARRYNUM)][i] = value;
			strings[(varnum-MAX_VARNUM-MAX_ARRYNUM)][i+1] = 0;
		}
	}else if(((varnum >= MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM))
			&& (varnum < (MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM+MAX_BIGVARNUM))) {
		variables_big[(varnum-MAX_VARNUM-MAX_ARRYNUM-MAX_STRINGNUM)] = value;
	}
}
/*---------------------------------------------------------------------------*/
void ubasic_set_variable_big(int varnum, long value)
{
	if(((varnum >= MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM))
			&& (varnum < (MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM+MAX_BIGVARNUM))) {
		variables_big[(varnum-MAX_VARNUM-MAX_ARRYNUM-MAX_STRINGNUM)] = value;
	}
}
/*---------------------------------------------------------------------------*/
void ubasic_set_array(int varnum, int i, int value)
{
	if(i < MAX_ARRYLEN){
		if((varnum >= MAX_VARNUM)&&(varnum < (MAX_VARNUM+MAX_ARRYNUM))){
			arrays[(varnum-MAX_VARNUM)][i] = value;
		}
	}
}
/*---------------------------------------------------------------------------*/
void ubasic_clear_array(int varnum)
{
	int i;
	if((varnum >= MAX_VARNUM)&&(varnum < (MAX_VARNUM+MAX_ARRYNUM))){
		for(i=0;i<MAX_ARRYLEN;i++)arrays[(varnum-MAX_VARNUM)][i] = 0;
	}
}
/*---------------------------------------------------------------------------*/
void ubasic_set_string(int varnum, char ch)
{
	int i;
	i = (int)(strlen(strings[(varnum-MAX_VARNUM-MAX_ARRYNUM)]));
	if((varnum >= (MAX_VARNUM+MAX_ARRYNUM))
		&&(varnum < (MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM))){
		if(i < (MAX_STRINGLEN-1)){
			strings[(varnum-MAX_VARNUM-MAX_ARRYNUM)][i] = ch;
			strings[(varnum-MAX_VARNUM-MAX_ARRYNUM)][i+1] = 0;
		}
	}
}
/*---------------------------------------------------------------------------*/
void ubasic_clear_strig(int varnum)
{
	int i;
	if((varnum >= (MAX_VARNUM+MAX_ARRYNUM))
		&&(varnum < (MAX_VARNUM+MAX_ARRYNUM+MAX_STRINGNUM))){
		for(i=0;i<MAX_STRINGLEN;i++)strings[(varnum-MAX_VARNUM-MAX_ARRYNUM)][i] = 0;
	}
}
/*---------------------------------------------------------------------------*/
int ubasic_get_variable(int varnum)
{
	if(varnum >= 0 && varnum < MAX_VARNUM) {
		return variables[varnum];
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int ubasic_get_adc(int ch)
{
	int var = 0xff;
	switch(ch){
		case 1:
			if (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) {
				var = 0xff;
			} else {
				var = ADC_GetConversionValue(ADC1) & 0x00ff;
				ADC_SoftwareStartConv(ADC1);
			}
			break;
		case 2:
			if (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET) {
				var = 0xff;
			} else {
				var = ADC_GetConversionValue(ADC2) & 0x00ff;
				ADC_SoftwareStartConv(ADC2);
			}
			break;
		case 3:
			if (ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET) {
				var = 0xff;
			} else {
				var = ADC_GetConversionValue(ADC3) & 0x00ff;
				ADC_SoftwareStartConv(ADC3);
			}
			break;
		default:
			var = 0xff;
			break;
	}
	return var;
}
/*---------------------------------------------------------------------------*/
