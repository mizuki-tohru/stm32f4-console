#include "stm32f4xx.h"
#include "glcd.h"
#include "editor.h"
#include "fontdata.h"
#include "hw_config.h"
#include "ubasic.h"
#include "tokenizer.h"

// 400x240ドットのグラフィック液晶を使う。
// フォントが半角 8x16なら、50文字x15行 750文字
// フォントが全角16x16なら、25文字x15行 375文字

uint8_t glcd_x,glcd_y,flgUpdate;
uint16_t glcd_kanji_text_buf[50*16];
char BASICBUF[MAXBASIC];
char LINEBUF[128];

uint16_t editor_line[MAXLINE];
uint16_t endline;
uint16_t viewtop;

int  cursor_x; //画面内のカーソル位置
int  cursor_y;
int  old_x;    //移動検出用
int  old_y;    
int  line_end;
int  line_ptr;

int end_ptr;

struct char_type C_C;

uint8_t invert8bit(uint8_t in){
	return	(((in >> 7)&0x01)
	        |((in >> 5)&0x02)
	        |((in >> 3)&0x04)
	        |((in >> 1)&0x08)
	        |((in << 1)&0x10)
	        |((in << 3)&0x20)
	        |((in << 5)&0x40)
	        |((in << 7)&0x80));
}

struct keyword_token {
  char *keyword;
  int token;
};

static const struct keyword_token keywords2[] = {
  {"print", TOKENIZER_PRINT},
  {"PRINT", TOKENIZER_PRINT},
  {"pset", TOKENIZER_PSET},
  {"PSET", TOKENIZER_PSET},
  {"cls", TOKENIZER_CLS},
  {"CLS", TOKENIZER_CLS},
  {"list", TOKENIZER_LIST},
  {"LIST", TOKENIZER_LIST},
  {"load", TOKENIZER_LOAD},
  {"LOAD", TOKENIZER_LOAD},
  {"files", TOKENIZER_FILES},
  {"FILES", TOKENIZER_FILES},
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
  {NULL, TOKENIZER_ERROR}
};

