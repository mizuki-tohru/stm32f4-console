void glcd_Init(void);
void glcd_TransFromBuf(void);
void glcd_SetPixel_transbuf(uint8_t x, uint8_t y, uint8_t color);
void glcd_SetPixel(uint8_t x, uint8_t y, uint8_t color);

extern uint8_t glcd_buf[50*240];
extern uint8_t glcd_check[31];/*LCD‚Ö“]‘—‚·‚éƒ‰ƒCƒ“‚ðŽ¦‚·ƒtƒ‰ƒO*/


