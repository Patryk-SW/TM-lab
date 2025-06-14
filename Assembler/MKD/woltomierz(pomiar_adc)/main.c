#include "main.h"

unsigned char xdata PTWE _at_ 0x8008; 
unsigned char xdata PTAC _at_ 0x8000; 
unsigned char xdata POT0 _at_ 0x8005;
unsigned char xdata POT1 _at_ 0x8006; 
unsigned char xdata POT2 _at_ 0x8007; 
unsigned char xdata PTSEG _at_ 0x8018; 
unsigned char xdata PTWY _at_ 0x8009;

unsigned char xdata LCDBUF[32];			
unsigned char Pomiar;
unsigned int  Napiecie;
unsigned char Przycisk;
unsigned char Modul7Seg=0; // Licznik kontrolujacy multipleksowanie wyswietlaczy 7-seg
unsigned int POMIARY[3] = {0,0,0};


unsigned char liczba[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};





unsigned char OdczytPrzycisku(){
	if (!(PTWE&1)&&(PTWE&2)&&(PTWE&4)) {
		return 1;
	}
	else if ((PTWE&1)&&!(PTWE&2)&&(PTWE&4)) {
		return 2;
	}
	else if ((PTWE&1)&&(PTWE&2)&&!(PTWE&4)) {
		return 3;
	}
	return 0;
}

void WysterujLCD(unsigned char a_Przycisk){

    switch (a_Przycisk) {
        case 1:
            strncpy(LCDBUF, "Kanal: 0        ", 16);
						strncpy(&LCDBUF[16], "                ", 16);
            break;
        case 2:
            strncpy(LCDBUF, "Kanal: 1        ", 16);
					  strncpy(&LCDBUF[16], "                ", 16);
            break;
        case 3:
            strncpy(LCDBUF, "Kanal: 2        ", 16);
						strncpy(&LCDBUF[16], "                ", 16);
            break;
        default:
            strncpy(LCDBUF, "Wybierz kanal   ", 16);
						strncpy(&LCDBUF[16], "                ", 16);
            break;
    }
}

unsigned char PomiarADC(unsigned char a_KanalAnalogowy){
	if (!(PTWE&1)) {
		a_KanalAnalogowy = POT0;
		return a_KanalAnalogowy;
	}
	
	else if (!(PTWE&2)) {
		a_KanalAnalogowy = POT1;
		return a_KanalAnalogowy;
	}
	else if (!(PTWE&4)) {
		a_KanalAnalogowy = POT2;
		return a_KanalAnalogowy;
	}
	
		return 0;
	
}	

unsigned int ADCtoVoltage(unsigned char a_Pomiar) {
    switch (a_Pomiar) {
        case 1:
            POT0 = 0;
            break;
        case 2:
            POT1 = 0;
            break;
        case 3:
            POT2 = 0;
            break;
        default:
            return 0; // nieprawidlowy kanal
    }

    WAIT_10US(15U);
    return PTAC;
}


	
void skalowanie(unsigned int a_Napiecie) {
    POMIARY[2] = (a_Napiecie / 51);           // Dziesiatki
    POMIARY[1] = (a_Napiecie * 10 / 51) % 10; // Jednosci
    POMIARY[0] = (a_Napiecie * 100 / 51) % 10;// Dziesietne
}
	
	


void IRT_TIMER0() interrupt 1 {
    // Odswiezanie co 2500 µs
    TH0 = (-2500) >> 8;
    TL0 = (-2500) & 0xff;

    // Multipleksowanie 7-segmentowe (0..2)
    Modul7Seg = (Modul7Seg + 1) % 3;

    PTSEG = 0;         // Wylaczenie segmentów przed zmiana
    P1 = Modul7Seg;    // Aktywacja odpowiedniego wyswietlacza

    // Wyswietlanie cyfry
    if (Modul7Seg == 2)
        PTSEG = liczba[POMIARY[Modul7Seg]] | 0x80; // kropka
    else
        PTSEG = liczba[POMIARY[Modul7Seg]];

    // Ponowna inicjalizacja pomiaru
    POT0 = 0;

}

void main(void) {
    prglcd();
	
    WAIT_10US(1000);       // Krótkie opóznienie po inicjalizacji LCD

    P1 = 0x3C;             // Konfiguracja portu
    TMOD = 1;
    TH0 = (-2500) >> 8;
    TL0 = (-2500) & 0xff;
    IE = 0x82;
    TCON = 0x10;

    while(1){
        
        Przycisk = OdczytPrzycisku();
        WysterujLCD(Przycisk);
        Napiecie = ADCtoVoltage(Przycisk); // Pobranie wartosci napiecia
				skalowanie(Napiecie);
        disptext(LCDBUF);
    }
}

void WAIT_10US(unsigned int wait){
	wait=(wait*10UL)/11UL;
	while(--wait) _nop_();	
}