//------------------------------------------------------------------------
//	long値実数から16進ASCIIへ。ltoa()を使わないのは、出力を大文字にしたい
//だけの理由から
//------------------------------------------------------------------------
int ltohex(unsigned long l,unsigned char * buf,unsigned char i)
{
	int j,k;
	unsigned long x;
	for(j = 0;j < i;j++) {
		x = l;
		for (k = 0;k < (i - j - 1);k++) x = (x >> 4);
		buf[j] = "0123456789ABCDEF"[(x & 0x0f)];
	}
	buf[j] = 0;
	return j;
}
char * ltodeci(long l,char * buf,unsigned char i)
{
	int j,k,m;
	unsigned long x,y;
	if(l < 0){
		y = -1*l;
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
/*-------------------------------------------------------------------------*/
int rcharnum(char ch)
{
	int i;
	if((ch >= '0')&&(ch <= '9')){	/*数*/
		if(C_C.flg == 0){
			C_C.flg = 3;	/*改行直後つまり行頭*/
		}else{
			if(C_C.flg != 3){
				C_C.flg = 4;
			}
		}
		i = strlen(C_C.text);
        C_C.text[i] = ch;
		C_C.text[i+1] = 0;
		return 0;
	}else{
		if(C_C.flg > 2){
			C_C.data = atol(C_C.text);
			C_C.text[0] = 0;
			C_C.flg = 1;
		}else{
			if(ch == 0x0a){			/*改行文字*/
				C_C.flg = 0;
			}else if (ch == 0x00){ /*末尾*/
				C_C.flg = 0;
			}else{							/*数字以外の文字*/
				C_C.flg = 2;
			}
		}
		return 1;
	}
}

void glcd_clearEditor(void)
{
	int i;
	for(i=0;i<MAXBASIC;i++)BASICBUF[i] = 0;
	for(i=0;i<50;i++)LINEBUF[i] = 0;
	for(i=0;i<MAXLINE;i++)editor_line[i] = 0;
	endline = 0;
	viewtop = 0;
	end_ptr = 0;
	cursor_x = 0;
	cursor_y = 0;
	old_x    = 0;
	old_y    = 0;
	line_end = 0;
	line_ptr = 0;
}


void glcd_redraw(void)
{
	uint16_t i,j,k,topptr;
	uint32_t sp;
	topptr = 0;
	j = 0;
	sp = 0;
	if(viewtop == 0){
		topptr = 0;
	}else{
		for(i=0;i<end_ptr;i++){
			if(BASICBUF[i] == '\n')j++;
			if(j == viewtop){
				topptr = i+1;
				break;
			}
		}
	}
	for(i=0;i<15;i++){
		for(j=0;j<editor_line[i];j++){
			if(BASICBUF[topptr+j] == '\n'){

			}else{
				for (k = 0;k < sizeOFblock1bytecode;k +=2) {
					if ((BASICBUF[topptr+j] >= block1bytecode[k]) 
					 && (BASICBUF[topptr+j] <= block1bytecode[k + 1])) {
						sp = block1bytesp[k / 2] 
						  + ((BASICBUF[topptr+j] - block1bytecode[k]) * width1bytefont * 2);
					}
				}
				for(k = 0;k < heightfont;k++) {
					glcd_buf[j+(k+i*heightfont)*GLCD_W] = 0xff ^ allFontDatas[sp+k];
					glcd_check[(240-(k+i*heightfont))>>3] |= 1<<((240-(k+i*heightfont))&0x07);
				}
			}
		}
		topptr +=(editor_line[viewtop+i]);
	}
}

/*editor_lineをBASICBUFに従い書き直す*/
void glcd_seteditor_line(void)
{
	int i,j,k;
	j = 0;
	k = 0;
	for(i=0;i<end_ptr;i++){
		j++;
		if(BASICBUF[i] == '\n'){
			editor_line[k] = j;/*改行文字を含む数*/
			j = 0;
			k++;
		}
	}
	endline = k;
}
/*カーソルのBASICBUF上の位置を出す*/
int glcd_getcursor_ptr(void)
{
	int i,r;
	r = 0;
	for(i=0;i<(cursor_y+viewtop);i++){
		r += editor_line[i];
	}
	r += cursor_x;
	return r;
}

void glcd_DrawCursor_first(void)
{
	uint16_t j;
	for(j = 0;j < heightfont;j++) {/*カーソルを書く*/
		glcd_buf[cursor_x+(j+cursor_y*heightfont)*GLCD_W] ^= 0xff;
		glcd_check[(240-(j+cursor_y*heightfont))>>3] |= 1<<((240-(j+cursor_y*heightfont))&0x07);
	}
}

void glcd_DrawCursor(void)
{
	/*行末かどうか*/
	uint16_t j;

	/*カーソル移動検出*/
	if((cursor_x != old_x)||(cursor_y != old_y)){
		/*古いカーソルを消す*/
		for(j = 0;j < heightfont;j++) {
			glcd_buf[old_x+(j+old_y*heightfont)*GLCD_W] ^= 0xff;
			glcd_check[(240-(j+old_y*heightfont))>>3] |= 1<<((240-(j+old_y*heightfont))&0x07);
		}
		old_x = cursor_x;
		old_y = cursor_y;
	}
	for(j = 0;j < heightfont;j++) {/*カーソルを書く*/
		glcd_buf[cursor_x+(j+cursor_y*heightfont)*GLCD_W] ^= 0xff;
		glcd_check[(240-(j+cursor_y*heightfont))>>3] |= 1<<((240-(j+cursor_y*heightfont))&0x07);
	}
}

void insert_basic_line(char * Buf,int len)
{
	int i,j,k;
	int line_num;
	int line_flg;
	int line_pos;
	int line_count;
	
	line_num = 0;
	line_flg = 0;
	line_pos = 0;
	line_count = 0;

	/*行番号があれば*/
	if((Buf[0] >= '0')
	 &&(Buf[0] <= '9')){
		C_C.flg = 0;
		C_C.text[0] = 0;
		for(i=0;i<len;i++){
			if(rcharnum(Buf[i])){
				if(C_C.flg == 1){
					line_num = C_C.data;/*編集行の行番号取り出し*/
				}
			}
		}
		line_pos = 0;
		line_flg = 0;
		line_count = 0;
		C_C.flg = 0;
		C_C.text[0] = 0;
		if(end_ptr == 0){/*BASICBUFは空*/
			for(j=0;j<len;j++){
				BASICBUF[j] = Buf[j];
			}
			endline++;
			editor_line[0] = len;
			editor_line[1] = 0;
			end_ptr += len;
		}else{
			for(i=0;i<end_ptr;i++){
				if(rcharnum(BASICBUF[i])){
					if(C_C.flg == 0){/*改行*/
						line_pos = i;
						line_count++;
					}else if(C_C.flg == 1){/*行番号*/
						if(line_num > C_C.data){
							line_flg = 1;
						}else{
							if(line_flg == 1){
								if(line_num == C_C.data){
								/*置き換え*/
									if(editor_line[line_count] > len){
										/*縮める*/
										k = editor_line[line_count] - len;
										for(j=(line_pos+len-k+1);j<end_ptr;j++){
											BASICBUF[j-k] = BASICBUF[j];
										}
										end_ptr-=k;
									}else if(editor_line[line_count] < len){
										/*伸ばす*/
										k = len - editor_line[line_count];
										for(j=end_ptr;j>(end_ptr+k);j--){
											BASICBUF[j+k] = BASICBUF[j];
										}
										end_ptr+=k;
									}
									for(j=0;j<len;j++){
										BASICBUF[line_pos+1+j] = Buf[j];
									}
									editor_line[line_count] = len;
								}else{
								/*挿入*/
									for(j=endline;j>line_count;j--){
										editor_line[j+1] = editor_line[j];
									}
									endline++;
									editor_line[line_count] = len;
									editor_line[endline+1] = 0;
									for(j=end_ptr+len;j>line_pos+1;j--){
										BASICBUF[j+len] = BASICBUF[j];
									}
									for(j=0;j<len;j++){
										BASICBUF[line_pos+1+j] = Buf[j];
									}
									end_ptr += len;
								}
								line_flg = 2;
							}
						}
					}
				}
			}
			if(line_flg == 1){/*行末に追加*/
				for(j=0;j<len;j++){
					BASICBUF[end_ptr+j] = Buf[j];
				}
				endline++;
				editor_line[line_count] = len;
				editor_line[line_count+1] = 0;
				end_ptr += len;
				BASICBUF[end_ptr+1] = 0x00;
			}
		}
	}
}

int glcd_PutEditor(uint16_t c)
{
//	int i,j,l,flgUp;
	int i,l,flgUp;
	int run_flg,key_flg;
	struct keyword_token const *kt;
#if 1
	char buf[13];
#endif	
	c &= 0x7fff;
	flgUp = 0;
	run_flg = 0;
	l = 0;
	
	if ((c == KEY_BS) 
	 || (c == KEY_DEL) 
	 || (c == KEY_TAB) 
	 || (c == KEY_UP)
	 || (c == KEY_DOWN)
	 || (c == KEY_LEFT)
	 || (c == KEY_RIGHT)
	 || (c == 0x0a) 
	 || (c == '\n')) {
		if ((c == KEY_BS) && (cursor_x >= 0)) {
			if(line_end > 0){
				/*文字を消して配列を縮める*/
				for(i=line_ptr;i<line_end;i++){
					LINEBUF[i] = LINEBUF[i+1];
				}
				LINEBUF[line_end] = 0x00;
				line_end--;
				line_ptr--;
			}else{/*前の行を修正*/
				/*できない*/
			}
		}
		if ((c == KEY_DEL) && (cursor_x >= 0)) {
			if((line_end > 0)&&(line_ptr < line_end)){
				/*文字を消して配列を縮める*/
				for(i=line_ptr+1;i<line_end;i++){
					LINEBUF[i] = LINEBUF[i+1];
				}
				LINEBUF[line_end] = 0x00;
				line_end--;
			}else{/*前の行を修正*/
				/*できない*/
			}
		}
		if (c == '\n') {
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
			}else{/*行番号が無ければ*/
				/*コマンドとして解釈*/
				if(strncmp(LINEBUF,"RUN",3) == 0) {
#if TEST
#else
					ubasic_init(BASICBUF);
#endif
				}else if(strncmp(LINEBUF,"run",3) == 0) {
#if TEST
#else
					ubasic_init(BASICBUF);
#endif
				}else{
					key_flg = TOKENIZER_ERROR;
					for(kt = keywords2; kt->keyword != NULL; ++kt) {
						if(strncmp(LINEBUF, kt->keyword, strlen(kt->keyword)) == 0) {
							key_flg = kt->token;
						}
					}
					if(key_flg != TOKENIZER_ERROR){
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
#if TEST
#else
						ubasic_init(LINEBUF);
#endif
					}else{
						for(i=0;i<50;i++)LINEBUF[i] = 0;
					}
					line_end = 0;
					line_ptr = 0;
				}
				run_flg = 1;
			}
		}
		if (c == KEY_TAB) {
			if(cursor_x+4 < 50){
				/*まずは文字挿入*/
				for(i=line_ptr;i>line_end;i--){
					LINEBUF[i+4] = LINEBUF[i];
				}
				LINEBUF[line_ptr+0] = ' ';
				LINEBUF[line_ptr+1] = ' ';
				LINEBUF[line_ptr+2] = ' ';
				LINEBUF[line_ptr+3] = ' ';
			//	end_ptr+=4;
				line_ptr+=4;
				line_end+=4;
			}
		}
		if ((c == KEY_RIGHT) && (cursor_x < 50)) {
			//
		}
		if ((c == KEY_LEFT) && (cursor_x > 0)) {
			//
		}
		if ((c == KEY_UP) && (viewtop+cursor_y > 0)){
			/*行移動に伴い、既編集行をBASICBUFに反映*/
			insert_basic_line(LINEBUF,line_end);
			/*LINEBUFに新行を反映*/
			line_end = 0;
			for(i=0;i<editor_line[viewtop+cursor_y-1];i++){
				LINEBUF[i] = BASICBUF[l+i];
				line_end++;
			}
			LINEBUF[line_end+1] = 0;
			if(cursor_x > editor_line[viewtop+cursor_y-1]){
				line_ptr = editor_line[viewtop+cursor_y-1];
			}else{
				line_ptr = cursor_x;
			}
		}
		if ((c == KEY_DOWN) && (editor_line[viewtop+cursor_y+1] > 0)) {
			/*行移動に伴い、既編集行をBASICBUFに反映*/
			insert_basic_line(LINEBUF,line_end);
			/*LINEBUFに新行を反映*/
			line_end = 0;
			for(i=0;i<editor_line[viewtop+cursor_y+1];i++){
				LINEBUF[i] = BASICBUF[l+i];
				line_end++;
			}
			LINEBUF[line_end+1] = 0;
			if(cursor_x > editor_line[viewtop+cursor_y-1]){
				line_ptr = editor_line[viewtop+cursor_y-1];
			}else{
				line_ptr = cursor_x;
			}
		}
	} else {
		if(cursor_x < 50){
			if(line_end > cursor_x){/*中間挿入*/
				for(i=line_end;i>=cursor_x;i--){
					LINEBUF[i+1] = LINEBUF[i];
				}
			}
			LINEBUF[line_ptr] = c;
			line_end++;
			line_ptr++;
		}
	}
#if 0
	if(c == '#'){
	  buf[0] = (viewtop/10) + '0';
	  buf[1] = (viewtop%10) + '0';
//	  buf[0] = (line_end/10) + '0';
//	  buf[1] = (line_end%10) + '0';
	  buf[2] = ' ';
//	  buf[3] = (line_ptr/10) + '0';
//	  buf[4] = (line_ptr%10) + '0';
//	  buf[3] = (old_x/10) + '0';
//	  buf[4] = (old_x%10) + '0';
	  buf[3] = (old_y/10) + '0';
	  buf[4] = (old_y%10) + '0';
	  buf[5] = ' ';
//	  buf[6] = (cursor_y/10) + '0';
//	  buf[7] = (cursor_y%10) + '0';
	  buf[6] = (line_ptr/10) + '0';
	  buf[7] = (line_ptr%10) + '0';
	  buf[8] = ' ';
//	  buf[9] = (endline/10) + '0';
//	  buf[10] = (endline%10) + '0';
	  buf[9] = (line_end/10) + '0';
	  buf[10] = (line_end%10) + '0';
	  buf[11] = '\n';
	  buf[12] = 0x00;
//	  cputs_p(6,buf);
	  glcd_PutsD(buf);
	}
#endif
	glcd_PutCharA(c);
	if (flgUp && flgUpdate) {
		glcd_TransFromBuf();
	}
	return run_flg;
}

void glcd_BufClear(uint8_t f)
{
	int i;
	if (f)f = 0xff;
	for (i = 0;i < (50*240);i++)glcd_buf[i] = f;
	for (i = 0;i < (50*16);i++) {
		if (f) {
			glcd_kanji_text_buf[i] = 0x8000;
		} else {
			glcd_kanji_text_buf[i] = 0x0;
		}
	}
	for(i=0;i<31;i++)glcd_check[i] = 0xff;
	for(i=0;i<MAXLINE;i++)editor_line[i] = 0;
}
void glcd_posClear(void)
{
	old_x = old_y = 0;
	glcd_x = glcd_y = 0;
	cursor_x = cursor_y = 0;
}

void glcd_PutChar(uint16_t c)
{
	int i,flg,flgUp,w;
	uint8_t flgInverse;
	uint32_t sp;
	if (c & 0x8000) {
		flgInverse = 0xff;
	} else {
		flgInverse = 0;
	}
	c &= 0x7fff;
	flg = 0;
	flgUp = 0;
	sp = 0;
	w = width1bytefont;
	if ((c == 0x07) || (c == 0x08) || (c == 0x09) || (c == 0x0a) || (c == 0x0d)) {
		if ((c == 0x08) && (glcd_x > 0)) {
			glcd_x--;
		}
		if (c == 0x0d) {
			glcd_x = 0;
		}
		if (c == 0x0a) {
			glcd_y ++;
		}
		if (c == 0x09) {
			glcd_x+=4;	/*4tab*/
		}
		return;
	} else {
		if (glcd_y >= GLCD_H) {
			glcd_y = GLCD_H - 1;
			for(i = 0;i < (50*240-400*2);i++) {
				glcd_buf[i] = glcd_buf[i+400*2];
				glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
			}
			for(i = 50*240-400*2;i < (50*240);i++) {
				glcd_buf[i] = 0xff;
				glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
			}
			flgUp = 1;
		}
		for (i = 0;i < sizeOFblock1bytecode;i +=2) {
			if ((c >= block1bytecode[i]) && (c <= block1bytecode[i + 1])) {
				flg = 1;
				w = width1bytefont;
				if (width1bytefont > 4) {
					sp = block1bytesp[i / 2] + ((c - block1bytecode[i]) * w * 2);
				} else {
					sp = block1bytesp[i / 2] + ((c - block1bytecode[i]) * w);
				}
			}
		}
		if ((flg == 0) && (c >= 0x100))
			for (i = 0;i < sizeOFblock2bytecode;i +=2) {
				if ((c >= block2bytecode[i]) && (c <= block2bytecode[i + 1])) {
					flg = 1;
					w = width2bytefont;
					if (width1bytefont > 4) {
						sp = block2bytesp[i / 2] + ((c - block2bytecode[i]) * w * 2);
					} else {
						sp = block2bytesp[i / 2] + ((c - block2bytecode[i]) * w);
					}
				}
			}
	}
	if (flg == 0) {
		if (c >= 0x100)
			w = width2bytefont;
	}
	if ((glcd_x >= GLCD_W) || ((glcd_x >= (GLCD_W - 1)) && (w > width1bytefont))) {
		glcd_x = 0;
		glcd_y ++;
	}
	if (flg) {
		for(i = 0;i < heightfont;i++) {
			glcd_buf[glcd_x+(i+glcd_y*heightfont)*GLCD_W] = flgInverse ^ allFontDatas[sp+i];
			glcd_check[(240-(i+glcd_y*heightfont))>>3] |= 1<<((240-(i+glcd_y*heightfont))&0x07);
			if (w > 8) {
				glcd_buf[glcd_x+(i+glcd_y*heightfont)*GLCD_W+1] = flgInverse ^ allFontDatas[sp+w+i];
				glcd_check[(240-(i+glcd_y*heightfont))>>3] |= 1<<((240-(i+glcd_y*heightfont))&0x07);
			}
		}
	}
	if (w == width2bytefont) {
		glcd_x += 2;
	} else {
		glcd_x++;
	}
	if (flgUp && flgUpdate) {
		glcd_TransFromBuf();
	}
}

void glcd_PutCharA(uint16_t c)
{
	int j,k,flg,flgUp,w;
	long i;
	uint8_t flgInverse;
	uint32_t sp;
	int cursor_ptr;
	
	if (c & 0x8000) {
		flgInverse = 0xff;
	} else {
		flgInverse = 0;
	}
	c &= 0x7fff;
	flg = 0;
	flgUp = 0;
	sp = 0;
	w = width1bytefont;
	if ((c == KEY_DEL) || (c == KEY_BS) || (c == KEY_TAB) 
	 || (c == KEY_UP) || (c == KEY_DOWN)
	 || (c == KEY_LEFT) || (c == KEY_RIGHT)
	 || (c == 0x0a) || (c == 0x0d)) {
		if ((c == KEY_BS) && (cursor_x > 0)) {
			/*現在のカーソル位置を取得*/
			/*editor_lineをBASICBUFに従い書き直す*/
			glcd_seteditor_line();
			old_x = cursor_x;
			cursor_x--;
		}
		if ((c == KEY_DEL) && (cursor_x > 0)) {
			/*editor_lineをBASICBUFに従い書き直す*/
			glcd_seteditor_line();
		}
		if (c == '\n') {
			if (cursor_y >= GLCD_H) {/*最下部行末なら*/
				if(viewtop + GLCD_H < MAXLINE){ /*スクロール*/
					glcd_DrawCursor_first();
					for(i = 0;i < ((50*240)-(400*2));i++) {
						glcd_buf[i] = glcd_buf[i+(400*2)];
						glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
					}
					for(i = ((50*240)-(400*2));i < (50*240);i++) {
						glcd_buf[i] = 0xff;
						glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
					}
					viewtop++;
					glcd_DrawCursor_first();
				}
				old_x = cursor_x;
				cursor_x = 0;
			}else{
				/*何処かの行間であれば カーソルのある位置に行挿入*/
				if(cursor_x < editor_line[viewtop+cursor_y]){
					/*画面描画配列を更新する*/
					if (cursor_y+1 >= GLCD_H) {
						if(viewtop+1 + GLCD_H > MAXLINE){
							//
						}else{
							old_y = cursor_y;
							cursor_y++;
							old_x = cursor_x;
							cursor_x = 0;
					//		glcd_redraw();
						}
					}else{
						old_y = cursor_y;
						cursor_y++;
						old_x = cursor_x;
						cursor_x = 0;
					//	glcd_redraw();
					}
				}else{
					/*行末での行挿入。空行挿入になる*/
					/*まずは改行挿入*/
					/*画面描画配列を更新する*/
					if (cursor_y-1 >= GLCD_H) {
						if(viewtop + GLCD_H < MAXLINE){
							glcd_DrawCursor_first();
							for(i = ((cursor_y+1)*240);i < (50*240);i++) {
								glcd_buf[i] = glcd_buf[i-(400*2)];
								glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
							}
							for(i = 0;i < 2*400;i++) {
								glcd_buf[cursor_y*240+i] = 0xff;
								glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
							}
							glcd_DrawCursor_first();
							old_y = cursor_y;
							cursor_y++;
							old_x = cursor_x;
							cursor_x = 0;
						}
					}else{
						old_y = cursor_y;
						cursor_y++;
						old_x = cursor_x;
						cursor_x = 0;
					}
				}
			}
			glcd_seteditor_line();
		}
		if ((c == KEY_TAB) && (cursor_x < 46)){	/*KEY_TAB*/
			old_x = cursor_x;
			cursor_x+=4;	/*4tab*/
			for (k = 0;k < sizeOFblock1bytecode;k +=2) {
				if ((' ' >= block1bytecode[k]) 
				 && (' ' <= block1bytecode[k + 1])) {
					sp = block1bytesp[k / 2] 
					  + ((c - block1bytecode[k]) * width1bytefont * 2);
				}
			}
			for(i=0;i<4;i++){
				for(k = 0;k < heightfont;k++) {
					glcd_buf[cursor_x+i+(k+cursor_y*heightfont)*GLCD_W] = 0xff ^ allFontDatas[sp+k];
					glcd_check[(240-(k+cursor_y*heightfont))>>3] |= 1<<((240-(k+cursor_y*heightfont))&0x07);
				}
			}
		}
		if (c == KEY_UP){
			/*上行に行番号があれば*/
			if(1){//-----------------------------------------------------------------
				if (cursor_y == 0) {/*画面最上行*/
					if(viewtop > 0){/*下にスクロールできる*/
						for(i = 50*240;i >= (400*2);i--) {/*表示を一行下に*/
							glcd_buf[i] = glcd_buf[i-400*2];
							glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
						}
						viewtop--;
						if(editor_line[viewtop] < cursor_x){/*カーソルのx位置を得る*/
							old_x = cursor_x;
							cursor_x = editor_line[viewtop];
						}
						/*現れた最上行の描画*/
						cursor_ptr = glcd_getcursor_ptr();
						for(i=(cursor_ptr-cursor_x);i<(cursor_ptr-cursor_x+editor_line[viewtop]);i++){
							for (k = 0;k < sizeOFblock1bytecode;k +=2) {
								if ((BASICBUF[i] >= block1bytecode[k]) 
								 && (BASICBUF[i] <= block1bytecode[k + 1])) {
									sp = block1bytesp[k / 2] 
									  + ((BASICBUF[i] - block1bytecode[k]) * width1bytefont * 2);
								}
							}
							for(j = 0;j < heightfont;j++) {
								glcd_buf[i+(j+cursor_y*heightfont)*GLCD_W] = 0xff ^ allFontDatas[sp+j];
								glcd_check[(240-(j+cursor_y*heightfont))>>3] |= 1<<((240-(j+cursor_y*heightfont))&0x07);
							}
						}
					}else{
						/*スクロールする余地なし*/
					}
				}else{
					/*画面内をカーソルが移動する*/
					if(editor_line[viewtop+cursor_y-1] < cursor_x){
						old_x = cursor_x;
						cursor_x = editor_line[viewtop+cursor_y-1];
					}
					old_y = cursor_y;
					cursor_y--;
				}
			}else{
				
			}
		}
		if (c == KEY_DOWN){
			if(editor_line[viewtop+cursor_y+1] != 0){/*次の行に描く物がある*/
				if (cursor_y >= GLCD_H) {/*画面最下行*/
					if(viewtop + GLCD_H < MAXLINE){/*上にスクロール*/
						glcd_DrawCursor_first();
						for(i = 0;i < (50*240-400*2);i++) {/*一行上に*/
							glcd_buf[i] = glcd_buf[i+400*2];
							glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
						}
						glcd_DrawCursor_first();
						viewtop++;
						if(editor_line[viewtop+cursor_y] < cursor_x){/*カーソルのx位置を得る*/
							old_x = cursor_x;
							cursor_x = editor_line[viewtop+cursor_y];
						}
						cursor_ptr = glcd_getcursor_ptr();
						for(i= cursor_ptr-cursor_x;i<cursor_ptr-cursor_x+editor_line[viewtop + cursor_y];i++){
							for (k = 0;k < sizeOFblock1bytecode;k +=2) {
								if ((BASICBUF[i] >= block1bytecode[k]) 
								 && (BASICBUF[i] <= block1bytecode[k + 1])) {
									sp = block1bytesp[k / 2] 
									  + ((BASICBUF[i] - block1bytecode[k]) * width1bytefont * 2);
								}
							}
							for(j = 0;j < heightfont;j++) {
								glcd_buf[i+(j+cursor_y*heightfont)*GLCD_W] = 0xff ^ allFontDatas[sp+j];
								glcd_check[(240-(j+cursor_y*heightfont))>>3] |= 1<<((240-(j+cursor_y*heightfont))&0x07);
							}
						}
					}
				}else{
					/*画面内をカーソルが移動する*/
					old_y = cursor_y;
					cursor_y++;
					if(editor_line[viewtop+cursor_y] < cursor_x){
						old_x = cursor_x;
						cursor_x = editor_line[viewtop+cursor_y];
					}
				}
			}
		}
		if ((c == KEY_RIGHT) && (cursor_x < 50)) {
			/*改行位置までしか移動しない*/
			if(editor_line[viewtop+cursor_y] >= cursor_x+1){
				old_x = cursor_x;
				cursor_x++;
			}
		}
		if ((c == KEY_LEFT) && (cursor_x > 0)) {
			old_y = cursor_y;
			cursor_x--;
		}
		glcd_DrawCursor();
		return;
	}else{/*通常文字*/
		if (cursor_y > GLCD_H) {
			old_y = cursor_y;
			cursor_y = GLCD_H - 1;
			glcd_DrawCursor_first();
			for(i = 0;i < (50*240-400*2);i++) {
				glcd_buf[i] = glcd_buf[i+400*2];
				glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
			}
			for(i = 50*240-400*2;i < (50*240);i++) {
				glcd_buf[i] = 0xff;
				glcd_check[(240-(i/GLCD_W))>>3] |= 1<<((240-(i/GLCD_W))&0x07);
			}
			glcd_DrawCursor_first();
			flgUp = 1;
		}
		for (i = 0;i < sizeOFblock1bytecode;i +=2) {
			if ((c >= block1bytecode[i]) && (c <= block1bytecode[i + 1])) {
				flg = 1;
				w = width1bytefont;
				if (width1bytefont > 4) {
					sp = block1bytesp[i / 2] + ((c - block1bytecode[i]) * w * 2);
				} else {
					sp = block1bytesp[i / 2] + ((c - block1bytecode[i]) * w);
				}
			}
		}
		if ((flg == 0) && (c >= 0x100)){
			for (i = 0;i < sizeOFblock2bytecode;i +=2) {
				if ((c >= block2bytecode[i]) && (c <= block2bytecode[i + 1])) {
					flg = 1;
					w = width2bytefont;
					if (width1bytefont > 4) {
						sp = block2bytesp[i / 2] + ((c - block2bytecode[i]) * w * 2);
					} else {
						sp = block2bytesp[i / 2] + ((c - block2bytecode[i]) * w);
					}
				}
			}
		}
		if (flg == 0) {
			if (c >= 0x100)	w = width2bytefont;
		}
		if (flg) {
			for(i = 0;i < heightfont;i++) {
				glcd_buf[cursor_x+(i+cursor_y*heightfont)*GLCD_W] = flgInverse ^ allFontDatas[sp+i];
				glcd_check[(240-(i+cursor_y*heightfont))>>3] |= 1<<((240-(i+cursor_y*heightfont))&0x07);
				if (w > 8) {
					glcd_buf[cursor_x+(i+cursor_y*heightfont)*GLCD_W+1] = flgInverse ^ allFontDatas[sp+w+i];
					glcd_check[(240-(i+cursor_y*heightfont))>>3] |= 1<<((240-(i+cursor_y*heightfont))&0x07);
				}
			}
		}
		/*カーソル位置更新*/
		if ((cursor_x >= GLCD_W) || ((cursor_x >= (GLCD_W - 1)) && (w > width1bytefont))) {
			old_x = cursor_x;
			cursor_x = 0;
			old_y = cursor_y;
			cursor_y ++;
			editor_line[viewtop+cursor_y] = 0;
		}else{
			if (w == width2bytefont) {
				old_x = cursor_x;
				cursor_x += 2;
				editor_line[viewtop+cursor_y] +=2;
			} else {
				old_x = cursor_x;
				cursor_x++;
				editor_line[viewtop+cursor_y] ++;
			}
		}
	}
	glcd_DrawCursor();
	if (flgUp && flgUpdate) {
		glcd_TransFromBuf();
	}
}

void glcd_Puts(const char *buf)
{
	int flg16 = 0;
	int i = 0;
	short j = 0;
	int k;
	while(buf[i]) {
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 1;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x40)) {
			i += 3;
			flg16 = 1;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x28)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 0;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='H')) {
			i += 3;
			glcd_x = glcd_y = 0;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='2')&&(buf[i+2] =='2')) {
			i += 4;
			for (k = 0;k < (50*240);k++){
				glcd_buf[k] = 0xff;
				glcd_check[(240-(k/GLCD_W))>>3] |= 1<<((240-(k/GLCD_W))&0x07);
			}
		} else if (flg16) {
			if (j) {
				glcd_PutChar(0x8000 | (j << 8 | buf[i]));
				j = 0;
			} else {
				j = buf[i];
			}
			i++ ;
		} else {
			glcd_PutChar(0x8000 | (buf[i] & 0x0ff));
			i++ ;
		}
		if(i>128)break;
	}
}

