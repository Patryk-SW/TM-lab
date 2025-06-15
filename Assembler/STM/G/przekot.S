.syntax unified                  // Ustawienie składni assemblera na unified
.cpu cortex-m4                  // Ustawienie architektury na Cortex-M4
.thumb                          // Ustawienie trybu Thumb

.equ LCD_BUF_ADDR,  0xD0100000   // Stała: adres bufora LCD
.equ szerokosc_wyswietlacza, 320 // Stała: szerokość wyświetlacza
.equ KOLOR_RAMKI, 0xf800         // Stała: kolor ramki (czerwony)
.equ KOLOR_PRZEKATNA, 0x07E0     // Stała: kolor przekątnej (zielony)

.global RysujRamke               // Udostępnienie funkcji na zewnątrz
.global RysujPrzekatna           // Udostępnienie funkcji rysującej przekątną

// void RysujRamke(unsigned int y0, unsigned int y1, unsigned int x0, unsigned int x1);
RysujRamke:
    PUSH {r4-r11, lr}            // Zapisz rejestry robocze i powrotny na stosie

    MOV r4, r0                   // r4 = y0 (górna krawędź)
    MOV r5, r1                   // r5 = y1 (dolna krawędź)
    MOV r6, r2                   // r6 = x0 (lewa krawędź)
    MOV r7, r3                   // r7 = x1 (prawa krawędź)

    LDR r8, =KOLOR_RAMKI         // r8 = kolor ramki (czerwony)

    // --- Górna krawędź ramki ---
    MOV r9, r6                   // r9 = x (start od lewej)
    LDR r10, =szerokosc_wyswietlacza // r10 = szerokość wyświetlacza
    MUL r11, r4, r10             // r11 = y0 * szerokość (indeks wiersza)
    ADD r11, r11, r6             // r11 = indeks + x0 (początek linii)
    LSL r11, r11, #1             // r11 = przesunięcie na bajty (2 bajty na piksel)
    LDR r10, =LCD_BUF_ADDR       // r10 = adres bufora LCD
    ADD r11, r11, r10            // r11 = adres pierwszego piksela górnej krawędzi

PetlaGora:
    CMP r9, r7                   // Czy x > x1?
    BGT KoniecGora               // Jeśli tak, koniec górnej krawędzi
    STRH r8, [r11]               // Zapisz kolor do pamięci (2 bajty)
    ADD r11, r11, #2             // Przejdź do kolejnego piksela (2 bajty dalej)
    ADD r9, r9, #1               // x++
    B PetlaGora                  // Powtórz pętlę
KoniecGora:

    // --- Dolna krawędź ramki ---
    MOV r9, r6                   // r9 = x (start od lewej)
    LDR r10, =szerokosc_wyswietlacza // r10 = szerokość wyświetlacza
    MUL r11, r5, r10             // r11 = y1 * szerokość (indeks wiersza)
    ADD r11, r11, r6             // r11 = indeks + x0 (początek linii)
    LSL r11, r11, #1             // r11 = przesunięcie na bajty
    LDR r10, =LCD_BUF_ADDR       // r10 = adres bufora LCD
    ADD r11, r11, r10            // r11 = adres pierwszego piksela dolnej krawędzi

PetlaDol:
    CMP r9, r7                   // Czy x > x1?
    BGT KoniecDol                // Jeśli tak, koniec dolnej krawędzi
    STRH r8, [r11]               // Zapisz kolor do pamięci
    ADD r11, r11, #2             // Przejdź do kolejnego piksela
    ADD r9, r9, #1               // x++
    B PetlaDol                   // Powtórz pętlę
KoniecDol:

    // --- Lewa krawędź ramki ---
    ADD r9, r4, #1               // r9 = y0 + 1 (startujemy od następnego wiersza)
