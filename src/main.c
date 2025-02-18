#include "MKL05Z4.h"
#include "uart0.h"
#include "ADC.h"
#include <stdio.h>
#include <string.h>

#define LED_RED 0

#define MORSE_UNIT 90         // Podstawowa jednostka czasu w ms
#define THRESHOLD 1000        // Próg jasności dla wykrycia światła 0-4096
#define MAX_MORSE_LEN 6       // Maksymalna liczba kropek i kresek w jednym znaku
#define TOLERANCE_PERCENT 15  // Tolerancja w procentach

#define LOWER_LIMIT(value) ((value) - ((value) * TOLERANCE_PERCENT / 100))
#define UPPER_LIMIT(value) ((value) + ((value) * TOLERANCE_PERCENT / 100))

static volatile uint32_t time_ms = 0;
static volatile uint32_t wynik;
static volatile uint8_t stateChanged = 0;
static volatile uint8_t currentState = 0;
static volatile uint32_t stateStartTime = 0;

static char morseBuffer[MAX_MORSE_LEN + 1];
static uint8_t morseIndex = 0;
static const uint32_t MaskLED[] = {1UL << 8};

void SysTick_Handler(void);
void ADC0_IRQHandler(void);

static void LED_Init(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
    PORTB->PCR[8] = PORT_PCR_MUX(1UL);
    PTB->PDDR |= MaskLED[LED_RED];
}

static void LED_Set(uint8_t state) {
    if (state) {
        PTB->PCOR = MaskLED[LED_RED];
    } else {
        PTB->PSOR = MaskLED[LED_RED];
    }
}

static void UART_SendString(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        while (!(UART0->S1 & UART0_S1_TDRE_MASK));
        UART0->D = str[i];
    }
}

void SysTick_Handler(void) {
    time_ms++;
}

void ADC0_IRQHandler(void) {
    wynik = ADC0->R[0];
    uint8_t newState = (wynik > THRESHOLD) ? 1 : 0;

    if (newState != currentState) {
        stateChanged = 1;
        currentState = newState;
    }
}

static char DecodeMorseBuffer(const char *morse) {
    if (strcmp(morse, ".-") == 0) return 'A';
    if (strcmp(morse, "-...") == 0) return 'B';
    if (strcmp(morse, "-.-.") == 0) return 'C';
    if (strcmp(morse, "-..") == 0) return 'D';
    if (strcmp(morse, ".") == 0) return 'E';
    if (strcmp(morse, "..-.") == 0) return 'F';
    if (strcmp(morse, "--.") == 0) return 'G';
    if (strcmp(morse, "....") == 0) return 'H';
    if (strcmp(morse, "..") == 0) return 'I';
    if (strcmp(morse, ".---") == 0) return 'J';
    if (strcmp(morse, "-.-") == 0) return 'K';
    if (strcmp(morse, ".-..") == 0) return 'L';
    if (strcmp(morse, "--") == 0) return 'M';
    if (strcmp(morse, "-.") == 0) return 'N';
    if (strcmp(morse, "---") == 0) return 'O';
    if (strcmp(morse, ".--.") == 0) return 'P';
    if (strcmp(morse, "--.-") == 0) return 'Q';
    if (strcmp(morse, ".-.") == 0) return 'R';
    if (strcmp(morse, "...") == 0) return 'S';
    if (strcmp(morse, "-") == 0) return 'T';
    if (strcmp(morse, "..-") == 0) return 'U';
    if (strcmp(morse, "...-") == 0) return 'V';
    if (strcmp(morse, ".--") == 0) return 'W';
    if (strcmp(morse, "-..-") == 0) return 'X';
    if (strcmp(morse, "-.--") == 0) return 'Y';
    if (strcmp(morse, "--..") == 0) return 'Z';
    if (strcmp(morse, "-----") == 0) return '0';
    if (strcmp(morse, ".----") == 0) return '1';
    if (strcmp(morse, "..---") == 0) return '2';
    if (strcmp(morse, "...--") == 0) return '3';
    if (strcmp(morse, "....-") == 0) return '4';
    if (strcmp(morse, ".....") == 0) return '5';
    if (strcmp(morse, "-....") == 0) return '6';
    if (strcmp(morse, "--...") == 0) return '7';
    if (strcmp(morse, "---..") == 0) return '8';
    if (strcmp(morse, "----.") == 0) return '9';
    return '?';
}

// static void debugTime(uint32_t value) {
// 		char buffer[10];
//     int i = 0;
//     char temp[10];
//     do {
//         temp[i++] = (value % 10) + '0';
//         value /= 10;
//     } while (value > 0);

//     // Odwracanie kolejności cyfr
//     int j = 0;
//     while (i > 0) {
//         buffer[j++] = temp[--i];
//     }
//     buffer[j++] = ' '; // space
//     buffer[j] = '\0'; // zakończenie ciągu
//     UART_SendString(buffer);
// }

static void ProcessStateChange(uint8_t isOn, uint32_t duration) {
    if (isOn) {
        if (duration >= LOWER_LIMIT(MORSE_UNIT * 3) && duration <= UPPER_LIMIT(MORSE_UNIT * 3)) {
            morseBuffer[morseIndex] = '\0';
            char charBuffer[2] = {DecodeMorseBuffer(morseBuffer), '\0'};
            UART_SendString(charBuffer);
            morseIndex = 0;
        } else if (duration >= LOWER_LIMIT(MORSE_UNIT * 7)) {
            morseBuffer[morseIndex] = '\0';
            char charBuffer[2] = {DecodeMorseBuffer(morseBuffer), '\0'};
            UART_SendString(charBuffer);
            morseIndex = 0;
            UART_SendString(" ");
        }
    } else {
        if (duration >= LOWER_LIMIT(MORSE_UNIT) && duration <= UPPER_LIMIT(MORSE_UNIT)) {
            if (morseIndex < MAX_MORSE_LEN) {
                morseBuffer[morseIndex++] = '.';
                //UART_SendString("\r\n. ");
                //debugTime(duration);
            }        
        } else if (duration >= LOWER_LIMIT(MORSE_UNIT * 3) && duration <= UPPER_LIMIT(MORSE_UNIT * 3)) {
            if (morseIndex < MAX_MORSE_LEN) {
                morseBuffer[morseIndex++] = '-';
                //UART_SendString("\r\n- ");
                //debugTime(duration);
            }
        }
    }
}

int main(void) {
    UART0_Init();
    LED_Init();
    SysTick_Config(SystemCoreClock / 1000);
    ADC_Init();
    ADC0->SC1[0] = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(8);
	
    //UART_SendString("DEKODER\r\n");

    stateStartTime = time_ms;

    while (1) {
		LED_Set(currentState);
        if (stateChanged) {
            uint32_t duration = time_ms - stateStartTime;
            ProcessStateChange(currentState, duration);
            stateStartTime = time_ms;
            stateChanged = 0;
        }
    }
}
