; Plik asemblera - funkcje pomocnicze do mikrokontrolera 8051
; Każda sekcja i linia została opisana poniżej

; Oznaczenie typów pamięci
; BIT, CODE, DATA, IDATA, XDATA, NUMBER

; Opis przekazywania argumentów do funkcji
; Wejście
; Arg Number               char, 1-byte ptr       int, 2-byte ptr                long, float                         generic ptr
;    1                          R7                   R6 & R7                       R4—R7                              R1—R3
;                                             (MSB w R6,LSB w R7)           (MSB w R4,LSB w R7)      (Mem type w R3, MSB w R2, LSB w R1)
;    2                          R5                   R4 & R5                       R4—R7                              R1—R3
;                                             (MSB w R4,LSB w R5)           (MSB w R4,LSB w R7)      (Mem type w R3, MSB w R2, LSB w R1)
;    3                          R3                   R2 & R3                     ----------                           R1—R3
;                                             (MSB w R2,LSB w R3)                                      (Mem type w R3, MSB w R2, LSB w R1)

; Wyjście
; bit                                 - Carry bit
; char, unsigned char, 1-byte pointer - R7 
; int, unsigned int, 2-byte ptr       - R6 & R7 MSB w R6, LSB w R7. 
; long,  unsigned long                - R4-R7 MSB w R4, LSB w R7. 
; float R4-R7                         - 32-Bit IEEE format. 
; generic pointer                     - R1-R3 Memory type w R3, MSB R2, LSB R1. 

; Udostępnienie symboli na zewnątrz (publiczne funkcje)
        PUBLIC  _WAIT_10US_ASM      ; Opóźnienie 10us
        PUBLIC  INWERSJAP1_6_ASM    ; Inwersja bitu P1.6

; Definicja segmentu kodu
PRGSEG  SEGMENT CODE
        RSEG    PRGSEG

; Procedura: Opóźnienie czasowe (dopełnienie do wartości maksymalnej 16-bitowej)
_WAIT_10US_ASM:
    MOV DPL,R7   ; Umieszcza młodszy bajt argumentu w DPL
    MOV DPH,R6   ; Umieszcza starszy bajt argumentu w DPH
WAIT_U:
    NOP          ; 4x NOP = 4us (razem z pętlą daje ok. 10us na iterację)
    NOP
    NOP
    NOP
    INC DPTR     ; Zwiększ DPTR o 1
    MOV A,DPH    ; Przenieś starszy bajt DPTR do akumulatora
    ORL A,DPL    ; Sumuj logicznie z młodszym bajtem DPTR
    JNZ WAIT_U   ; Jeśli DPTR ≠ 0, powtórz pętlę
    RET          ; Powrót z funkcji

; Procedura: Inwersja bitu 6 portu P1 (dioda L8)
INWERSJAP1_6_ASM:
    CPL P1.6     ; Zmień stan bitu P1.6
    RET          ; Powrót z funkcji

; Procedura: Kopiowanie stanów przycisków X0..X5 na diody L0..L5
; PTWE - rejestr wejściowy (przyciski), adres 0x8008
; PTWY - rejestr wyjściowy (diody), adres 0x8008
        PUBLIC STERUJ_DIODY_X0_X5
SEGMENT_CODE SEGMENT CODE
        RSEG SEGMENT_CODE
STERUJ_DIODY_X0_X5:
    MOV DPTR, #0x8008     ; Ustaw adres PTWE/PTWY
    MOVX A, @DPTR         ; Odczytaj bajt z PTWE
    ANL A, #00111111b     ; Zamaskuj tylko bity 0-5
    CPL A                 ; Odwróć logikę (przycisk wciśnięty = LED ON)
    MOVX @DPTR, A         ; Zapisz do PTWY (ten sam adres)
    RET

; Procedura: Zmień stan diody L8 (P1.6), poczekaj zadany czas
; Argument: delay (unsigned int) w R6 (MSB) i R7 (LSB)
        PUBLIC  _TOGGLE_L8_DELAY
CODESEG SEGMENT CODE
        RSEG CODESEG
_TOGGLE_L8_DELAY:
    CPL P1.6              ; Zmień stan diody L8
    MOV  A, R7            ; Przenieś LSB opóźnienia do DPL
    MOV  DPL, A
    MOV  A, R6            ; Przenieś MSB opóźnienia do DPH
    MOV  DPH, A
    LCALL _WAIT_10US_ASM  ; Wywołaj opóźnienie
    RET                   ; Powrót z funkcji

   
;---------------------------------------------------------------

        ; Zmienna licznika (adres 0x30 – pierwszy wolny w DATA)
        ; Mozesz zmienic na inna, jesli potrzebujesz
        ; Dla 8051 DATA 0x30–0x7F to RAM ogólnego przeznaczenia
        ; (Mozesz tez zadeklarowac w C i przekazywac adres przez argument)
         PUBLIC  BIN_COUNTER_DIR
		 PUBLIC INIT_COUNTER
CNT     DATA    40H        ; zmienna licznikowa w RAM

; Jednorazowa inicjalizacja licznika na zero
; (wywolaj podczas inicjalizacji systemu, np. osobna funkcja)
INIT_COUNTER:
        MOV     CNT, #0
        RET

; Procedura licznika binarnego z kierunkiem zaleznym od przycisku
BIN_COUNTER_DIR:
        ; Odczytaj stan przycisku (np. X0 - bit 0 portu wejsciowego)
        MOV     DPTR, #0x8008
        MOVX    A, @DPTR
        JNB     ACC.0, INC_COUNT   ; jesli X0==0 (przycisk wcisniety): zlicz w góre

