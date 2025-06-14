// ===================== INCLUDES =====================
#include "main.h"
#include "stdio.h"

// ===================== DEFINES & ENUMS =====================
unsigned short int *const LCD_FB = (unsigned short int*) 0xD0000000;
unsigned short int *const LCD_BUF = (unsigned short int*) 0xD0100000;

enum AppState_t {
    A_MenuGlowne,
    A_AnimPodswietleniaMenu,
    A_AktywacjaWyboruMenu,
    A_Oswietlenie,
    A_Ogrzewanie,
    A_Alarm,
    A_oAplikacji,
    A_Idle_Start,
    A_Idle_Stay
};

enum AppState_t AppState = A_MenuGlowne;

enum PozycjeMenu_t {
    M_Oswietlenie = 0,
    M_Ogrzewanie = 1,
    M_Alarm = 2,
    M_oAplikacji = 3
};

unsigned short int PozycjePionoweWpisowMenu[] = {20, 50, 80, 110};
enum PozycjeMenu_t KolejnePozycjeMenu[] = {M_Ogrzewanie, M_Alarm, M_oAplikacji, M_oAplikacji};
enum PozycjeMenu_t PoprzedniePozycjeMenu[] = {M_Oswietlenie, M_Oswietlenie, M_Ogrzewanie, M_Alarm};
enum PozycjeMenu_t AktualnaPozycjaMenu = M_Alarm;
enum PozycjeMenu_t KolejnaPozycjaMenu;

unsigned int MenuActivationTimer;
unsigned int MenuAnimationTimer;
int spoczynek = 0;

extern void MEMORY_RANGE_RGB_SET(int RGB, uint16_t* MEM, int SIZE);
//unsigned int pomiar;

// ===================== FUNKCJE POMOCNICZE =====================

unsigned int GetJoyPos(void) {
    // 1. Automat skończony
    /*
    enum ADC_SEQ_t {S_ADC_START, S_ADC_MEASURE, S_ADC_RESULT};
    static enum ADC_SEQ_t M_ADC_SEQ=S_ADC_START;
    static unsigned int l_ADC=0xffffffff;
    ADC1->CR2=ADC_CR2_ADON;// włączenie ADC1
    switch(M_ADC_SEQ) {
        case S_ADC_START:
            ADC1->CR2|=ADC_CR2_SWSTART;// start pomiaru
            M_ADC_SEQ = S_ADC_MEASURE;
            break;
        case S_ADC_MEASURE:
            while(!(ADC1->SR&ADC_SR_EOC));// oczekiwanie na zakończenie pomiaru
            M_ADC_SEQ = S_ADC_RESULT;
            //POLLING
            break;
        case S_ADC_RESULT:
            pomiar=ADC1->DR; // odczyt wyniku pomiaru
            M_ADC_SEQ = S_ADC_START;
            break;
    }
    */
    // 2. Prosty pomiar z oczekiwaniem na wynik
    static unsigned int l_ADC = 0;
    // a. ADC START
    ADC1->CR2 |= ADC_CR2_SWSTART;
    // b. ADC CZEKAJ NA ZAKOŃCZENIE POMIARU
    while (!(ADC1->SR & ADC_SR_EOC));
    // c. ADC POBIERZ POMIAR
    l_ADC = ADC1->DR;
    return l_ADC;
}

unsigned int GetButton9(void) {
    if (!(GPIOC->IDR & 0b100000000000))
        return 1;
    else
        return 0;
}

unsigned int GetButton10(void) {
    if (!(GPIOC->IDR & 0b1000000000000))
        return 1;
    else
        return 0;
}

void WypiszMenu() {
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_Oswietlenie], (uint8_t*) "Oswietlenie", 1);
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_Ogrzewanie], (uint8_t*) "Ogrzewanie", 1);
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_Alarm], (uint8_t*) "M_Alarm", 1);
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_oAplikacji], (uint8_t*) "M_oAplikacji", 1);
}

void RysujPodswietlenie(unsigned int pozycja) {
    uint16_t* start = LCD_BUF + pozycja * LCD_WIDTH + 10;
    int width = LCD_WIDTH - 20;
    for (int i = 0; i < 12; i++) {
        MEMORY_RANGE_RGB_SET(0x07E0, start, width);
        start += LCD_WIDTH;
    }
}

// ===================== MAIN =====================

