; =================== OPIS OGÓLNY ===================
; Plik asemblera do mikrokontrolera 8051
; Każda sekcja i linia została szczegółowo opisana poniżej

; Oznaczenie typów pamięci
; BIT, CODE, DATA, IDATA, XDATA, NUMBER

; Opis przekazywania argumentów do funkcji
;    1                          R7                   R6 & R7                       R4—R7                              R1—R3
;                                             (MSB in R6,LSB in R7)           (MSB in R4,LSB in R7)      (Mem type in R3, MSB in R2, LSB in R1)
;    2                          R5                   R4 & R5                       R4—R7                              R1—R3
;                                             (MSB in R4,LSB in R5)           (MSB in R4,LSB in R7)      (Mem type in R3, MSB in R2, LSB in R1)
;    3                          R3                   R2 & R3                     ----------                           R1—R3
;                                             (MSB in R2,LSB in R3)                                      (Mem type in R3, MSB in R2, LSB in R1)

; =================== PUBLICZNE FUNKCJE ===================
        PUBLIC  _WAIT_10US_ASM      ; Opóźnienie 10us
        PUBLIC  INWERSJAP1_6_ASM    ; Inwersja bitu P1.6
        PUBLIC  _SEG                ; Główna funkcja wyświetlania

; =================== SEGMENT KODU ===================
PRGSEG  SEGMENT CODE
        RSEG    PRGSEG

; =================== OPÓŹNIENIE 10us ===================
_WAIT_10US_ASM:
    MOV DPL, R7    ; Umieść młodszy bajt argumentu w DPL
    MOV DPH, R6    ; Umieść starszy bajt argumentu w DPH
WAIT_U:
    NOP            ; 4x NOP = 4us (razem z pętlą daje ok. 10us na iterację)
    NOP
    NOP
    NOP
    INC DPTR       ; Zwiększ DPTR o 1
    MOV A, DPH     ; Przenieś starszy bajt DPTR do akumulatora
    ORL A, DPL     ; Sumuj logicznie z młodszym bajtem DPTR
    JNZ WAIT_U     ; Jeśli DPTR ≠ 0, powtórz pętlę
    RET            ; Powrót z funkcji

; =================== INWERSJA BITU P1.6 ===================
INWERSJAP1_6_ASM:
    CPL P1.6       ; Zmień stan bitu P1.6 (dioda L8)
    RET            ; Powrót z funkcji

; =================== OPÓŹNIENIE PROGRAMOWE (DELAY) ===================
DELAY:
    mov r0, #0x30      ; Ustaw r0 na 0x30 (licznik zewnętrzny)
del1:
    mov r1, #0x30      ; Ustaw r1 na 0x30 (licznik środkowy)
del2:
    mov r2, #0x30      ; Ustaw r2 na 0x30 (licznik wewnętrzny)
del3:
    djnz r2, del3      ; Decrement r2, jeśli nie zero, skocz do del3
    djnz r1, del2      ; Decrement r1, jeśli nie zero, skocz do del2
    djnz r0, del1      ; Decrement r0, jeśli nie zero, skocz do del1
    RET                ; Powrót z funkcji

; =================== GŁÓWNA PĘTLA WYŚWIETLANIA ===================
_SEG:
START:
    mov R7, #0              ; R7 = indeks 0..7 (cyfra do wyświetlenia)

LOOP:
    ; Wyswietlanie cyfr od lewej do prawej
    mov A, R7               ; A = indeks cyfry
    mov DPTR, #CODETAB      ; DPTR = adres tablicy kodów cyfr
    movc A, @A+DPTR         ; A = kod cyfry z tablicy
    mov DPTR, #8018h        ; DPTR = adres rejestru segmentów
    movx @DPTR, A           ; Wyślij kod cyfry do rejestru segmentów

    mov A, R7               ; A = indeks cyfry
    mov DPTR, #MASKTAB      ; DPTR = adres tablicy masek
    movc A, @A+DPTR         ; A = maska pozycji
    mov DPTR, #8008h        ; DPTR = adres rejestru wyboru pozycji
    movx @DPTR, A           ; Wyślij maskę do rejestru wyboru pozycji

    ACALL DELAY1            ; Wywołaj opóźnienie
    INC R7                  ; Przejdź do kolejnej cyfry
    CJNE R7, #8, LOOP       ; Jeśli R7 < 8, wróć do LOOP

; >>> Petla cofajaca sie <<<
    mov R6, #0              ; R6 = pomocniczy licznik (0..7)
POL:
    MOV A, #7               ; A = 7
    CLR C                   ; Wyzeruj przeniesienie
    SUBB A, R6              ; A = 7 - R6 (odliczanie wstecz)
    MOV R5, A               ; R5 = aktualny indeks (od 7 do 0)

    mov A, R5               ; A = indeks cyfry
    mov DPTR, #CODETAB      ; DPTR = adres tablicy kodów cyfr
    movc A, @A+DPTR         ; A = kod cyfry z tablicy
    mov DPTR, #8018h        ; DPTR = adres rejestru segmentów
    movx @DPTR, A           ; Wyślij kod cyfry do rejestru segmentów

    mov A, R5               ; A = indeks cyfry
    mov DPTR, #MASKTAB      ; DPTR = adres tablicy masek
    movc A, @A+DPTR         ; A = maska pozycji
    mov DPTR, #8008h        ; DPTR = adres rejestru wyboru pozycji
    movx @DPTR, A           ; Wyślij maskę do rejestru wyboru pozycji

    ACALL DELAY1            ; Wywołaj opóźnienie
    INC R6                  ; Przejdź do kolejnej pozycji
    CJNE R6, #8, POL        ; Powtórz aż R6 == 8

    SJMP START              ; Nieskończona pętla programu

; =================== TABLICE KODÓW I MASEK ===================
CODETAB:
    db 06h, 5Bh, 4Fh, 66h, 6Dh, 7Dh, 07h, 7Fh ; Kody cyfr 0-7 dla 7-segmentów
MASKTAB:
    db 01h, 02h, 04h, 08h, 10h, 20h, 40h, 80h ; Maski pozycji dla 8 wyświetlaczy

; =================== OPÓŹNIENIE PROGRAMOWE (DELAY1) ===================
DELAY1:
    mov r0, #0x30      ; Ustaw r0 na 0x30 (licznik zewnętrzny)
dell1:
    mov r1, #0x30      ; Ustaw r1 na 0x30 (licznik środkowy)
dell2:
    mov r2, #0x30      ; Ustaw r2 na 0x30 (licznik wewnętrzny)
dell3:
    djnz r2, dell3     ; Decrement r2, jeśli nie zero, skocz do dell3
    djnz r1, dell2     ; Decrement r1, jeśli nie zero, skocz do dell2
    djnz r0, dell1     ; Decrement r0, jeśli nie zero, skocz do dell1
    RET                ; Powrót z funkcji

; =================== KONIEC KODU ===================
    END                ; Dyrektywa kończąca kod źródłowy

