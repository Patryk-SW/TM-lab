// ===================== INCLUDES =====================
#include <stdint.h>
#include <stdio.h>
#include "stm32f429xx.h"
#include "stm32f4xx_gpio.h"

// ===================== DEFINES =====================
#define LCD_WIDTH 320U
#define LCD_HEIGHT 240U
#define LCD_LAST_X 318U
#define LCD_LAST_Y 239U

#define MAX_R 31U
#define MAX_G 63U
#define MAX_B 31U

// ===================== STRUCTS =====================
typedef struct {
    long long free_milis;
    int secs;
    int mins;
} zegar_t;

typedef struct {
    unsigned int x, y;
} __attribute__((packed,aligned(4))) kwadrat_t;

// ===================== ENUMS =====================
typedef enum {
    CENTER_MODE = 0x01, /* center mode */
    RIGHT_MODE  = 0x02, /* right mode  */
    LEFT_MODE   = 0x03  /* left mode   */
} Text_AlignModeTypdef;

typedef enum {
    HAL_OK      = 0x00U,
    HAL_ERROR   = 0x01U,
    HAL_BUSY    = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

// ===================== FUNCTION PROTOTYPES =====================
unsigned int GetButton9(void);
unsigned int GetButton10(void);
void RysujPodswietlenie(unsigned int pozycja);
void WypiszMenu(void);
void PrzyciemnijEkran(void);
void Zegar(void);
void SysTick_IRQ(void);
unsigned int GetJoyPos(void);
void System_Init(void);
HAL_StatusTypeDef HAL_Init(void);
void SystemClock_Config(void);
void Clear_And_Reload_Screen(void);
void drawElements(void);
void printMenu(void);
void BSP_SDRAM_Init(void);
void BSP_LCD_Init(void);
void BSP_LCD_DisplayStringAt(uint16_t X, uint16_t Y, uint8_t *pText, Text_AlignModeTypdef mode);
void BSP_LCD_DisplayRot90StringAt(uint16_t X, uint16_t Y, uint8_t *pText, Text_AlignModeTypdef mode);

void move_square_asm(void);
void kopiuj_blok_pamieci(unsigned short int* FROM,unsigned short int* INTO,int SIZE);
void kasuj_blok_pamieci(unsigned short int* WHERE,int SIZE);
unsigned short int RGB16Pack(unsigned int B,unsigned int G,unsigned int R);
void fillMemory(void* adres_bazowy, unsigned long rozmiar,unsigned short int wartosc);
void SysTick_Handler(void);
void HAL_IncTick(void);
