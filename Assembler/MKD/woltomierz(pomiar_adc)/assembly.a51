;Oznaczenie pamieci
;BIT, CODE, DATA, IDATA, XDATA, NUMBER

;Wejscie
;Arg Number               char, 1-byte ptr       int, 2-byte ptr                long, float                         generic ptr
;    1                          R7                   R6 & R7                       R4—R7                              R1—R3
;                                             (MSB in R6,LSB in R7)           (MSB in R4,LSB in R7)      (Mem type in R3, MSB in R2, LSB in R1)
;    2                          R5                   R4 & R5                       R4—R7                              R1—R3
;                                             (MSB in R4,LSB in R5)           (MSB in R4,LSB in R7)      (Mem type in R3, MSB in R2, LSB in R1)
;    3                          R3                   R2 & R3                     ----------                           R1—R3
;                                             (MSB in R2,LSB in R3)                                      (Mem type in R3, MSB in R2, LSB in R1)

;Wyjscie
;bit                                 - Carry bit
;char, unsigned char, 1-byte pointer - R7 
;int, unsigned int, 2-byte ptr       - R6 & R7 MSB in R6, LSB in R7. 
;long,  unsigned long                - R4-R7 MSB in R4, LSB in R7. 
;float R4-R7                         - 32-Bit IEEE format. 
;generic pointer                     - R1-R3 Memory type in R3, MSB R2, LSB R1. 

;Upublicznie (udostepnienie na zewnatrz) symbolu zwiazanego z procedura/funkcja (jesli przyjmuje argumenty wejsciowe, to wymaga uzupelnienia symbolu dodatkowym znakiem "podlogi")
		PUBLIC  _WAIT_10US_ASM
		PUBLIC  INWERSJAP1_6_ASM
		
;Zdefiniowanie segmentu kodu wynikowego 				
PRGSEG  SEGMENT CODE
		RSEG    PRGSEG
;Definicja procedury realizujacej opoznienie czasowe bedace dopelnieniem do wartosci maksymalnej 16-bitowej
_WAIT_10US_ASM:
; Odebranie argumentu wejsciowego podanego przy wywolaniu procedury
		MOV DPL,R7   ;Umieszcza mlodszy bajty argumentu wejsciowego w mlodszym bajcie rejestru DPTR (DPL)
		MOV DPH,R6   ;Umieszcza starszy bajt argumentu wejsciowego w starszym bajcie rejestru DPTR (DPH)
;Petla opozniajaca wykorzystujaca wartosc rejestru DPTR jako zmienna iteracyjna			
WAIT_U:
; czterokrotne wykonanie instrukcji NOP, implementujce opoznienie 4us w celu zapewnienia opoznienia rownego 10us dla jednej iteracji petli
		NOP
		NOP
		NOP
		NOP
;Zwiekszenie DPTR o jeden
		INC DPTR
;Przeslanie do akumulatora starszego bajtu DPTR-a
		MOV A,DPH
;Wykonanie sumy logicznej miedzy akumulatorenm a mlodszym bajtem DPTR w celu przygotwania akumulatora do sprawdzenia czy rejestr DPTR ulegl wyzerowaniu
		ORL A,DPL
;Sprawdzenie niezerowego stanu rejestru DPTR poprzez weryfikacje stanu akumulatora ustalonego poprzednia instrukcja sumy logicznej
;Wykonanie skoku do kolejnej iteracji petli jesli akumulator jest rozny od zera (oparciu o powyzsze instrukcji rowniez jesli DPTR jest rozny od zera)
		JNZ WAIT_U
;Powrot z procedury		
		RET
;Etykieta rozpoczynajaca procedure INWERSJAP1_6_A
INWERSJAP1_6_ASM:
; Inwersja bitu 6 portu P1, ktory steruje dioda L8
	CPL P1.6 
;Powrot z procdury (podprogramu) - skok bezwarunkowy pod adres zapisany na stosie
	RET

; Procedura kopiujaca stany przycisków X0..X5 na diody L0..L5
; PTWE - rejestr wejsciowy (przyciski), adres 0x8008
; PTWY - rejestr wyjsciowy (diody), adres 0x8008

	PUBLIC STERUJ_DIODY_X0_X5

SEGMENT_CODE SEGMENT CODE
	RSEG SEGMENT_CODE

STERUJ_DIODY_X0_X5:
	; Odczytaj stan przycisków X0-X5
	MOV DPTR, #0x8008     ; Adres PTWE/PTWY
	MOVX A, @DPTR         ; Odczytaj bajt z PTWE

	; Zamaskuj tylko bity 0-5 (pozostale wyzeruj)
	ANL A, #00111111b     ; Zostaw tylko bity 0-5
	; Wpisz stan na diody L0-L5 (PTWY)
	CPL A 				  ; Odwrócenie logiki (przycisk wcisniety==LED ON)
	MOVX @DPTR, A         ; Zapisz do PTWY (adres ten sam)
	
	RET

; Procedura: Zmien stan diody L8 (P1.6), poczekaj zadany czas
; Argument: delay (unsigned int) w R6 (MSB) i R7 (LSB)

        PUBLIC  _TOGGLE_L8_DELAY
        

CODESEG SEGMENT CODE
        RSEG CODESEG

_TOGGLE_L8_DELAY:
        CPL P1.6

        MOV  A, R7
        MOV  DPL, A
        MOV  A, R6
        MOV  DPH, A
        LCALL _WAIT_10US_ASM

        RET

   
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
