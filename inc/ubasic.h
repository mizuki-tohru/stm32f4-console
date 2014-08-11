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
#ifndef __UBASIC_H__
#define __UBASIC_H__

#include "ff.h"

int isEx(char *,char *);
char upper(char);

void accept(int);
void comment_accept(void);
int varfactor(void);
int factor(void);
int term(void);
int expr(void);
int relation(void);
void jump_linenum(int);
void goto_statement(void);
void print_statement(void);
void if_statement(void);
void let_statement(void);
void gosub_statement(void);
void return_statement(void);
void next_statement(void);
void for_statement(void);
void pset_statement(void);
void end_statement(void);
void cls_statement(void);
void refresh_statement(void);
void rem_statement(void);
void list_statement(void);
void load_statement(void);
void files_statement(void);
void peek_statement(void);
void poke_statement(void);
void wait_statement(void);
void input_statement(void);
void inp_statement(void);
void inr_statement(void);
void ina_statement(void);
void run_statement(void);
void statement(void);
void line_statement(void);

//void ubasic_init(const char *program);
void ubasic_init(char *);
void ubasic_run(void);
int ubasic_finished(void);

int ubasic_get_variable(int );
int ubasic_get_adc(int);

void ubasic_set_variable(int , long);
void ubasic_set_variable_big(int, long);
void ubasic_set_array(int , int, int);
void ubasic_clear_array(int );
void ubasic_set_string(int, char);
void ubasic_clear_strig(int);

FRESULT lsSD(void);
FRESULT lsSDA(void);

#endif /* __UBASIC_H__ */
