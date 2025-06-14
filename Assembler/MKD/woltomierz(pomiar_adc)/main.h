// Plik nagłówkowy - deklaracje funkcji i dołączane biblioteki
#include <intrins.h>      // Biblioteka z funkcjami asemblerowymi (np. _nop_)
#include <reg52.h>        // Definicje rejestrów mikrokontrolera 8051
#include <intrins.h>      // (powtórzone, ale nie szkodzi)
#include "lcd.h"          // Własna biblioteka do obsługi LCD
#include "mkd51sim.h"     // Własna biblioteka do symulacji
#include <string.h>       // Standardowa biblioteka do operacji na łańcuchach

// Prototypy funkcji opóźniających
void WAIT_10US_ASM(unsigned int negative_wait); // Opóźnienie w asemblerze
void WAIT_10US(unsigned int wait);              // Opóźnienie w C

// Prototypy funkcji do inwersji bitu P1.6 (dioda L8)
void INWERSJAP1_6_ASM(void); // Wersja asemblerowa
void INWERSJAP1_6(void);     // Wersja w C

// Prototypy funkcji ADC
unsigned char PomiarADC();                       // Odczyt wartości ADC
unsigned int ADCtoVoltage(unsigned char a_Pomiar);// Przeliczenie ADC na napięcie

// Prototypy funkcji do obsługi LCD i konwersji
void KonwersjaDoLCDBUF(unsigned int a_Napiecie,unsigned char xdata *a_LCDBUF); // Konwersja wyniku do bufora LCD
unsigned int LicznikModuloN(unsigned int a_Napiecie); // Licznik modulo N

// Prototypy funkcji do obsługi diod i liczników
void STERUJ_DIODY_X0_X5(void); // Sterowanie diodami na podstawie przycisków
void TOGGLE_L8_DELAY(unsigned int delay); // Miganie diodą L8 z opóźnieniem
void BIN_COUNTER_DIR(void);               // Licznik binarny
void INIT_COUNTER(void);                  // Inicjalizacja licznika

// Zewnętrzne funkcje (zaimplementowane gdzie indziej)
extern unsigned char GET_KEY_NUM(void);           // Odczyt numeru klawisza
extern unsigned char GETADC_ASM(unsigned char nr_kanalu); // Odczyt ADC w asemblerze
void Konwersja7Seg(unsigned int a_Napiecie);      // Konwersja na 7-segment
extern unsigned char GETPOT(void);                // Odczyt potencjometru