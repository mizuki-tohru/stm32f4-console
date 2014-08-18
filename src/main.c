#include "stm32f4xx.h"
#include "platform_config.h"
#include "hw_config.h"
#include "stm32f4xx_dcmi.h"
#include "glcd.h"
#ifdef USE_KANJI
#include "libnkf.h"
#endif
#include "editor.h"
#include "ubasic.h"

//#ifdef USE_USB
#include "usbd_msc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usb_conf.h"
//#endif

#ifdef USE_MRUBY
#include "mruby.h"
#include "mruby/proc.h"
#endif

//#ifdef USE_USB
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;
//#endif

#define  MORPHY 0
#define  GRAPHIC_TEST 1
#define  ADC_FIFO 400

const int stepY = 28;
const int offsetYC = 16;
const int stepYA = 100;
const int offsetYCA = 7;
const int offsetYH = 16+3;
const int offsetYL = 16+24;

uint8_t fifoa1[ADC_FIFO];
uint8_t fifoa2[ADC_FIFO];
uint16_t fifo_in;

void _exit(int n) {
label:  goto label; /* endless loop */
}

void fifo_clear()
{
	int i;
	fifo_in = 0;
	for (i = 0;i < ADC_FIFO;i++) {
		fifoa1[i] = 0;
		fifoa2[i] = 0;
	}
}

uint8_t getData(void)
{
//	DIN1:PC3 DIN2:PA0 DIN3:PA1 DIN4:PA2
//	DIN5:PA3 DIN6:PA6 DIN7:PA7 DIN8:PC4

	uint8_t pa,pc,r;
	pa = GPIO_ReadInputData(GPIOA);
	pc = GPIO_ReadInputData(GPIOC);
	r = (pa << 1) & 0x1e;
	r |= ((pa >> 1) & 0x60);
	r |= ((pc >> 3) & 0x01);
	r |= ((pc << 3) & 0x80);
	return r;
}

uint16_t divMS = 5;
uint16_t divC = 1;

void getDataHandler(void)
{
	static uint16_t c = 1;
	if(--c) return;
	c = divC;
//	fifo[fifo_in] = getData();
	if (fifo_in >= ADC_FIFO)fifo_in = 0;

	if (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) {
		fifoa1[fifo_in] = 0xff;
	} else {
		fifoa1[fifo_in] = ADC_GetConversionValue(ADC1) & 0x00ff;
		ADC_SoftwareStartConv(ADC1);
	}
	if (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET){
		fifoa2[fifo_in] = 0xff;
	} else {
		fifoa2[fifo_in] = ADC_GetConversionValue(ADC2) & 0x00ff;
		ADC_SoftwareStartConv(ADC2);
	}
	fifo_in++;
	if (fifo_in >= ADC_FIFO)fifo_in = 0;
}


DWORD acc_size;
WORD acc_files, acc_dirs;
FILINFO Finfo;
DIR  dir;
//FATFS Fatfs[_VOLUMES];
FATFS Fatfs;
FIL fileR[2];
// BYTE Buff[BUFSIZE] __attribute__ ((aligned (4))) ;
__IO uint8_t Command_index = 0;
UINT nTextFile; // TXT ファイルの数
FATFS *fs;  /* Pointer to file system object */

#if _USE_LFN
char Lfname[512];
#endif

uint32_t get_fattime (void)
{
  uint32_t res;
  /* See rtc_support.h */
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);  
  RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
  
  res =  (( (uint32_t)ts_year + (2000 - 1980)) << 25)
      | ((uint32_t)(ts_mon) << 21)
      | ((uint32_t)ts_mday << 16)
      | (uint32_t)(ts_hour << 11)
      | (uint32_t)(ts_min << 5)
      | (uint32_t)(ts_sec);
  return res;
}

static void put_rc (FRESULT rc)
{
  const char *str =
    "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
    "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
    "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
    "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
  FRESULT i;

  for (i = 0; i != rc && *str; i++) {
    while (*str++) ;
  }
  glcd_Puts(" rc=");
  glcd_PutsUint((UINT)rc);
  glcd_Puts("FR_");
  glcd_Puts(str);
  glcd_Puts("\r\n");
}

