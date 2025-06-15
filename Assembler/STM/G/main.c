#include "main.h"

// Wskaźniki na bufory LCD
unsigned short int *const LCD_FB = (unsigned short int*) 0xD0000000; // Bufor ramki LCD
unsigned short int *const LCD_BUF = (unsigned short int*) 0xD0100000; // Bufor roboczy LCD

// Menu - napisy do wyświetlenia
char menu1[] = "1. ABC";
char menu2[] = "2. DEF";
char menu3[] = "3. Nie ChCE";
char menu4[] = "4. MiSiE";
char *menu[] = {menu1, menu2, menu3, menu4}; // Tablica wskaźników na napisy menu
uint32_t Y[] = {20, 40, 60, 80}; // Pozycje Y dla kolejnych pozycji menu
uint8_t k = 0; // Indeks aktualnie wybranej pozycji menu
uint32_t licznik = 0; // Licznik pomocniczy

// Definicja stanów aplikacji
typedef enum {
    menuu,      // Menu główne
    animacja,   // Animacja przejścia
    opcja1,     // Opcja 1
    opcja2,     // Opcja 2
    opcja3,     // Opcja 3
    opcja4      // Opcja 4
} stany_aplikacji;

stany_aplikacji stan_aplikacji[] = {menuu, animacja, opcja1, opcja2, opcja3, opcja4};
stany_aplikacji stany = menuu; // Aktualny stan aplikacji
stany_aplikacji kafelek[] = {opcja1, opcja2, opcja3, opcja4}; // Mapowanie wyboru na opcje
stany_aplikacji kaf = opcja1;

// Deklaracje funkcji zewnętrznych
//extern void RysujPodswietlenie(int pozycja, unsigned short int kolor);
extern void mruganie(void);
extern void RysujRamke(unsigned int y0, unsigned int y1, unsigned int x0, unsigned int x1);

// Konfiguracja przycisków
void ButtonSetup(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // Włącz zegar portu C
    GPIOC->MODER &= ~(3 << 11 * 2);      // Ustaw wejście na pinie 11
    GPIOC->MODER |= GPIO_Mode_IN << (11 * 2);
    GPIOC->PUPDR &= ~(3 << (11 * 2));    // Wyzeruj pull-up/down
    GPIOC->PUPDR |= GPIO_PuPd_UP << (11 * 2); // Pull-up na pinie 11
    GPIOC->MODER &= ~(3 << 12 * 2);      // Ustaw wejście na pinie 12
    GPIOC->MODER |= GPIO_Mode_IN << (12 * 2);
    GPIOC->PUPDR &= ~(3 << (12 * 2));    // Wyzeruj pull-up/down
    GPIOC->PUPDR |= GPIO_PuPd_UP << (12 * 2); // Pull-up na pinie 12+
}

// Konfiguracja diody LED
void LedSetup(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN; // Włącz zegar portu G
    GPIOG->MODER &= ~(3 << 14 * 2);      // Ustaw wyjście na pinie 14
    GPIOG->MODER |= (GPIO_Mode_OUT << 14 * 2);
    GPIOG->OTYPER &= ~(1 << 14);         // Push-pull
    GPIOG->OTYPER |= GPIO_OType_PP;
    GPIOG->PUPDR &= ~(3 << 14 * 2);      // Brak pull-up/down
    GPIOG->PUPDR |= (GPIO_PuPd_NOPULL << 14 * 2);
}

// Sprawdzenie stanu przycisku (1 = nie wciśnięty, 0 = wciśnięty)
uint16_t CheckButton(uint16_t pin) {
    return GPIOC->IDR & (1 << pin);
}

// Ustawienie stanu diody LED (1 = zapal, 0 = zgaś)
void SetLed(uint16_t stan) {
    if (stan) {
        GPIOG->BSRR = 1 << 14;           // Zapal LED
    } else {
        GPIOG->BSRR = 1 << (14 + 16);    // Zgaś LED
    }
}

// Konfiguracja ADC (joystick)
void ADC_Setup(void) {
    GPIOF->MODER &= ~(3 << 6 * 2);       // Ustaw wejście analogowe na pinie PF6
    GPIOF->MODER |= (GPIO_Mode_AN << 6 * 2);
    RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;  // Włącz zegar ADC3
    ADC3->CR2 = ADC_CR2_ADON;            // Włącz ADC3
    ADC3->JSQR = 4 << (3 * 5);           // Konfiguracja sekwencji ADC3
}

