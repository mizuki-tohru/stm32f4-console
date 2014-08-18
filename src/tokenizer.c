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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "stm32f4xx_dcmi.h"
#include "stm32f4xx.h"
#include "tokenizer.h"
#include "glcd.h"
#include "editor.h"

char *ptr, *nextptr;

#define MAX_NUMLEN 6

int variables[MAX_VARNUM];
int arrays[MAX_ARRYNUM][MAX_ARRYLEN];
char strings[MAX_STRINGNUM+1][MAX_STRINGLEN];
long variables_big[MAX_BIGVARNUM];

struct keyword_token {
  char *keyword;
  int token;
};

int current_token = TOKENIZER_ERROR;
int ErrLine;

static const struct keyword_token keywords[] = {
  {"let", TOKENIZER_LET},
  {"LET", TOKENIZER_LET},
  {"print", TOKENIZER_PRINT},
  {"PRINT", TOKENIZER_PRINT},
  {"if", TOKENIZER_IF},
  {"IF", TOKENIZER_IF},
  {"then", TOKENIZER_THEN},
  {"THEN", TOKENIZER_THEN},
  {"else", TOKENIZER_ELSE},
  {"ELSE", TOKENIZER_ELSE},
  {"for", TOKENIZER_FOR},
  {"FOR", TOKENIZER_FOR},
  {"to", TOKENIZER_TO},
  {"TO", TOKENIZER_TO},
  {"next", TOKENIZER_NEXT},
  {"NEXT", TOKENIZER_NEXT},
  {"goto", TOKENIZER_GOTO},
  {"GOTO", TOKENIZER_GOTO},
  {"gosub", TOKENIZER_GOSUB},
  {"GOSUB", TOKENIZER_GOSUB},
  {"return", TOKENIZER_RETURN},
  {"RETURN", TOKENIZER_RETURN},
  {"call", TOKENIZER_CALL},
  {"CALL", TOKENIZER_CALL},
  {"end", TOKENIZER_END},
  {"END", TOKENIZER_END},
  {"pset", TOKENIZER_PSET},
  {"PSET", TOKENIZER_PSET},
  {"cls", TOKENIZER_CLS},
  {"CLS", TOKENIZER_CLS},
  {"list", TOKENIZER_LIST},
  {"LIST", TOKENIZER_LIST},
  {"load", TOKENIZER_LOAD},
  {"LOAD", TOKENIZER_LOAD},
  {"save", TOKENIZER_SAVE},
  {"SAVE", TOKENIZER_SAVE},
  {"files", TOKENIZER_FILES},
  {"FILES", TOKENIZER_FILES},
  {"peek", TOKENIZER_PEEK},
  {"PEEK", TOKENIZER_PEEK},
  {"poke", TOKENIZER_POKE},
  {"POKE", TOKENIZER_POKE},
  {"wait", TOKENIZER_WAIT},
  {"WAIT", TOKENIZER_WAIT},
  {"input", TOKENIZER_INPUT},
  {"INPUT", TOKENIZER_INPUT},
  {"inp", TOKENIZER_INP},
  {"INP", TOKENIZER_INP},
  {"inr", TOKENIZER_INR},
  {"INR", TOKENIZER_INR},
  {"ina", TOKENIZER_INA},
  {"INA", TOKENIZER_INA},
  {"rem", TOKENIZER_REM},
  {"REM", TOKENIZER_REM},
  {"run", TOKENIZER_RUN},
  {"RUN", TOKENIZER_RUN},
  {"refresh", TOKENIZER_REFRESH},
  {"REFRESH", TOKENIZER_REFRESH},
  {NULL, TOKENIZER_ERROR}
};