DEC_COUNT:
        MOV     A, CNT
        CJNE    A, #0, DO_DEC
        MOV     CNT, #255          ; przepelnienie w dól: licznik od 0 ? 255
        SJMP    SHOW
DO_DEC:
        DEC     CNT
        SJMP    SHOW

INC_COUNT:
        MOV     A, CNT
        CJNE    A, #255, DO_INC
        MOV     CNT, #0            ; przepelnienie w góre: licznik od 255 ? 0
        SJMP    SHOW
DO_INC:
        INC     CNT
		SJMP    SHOW

SHOW:
        MOV     A, CNT
        MOV     DPTR, #0x8008      ; wyslij na diody L0..L7
        MOVX    @DPTR, A

        ; Opóznienie (zmniejsz np. dla szybszego liczenia)
        MOV     R6, #0
        MOV     R7, #100           ; krótsze opóznienie
        MOV     R4, #1
DELAY_LOOP:
        LCALL   _WAIT_10US_ASM
        DJNZ    R4, DELAY_LOOP

        RET
		
;------------------------------------------------

;----------------------------------------------------
; unsigned char GETADC(unsigned char nr_kanalu);
; nr_kanalu przekazywany jest w rejestrze R7 (C51 konwencja wywolania)
; Wynik zwracany w A (ACC)
;----------------------------------------------------
;_GETADC:
;        MOV     A, R7         ; pobierz nr_kanalu do A

 ;       CJNE    A, #0, SPRAWDZ1
  ;      ; Kanal 0
    ;    MOV     DPTR, #0x8005 ; POT0
   ;     MOV     A, #0
     ;   MOVX    @DPTR, A      ; start przetwarzania
      ;  SJMP    DALEJ

;SPRAWDZ1:
;        CJNE    A, #1, SPRAWDZ2
        ; Kanal 1
 ;       MOV     DPTR, #0x8006 ; POT1
 ;       MOV     A, #0
  ;      MOVX    @DPTR, A
   ;     SJMP    DALEJ

;SPRAWDZ2:
 ;       CJNE    A, #2, BRAK_KANALU
        ; Kanal 2
  ;      MOV     DPTR, #0x8007 ; POT2
   ;     MOV     A, #0
    ;    MOVX    @DPTR, A
     ;   SJMP    DALEJ

;BRAK_KANALU:
 ;       MOV     A, #0xFF      ; nieznany kanal, zwróc 0xFF
  ;      RET

;DALEJ:
        ; Opóznienie na czas konwersji ADC (mozesz dostosowac)
 ;       MOV     R0, #20
;OP:
 ;       DJNZ    R0, OP

        ; Odczytaj wynik z PTAC
  ;      MOV     DPTR, #0x8000
   ;     MOVX    A, @DPTR
    ;    RET

;unsigned char GETADC(unsigned char nr_kanalu);
PUBLIC  _GETADC_ASM
_GETADC_ASM:
; Rejestry A, R0-R7
; r7 <- nr_kanalu

; 1. POTn=x;
	mov dptr,#0x8000   ;0x8000-0x8007
; dpl <- numer kanalu analogowego: dptr = dph<<8 | dpl
;...
	//movx @dptr,A  ; uruchomienie pomiaru - start konwersji ADC
	
	mov A, #0
	add A, #0x04
	add A, r7
	
	mov dpl,A  
	mov A, #0
	movx @dptr, A ;uruchomienie pomiaru - start konwersji ADC
	
	
; 2. delay 120us - petla o liczbie iteracji (120/t_iter) 
;... 
	mov r0, #100
loop:
	djnz r0,loop
// 3. pomiar=POTn
	movx A,@dptr   ; odczyt wyniku konwersji
	mov r7, A
; r7 <- unsigned char GETADC
;...	
	ret

               PUBLIC  _GETPOT

_GETPOT:
        ; Wczytaj wartosc portu PTWE (0x8008)
        MOV     DPTR, #0x8008
        MOVX    A, @DPTR
        MOV     R0, A      ; zachowaj oryginal

        ; Sprawdz warunek 1: !(PTWE&1) && (PTWE&2) && (PTWE&4)
        MOV     A, R0
        ANL     A, #1
        JNZ     check2      ; jesli PTWE&1==1, idz dalej

        MOV     A, R0
        ANL     A, #2
        JZ      check2      ; jesli PTWE&2==0, idz dalej

        MOV     A, R0
        ANL     A, #4
        JZ      check2      ; jesli PTWE&4==0, idz dalej

        MOV     R7, #1      ; warunek 1 spelniony
        RET

check2:
        MOV     A, R0
        ANL     A, #2
        JNZ     check3      ; jesli PTWE&2==1, idz dalej

        MOV     A, R0
        ANL     A, #1
        JZ      check3      ; jesli PTWE&1==0, idz dalej

        MOV     A, R0
        ANL     A, #4
        JZ      check3      ; jesli PTWE&4==0, idz dalej

        MOV     R7, #2      ; warunek 2 spelniony
        RET

check3:
        MOV     A, R0
        ANL     A, #4
        JNZ     ret0        ; jesli PTWE&4==1, idz do ret0

        MOV     A, R0
        ANL     A, #2
        JZ      ret0        ; jesli PTWE&2==0, idz do ret0

        MOV     A, R0
        ANL     A, #1
        JZ      ret0        ; jesli PTWE&1==0, idz do ret0

        MOV     R7, #3      ; warunek 3 spelniony
        RET

ret0:
        MOV     R7, #0      ; zaden warunek: return 0
        RET

       
;Dyrektywa asemblera konczaca kod zrodlowy
	END
