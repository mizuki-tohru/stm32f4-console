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
#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

enum {
  TOKENIZER_ERROR,
  TOKENIZER_ENDOFINPUT,
  TOKENIZER_NUMBER,
  TOKENIZER_STRING,
  TOKENIZER_VARIABLE,
  TOKENIZER_ARRAY,
  TOKENIZER_LET,
  TOKENIZER_PRINT,
  TOKENIZER_IF,
  TOKENIZER_THEN,
  TOKENIZER_ELSE,
  TOKENIZER_FOR,
  TOKENIZER_TO,
  TOKENIZER_NEXT,
  TOKENIZER_GOTO,
  TOKENIZER_GOSUB,
  TOKENIZER_RETURN,
  TOKENIZER_CALL,
  TOKENIZER_END,
  TOKENIZER_COMMA,
  TOKENIZER_SEMICOLON,
  TOKENIZER_PLUS,
  TOKENIZER_MINUS,
  TOKENIZER_AND,
  TOKENIZER_OR,
  TOKENIZER_ASTR,
  TOKENIZER_SLASH,
  TOKENIZER_MOD,
  TOKENIZER_LEFTPAREN,
  TOKENIZER_RIGHTPAREN,
  TOKENIZER_LT,
  TOKENIZER_GT,
  TOKENIZER_EQ,
  TOKENIZER_CR,
  TOKENIZER_PSET,
  TOKENIZER_CLS,
  TOKENIZER_LIST,
  TOKENIZER_LOAD,
  TOKENIZER_FILES,
  TOKENIZER_PEEK,
  TOKENIZER_POKE,
  TOKENIZER_WAIT,
  TOKENIZER_INPUT,
  TOKENIZER_INP,
  TOKENIZER_INR,
  TOKENIZER_INA,
  TOKENIZER_REM,
  TOKENIZER_RUN,
  TOKENIZER_REFRESH,
  TOKENIZER_SAVE,
};

int singlechar(void);
int get_next_token(void);

void tokenizer_init(char *program);
int tokenizer_token(void);
void tokenizer_next(void);
char * tokenizer_nextptr(void);
int tokenizer_num(void);
void tokenizer_string(char *dest, int len);
void tokenizer_error_print(void);
int tokenizer_finished(void);
long tokenizer_variable_num(void);
long tokenizer_num_or_variable_num(void);
long tokenizer_bignum_or_variable_bignum(void);

#define MAX_VARNUM 26	/*a - z*/
#define MAX_ARRYNUM 4   /*ABCD*/
#define MAX_ARRYLEN 2048
#define MAX_STRINGNUM 4 /*EFGH*/
#define MAX_BIGVARNUM 6 /*IJKLMN*/
#define MAX_STRINGLEN 4096

extern int ErrLine;
extern int variables[MAX_VARNUM];
extern int arrays[MAX_ARRYNUM][MAX_ARRYLEN];
extern char strings[MAX_STRINGNUM+1][MAX_STRINGLEN];
extern long variables_big[MAX_BIGVARNUM];
extern int ended;

extern char *ptr, *nextptr;
extern int current_token;

#endif /* __TOKENIZER_H__ */
