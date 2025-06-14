// ===================== INCLUDES =====================
#include "main.h"
#include "core_cm4.h" // Plik nagłówkowy specyficzny dla rdzenia ARM Cortex-M4
#include "stdio.h"

// ===================== DEFINES & GLOBALS =====================
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
enum AppState_t AppState = A_MenuGlowne; // Deklaracja zmiennej typu AppState_t o nazwie AppState, która przechowuje obecny stan aplikacji
// Domyślny stan: menu główne

enum PozycjeMenu_t {
    M_Oswietlenie = 0,
    M_Ogrzewanie = 1,
    M_Alarm = 2,
    M_oAplikacji = 3
};
unsigned short int PozycjePionoweWpisowMenu[] = {30, 60, 90, 120};
enum PozycjeMenu_t KolejnePozycjeMenu[] = {M_Ogrzewanie, M_Alarm, M_oAplikacji, M_oAplikacji};
enum PozycjeMenu_t PoprzedniePozycjeMenu[] = {M_Oswietlenie, M_Oswietlenie, M_Ogrzewanie, M_Alarm};
enum PozycjeMenu_t AktualnaPozycjaMenu = M_Alarm;
enum PozycjeMenu_t KolejnaPozycjaMenu;
unsigned int MenuActivationTimer;
unsigned int MenuAnimationTimer;
unsigned int first_entry;
unsigned int pomiar1, pomiar2, pomiar3, pomiar4;
unsigned int spoczynek = 0;
zegar_t g_zegar = {0, 0, 0};

// ===================== FUNKCJE POMOCNICZE =====================
unsigned int GetJoyPos(void) {
    // a. ADC START
    ADC1->CR2 |= ADC_CR2_JSWSTART;
    ADC3->CR2 |= ADC_CR2_JSWSTART;
    // b. ADC CZEKAJ NA ZAKOŃCZENIE POMIARU
    while (!(ADC1->SR & ADC_SR_JEOC));
    // c. ADC POBIERZ POMIAR
    pomiar1 = ADC1->JDR1;
    pomiar2 = ADC1->JDR2;
    pomiar3 = ADC1->JDR3;
    pomiar4 = ADC3->JDR1;
    ADC1->SR &= ~ADC_SR_JEOC;
    ADC3->SR &= ~ADC_SR_JEOC;
    return pomiar1;
}

unsigned int GetButton9(void) {
    return GPIOC->IDR & (1 << 11);
}

unsigned int GetButton10(void) {
    return GPIOC->IDR & (1 << 12);
}

void WypiszMenu() {
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_Oswietlenie], (uint8_t*) "Oswietlenie", 1);
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_Ogrzewanie], (uint8_t*) "Ogrzewanie", 1);
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_Alarm], (uint8_t*) "Alarm", 1);
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_oAplikacji], (uint8_t*) "O aplikacji", 1);
}

void RysujPodswietlenie(unsigned int pozycja) {
    for (unsigned int y = pozycja; y < pozycja + 12; y++) {
        for (unsigned int x = 100; x < 220 && x < LCD_WIDTH; x++) {
            LCD_BUF[y * LCD_WIDTH + x] = 0xc0cb;
        }
    }
}

void PrzyciemnijEkran(void) {
    for (int idx = 0; idx < 320 * 240; idx++) {
        LCD_BUF[idx] = (LCD_BUF[idx] & 0xf7de) >> 1;
    }
}

void Zegar(void) {
    asm ("cpsid i");
    zegar_t l_zegar = g_zegar;
    asm ("cpsie i");
    char zegar_str[40];
    snprintf(zegar_str, sizeof(zegar_str), "Brak aktywnosci %02i:%02i", g_zegar.mins, g_zegar.secs);
    BSP_LCD_DisplayStringAt(320, 220, (uint8_t*)zegar_str, LEFT_MODE);
}