void glcd_PutsA(const char *buf)
{
	int flg16 = 0;
	int i = 0;
	short j = 0;
	int k;
	while(buf[i]) {   /*ESCで漢字*/
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 1;
		} else        /*ESCで漢字*/
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x40)) {
			i += 3;
			flg16 = 1;
		} else        /*ESCで漢字*/
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x28)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 0;
		} else       /*ESCで漢字*/
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='H')) {
			i += 3;
			old_x = cursor_x;
			old_y = cursor_y;
			cursor_x = cursor_y = 0;
		} else		/*ESCで*/
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='2')&&(buf[i+2] =='2')) {
			i += 4;
			for (k = 0;k < (50*240);k++){
				glcd_buf[k] = 0xff;
				glcd_check[(240-(k/GLCD_W))>>3] |= 1<<((240-(k/GLCD_W))&0x07);
			}
		} else if (flg16) {
			if (j) {
				glcd_PutCharA(j << 8 | buf[i]);
				j = 0;
			} else {
				j = buf[i];
			}
			i++ ;
		} else {
			glcd_PutCharA(buf[i]&0xff);
			if(buf[i] == '\n'){
				for(k=0;k<50;k++)LINEBUF[k] = 0;
				line_ptr = 0;
				line_end = 0;
			}else{
				LINEBUF[line_ptr] = buf[i] & 0x0ff;
				line_ptr++;
				line_end++;
			}
			i++ ;
		}
		if(i>128)break;
	}
}