static void put_rcD (FRESULT rc)
{
  const char *str =
    "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
    "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
    "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
    "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
  FRESULT i;

  for (i = 0; i != rc && *str; i++) {
    while (*str++) ;
  }
  glcd_PutsD("FR_");
  glcd_PutsD(str);
  glcd_PutsD("\n");
}

static FRESULT scan_files (
  char* path /* Pointer to the path name working buffer */
)
{
  DIR dirs;
  FRESULT res;
  BYTE i;
  char *fn;
  if ((res = f_opendir(&dirs, path)) == FR_OK) {
    i = strlen(path);
    while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]){
      if (_FS_RPATH && Finfo.fname[0] == '.') continue;
#if _USE_LFN
      fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
      fn = Finfo.fname;
#endif
      if (Finfo.fattrib & AM_DIR) {
        acc_dirs++;
        *(path+i) = '/';
        strcpy(path+i+1, fn);
        res = scan_files(path);
        *(path+i) = '\0';
        if (res != FR_OK) break;
      } else {
        acc_files++;
        acc_size += Finfo.fsize;
      }
    }
  }
  return res;
}

FRESULT initSD()
{
  BYTE res;
  long p2;
  static const BYTE ft[] = {0, 12, 16, 32};

#if _USE_LFN
  Finfo.lfname = Lfname;
  Finfo.lfsize = sizeof(Lfname);
#endif
  f_mount(&Fatfs, "", 1);
//  f_mount(0, &Fatfs[0]);
  glcd_Puts(" _MAX_SS : ");
  glcd_PutsInt(_MAX_SS);
  glcd_Puts("\r\n");
  glcd_Puts(" FatFs module test terminal for ");
  glcd_Puts(MPU_SUBMODEL);
  glcd_Puts("\r\n");
  glcd_Puts(_USE_LFN ? " LFN Enabled" : " LFN Disabled");
  glcd_Puts(", Code page: ");
  glcd_PutsUint(_CODE_PAGE);
  glcd_Puts("\r\n");
  res = f_getfree("", (DWORD*)&p2, &fs);
  if (res) {
    put_rc(res);
  } else {
   	glcd_Puts(" FAT type = FAT");
    glcd_PutsUint(ft[fs->fs_type & 3]);
   	glcd_Puts("\r\n Bytes/Cluster = ");
    glcd_PutsUint((DWORD)fs->csize * 512);
   	glcd_Puts("\r\n Number of FATs = ");
    glcd_PutsUint((WORD)fs->n_fats);
   	glcd_Puts("\r\n");
   	glcd_Puts(" Root DIR entries = ");
    glcd_PutsUint(fs->n_rootdir);
   	glcd_Puts("\r\n Sectors/FAT = ");
    glcd_PutsInt(fs->fsize);
   	glcd_Puts("\r\n Number of clusters ="); 
    glcd_PutsInt((DWORD)fs->n_fatent - 2);
   	glcd_Puts("\r\n");
   	glcd_Puts(" FAT start (lba) = ");
    glcd_PutsInt(fs->fatbase);
   	glcd_Puts("\r\n DIR start (lba,clustor) = ");
    glcd_PutsInt(fs->dirbase);
   	glcd_Puts("\r\n Data start (lba) = ");
    glcd_PutsInt(fs->database);
   	glcd_Puts("\r\n\r\n ...");
    acc_size = acc_files = acc_dirs = 0;
    Finfo.lfname = Lfname;
    Finfo.lfsize = sizeof(Lfname);
    res = scan_files("");
    if (res) {
      put_rc(res);
    } else {
      glcd_Puts("\r\n ");
      glcd_PutsUint(acc_files);
      glcd_Puts(" files,"); 
      glcd_PutsInt(acc_size);
      glcd_Puts("bytes.\r\n ");
      glcd_PutsUint(acc_dirs);
      glcd_Puts(" folders.\r\n ");
      glcd_PutsInt((fs->n_fatent - 2) * (fs->csize / 2));
      glcd_Puts(" KB total disk space.\r\n ");
      glcd_PutsInt(p2 * (fs->csize / 2));
      glcd_Puts(" KB available.\r\n");
    }
  }
  return res;
}


