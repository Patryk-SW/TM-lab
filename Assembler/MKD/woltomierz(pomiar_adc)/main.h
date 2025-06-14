#include <intrins.h>
#include <reg52.h>
#include <intrins.h>
#include "lcd.h"
#include "mkd51sim.h"
#include <string.h>

void WAIT_10US_ASM(unsigned int negative_wait);
void WAIT_10US(unsigned int wait);
void INWERSJAP1_6_ASM(void);
void INWERSJAP1_6(void);
unsigned char PomiarADC();
unsigned int ADCtoVoltage(unsigned char a_Pomiar);
void KonwersjaDoLCDBUF(unsigned int a_Napiecie,unsigned char xdata *a_LCDBUF);
unsigned int LicznikModuloN(unsigned int a_Napiecie);
void STERUJ_DIODY_X0_X5(void);
void TOGGLE_L8_DELAY(unsigned int delay);
void BIN_COUNTER_DIR(void);
void INIT_COUNTER(void);
extern unsigned char GET_KEY_NUM(void);
extern unsigned char GETADC_ASM(unsigned char nr_kanalu);
void Konwersja7Seg(unsigned int a_Napiecie);
extern unsigned char GETPOT(void);