void glcd_PutsD(const char *buf)
{
	int i = 0;
	int k;

	while(buf[i]) {
		glcd_PutCharA(buf[i] & 0xff);
		if(buf[i] == '\n'){
			for(k=0;k<50;k++)LINEBUF[k] = 0;
			line_ptr = 0;
			line_end = 0;
		}else{
			LINEBUF[line_ptr] = buf[i] & 0x0ff;
			line_ptr++;
			line_end++;
		}
		i++;
		if(i>128)break;
	}
}

void glcd_UnPutsA(const char *buf)
{
	int flg16 = 0;
	int i = 0;
	short j = 0;
	int k;
	end_ptr = 0;
	while(buf[i]) {   /*ESCで漢字*/
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 1;
		} else        /*ESCで漢字*/
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x40)) {
			i += 3;
			flg16 = 1;
		} else        /*ESCで漢字*/
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x28)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 0;
		} else       /*ESCで漢字*/
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='H')) {
			i += 3;
			old_x = cursor_x;
			old_y = cursor_y;
			cursor_x = cursor_y = 0;
		} else		/*ESCで*/
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='2')&&(buf[i+2] =='2')) {
			i += 4;
			for (k = 0;k < (50*240);k++){
				glcd_buf[k] = 0xff;
				glcd_check[(240-(k/GLCD_W))>>3] |= 1<<((240-(k/GLCD_W))&0x07);
			}
		} else if (flg16) {
			if (j) {
				glcd_PutCharA(j << 8 | buf[i]);
				j = 0;
			} else {
				j = buf[i];
			}
			i++ ;
		} else {
			BASICBUF[i] = buf[i] & 0x0ff;
			end_ptr++;
			i++ ;
		}
		if(i>128)break;
	}
	BASICBUF[i] = 0;
}