FRESULT lsSDA()
{
	BYTE res;
	DIR dirs;
	res = f_opendir(& dirs, "");
	if (res) {
		put_rcD(res);
	} else {
		for(;;) {
			res = f_readdir(& dirs, &Finfo);
			if ((res != FR_OK) || !Finfo.fname[0]) break;
			glcd_PutsD(" ");
			glcd_PutsD((Finfo.fattrib & AM_DIR) ? ("D"):("-"));
			glcd_PutsD((Finfo.fattrib & AM_RDO) ? ("R"):("-"));
			glcd_PutsD((Finfo.fattrib & AM_HID) ? ("H"):("-"));
			glcd_PutsD((Finfo.fattrib & AM_SYS) ? ("S"):("-"));
			glcd_PutsD((Finfo.fattrib & AM_ARC) ? ("A"):("-"));
			glcd_PutsD(" ");
			glcd_PutsUint8D((Finfo.fdate >> 9) + 1980);
			glcd_PutsD("/");
			glcd_PutsUint8D((Finfo.fdate >> 5) & 15);
			glcd_PutsD("/");
			glcd_PutsUint8D(Finfo.fdate & 31);
			glcd_PutsD(" ");
			glcd_PutsUint8D(Finfo.ftime >> 11);
			glcd_PutsD(":");
			glcd_PutsUint8D((Finfo.ftime >> 5) & 63);
			glcd_PutsD(" ");
			glcd_PutsUint8D(Finfo.fsize);
			glcd_PutsD("  ");
			glcd_PutsD(&(Finfo.fname[0]));
			glcd_PutsD("\n");
		}
		f_closedir(&dirs);
	}
	return res;
}

FRESULT lsSD()
{
  BYTE res;
  long p1, p2;
  UINT s1, s2;
  res = f_getfree("", (DWORD*)&p2, &fs);// testcode
  res = f_opendir(& dir, "");
  if (res) {
    put_rc(res);
  } else {
    p1 = s1 = s2 = 0;
    nTextFile = 0;
    for(;;) {
      res = f_readdir(& dir, &Finfo);
      if ((res != FR_OK) || !Finfo.fname[0]) break;
      if (Finfo.fattrib & AM_DIR) {
        s2++;
      } else {
        s1++; p1 += Finfo.fsize;
        if (isEx(&(Finfo.fname[0]),"TXT")) {
          nTextFile++;
        }
      }
      glcd_Puts(" ");
      glcd_PutChar((Finfo.fattrib & AM_DIR) ? (0x8000|'D'):(0x8000|'-'));
      glcd_PutChar((Finfo.fattrib & AM_RDO) ? (0x8000|'R'):(0x8000|'-'));
      glcd_PutChar((Finfo.fattrib & AM_HID) ? (0x8000|'H'):(0x8000|'-'));
      glcd_PutChar((Finfo.fattrib & AM_SYS) ? (0x8000|'S'):(0x8000|'-'));
      glcd_PutChar((Finfo.fattrib & AM_ARC) ? (0x8000|'A'):(0x8000|'-'));
      glcd_PutChar(0x8000 | ' ');
      glcd_PutsUint((Finfo.fdate >> 9) + 1980);
      glcd_PutChar(0x8000 | '/');
      glcd_PutsUint8((Finfo.fdate >> 5) & 15);
      glcd_PutChar(0x8000 | '/');
      glcd_PutsUint8(Finfo.fdate & 31);
      glcd_PutChar(0x8000 | ' ');
      glcd_PutsUint8(Finfo.ftime >> 11);
      glcd_PutChar(0x8000 | ':');
      glcd_PutsUint8((Finfo.ftime >> 5) & 63);
      glcd_PutChar(0x8000 | ' ');
      glcd_PutsInt(Finfo.fsize);
      glcd_Puts("  ");
      glcd_Puts(&(Finfo.fname[0]));
#if _USE_LFN
      for (p2 = strlen(Finfo.fname); p2 < 14; p2++)
       	glcd_Puts(" ");
      glcd_Puts(Lfname);
      glcd_Puts("\r\n");
#else
      glcd_Puts("\r\n");
#endif
    }
   	glcd_Puts(" "); 
    glcd_PutsUint(s1);
   	glcd_Puts(" File(s),"); 
    glcd_PutsInt(p1);
   	glcd_Puts(" bytes total\r\n ");
    glcd_PutsUint(s2);
   	glcd_Puts("  dir(s)");
    res = f_getfree("", (DWORD*)&p1, &fs);
    if (res == FR_OK) {
     	glcd_Puts(", ");
		if (p1 * fs->csize/2 > (1024 * 1024)) {
		  glcd_PutsInt(p1 * fs->csize /(2048*1-24));
     	  glcd_Puts(" Mbytes free\r\n");
		} else 
		if (p1 * fs->csize/2 > 1024) {
		  glcd_PutsInt(p1 * fs->csize /2048);
     	  glcd_Puts(" k bytes free\r\n");
		} else {
		  glcd_PutsInt(p1 * fs->csize /2);
     	  glcd_Puts(" k bytes free\r\n");
		}
    } else {
      put_rc(res);
    }
   	glcd_Puts(" TXT File ");
    glcd_PutsUint(nTextFile);
   	glcd_Puts("\r\n");
  }
  return res;
}