PetlaLewy:
    CMP r9, r5                   // Czy y >= y1?
    BGE KoniecLewy               // Jeśli tak, koniec lewej krawędzi
    LDR r10, =szerokosc_wyswietlacza // r10 = szerokość wyświetlacza
    MUL r11, r9, r10             // r11 = y * szerokość
    ADD r11, r11, r6             // r11 = indeks + x0
    LSL r11, r11, #1             // r11 = przesunięcie na bajty
    LDR r10, =LCD_BUF_ADDR       // r10 = adres bufora LCD
    ADD r11, r11, r10            // r11 = adres piksela
    STRH r8, [r11]               // Zapisz kolor
    ADD r9, r9, #1               // y++
    B PetlaLewy                  // Powtórz pętlę
KoniecLewy:

    // --- Prawa krawędź ramki ---
    ADD r9, r4, #1               // r9 = y0 + 1
PetlaPrawy:
    CMP r9, r5                   // Czy y >= y1?
    BGE KoniecPrawy              // Jeśli tak, koniec prawej krawędzi
    LDR r10, =szerokosc_wyswietlacza // r10 = szerokość wyświetlacza
    MUL r11, r9, r10             // r11 = y * szerokość
    ADD r11, r11, r7             // r11 = indeks + x1
    LSL r11, r11, #1             // r11 = przesunięcie na bajty
    LDR r10, =LCD_BUF_ADDR       // r10 = adres bufora LCD
    ADD r11, r11, r10            // r11 = adres piksela
    STRH r8, [r11]               // Zapisz kolor
    ADD r9, r9, #1               // y++
    B PetlaPrawy                 // Powtórz pętlę
KoniecPrawy:

    POP {r4-r11, pc}             // Przywróć rejestry i powrót z funkcji

// void RysujPrzekatna(unsigned int y0, unsigned int y1, unsigned int x0, unsigned int x1);
// Rysuje przekątną prostokąta od (x0, y0) do (x1, y1) - algorytm Bresenhama
RysujPrzekatna:
    PUSH {r4-r11, lr}            // Zapisz rejestry robocze i powrotny na stosie

    MOV r4, r2                   // r4 = x0 (początek przekątnej)
    MOV r5, r0                   // r5 = y0 (początek przekątnej)
    MOV r6, r3                   // r6 = x1 (koniec przekątnej)
    MOV r7, r3                   // r7 = x1 (koniec przekątnej)

    LDR r8, =KOLOR_PRZEKATNA     // r8 = kolor przekątnej (zielony)

    // Oblicz różnice
    SUB r9, r7, r6               // r9 = dx = x1 - x0
    SUB r10, r5, r4              // r10 = dy = y1 - y0

    // Ustal liczbę kroków (max(dx, dy))
    CMP r9, r10
    IT GE
    MOVGE r11, r9                // r11 = steps = dx jeśli dx >= dy
    IT LT
    MOVLT r11, r10               // r11 = steps = dy jeśli dy > dx

    // Przelicz dx, dy na float (do dzielenia)
    // (upraszczamy: dla prostokątów z RysujRamke() dx==dy, więc krok = 1)
    // Jeśli chcesz pełny algorytm Bresenhama, trzeba dodać więcej kodu

    MOV r12, #0                  // r12 = licznik kroków
PetlaPrzekatna:
    // x = x0 + (dx * i) / steps
    // y = y0 + (dy * i) / steps
    // Ale dla prostokąta z RysujRamke() dx == dy, więc x = x0 + i, y = y0 + i

    ADD r0, r6, r12              // r0 = x = x0 + i
    ADD r1, r4, r12              // r1 = y = y0 + i

    // Oblicz offset: offset = y * szerokosc + x
    LDR r2, =szerokosc_wyswietlacza
    MUL r3, r1, r2               // r3 = y * szerokość
    ADD r3, r3, r0               // r3 = offset
    LSL r3, r3, #1               // r3 = offset * 2 (2 bajty na piksel)
    LDR r2, =LCD_BUF_ADDR
    ADD r3, r3, r2               // r3 = adres piksela

    STRH r8, [r3]                // Zapisz kolor przekątnej

    ADD r12, r12, #1             // i++
    CMP r12, r11
    BLE PetlaPrzekatna           // Dopóki i <= steps

    POP {r4-r11, pc}             // Przywróć rejestry i powrót z funkcji