void glcd_PutsU(const unsigned char *buf)
{
	int flg16 = 0;
	int i = 0;
	short j = 0;
	int k;
	while(buf[i]) {
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 1;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x40)) {
			i += 3;
			flg16 = 1;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x28)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 0;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='H')) {
			i += 3;
			glcd_x = glcd_y = 0;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='2')&&(buf[i+2] =='2')) {
			i += 4;
			for (k = 0;k < (50*240);k++){
				glcd_buf[k] = 0xff;
				glcd_check[(240-(k/GLCD_W))>>3] |= 1<<((240-(k/GLCD_W))&0x07);
			}
		} else
		if (flg16) {
			if (j) {
				glcd_PutChar(j << 8 | buf[i]);
				j = 0;
			} else {
				j = buf[i];
			}
			i++ ;
		} else {
			glcd_PutChar(buf[i] & 0x0ff);
			i++ ;
		}
		if(i>128)break;
	}
}

void glcd_transChar()
{
	int i,flg,w;
	uint8_t flgInverse;
	uint32_t sp;

	uint16_t c;
	int x,y;
	y = 0;
	while (y < GLCD_H) {
		x = 0;
		while (x < GLCD_W) {
			c = glcd_kanji_text_buf[x + y * GLCD_W];
			if (c & 0x8000) {
				flgInverse = 0xff;
			} else {
				flgInverse = 0;
			}
			c &= 0x7fff;
			flg = 0;
			sp = 0;
			w = width1bytefont;
			for (i = 0;i < sizeOFblock1bytecode;i +=2) {
				if ((c >= block1bytecode[i]) && (c <= block1bytecode[i + 1])) {
					flg = 1;
					w = width1bytefont;
					if (width1bytefont > 4) {
						sp = block1bytesp[i / 2] + ((c - block1bytecode[i]) * w * 2);
					} else {
						sp = block1bytesp[i / 2] + ((c - block1bytecode[i]) * w);
					}
				}
			}
			if ((flg == 0) && (c >= 0x100))
				for (i = 0;i < sizeOFblock2bytecode;i +=2) {
					if ((c >= block2bytecode[i]) && (c <= block2bytecode[i + 1])) {
						flg = 1;
						w = width2bytefont;
						if (width1bytefont > 4) {
							sp = block2bytesp[i / 2] + ((c - block2bytecode[i]) * w * 2);
						} else {
							sp = block2bytesp[i / 2] + ((c - block2bytecode[i]) * w);
						}
					}
				}
			if (flg == 0) {
				if (c >= 0x100)
					w = width2bytefont;
			}
			if (flg) {
				for(i = 0;i < heightfont;i++) {
					glcd_buf[x+(i+y*heightfont)*GLCD_W] = flgInverse ^ allFontDatas[sp+i];
					glcd_check[(240-(i+y*heightfont))>>3] |= 1<<((240-(i+y*heightfont))&0x07);
					if (w > 8) {
						glcd_buf[x+(i+y*heightfont)*GLCD_W+1] = flgInverse ^ allFontDatas[sp+w+i];
						glcd_check[(240-(i+y*heightfont))>>3] |= 1<<((240-(i+y*heightfont))&0x07);
					}
				}
			} else {
				for(i = 0;i < heightfont;i++) {
					glcd_buf[x+(i+y*heightfont)*GLCD_W] = flgInverse;
					glcd_check[(240-(i+y*heightfont))>>3] |= 1<<((240-(i+y*heightfont))&0x07);
					if (w > 8) {
						glcd_buf[x+(i+y*heightfont)*GLCD_W+1] = flgInverse;
						glcd_check[(240-(i+y*heightfont))>>3] |= 1<<((240-(i+y*heightfont))&0x07);
					}
				}
			}
			if (w == width2bytefont) {
				x += 2;
			} else {
				x++;
			}
		}
		y++;
	}
}