// ===================== MAIN =====================
int main(void) {
    System_Init();
    while (1) {
        if (spoczynek > 1) {
            Zegar();
        }
        switch (AppState) {
            case A_MenuGlowne:
                first_entry = 1;
                RysujPodswietlenie(PozycjePionoweWpisowMenu[AktualnaPozycjaMenu]);
                WypiszMenu();
                if (!(GetButton9())) {
                    AppState = A_AktywacjaWyboruMenu;
                    MenuActivationTimer = 20;
                    spoczynek = 0;
                    break;
                }
                unsigned int JPos = GetJoyPos();
                if (JPos > 3000) {
                    MenuAnimationTimer = 5;
                    AppState = A_AnimPodswietleniaMenu;
                    KolejnaPozycjaMenu = KolejnePozycjeMenu[AktualnaPozycjaMenu];
                }
                if (JPos < 1000) {
                    MenuAnimationTimer = 5;
                    AppState = A_AnimPodswietleniaMenu;
                    KolejnaPozycjaMenu = PoprzedniePozycjeMenu[AktualnaPozycjaMenu];
                }
                spoczynek = 0;
                g_zegar.free_milis = 0;
                g_zegar.secs = 0;
                g_zegar.mins = 0;
                break;
            case A_AnimPodswietleniaMenu:
                MenuAnimationTimer--;
                RysujPodswietlenie(PozycjePionoweWpisowMenu[AktualnaPozycjaMenu]);
                WypiszMenu();
                if (MenuAnimationTimer == 0) {
                    AktualnaPozycjaMenu = KolejnaPozycjaMenu;
                    AppState = A_MenuGlowne;
                }
                break;
            case A_AktywacjaWyboruMenu:
                MenuActivationTimer--;
                if (MenuActivationTimer == 0) {
                    switch (AktualnaPozycjaMenu) {
                        case M_Oswietlenie: AppState = A_Oswietlenie; break;
                        case M_Ogrzewanie: AppState = A_Ogrzewanie; break;
                        case M_Alarm: AppState = A_Alarm; break;
                        case M_oAplikacji: AppState = A_oAplikacji; break;
                    }
                }
                spoczynek = 0;
                break;
            case A_Oswietlenie:
                GPIOG->MODER |= GPIO_Mode_OUT << (13 * 2);
                if (first_entry == 1) {
                    GPIOG->BSRR = GPIO_Pin_13;
                    for (volatile int i = 0; i < 100000; i++);
                    GPIOG->BSRR = GPIO_Pin_13 << 16;
                    first_entry = 0;
                }
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[0], (uint8_t*) "Oswietlenie", 1);
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[1], (uint8_t*) "Powrot", 1);
                if (!(GetButton10())) {
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                while (!(GetButton10()));
                break;
            case A_Ogrzewanie:
                GPIOG->MODER |= GPIO_Mode_OUT << (13 * 2);
                if (first_entry == 1) {
                    GPIOG->BSRR = GPIO_Pin_13;
                    for (volatile int i = 0; i < 100000; i++);
                    GPIOG->BSRR = GPIO_Pin_13 << 16;
                    first_entry = 0;
                }
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[0], (uint8_t*) "Ogrzewanie", 1);
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[1], (uint8_t*) "Powrot", 1);
                if (!(GetButton10())) {
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                while (!(GetButton10()));
                break;
            case A_Alarm:
                GPIOG->MODER |= GPIO_Mode_OUT << (13 * 2);
                if (first_entry == 1) {
                    GPIOG->BSRR = GPIO_Pin_13;
                    for (volatile int i = 0; i < 10000; i++);
                    GPIOG->BSRR = GPIO_Pin_13 << 16;
                    first_entry = 0;
                }
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[0], (uint8_t*) "Alarm", 1);
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[1], (uint8_t*) "Powrot", 1);
                if (!(GetButton10())) {
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                while (!(GetButton10()));
                break;
            case A_oAplikacji:
                GPIOG->MODER |= GPIO_Mode_OUT << (13 * 2);
                if (first_entry == 1) {
                    GPIOG->BSRR = GPIO_Pin_13;
                    for (volatile int i = 0; i < 10000; i++);
                    GPIOG->BSRR = GPIO_Pin_13 << 16;
                    first_entry = 0;
                }
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[0], (uint8_t*) "o Aplikacji", 1);
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[1], (uint8_t*) "Powrot", 1);
                if (!(GetButton10())) {
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                while (!(GetButton10()));
                break;
            case A_Idle_Start:
                PrzyciemnijEkran();
                AppState = A_Idle_Stay;
                break;
            case A_Idle_Stay:
                if (GetButton9() || GetButton10() || GetJoyPos() > 3000 || GetJoyPos() < 1000) {
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                break;
            default:
                AppState = A_MenuGlowne;
        }
        if (spoczynek >= 3000) {
            AppState = A_Idle_Start;
        }
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
    // ADC
    GPIOA->MODER |= GPIO_Mode_AN << (5 * 2);
    GPIOA->MODER |= GPIO_Mode_AN << (7 * 2);
    GPIOC->MODER |= GPIO_Mode_AN << (3 * 2);
    GPIOF->MODER |= GPIO_Mode_AN << (6 * 2);
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
    ADC1->CR2 = ADC_CR2_ADON;
    ADC3->CR2 = ADC_CR2_ADON;
    ADC1->JSQR = 2 << 4 * 5 | 13 << 3 * 5 | 5 << 2 * 5 | 7 << 1 * 5;
    ADC3->JSQR = 4 << 3 * 5;
    ADC1->CR1 |= ADC_CR1_SCAN;
    RCC->AHB1ENR |= 1 << 2;
    // Przyciski
    GPIOC->PUPDR |= 1 << (11 * 2);
    GPIOC->PUPDR |= 1 << (12 * 2);
    // Obsługa przerwania z okresem 1ms
    SysTick->LOAD = (180000) - 1UL;
    SysTick->CTRL = 7UL;
    *((unsigned long*)(SCB->VTOR + 0x3c)) = (unsigned long)SysTick_IRQ;
}

void SysTick_IRQ() {
    g_zegar.free_milis++;
    if (g_zegar.free_milis == 1000) {
        g_zegar.free_milis = 0;
        g_zegar.secs++;
    }
    if (g_zegar.secs == 60) {
        g_zegar.secs = 0;
        g_zegar.mins++;
    }
    if (g_zegar.mins == 60) {
        g_zegar.mins = 0;
    }
    spoczynek++;
}

void Clear_And_Reload_Screen() {
    for (int off = 0; off < 320 * 240; off++) {
        LCD_FB[off] = LCD_BUF[off];
        LCD_BUF[off] = 0;
    }
}



