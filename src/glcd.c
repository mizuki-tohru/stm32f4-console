//#ifndef TEST
#include "stm32f4xx.h"
//#endif
#include "hw_config.h"
#include "platform_config.h"
#include "glcd.h"
#ifdef USE_KANJI
#include "libnkf.h"
#endif

#define GLCD_W 50
#define GLCD_H 15

// 400x240ドットのグラフィック液晶を使う。
// フォントが半角 8x16なら、50文字x15行 750文字
// フォントが全角16x16なら、25文字x15行 375文字

uint8_t glcd_buf[50*240];
uint8_t glcd_check[31];/*LCDへ転送するラインを示すフラグ*/

void glcd_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStruct;

	/* GPIOB clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	/* SPI2SCK:PB13 SPI2MOSI:PB15 SPI2_NSS:PB12	*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	/* GPIOB clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);

//	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; // 1.3MHz
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
//	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_LSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
	SPI_Init(SPI2, &SPI_InitStruct);
	SPI_Cmd(SPI2, ENABLE);
}

void glcd_TransFromBuf(void)
{
	int l,x;
	static int m0 = 0;
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	for (l = 1;l <= 240;l++) {
		if(glcd_check[(l>>3)]&(1<<(l&0x07))){
			if (m0) {
				SPI2->DR = 0x03;	// M0 = H M1 = H
				m0 = 0;
			} else {
				SPI2->DR = 0x01;	// M0 = H M1 = L
				m0 = 1;
			}
			while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
			SPI2->DR = l;	// ラインアドレス
			while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
			for (x = 0; x < 50;x++){
				SPI2->DR = glcd_buf[(240*GLCD_W)-(((l-1)*GLCD_W)+x)-1];/* CONSOLE用 */
				while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
			}
			glcd_check[(l>>3)] &= 0xff^(1<<(l&0x07));
		}
	}
	SPI2->DR = 0x00;	// DUMMY
	while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
	SPI2->DR = 0x00;	// DUMMY
	while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}

void glcd_SetPixel_transbuf(uint8_t x, uint8_t y, uint8_t color)
{

	int i;
	if(color &0x01){
		glcd_buf[49 + 50*y - ((399-x)>>3)] &= (0xff^(0x80 >> (x&0x07)));
	}else{
		glcd_buf[49 + 50*y - ((399-x)>>3)] |= (0x80 >> (x&0x07));
	}
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	SPI2->DR = 0x01;	// M0 = H M1 = L M2 = L 表示モード
	while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
	SPI2->DR = 240-y;	// ラインアドレス
	while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
	for (i = 0; i < 50;i++){
		SPI2->DR = glcd_buf[(250*50-1)-((50*y)+i)];/* CONSOLE用 */
		while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
	}
	SPI2->DR = 0x00;	// DUMMY
	while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
	SPI2->DR = 0x00;	// DUMMY
	while( !(SPI2->SR & SPI_I2S_FLAG_TXE) );
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}

void glcd_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
	if(color &0x01){
		glcd_buf[49+(50*y)-((399-x)>>3)] &= (0xff^(0x80 >> (x&0x07)));
	}else{
		glcd_buf[49+(50*y)-((399-x)>>3)] |= (0x80 >> (x&0x07));
	}
	glcd_check[(240-y)>>3] |= 1<<((240-y)&0x07);
}