void glcd_Puts_Serial(const char *buf)
{
	int flg16 = 0;
	int i = 0;
	int k;
	uint16_t j = 0;
	while(buf[i]) {
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 1;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x24)&&(buf[i+2] ==0x40)) {
			i += 3;
			flg16 = 1;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]==0x28)&&(buf[i+2] ==0x42)) {
			i += 3;
			flg16 = 0;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='H')) {
			i += 3;
			glcd_x = glcd_y = 0;
		} else
		if ((buf[i] == 0x1B)&&(buf[i+1]=='[')&&(buf[i+2] =='2')&&(buf[i+2] =='2')) {
			i += 4;
			for (k = 0;k < (50*16);k++)
				glcd_kanji_text_buf[k] = 0x8000;
		} else
		if (flg16) {
			if (j) {
				if (glcd_x >= (GLCD_W - 1)) {
					glcd_x = 0;
					glcd_y ++;
				}
				glcd_kanji_text_buf[glcd_x + glcd_y * GLCD_W] = 0x8000 | (j << 8 | buf[i]);
				j = 0;
				glcd_x += 2;
			} else {
				j = buf[i];
			}
			i++ ;
		} else {
			if ((buf[i] == 0x08) && (glcd_x > 0)) {
				glcd_x--;
			} else if (buf[i] == 0x0d) {
				glcd_x = 0;
			} else if (buf[i] == 0x0a) {
				glcd_y ++;
			} else {
				if (glcd_x >= GLCD_W) {
					glcd_x = 0;
					glcd_y ++;
				}
				glcd_kanji_text_buf[glcd_x + glcd_y * GLCD_W] = 0x8000 | (buf[i] & 0x0ff);
				glcd_x++;
			}
			i++ ;
		}
		if (glcd_y >= GLCD_H) {
			glcd_y = GLCD_H-1;
			for(k = 0;k < (GLCD_W*GLCD_H);k++) {
				glcd_kanji_text_buf[k] = glcd_kanji_text_buf[k+GLCD_W];
			}
			for(k = GLCD_W*GLCD_H;k < (GLCD_W*(GLCD_H+1));k++) {
				glcd_kanji_text_buf[k] = 0x8000;
			}
		}
		if(i>128)break;
	}
}