/*---------------------------------------------------------------------------*/
//static int singlechar(void)
int singlechar(void)
{
	if(*ptr == '\n') {
		return TOKENIZER_CR;
	} else if(*ptr == '\r'){
		ptr++;
		nextptr++;
  		return TOKENIZER_CR;
	} else if(*ptr == ',') {
		return TOKENIZER_COMMA;
	} else if(*ptr == ';') {
		return TOKENIZER_SEMICOLON;
	} else if(*ptr == '+') {
		return TOKENIZER_PLUS;
	} else if(*ptr == '-') {
		return TOKENIZER_MINUS;
	} else if(*ptr == '&') {
		return TOKENIZER_AND;
	} else if(*ptr == '|') {
		return TOKENIZER_OR;
	} else if(*ptr == '*') {
		return TOKENIZER_ASTR;
	} else if(*ptr == '/') {
		return TOKENIZER_SLASH;
	} else if(*ptr == '%') {
		return TOKENIZER_MOD;
	} else if(*ptr == '(') {
		return TOKENIZER_LEFTPAREN;
	} else if(*ptr == ')') {
		return TOKENIZER_RIGHTPAREN;
	} else if(*ptr == '<') {
		return TOKENIZER_LT;
	} else if(*ptr == '>') {
		return TOKENIZER_GT;
	} else if(*ptr == '=') {
		return TOKENIZER_EQ;
	} else if(*ptr == 0x00) {
		return TOKENIZER_END;
	} else if(ptr == NULL) {
		return TOKENIZER_END;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
//static int get_next_token(void)
int get_next_token(void)
{
	struct keyword_token const *kt;
	int i;

	if(*ptr == 0) {
		return TOKENIZER_ENDOFINPUT;
	}
	if(isdigit((int)*ptr)) {
		for(i = 0; i < MAX_NUMLEN; ++i) {
			if(!isdigit((int)(ptr[i]))) {
				if(i > 0) {
					nextptr = ptr + i;
					return TOKENIZER_NUMBER;
				} else {
					glcd_PutsA("get_next_token: error due to too short number\n");
					glcd_TransFromBuf();
					return TOKENIZER_ERROR;
				}
			}
			if(!isdigit((int)(ptr[i]))) {
				glcd_PutsA("get_next_token: error due to malformed number\n");
				glcd_TransFromBuf();
				return TOKENIZER_ERROR;
			}
		}
		glcd_PutsA("get_next_token: error due to too long number\n");
		glcd_TransFromBuf();

#if TEST
		glcd_PutsA(ptr);
		exit(1);
#else
		return TOKENIZER_ERROR;
#endif
	} else if(singlechar()) {
		nextptr = ptr + 1;
		return singlechar();
	} else if(*ptr == '"') {
		nextptr = ptr;
		do {
			++nextptr;
		} while(*nextptr != '"');
		++nextptr;
		return TOKENIZER_STRING;
	} else {
		for(kt = keywords; kt->keyword != NULL; ++kt) {
			if(strncmp(ptr, kt->keyword, strlen(kt->keyword)) == 0) {
				nextptr = ptr + strlen(kt->keyword);
				return kt->token;
			}
		}
	}
	if(*ptr >= 'a' && *ptr <= 'z') {
		nextptr = ptr + 1;
		return TOKENIZER_VARIABLE;
	}
	if(*ptr >= 'A' && *ptr <= 'D') {
		if(*(ptr+1) == '['){
			nextptr = ptr + 2;
			return TOKENIZER_VARIABLE;
		}
	}
	if(*ptr == ']') {
		nextptr = ptr + 1;
		return TOKENIZER_ARRAY;
	}
	if(*ptr >= 'E' && *ptr <= 'H') {
		nextptr = ptr + 1;
		return TOKENIZER_VARIABLE;
	}
	if(*ptr >= 'I' && *ptr <= 'N') {
		nextptr = ptr + 1;
		return TOKENIZER_VARIABLE;
	}
	if(*ptr == 0x00){
		return TOKENIZER_END;
	}
	if(ptr == NULL){
		return TOKENIZER_END;
	}
//	cputs_p(6,ptr);
	return TOKENIZER_ERROR;
}
/*---------------------------------------------------------------------------*/
//void tokenizer_init(const char *program)
void tokenizer_init(char * program)
{
	ptr = program;
	current_token = get_next_token();
}
/*---------------------------------------------------------------------------*/
int tokenizer_token(void)
{
	return current_token;
}
/*---------------------------------------------------------------------------*/
char * tokenizer_nextptr(void)
{
	return nextptr;
}
/*---------------------------------------------------------------------------*/
void tokenizer_next(void)
{
	if(tokenizer_finished()) {
		return;
	}
	ptr = nextptr;
	while(*ptr == ' ') {
		++ptr;
	}
	current_token = get_next_token();
	return;
}
/*---------------------------------------------------------------------------*/
int tokenizer_num(void)
{
	if(*ptr >= '0' && *ptr <= '9') {
		return atoi(ptr);
	}else{
		if(*ptr == '-'){
			if((*(ptr+1) >= '0') && (*(ptr+1) <= '9')){
				return (atoi(ptr+1))*-1;
			}else{
				return 0;
			}
		}else{
			return 0;
		}
	}
}
/*---------------------------------------------------------------------------*/
void tokenizer_string(char *dest, int len)
{
	char *string_end;
	int string_len;
	int i,j,flg;
  
	if(tokenizer_token() != TOKENIZER_STRING) {
		return;
	}
	string_end = strchr(ptr + 1, '"');
	if(string_end == NULL) {
		return;
	}
	string_len = string_end - ptr - 1;
	if(len < string_len) {
		string_len = len;
	}
	flg = 0;
	j = 0;
	for(i=0;i<string_len;i++){
		if(ptr[1+i] == 0x5c){
			flg = 1;
		}else{
			if((flg == 1)&&(ptr[1+i] == 'n')){
				dest[j] = '\n';
			}else{
				dest[j] = ptr[1+i];
			}
			j++;
			flg = 0;
		}
	}
	dest[j] = 0;
	cputs_p(6,dest);
}
/*---------------------------------------------------------------------------*/
void tokenizer_error_print(void)
{
	char buf[20];
	glcd_PutsA("Syntax Error: ");
	glcd_PutsA(ltodeci(ErrLine,buf,4));
	glcd_PutsA(" \n");
	glcd_TransFromBuf();
}
/*---------------------------------------------------------------------------*/
int tokenizer_finished(void)
{
//	cputs_p(6,ptr);

	return *ptr == 0 || current_token == TOKENIZER_ENDOFINPUT;
}
/*---------------------------------------------------------------------------*/
long tokenizer_variable_num(void)
{
	if(*ptr >= 'a' && *ptr <= 'z') {
		return *ptr - 'a';
	}else if(*ptr >= 'A' && *ptr <= 'N') {
		return (*ptr - 'A')+MAX_VARNUM;
	}else{
		return -1;
	}
}
/*---------------------------------------------------------------------------*/
long tokenizer_num_or_variable_num(void)
{
	int i;
	char buf[10];
	
	if(isdigit(*ptr)){
		return atoi(ptr);
	}else{
		if(*ptr >= 'a' && *ptr <= 'z') {
		//	buf[0] = (*ptr - 'a')+'0';
		//	buf[1] = '\n';
		//	buf[2] =0x00;
		//	cputs_p(6,buf);
			return (long)variables[(*ptr - 'a')];
		}else if(*ptr >= 'A' && *ptr <= 'D') {
			if(*(ptr+1) == '['){
				i = atoi(ptr+2);
				if(i<MAX_ARRYLEN){
					return arrays[(*ptr - 'A')][(atoi(ptr+2))];
				}else{
					return 0;
				}
			}else{
				return 0;
			}
		}else if(*ptr >= 'E' && *ptr <= 'H') {
			if(*(ptr+1) == '['){
				i = atoi(ptr+2);
				if(i<MAX_STRINGLEN){
					return (long)(strings[(*ptr - 'E')][(atoi(ptr+2))]);
				}else{
					return 0;
				}
			}else{
				return 0;
			}
		}else if(*ptr >= 'I' && *ptr <= 'N') {
			return variables_big[(*ptr - 'I')];
		}else{
			return 0;
		}
	}
}
/*---------------------------------------------------------------------------*/
long tokenizer_bignum_or_variable_bignum(void)
{
	if(isdigit(*ptr)){
		return strtol(ptr, NULL, 0);
	}else{
		if(*ptr >= 'I' && *ptr <= 'N') {
			return variables_big[(*ptr - 'I')];
		}else{
			if(*ptr >= 'a' && *ptr <= 'z') {
				return (long)variables[(*ptr - 'a')];
			}else{
				return 0;
			}
		}
	}
}
