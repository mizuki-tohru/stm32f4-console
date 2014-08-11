#define MAXLINE 256
#define MAXBASIC 12800

void glcd_BufClear(uint8_t flg);
void glcd_posClear(void);
void glcd_PutChar(uint16_t c);
void glcd_PutCharA(uint16_t c);
void glcd_Puts(const char *buf);
void glcd_PutsA(const char *buf);
void glcd_PutsD(const char *buf);
void glcd_UnPutsA(const char *buf);
void glcd_PutsU(const unsigned char *buf);
void glcd_Puts_Serial(const char *buf);
void glcd_transChar();
void glcd_PutCharAt(uint16_t x,uint16_t y,uint16_t c);
void glcd_PutsAt(uint16_t x,uint16_t y,const char *);
void glcd_Dec(uint16_t x,uint16_t y,int,int,uint8_t);
void glcd_Hex(uint16_t x,uint16_t y,int,int32_t);
void glcd_Vline(uint16_t i,int32_t j);
void glcd_setLine(uint16_t x,uint16_t y,uint8_t d);
void glcd_setDot(uint16_t x,uint16_t y);

void glcd_PutsUint(int32_t d);
void glcd_PutsUint8(uint32_t d);
void glcd_PutsInt(int32_t d);
void glcd_PutsUintA(int32_t d);
void glcd_PutsUint8A(uint32_t d);
void glcd_PutsUint8D(uint32_t d);
void glcd_PutsIntD(int32_t d);

int ltohex(unsigned long l,unsigned char * buf,unsigned char i);
char * ltodeci(long l,char * buf,unsigned char i);
void glcd_clearEditor(void);
void glcd_DrawCursor_first(void);
void glcd_DrawCursor(void);
int glcd_PutEditor(uint16_t);
int rcharnum(char ch);
void insert_basic_line(char *,int);
int glcd_getcursor_ptr(void);

extern uint8_t glcd_x,glcd_y,flgUpdate;
extern int  line_end;
extern int  line_flg;
extern int  line_ptr;

extern char BASICBUF[MAXBASIC];
extern char LINEBUF[128];
extern int end_ptr;

struct char_type{
	char text[128];
	char flg;
	char level;
	long data;
};