void glcd_PutCharAt(uint16_t x,uint16_t y,uint16_t c)
{
	int i,flg,w;
	uint8_t flgInverse;
	uint32_t sp;
	if (c & 0x8000) {
		flgInverse = 0xff;
	} else {
		flgInverse = 0;
	}
	x /= 8;
	c &= 0x7fff;
	flg = 0;
	sp = 0;
	w = width1bytefont;
	if ((c == 0x07) || (c == 0x08) || (c == 0x0a) || (c == 0x0d)) {
		return;
	} else {
		for (i = 0;i < sizeOFblock1bytecode;i +=2) {
			if ((c >= block1bytecode[i]) && (c <= block1bytecode[i + 1])) {
				flg = 1;
				w = width1bytefont;
				if (width1bytefont > 4) {
					sp = block1bytesp[i / 2] + ((c - block1bytecode[i]) * w * 2);
				} else {
					sp = block1bytesp[i / 2] + ((c - block1bytecode[i]) * w);
				}
			}
		}
		if ((flg == 0) && (c >= 0x100))
			for (i = 0;i < sizeOFblock2bytecode;i +=2) {
				if ((c >= block2bytecode[i]) && (c <= block2bytecode[i + 1])) {
					flg = 1;
					w = width2bytefont;
					if (width1bytefont > 4) {
						sp = block2bytesp[i / 2] + ((c - block2bytecode[i]) * w * 2);
					} else {
						sp = block2bytesp[i / 2] + ((c - block2bytecode[i]) * w);
					}
				}
			}
	}
	if (flg == 0) {
		if (c >= 0x100)
			w = width2bytefont;
	}
	if (flg) {
		for(i = 0;i < heightfont;i++) {
			glcd_buf[x+(i+y)*GLCD_W] = flgInverse ^ allFontDatas[sp+i];
			glcd_check[(240-(i+y))>>3] |= 1<<((240-(i+y))&0x07);
			if (w > 8) {
				glcd_buf[x+(i+y)*GLCD_W+1] = flgInverse ^ allFontDatas[sp+w+i];
				glcd_check[(240-(i+y))>>3] |= 1<<((240-(i+y))&0x07);
			}
		}
	}
}

void glcd_PutsAt(uint16_t x,uint16_t y,const char *buf)
{
	int i=0;
	while(buf[i]) {
		glcd_PutCharAt(x,y, buf[i]);
		i++;
		x += 8;
		if(i>128)break;
	}
}

