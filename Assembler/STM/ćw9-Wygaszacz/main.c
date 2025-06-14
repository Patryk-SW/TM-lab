// ===================== INCLUDES =====================
#include "main.h"           // Plik nagłówkowy projektu
#include "core_cm4.h"       // Definicje rdzenia ARM Cortex-M4
#include "stdio.h"          // Standardowa biblioteka C

// ===================== DEFINES & GLOBALS =====================
unsigned short int *const LCD_FB = (unsigned short int*) 0xD0000000; // Wskaźnik na bufor ramki LCD
unsigned short int *const LCD_BUF = (unsigned short int*) 0xD0100000; // Wskaźnik na bufor roboczy LCD

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
// Odczyt pozycji joysticka (ADC)
unsigned int GetJoyPos(void) {
    ADC1->CR2 |= ADC_CR2_JSWSTART; // Rozpocznij konwersję ADC1
    ADC3->CR2 |= ADC_CR2_JSWSTART; // Rozpocznij konwersję ADC3
    while (!(ADC1->SR & ADC_SR_JEOC)); // Czekaj na zakończenie konwersji ADC1
    pomiar1 = ADC1->JDR1; // Odczytaj wynik ADC1
    pomiar2 = ADC1->JDR2;
    pomiar3 = ADC1->JDR3;
    pomiar4 = ADC3->JDR1;
    ADC1->SR &= ~ADC_SR_JEOC; // Wyzeruj flagę zakończenia konwersji
    ADC3->SR &= ~ADC_SR_JEOC;
    return pomiar1; // Zwróć wynik
}

// Odczyt stanu przycisku 9
unsigned int GetButton9(void) {
    return GPIOC->IDR & (1 << 11); // Zwraca 1 jeśli przycisk nie wciśnięty, 0 jeśli wciśnięty
}

// Odczyt stanu przycisku 10
unsigned int GetButton10(void) {
    return GPIOC->IDR & (1 << 12); // Zwraca 1 jeśli przycisk nie wciśnięty, 0 jeśli wciśnięty
}

// Wyświetlanie menu na LCD
void WypiszMenu() {
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_Oswietlenie], (uint8_t*) "Oswietlenie", 1);
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_Ogrzewanie], (uint8_t*) "Ogrzewanie", 1);
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_Alarm], (uint8_t*) "Alarm", 1);
    BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[M_oAplikacji], (uint8_t*) "O aplikacji", 1);
}

// Rysowanie podświetlenia wybranej pozycji menu
void RysujPodswietlenie(unsigned int pozycja) {
    for (unsigned int y = pozycja; y < pozycja + 12; y++) {
        for (unsigned int x = 100; x < 220 && x < LCD_WIDTH; x++) {
            LCD_BUF[y * LCD_WIDTH + x] = 0xc0cb; // Kolor podświetlenia
        }
    }
}

// Przyciemnianie ekranu (wygaszacz)
void PrzyciemnijEkran(void) {
    for (int idx = 0; idx < 320 * 240; idx++) {
        LCD_BUF[idx] = (LCD_BUF[idx] & 0xf7de) >> 1; // Operacja na kolorze piksela
    }
}

// Wyświetlanie zegara bezczynności
void Zegar(void) {
    asm ("cpsid i"); // Wyłącz przerwania
    zegar_t l_zegar = g_zegar; // Kopia zegara
    asm ("cpsie i"); // Włącz przerwania
    char zegar_str[40];
    snprintf(zegar_str, sizeof(zegar_str), "Brak aktywnosci %02i:%02i", g_zegar.mins, g_zegar.secs);
    BSP_LCD_DisplayStringAt(320, 220, (uint8_t*)zegar_str, LEFT_MODE); // Wyświetl zegar
}

// ===================== MAIN =====================
int main(void) {
    System_Init(); // Inicjalizacja systemu
    while (1) {
        if (spoczynek > 1) {
            Zegar(); // Wyświetl zegar bezczynności
        }
        switch (AppState) {
            case A_MenuGlowne:
                first_entry = 1; // Pierwsze wejście do menu
                RysujPodswietlenie(PozycjePionoweWpisowMenu[AktualnaPozycjaMenu]); // Podświetl pozycję
                WypiszMenu(); // Wyświetl menu
                if (!(GetButton9())) { // Jeśli przycisk 9 wciśnięty
                    AppState = A_AktywacjaWyboruMenu;
                    MenuActivationTimer = 20;
                    spoczynek = 0;
                    break;
                }
                unsigned int JPos = GetJoyPos(); // Odczytaj joystick
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
                MenuAnimationTimer--; // Odliczanie animacji
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
                GPIOG->MODER |= GPIO_Mode_OUT << (13 * 2); // Ustaw port G13 jako wyjście
                if (first_entry == 1) {
                    GPIOG->BSRR = GPIO_Pin_13; // Włącz LED
                    for (volatile int i = 0; i < 100000; i++); // Krótkie opóźnienie
                    GPIOG->BSRR = GPIO_Pin_13 << 16; // Wyłącz LED
                    first_entry = 0;
                }
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[0], (uint8_t*) "Oswietlenie", 1);
                BSP_LCD_DisplayStringAt(0, PozycjePionoweWpisowMenu[1], (uint8_t*) "Powrot", 1);
                if (!(GetButton10())) { // Jeśli przycisk 10 wciśnięty
                    AppState = A_MenuGlowne;
                    spoczynek = 0;
                }
                while (!(GetButton10())); // Czekaj na puszczenie przycisku
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
                PrzyciemnijEkran(); // Wygaszacz ekranu
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
            AppState = A_Idle_Start; // Przejście do wygaszacza
        }
        Clear_And_Reload_Screen(); // Odśwież ekran
    }
}

// ===================== HANDLERY & INICJALIZACJA =====================
// Obsługa przerwania SysTick (1ms)
void SysTick_Handler(void) {
    HAL_IncTick(); // Zwiększ licznik systemowy
}

// Inicjalizacja systemu
void System_Init() {
    HAL_Init(); // Inicjalizacja HAL
    SystemClock_Config(); // Konfiguracja zegara
    BSP_SDRAM_Init(); // Inicjalizacja SDRAM
    BSP_LCD_Init(); // Inicjalizacja LCD
    // Konfiguracja ADC
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
    // Konfiguracja przycisków
    GPIOC->PUPDR |= 1 << (11 * 2);
    GPIOC->PUPDR |= 1 << (12 * 2);
    // Przerwanie SysTick co 1ms
    SysTick->LOAD = (180000) - 1UL;
    SysTick->CTRL = 7UL;
    *((unsigned long*)(SCB->VTOR + 0x3c)) = (unsigned long)SysTick_IRQ;
}

// Przerwanie SysTick - obsługa zegara i licznika bezczynności
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

// Kopiowanie bufora LCD i czyszczenie bufora roboczego
void Clear_And_Reload_Screen() {
    for (int off = 0; off < 320 * 240; off++) {
        LCD_FB[off] = LCD_BUF[off]; // Przenieś piksele na ekran
        LCD_BUF[off] = 0;           // Wyczyść bufor roboczy
    }
}



