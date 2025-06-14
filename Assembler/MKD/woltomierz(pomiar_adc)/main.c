#include "main.h"

// Definicje rejestrów sprzętowych (adresowanych przez xdata)
unsigned char xdata PTWE _at_ 0x8008; // Rejestr wejściowy przycisków
unsigned char xdata PTAC _at_ 0x8000; // Rejestr odczytu ADC
unsigned char xdata POT0 _at_ 0x8005; // Rejestr wyboru kanału ADC 0
unsigned char xdata POT1 _at_ 0x8006; // Rejestr wyboru kanału ADC 1
unsigned char xdata POT2 _at_ 0x8007; // Rejestr wyboru kanału ADC 2
unsigned char xdata PTSEG _at_ 0x8018; // Rejestr sterowania segmentami 7-seg
unsigned char xdata PTWY _at_ 0x8009; // Rejestr wyjściowy (np. diody)

unsigned char xdata LCDBUF[32]; // Bufor tekstu do wyświetlenia na LCD
unsigned char Pomiar;            // Zmienna pomocnicza do pomiaru
unsigned int  Napiecie;          // Zmienna przechowująca wartość napięcia
unsigned char Przycisk;          // Zmienna przechowująca numer wciśniętego przycisku
unsigned char Modul7Seg=0;       // Licznik do multipleksowania wyświetlaczy 7-seg
unsigned int POMIARY[3] = {0,0,0}; // Tablica przechowująca cyfry do wyświetlenia

unsigned char liczba[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; // Wzorce cyfr 0-9 dla 7-seg

// Funkcja odczytuje, który przycisk został naciśnięty
unsigned char OdczytPrzycisku(){
    // Sprawdzenie kombinacji bitów w rejestrze PTWE
    if (!(PTWE&1)&&(PTWE&2)&&(PTWE&4)) {
        return 1; // Pierwszy przycisk
    }
    else if ((PTWE&1)&&!(PTWE&2)&&(PTWE&4)) {
        return 2; // Drugi przycisk
    }
    else if ((PTWE&1)&&(PTWE&2)&&!(PTWE&4)) {
        return 3; // Trzeci przycisk
    }
    return 0; // Żaden lub nieprawidłowa kombinacja
}

// Funkcja ustawia odpowiedni tekst na LCD w zależności od wybranego kanału
void WysterujLCD(unsigned char a_Przycisk){
    switch (a_Przycisk) {
        case 1:
            strncpy(LCDBUF, "Kanal: 0        ", 16); // Ustaw tekst dla kanału 0
            strncpy(&LCDBUF[16], "                ", 16); // Wyczyść drugą linię
            break;
        case 2:
            strncpy(LCDBUF, "Kanal: 1        ", 16); // Ustaw tekst dla kanału 1
            strncpy(&LCDBUF[16], "                ", 16);
            break;
        case 3:
            strncpy(LCDBUF, "Kanal: 2        ", 16); // Ustaw tekst dla kanału 2
            strncpy(&LCDBUF[16], "                ", 16);
            break;
        default:
            strncpy(LCDBUF, "Wybierz kanal   ", 16); // Prośba o wybór kanału
            strncpy(&LCDBUF[16], "                ", 16);
            break;
    }
}

// Funkcja odczytuje wartość ADC z wybranego kanału
unsigned char PomiarADC(unsigned char a_KanalAnalogowy){
    if (!(PTWE&1)) {
        a_KanalAnalogowy = POT0; // Odczyt z kanału 0
        return a_KanalAnalogowy;
    }
    else if (!(PTWE&2)) {
        a_KanalAnalogowy = POT1; // Odczyt z kanału 1
        return a_KanalAnalogowy;
    }
    else if (!(PTWE&4)) {
        a_KanalAnalogowy = POT2; // Odczyt z kanału 2
        return a_KanalAnalogowy;
    }
    return 0; // Brak odczytu
}

// Funkcja uruchamia konwersję ADC i zwraca wynik dla wybranego kanału
unsigned int ADCtoVoltage(unsigned char a_Pomiar) {
    switch (a_Pomiar) {
        case 1:
            POT0 = 0; // Wybierz kanał 0
            break;
        case 2:
            POT1 = 0; // Wybierz kanał 1
            break;
        case 3:
            POT2 = 0; // Wybierz kanał 2
            break;
        default:
            return 0; // Nieprawidłowy kanał
    }
    WAIT_10US(15U); // Krótkie opóźnienie na stabilizację
    return PTAC;    // Zwróć wynik konwersji ADC
}

// Funkcja przelicza wartość napięcia na cyfry do wyświetlenia na 7-seg
void skalowanie(unsigned int a_Napiecie) {
    POMIARY[2] = (a_Napiecie / 51);           // Dziesiątki
    POMIARY[1] = (a_Napiecie * 10 / 51) % 10; // Jedności
    POMIARY[0] = (a_Napiecie * 100 / 51) % 10;// Dziesiętne
}

// Przerwanie od timera 0 - obsługuje multipleksowanie wyświetlaczy 7-seg
void IRT_TIMER0() interrupt 1 {
    TH0 = (-2500) >> 8;      // Ustawienie wartości początkowej timera (górny bajt)
    TL0 = (-2500) & 0xff;    // Ustawienie wartości początkowej timera (dolny bajt)

    Modul7Seg = (Modul7Seg + 1) % 3; // Zwiększ licznik wyświetlacza (0..2)

    PTSEG = 0;              // Wyłącz segmenty przed zmianą
    P1 = Modul7Seg;         // Aktywuj odpowiedni wyświetlacz

    // Wyświetl odpowiednią cyfrę
    if (Modul7Seg == 2)
        PTSEG = liczba[POMIARY[Modul7Seg]] | 0x80; // Wyświetl cyfrę z kropką
    else
        PTSEG = liczba[POMIARY[Modul7Seg]];        // Wyświetl cyfrę

    POT0 = 0; // Ponowna inicjalizacja pomiaru
}

// Główna funkcja programu
void main(void) {
    prglcd();                  // Inicjalizacja LCD
    WAIT_10US(1000);           // Krótkie opóźnienie po inicjalizacji
    P1 = 0x3C;                 // Konfiguracja portu P1
    TMOD = 1;                  // Tryb timera
    TH0 = (-2500) >> 8;        // Ustawienie timera (górny bajt)
    TL0 = (-2500) & 0xff;      // Ustawienie timera (dolny bajt)
    IE = 0x82;                 // Włączenie przerwań
    TCON = 0x10;               // Start timera

    while(1){
        Przycisk = OdczytPrzycisku();         // Odczytaj przycisk
        WysterujLCD(Przycisk);                // Ustaw tekst na LCD
        Napiecie = ADCtoVoltage(Przycisk);    // Pobierz wartość napięcia
        skalowanie(Napiecie);                 // Przelicz na cyfry 7-seg
        disptext(LCDBUF);                     // Wyświetl tekst na LCD
    }
}

// Funkcja realizująca opóźnienie ~10us * wait
void WAIT_10US(unsigned int wait){
    wait=(wait*10UL)/11UL;    // Korekta czasu opóźnienia
    while(--wait) _nop_();    // Pętla opóźniająca
}