// Odczyt wartości z joysticka (ADC)
uint32_t Get_Joy(void) {
    ADC3->CR2 |= ADC_CR2_JSWSTART;       // Rozpocznij konwersję
    while (!(ADC3->SR & ADC_SR_JEOC));   // Czekaj na zakończenie
    uint32_t pomiar = ADC3->JDR1;        // Odczytaj wynik
    ADC3->SR &= ~(ADC_SR_JEOC);          // Wyzeruj flagę zakończenia
    return pomiar;
}

// Wyświetlenie menu na LCD
void WypiszMenu(void) {
    for (uint16_t i = 0; i < 4; i++) {
        BSP_LCD_DisplayStringAt(20, Y[i], (uint8_t*)menu[i], 0); // Wyświetl kolejną pozycję menu
    }
}

/*
// Funkcja rysująca podświetlenie (prototyp, zakomentowana)
void RysujPodswietlenie(unsigned int pozycja, unsigned short int kolor) {
    for (unsigned int y = pozycja; y < pozycja + 15; y++) {
        for (unsigned int x = 10; x <= 160; x++) {
            LCD_BUF[y * LCD_WIDTH + x] = kolor;
        }
    }
}
*/

int main(void) {
    System_Init();           // Inicjalizacja systemu
    ButtonSetup();           // Konfiguracja przycisków
    LedSetup();              // Konfiguracja LED
    ADC_Setup();             // Konfiguracja ADC
    RysujRamke(10, 229, 10, 309); // Rysuj ramkę na LCD
    Clear_And_Reload_Screen();    // Odśwież ekran
    while (1) {}             // (niepotrzebna pusta pętla)
    while (1) {
        switch (stany) {
            case menuu:
                //RysujPodswietlenie(Y[k], czerwony);
                WypiszMenu();
                unsigned int JPos = Get_Joy();
                if (JPos > 3000) {
                    while (JPos > 2500) {
                        JPos = Get_Joy();
                    }
                    if (k == 3) {
                        k = 0;
                    } else {
                        k++;
                    }
                }
                if (JPos < 1000) {
                    while (JPos < 1500) {
                        JPos = Get_Joy();
                    }
                    if (k == 0) {
                        k = 3;
                    } else {
                        k--;
                    }
                }
                if (!(CheckButton(11))) {
                    while (!(CheckButton(11)));
                    stany = animacja;
                }
                break;
            case animacja:
                SetLed(1); // Zapal LED
                //RysujPodswietlenie(Y[k], niebieski);
                WypiszMenu();
                if (licznik >= 100) {
                    SetLed(0); // Zgaś LED
                    stany = kafelek[k]; // Przejdź do wybranej opcji
                    licznik = 0;
                } else {
                    licznik++;
                }
                break;
            case opcja1:
                BSP_LCD_DisplayStringAt(20, Y[k], (uint8_t*)menu[k], 0);
                if (!CheckButton(12)) {
                    stany = menuu;
                }
                break;
            case opcja2:
                BSP_LCD_DisplayStringAt(20, Y[k], (uint8_t*)menu[k], 0);
                if (!CheckButton(12)) {
                    stany = menuu;
                }
                break;
            case opcja3:
                BSP_LCD_DisplayStringAt(20, Y[k], (uint8_t*)menu[k], 0);
                if (!CheckButton(12)) {
                    stany = menuu;
                }
                break;
            case opcja4:
                BSP_LCD_DisplayStringAt(20, Y[k], (uint8_t*)menu[k], 0);
                if (!CheckButton(12)) {
                    stany = menuu;
                }
                break;
        }
        mruganie(); // Funkcja mrugania (zewnętrzna)
        //Clear_And_Reload_Screen(); // (opcjonalne odświeżanie ekranu)
    }
}

// Handler przerwania SysTick
void SysTick_Handler(void) {
    HAL_IncTick(); // Zwiększ licznik systemowy
}

// Inicjalizacja systemu (HAL, zegar, SDRAM, LCD)
void System_Init() {
    HAL_Init();
    SystemClock_Config();
    BSP_SDRAM_Init();
    BSP_LCD_Init();
}

// Kopiowanie bufora LCD i czyszczenie bufora roboczego
void Clear_And_Reload_Screen() {
    for (int off = 0; off < 320 * 240; off++) {
        LCD_FB[off] = LCD_BUF[off]; // Przenieś piksele na ekran
        LCD_BUF[off] = 0;           // Wyczyść bufor roboczy
    }
}