FIL file;
#define FBUF_SIZE 512		// SDカードへ書き込むためのバッファサイズ
#define RING_BUF_SIZE 8
uint8_t ring_buffer[RING_BUF_SIZE][FBUF_SIZE];
volatile uint8_t ring_buffer_p_in  = 0;
volatile uint8_t ring_buffer_p_out = 0;
UINT pBuf = 0;
UINT flgSDcard = 0;

void putcSD(uint8_t data)
{
	ring_buffer[ring_buffer_p_in][pBuf] = data;
	pBuf++;
	if (pBuf >= FBUF_SIZE) {
		pBuf = 0;
		ring_buffer_p_in ++;
		if (ring_buffer_p_in >= RING_BUF_SIZE) {
			ring_buffer_p_in = 0;
		}
	}
}

void initADC();

uint8_t d[8];
uint8_t f[8];

char input_str[1024];
int input_p,input_k;
#ifdef USE_KANJI
char output_str[1536];
#endif
int flgChangeGLCD = 0;

uint8_t psw;
#ifdef USE_BLUETOOTH_HID
enum eDISP_STATUS { BASIC, ANALOGIN, SERIALIN, BLUEKEYBOARD, CLOCKSETTING };
#else
enum eDISP_STATUS { BASIC, ANALOGIN, SERIALIN, CLOCKSETTING };
#endif
enum eDISP_STATUS disp_status = BASIC;
uint8_t flgTRIG = 1;

uint16_t tmp_year;
uint8_t tmp_mon,tmp_day;
uint8_t tmp_hour,tmp_min,tmp_sec;
uint8_t cs;

#ifdef SERIAL_DEBUG
int serial_debug_c = 0;
#endif

char program[] = "10 cls \n20 print \"MODE BASIC\"\n30 print \"OK\"\n40 end\n";