void glcd_Vline(uint16_t x,int32_t d)
{
	int i,p;
	int xx;
	uint8_t e;
	uint8_t buf[20];
	xx = x / 8;
	e = (0x01 << (x % 8));
	for(i = 0;i < 240;i++) {
		if ((i/2) % 2){
			glcd_buf[xx+i*GLCD_W] &= (~e);
			glcd_check[(239-i)>>3] |= 1<<((239-i)&0x07);
		}
	}
	p = 0;
	if (d < 0) {
		d = -d;
		buf[p++] = '-';
		return;
	}
	uint8_t flg = 1;
	int32_t j = 10;
	if (d >= 1000000000) {
		buf[p++] = '0' + (d / 1000000000);
		d = d % 1000000000;
	}
	if ((d >= 1000) && (d % 1000 == 0)) {
		flg = 0;
		d /= 1000;
	} 
	while (d >= j) {
		j *= 10;
	}
	j /= 10;
	while (j >= 1) {
		buf[p++] = '0' + (d / j);
		d = d % j;
		j /= 10;
	}
	if (flg)
		buf[p++] = 'm';
	buf[p++] = 's';
	x -= (8 * ((p-1)/2));
	for(j = 0;j < p;j++) {
		glcd_PutCharAt(x,0,buf[j]);
		x += 8;
	}
}

void glcd_setLine(uint16_t x,uint16_t y,uint8_t d)
{
	x /= 8;
	glcd_buf[x+y*GLCD_W] &= d;
	glcd_check[(239-y)>>3] |= 1<<((239-y)&0x07);
}

void glcd_setDot(uint16_t x,uint16_t y)
{
	glcd_buf[(x/8)+y*GLCD_W] &= (~(0x01 << (x % 8)));
	glcd_check[(239-y)>>3] |= 1<<((239-y)&0x07);
}

void glcd_Dec(uint16_t x,uint16_t y,int n,int d,uint8_t flg)
{
	int i,j,k;
	uint8_t c;
	for (i = 0;i < n;i++) {
		k = 1;
		for(j = 0;j < (n - i - 1);j ++)
			k *= 10;
		c = (d / k) % 10;
		if (flg) {
			glcd_PutCharAt(x,y,0x8000|('0'+c));
		} else {
			glcd_PutCharAt(x,y,'0'+c);
		}
		x += 8;
	}
}

void glcd_Hex(uint16_t x,uint16_t y,int n,int32_t d)
{
	int i,j,k;
	uint8_t c;
	for (i = 0;i < n;i++) {
		k = 1;
		for(j = 0;j < (n - i - 1);j ++)
			k *= 16;
		c = (d / k) % 16;
		if (c >= 10) {
			glcd_PutCharAt(x,y,'A'+c-10);
		} else {
			glcd_PutCharAt(x,y,'0'+c);
		}
		x += 8;
	}
}

void glcd_PutsUint(int32_t d)
{
	int j = 10;
	if (d >= 1000000000) {
		glcd_PutChar(0x8000 | ('0' + (d / 1000000000)));
		d = d % 1000000000;
	}
	while (d >= j) {
		j *= 10;
	}
	j /= 10;
	while (j >= 1) {
		glcd_PutChar(0x8000 | ('0' + (d / j)));
		d = d % j;
		j /= 10;
	}
}

void glcd_PutsUintA(int32_t d)
{
	int j = 10;
	if (d >= 1000000000) {
		glcd_PutCharA('0' + (d / 1000000000));
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
		glcd_PutCharA('0' + (d / j));
		LINEBUF[line_ptr] = ('0' + (d / j)) & 0x0ff;
		line_ptr++;
		line_end++;
		d = d % j;
		j /= 10;
	}
}

void glcd_PutsUint8(uint32_t d)
{
	if (d < 10) {
		glcd_PutChar('0');
		LINEBUF[line_ptr] = '0' & 0x0ff;
		line_ptr++;
		line_end++;
	}
	int j = 10;
	if (d >= 1000000000) {
		glcd_PutChar('0' + (d / 1000000000));
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
		glcd_PutChar('0' + (d / j));
		LINEBUF[line_ptr] = ('0' + (d / j)) & 0x0ff;
		line_ptr++;
		line_end++;
		d = d % j;
		j /= 10;
	}
}

void glcd_PutsUint8A(uint32_t d)
{
	if (d < 10) {
		glcd_PutCharA('0');
		LINEBUF[line_ptr] = '0' & 0x0ff;
		line_ptr++;
		line_end++;
	}
	int j = 10;
	if (d >= 1000000000) {
		glcd_PutCharA('0' + (d / 1000000000));
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
		glcd_PutCharA('0' + (d / j));
		LINEBUF[line_ptr] = ('0' + (d / j)) & 0x0ff;
		line_ptr++;
		line_end++;
		d = d % j;
		j /= 10;
	}
}

void glcd_PutsUint8D(uint32_t d)
{
	if (d < 10) {
		glcd_PutCharA('0');
		LINEBUF[line_ptr] = '0' & 0x0ff;
		line_ptr++;
		line_end++;
	}
	int j = 10;
	if (d >= 1000000000) {
		glcd_PutCharA('0' + (d / 1000000000));
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
		glcd_PutCharA('0' + (d / j));
		if(line_ptr < 50){
			LINEBUF[line_ptr] = ('0' + (d / j)) & 0x0ff;
			line_ptr++;
			line_end++;
		}
		d = d % j;
		j /= 10;
	}
}

void glcd_PutsInt(int32_t d)
{
	if (d < 0) {
		d = -d;
		glcd_PutChar(0x8000 | ('-'));
	}
	int i = 0;
	int j = 10;
	if (d >= 1000000000) {
		glcd_PutChar(0x8000 | ('0' + (d / 1000000000)));
		d = d % 1000000000;
	}
	while (d >= j) {
		j *= 10;
		i++;
	}
	j /= 10;
	while (j >= 1) {
		glcd_PutChar(0x8000 | ('0' + (d / j)));
		d = d % j;
		j /= 10;
	}
}

void glcd_PutsIntD(int32_t d)
{
	if (d < 0) {
		d = -d;
		glcd_PutCharA('-');
		if(line_ptr < 50){
			LINEBUF[line_ptr] = '0' & 0x0ff;
			line_ptr++;
		}
	}
	int i = 0;
	int j = 10;
	if (d >= 1000000000) {
		glcd_PutCharA('0' + (d / 1000000000));
		if(line_ptr < 50){
			LINEBUF[line_ptr] = ('0' + (d / 1000000000)) & 0x0ff;
			line_ptr++;
		}
		d = d % 1000000000;
	}
	while (d >= j) {
		j *= 10;
		i++;
	}
	j /= 10;
	while (j >= 1) {
		glcd_PutCharA('0' + (d / j));
		if(line_ptr < 50){
			LINEBUF[line_ptr] = ('0' + (d / j)) & 0x0ff;
			line_ptr++;
		}
		d = d % j;
		j /= 10;
	}
}