int main(void) {
    System_Init();
    int wasScreenOff = 0;
    while (1) {
        if (GetButton9())
            GPIOG->BSRR = GPIO_Pin_14;
        else
            GPIOG->BSRR = GPIO_Pin_14 << 16;

        switch (AppState) {
            case A_MenuGlowne:
                RysujPodswietlenie(PozycjePionoweWpisowMenu[AktualnaPozycjaMenu]);
                WypiszMenu();
                if (GetButton9()) {
                    AppState = A_AktywacjaWyboruMenu;
                    MenuActivationTimer = 20;
                    spoczynek = 0;
                    break;
                }
                unsigned int JPos = GetJoyPos();
                if (JPos > 3000) {
                    MenuAnimationTimer = 10;
                    AppState = A_AnimPodswietleniaMenu;
                    KolejnaPozycjaMenu = KolejnePozycjeMenu[AktualnaPozycjaMenu];
                    spoczynek = 0;
                }
                if (JPos < 1000) {
                    MenuAnimationTimer = 10;
                    AppState = A_AnimPodswietleniaMenu;
                    KolejnaPozycjaMenu = PoprzedniePozycjeMenu[AktualnaPozycjaMenu];
                    spoczynek = 0;
                }
                break;
            case A_AnimPodswietleniaMenu:
                MenuAnimationTimer--;
                RysujPodswietlenie(PozycjePionoweWpisowMenu[AktualnaPozycjaMenu]);
                WypiszMenu();
                if (MenuAnimationTimer == 0) {
                    AktualnaPozycjaMenu = KolejnaPozycjaMenu;
                    AppState = A_MenuGlowne;
                }
                spoczynek = 0;
                break;
            case A_AktywacjaWyboruMenu:
                MenuActivationTimer--;
                if (MenuActivationTimer == 0) {
                    switch (AktualnaPozycjaMenu) {
                        case M_Oswietlenie:
                            AppState = A_Oswietlenie;
                            break;
                        case M_Ogrzewanie:
                            AppState = A_Ogrzewanie;
                            break;
                        case M_Alarm:
                            AppState = A_Alarm;
                            break;
                        case M_oAplikacji:
                            AppState = A_oAplikacji;
                            break;
                    }
                }
                spoczynek = 0;
                break;
            case A_Oswietlenie:
                if (GetButton10()) {
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                BSP_LCD_DisplayStringAt(0, 40, (uint8_t*) "Oswietlenie aktywne", 1);
                break;
            case A_Ogrzewanie:
                if (GetButton10()) {
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                BSP_LCD_DisplayStringAt(0, 40, (uint8_t*) "Ogrzewanie aktywne", 1);
                break;
            case A_Alarm:
                if (GetButton10()) {
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                BSP_LCD_DisplayStringAt(0, 40, (uint8_t*) "Alarmy aktywny", 1);
                break;
            case A_oAplikacji:
                if (GetButton10()) {
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                BSP_LCD_DisplayStringAt(0, 40, (uint8_t*) "Tak slucham, hej mercedes", 1);
                break;
            default:
                AppState = A_MenuGlowne;
        }
        // Jeżeli ekran ponownie się zapali (wznawiamy działanie)
        Clear_And_Reload_Screen();
    }
}

// ===================== HANDLERY & INICJALIZACJA =====================

void SysTick_Handler(void) {
    HAL_IncTick();
}

void System_Init() {
    HAL_Init();
    SystemClock_Config();
    BSP_SDRAM_Init();
    BSP_LCD_Init();
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    GPIOG->MODER |= GPIO_Mode_OUT << (14*2);// 1 - output
    GPIOG->OTYPER|= GPIO_OType_PP << 14;//0 - push-pull (def.)
    GPIOG->PUPDR |= GPIO_PuPd_NOPULL << (14*2);//0 - (default)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    GPIOC->MODER |= GPIO_Mode_IN << (11*2); // 0 - input (default)
    GPIOC->PUPDR |= GPIO_PuPd_UP << (11*2); // 1
    GPIOC->MODER |= GPIO_Mode_IN << (12*2); // 0 - input (default)
    GPIOC->PUPDR |= GPIO_PuPd_UP << (12*2); // 1
    GPIOA->MODER |= GPIO_Mode_AN << (5*2);
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    ADC1->CR2=ADC_CR2_ADON;// włączenie ADC1
    ADC1 -> SQR3 = 5;
}

void Clear_And_Reload_Screen() {
    for (int off = 0; off < LCD_WIDTH*LCD_HEIGHT; off++) {
        LCD_FB[off] = LCD_BUF[off];
        LCD_BUF[off] = 0;
    }
}