int main(void)
{
	int i,j,n,t,x;
	uint16_t c;
	UINT fbuflen;
	
#ifdef USE_MRUBY
  mrb_state *mrb = NULL;
  extern const uint8_t appbin[];
#endif
	
//	bv = RTC_ReadBackupRegister(RTC_BKP_DR2);
	/* Set Basis System */
	Set_System();
	/* Set SysTickCounter for _delay_ms(); */
	SysTickInit(INTERVAL);

	initADC();
	fifo_clear();
	glcd_Init();
	glcd_BufClear(1);
	glcd_posClear();
//	conio_init(UART_DEFAULT_NUM,UART_BAUDLATE);
	conio_init(6,115200);/*Bluetooth*/

	input_p = 0;
	input_k = 0;
	flgChangeGLCD = 0;
	RTC_Configuration(0);
	set_RTC(2014,8,17,12,00,0);


	glcd_x = 0;
	glcd_y = 0;
	flgUpdate = 1;
	initSD();

	USBD_Init(&USB_OTG_dev,
            USB_OTG_FS_CORE_ID,
            &USR_desc,
            &USBD_MSC_cb, 
            &USR_cb);
            
	_delay_ms(100);

	glcd_clearEditor();
	glcd_DrawCursor_first();
	ubasic_init(program);

	_delay_ms(100);


#ifdef USE_MRUBY
	/* initialize mruby */
	mrb = mrb_open();
	/* start mruby application */
	mrb_load_irep(mrb, appbin);
	if (mrb->exc) {
		mrb_p(mrb, mrb_obj_value(mrb->exc));
	}
	/* finalize mruby */
	mrb_close(mrb);
#endif


	glcd_TransFromBuf();
	flgSDcard = 0;
	for (i = 0;i < 30;i++) {
		glcd_TransFromBuf();
		if(CmdKey != 0x00)i=30;
		_delay_ms(10);
	}

	/* Main Loop */
	while (1)
	{
		glcd_TransFromBuf();
		if(CmdKey == 0x1B){/*ESC*/
			CmdKey = 0;
			disp_status ++;
			if (disp_status > CLOCKSETTING) {
				disp_status = BASIC;
				glcd_BufClear(1);
				glcd_posClear();
				ubasic_init(program);
			}
			if (disp_status == SERIALIN) {
				glcd_BufClear(1);
				glcd_x = 0;
				glcd_y = 0;
				flgUpdate = 0;
			//	cputs_p(6,"$$$ SM,0 S~,0 R,1\r");
				glcd_PutsD("MODE SERIAL\n");
				_delay_ms(200);
			}
#ifdef USE_BLUETOOTH_HID
			if (disp_status == BLUEKEYBOARD) {
				glcd_BufClear(1);
				glcd_x = 0;
				glcd_y = 0;
				flgUpdate = 0;
				cputs_p(6,"$$$ S~,6 SM,6 SH,0200 R,1\r");
				_delay_ms(200);
				glcd_PutsD("MODE BLUETOOTH KEYBOARD\n");
			}
#endif
			if (disp_status == CLOCKSETTING) {
				RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);  
				RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
				tmp_year=2000+ts_year;
				tmp_mon=ts_mon;
				tmp_day=ts_mday;
				tmp_hour=ts_hour;
				tmp_min=ts_min;
				tmp_sec=ts_sec;
				cs = 0;
			}
		}
		if (disp_status == CLOCKSETTING) {
			if((c = kgetc()) != 0){
				if((c == 'K')||(c == 'k')||(c == KEY_LEFT)){
					cs--;
					if(cs < 0)cs = 5;
				}else if((c == 'L')||(c == 'l')||(c == KEY_RIGHT)){
					cs++;
					if(cs >= 6)cs = 0;
				}
				if((c == 'M')||(c == 'm')||(c == KEY_DOWN)){
					switch (cs) {
						case 0:
							tmp_year --;
							break;
						case 1:
							tmp_mon --;
							if(tmp_mon == 0) {
								tmp_mon = 12;
							}
							break;
						case 2:
							tmp_day --;
							if(tmp_day == 0) {
								tmp_day = 31;
							}
							break;
						case 3:
							if(tmp_hour == 0) {
								tmp_hour =23;
							} else {
								tmp_hour --;
							}
							break;
						case 4:
							if(tmp_min == 0) {
								tmp_min =59;
							} else {
								tmp_min --;
							}
							break;
						case 5:
							if(tmp_sec == 0) {
								tmp_sec =59;
							} else {
								tmp_sec --;
							}
							break;
					}
				}
				if((c == 'I')||(c == 'i')||(c == KEY_UP)){
					switch (cs) {
						case 0:
							tmp_year ++;
							break;
						case 1:
							tmp_mon ++;
							if(tmp_mon == 13) {
								tmp_mon = 1;
							}
							break;
						case 2:
							tmp_day ++;
							if(tmp_day == 32) {
								tmp_day = 1;
							}
							break;
						case 3:
							if(tmp_hour == 23) {
								tmp_hour = 0;
							} else {
								tmp_hour ++;
							}
							break;
						case 4:
							if(tmp_min == 59) {
								tmp_min = 0;
							} else {
								tmp_min ++;
							}
							break;
						case 5:
							if(tmp_sec == 59) {
								tmp_sec = 0;
							} else {
								tmp_sec ++;
							}
							break;
					}
				}
				if(c == '\n'){
					RTC_Configuration(1);
					set_RTC(tmp_year,tmp_mon,tmp_day,tmp_hour,tmp_min,tmp_sec);
				}
			}
		}
		if (disp_status == ANALOGIN) {
			if(CmdKey == 0x05){/*CURSOR*/
				flgTRIG ++;
				if (flgTRIG > 2){
					flgTRIG = 0;
				}
			}
			if(CmdKey == 0x08){/*BS_DEL*/
				switch (divMS) {
					case 5:
						break;
					case 10:
						divMS = 5;
						divC = 1;
						fifo_clear();
						break;
					case 20:
						divMS = 10;
						divC = 2;
						fifo_clear();
						break;
					case 50:
						divMS = 20;
						divC = 4;
						fifo_clear();
						break;
					case 100:
						divMS = 50;
						divC = 10;
						fifo_clear();
						break;
					case 200:
						divMS = 100;
						divC = 20;
						fifo_clear();
						break;
					case 500:
						divMS = 200;
						divC = 40;
						fifo_clear();
						break;
					case 1000:
						divMS = 500;
						divC = 100;
						fifo_clear();
						break;
				}
			}
			if(CmdKey == 0x02){/*CAPS_NUM*/
				switch (divMS) {
					case 5:
						divMS = 10;
						divC = 2;
						fifo_clear();
						break;
					case 10:
						divMS = 20;
						divC = 4;
						fifo_clear();
						break;
					case 20:
						divMS = 50;
						divC = 10;
						fifo_clear();
						break;
					case 50:
						divMS = 100;
						divC = 20;
						fifo_clear();
						break;
					case 100:
						divMS = 200;
						divC = 40;
						fifo_clear();
						break;
					case 200:
						divMS = 500;
						divC = 100;
						fifo_clear();
						break;
					case 500:
						divMS = 1000;
						divC = 200;
						fifo_clear();
						break;
					case 1000:
						break;
				}
			} 
		}
		if ((disp_status != SERIALIN)
#ifdef USE_BLUETOOTH_HID
			&&(disp_status != BLUEKEYBOARD)
#endif
			&&(disp_status != BASIC)) {
			glcd_BufClear(1);
			glcd_posClear();
		}
		switch (disp_status) {
			case BASIC:
				if(ubasic_finished() == 0){
					ubasic_run();
				}else{
					if((c = kgetc()) != 0){
						if(glcd_PutEditor(c)){
						//	glcd_PutsA(LINEBUF);
						//	ubasic_init(LINEBUF);
						}
					}
				}
				break;
			case ANALOGIN:
				for (i = 0;i < 2;i++) {
					glcd_PutCharAt(8,i*stepYA+offsetYCA+stepYA/2,0x8000|('1'+i));
					glcd_PutCharAt(16,i*stepYA+offsetYCA+stepYA/2,0x8000|'c');
					glcd_PutCharAt(24,i*stepYA+offsetYCA+stepYA/2,0x8000|'h');
				}
				j = -divMS;
				for (i = 32;i < 400;i += 50) {
					glcd_Vline(i,j);
					j += divMS;
				}
				if (flgTRIG == 0) {
					t = fifo_in - 400;
					if (t < 0) t += ADC_FIFO;
				} else {
					t = fifo_in - 800;
					if (t < 0) t += ADC_FIFO;
					if (flgTRIG == 1) {
						while ((t != fifo_in) && (fifoa1[t] >= 0x80)) {
							t++;
							if (t >= ADC_FIFO) t = 0;
						}
						while ((t != fifo_in) && (!(fifoa1[t] >= 0x80))) {
							t++;
							if (t >= ADC_FIFO) t = 0;
						}
						if (t == fifo_in) {
							t = fifo_in - 400;
							if (t < 0) t += ADC_FIFO;
						} else {
							t = t - 42;
							if (t < 0) t += ADC_FIFO;
						}
					} else {
						while ((t != fifo_in) && (!(fifoa1[t] >= 0x80))) {
							t++;
							if (t >= ADC_FIFO) t = 0;
						}
						while ((t != fifo_in) && (fifoa1[t] >= 0x80)) {
							t++;
							if (t >= ADC_FIFO) t = 0;
						}
						if (t == fifo_in) {
							t = fifo_in - 400;
							if (t < 0) t += ADC_FIFO;
						} else {
							t = t - 42;
							if (t < 0) t += ADC_FIFO;
						}
					}
				}
				for (x = 40;x < 400;x++) {
					glcd_setDot(x,1*stepYA+offsetYCA-fifoa1[t]/3);
					glcd_setDot(x,2*stepYA+offsetYCA-fifoa2[t]/3);
					t++;
					if (t >= ADC_FIFO) t = 0;
				}
				break;
			case SERIALIN:
#ifdef USE_BLUETOOTH_HID
			case BLUEKEYBOARD:
#endif
				if((c = kgetc()) != 0){
					if(c == 0x1B){/*ESC*/
						
					}else{
						glcd_PutChar(c | 0x8000);
				//		putch_p(6,c);/*Send Bluetooth*/
						putch_p(1,c);/*Send UART1*/
						if(c == 0x0d){
							glcd_PutChar(0x0a | 0x8000);
				//			putch_p(6,0x0a);/*Send Bluetooth*/
							putch_p(1,0x0a);/*Send UART1*/
						}
					}
				}
			//	if((c = getch_p16(6)) != 0xffff){/*Rcv Bluetooth*/
				if((c = getch_p16(1)) != 0xffff){/*Rcv UART1*/
					glcd_PutChar(c | 0x8000);
				}
				break;
			case CLOCKSETTING:
				glcd_PutsAt(48,70,"Clock setting");
				RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);  
				RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
				glcd_Dec(120,94,4,2000+ts_year,1);
				glcd_PutCharAt(120+8*4,94,0x8000|'/');
				glcd_Dec(120+8*5,94,2,ts_mon,1);
				glcd_PutCharAt(120+8*7,94,0x8000|'/');
				glcd_Dec(120+8*8,94,2,ts_mday,1);
				glcd_PutCharAt(120+8*10,94,0x8000|' ');
				glcd_Dec(120+8*11,94,2,ts_hour,1);
				glcd_PutCharAt(120+8*13,94,0x8000|':');
				glcd_Dec(120+8*14,94,2,ts_min,1);
				glcd_PutCharAt(120+8*16,94,0x8000|':');
				glcd_Dec(120+8*17,94,2,ts_sec,1);
				glcd_Dec(120,110,4,tmp_year,cs != 0);
				glcd_PutCharAt(120+8*4,110,0x8000|'/');
				glcd_Dec(120+8*5,110,2,tmp_mon,cs != 1);
				glcd_PutCharAt(120+8*7,110,0x8000|'/');
				glcd_Dec(120+8*8,110,2,tmp_day,cs != 2);
				glcd_PutCharAt(120+8*10,110,0x8000|' ');
				glcd_Dec(120+8*11,110,2,tmp_hour,cs != 3);
				glcd_PutCharAt(120+8*13,110,0x8000|':');
				glcd_Dec(120+8*14,110,2,tmp_min,cs != 4);
				glcd_PutCharAt(120+8*16,110,0x8000|':');
				glcd_Dec(120+8*17,110,2,tmp_sec,cs != 5);
			default:
				break;
		}
		input_k++;
		while((input_p < 1000) && ((c = getch_p16(6)) != 0xffff)){
			if (flgChangeGLCD == 0) flgChangeGLCD = 2;
			putcSD(c);
			input_str[input_p++] = c;
			if ((c == '\r') || (c == '\n')) {
				if (disp_status == SERIALIN) {
					// UNIX系 は    LFのみ
					// WINDOWS系 は CR LF
					// MAC系 は     CRのみ
					// LF: 0x0A \n Control-J
					// CR: 0x0D \r Control-M
					if (c == '\n') { // UNIX用の修正
						input_str[input_p-1] = '\r';
						input_str[input_p++] = '\n';
					}
					input_str[input_p++] = 0;
#ifdef USE_KANJI
					nkf(input_str, output_str, 1024, "-jW");
					glcd_Puts_Serial(output_str);
#else
					glcd_Puts_Serial(input_str);
#endif
				}
				input_p = 0;
			}
			input_k = 0;
		}
		if (disp_status == SERIALIN) {
			if ((input_p > 0) && (2 < input_k)) {
				input_str[input_p++] = 0;
#ifdef USE_KANJI
				nkf(input_str, output_str, 1024, "-jW");
				glcd_Puts_Serial(output_str);
#else
				glcd_Puts_Serial(input_str);
#endif
				input_p = 0;
				glcd_transChar();
				flgChangeGLCD = 0;
			}
			if((flgChangeGLCD) && (2 < input_k)) {
				glcd_transChar();
				flgChangeGLCD = 0;
			} else if(flgChangeGLCD == 1) {
				glcd_transChar();
				flgChangeGLCD = 0;
			} else if(flgChangeGLCD > 0) {
				flgChangeGLCD--;
			}
		}

		if (input_p >= 1000)
			input_p = 0;

		if (flgSDcard) {
			if (input_k > 100) { // 10 秒以上シリアル入力が無い時
				while(pBuf) putcSD('\n');
				input_k = 0;
			}
			n = ring_buffer_p_in - ring_buffer_p_out;
			if (n < 0) {
				n += RING_BUF_SIZE;
			}
			if (((input_k > 0) && (n > 0)) || (n > (RING_BUF_SIZE - 2))) {

				f_open(&file,Lfname,FA_READ|FA_WRITE);
				f_lseek(&file,f_size(&file));
				while (n > 0) {
					if (ring_buffer_p_out + n < RING_BUF_SIZE) {
						f_write (&file,ring_buffer[ring_buffer_p_out],n * FBUF_SIZE,&fbuflen);
						ring_buffer_p_out += n;
						n = 0;
					} else {
						f_write (&file,ring_buffer[ring_buffer_p_out],(RING_BUF_SIZE - ring_buffer_p_out) * FBUF_SIZE,&fbuflen);
						n -= (RING_BUF_SIZE - ring_buffer_p_out);
						ring_buffer_p_out = 0;	
					}
				}
				f_close(&file);
			}
		}
	}
}		

// ADC Init
// ADC1 :PC0/ADC123_IN10
// ADC2 :PC1/ADC123_IN11
// ADC3 :PC2/ADC123_IN12

void initADC()
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	ADC_InitTypeDef		ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	ADC_DeInit();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_8b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
//	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Cmd(ADC1, ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_3Cycles);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_8b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
//	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC2, &ADC_InitStructure);
	ADC_Cmd(ADC2, ENABLE);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_11, 1, ADC_SampleTime_3Cycles);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_8b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
//	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC3, &ADC_InitStructure);
	ADC_Cmd(ADC3, ENABLE);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);
	ADC_SoftwareStartConv(ADC1);
	ADC_SoftwareStartConv(ADC2);
	ADC_SoftwareStartConv(ADC3);
}

