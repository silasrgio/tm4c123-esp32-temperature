#include "TM4C123.h"

#define IBRD_Value 		8
#define FBRD_Value 		44
#define UART_FR_TXFF (1<<5)

uint16_t adcValue;

void delay (volatile long d){
	while (d--);
}

void UART0_WriteChar (char c){
	while ((UART0 -> FR & UART_FR_TXFF) != 0);
	UART0 -> DR = c;
}

void UART0_WriteString (const char *s){
	// Loop through s
	for (int i = 0; s[i] != '\0'; i++){
	// Call UART_WriteChar on each char
	UART0_WriteChar (s[i]);
	}
}

int main(){
	
	// GIOF For LED
	SYSCTL -> RCGCGPIO |= (1<<5);
	while ((SYSCTL -> PRGPIO & (1<<5)) == 0);
	
	GPIOF -> DIR |= (1<<3);
	GPIOF -> DEN |= (1<<3);
	
	//GPIOA For UART
	SYSCTL -> RCGCUART |= (1<<0);
	while ((SYSCTL -> PRUART & (1<<0)) == 0);
	
	SYSCTL -> RCGCGPIO |= (1<<0);
	while ((SYSCTL -> PRGPIO & (1<<0)) == 0);
	
	
	//Set alternate function for PA0 and 1
	GPIOA -> AFSEL |= (1<<0) | (1<<1);
	GPIOA -> PCTL = (GPIOA -> PCTL & ~0x00FF) | 0x11;
	GPIOA -> DEN |= (1<<0) |(1<<1);
	
	//UART Config
	UART0 -> CTL &= (~(1<<0));							// UART Disable 
	
	UART0 -> IBRD = IBRD_Value;
	UART0 -> FBRD = FBRD_Value;
	
	UART0 -> LCRH = (0x3 << 5) | (1<<4); 		// 8 bit word length
	UART0 -> CC = 0x5; 											// PIOSC Clock source
	UART0 -> CTL |= (1<<0) | (1<<8);				// UART Enable & TX Enable
	
	
	// Configuration of GPIO as ADC Channel
	
	SYSCTL -> RCGCADC |= (1<<0);    // Enable ADC0 Clock & Clock for GPIO Pin used (PE3)
	while ((SYSCTL -> PRADC & (1<<0)) == 0);
	
	SYSCTL -> RCGCGPIO |= (1<<4);
	while ((SYSCTL -> PRGPIO & (1<<4)) == 0);
	
	GPIOE -> AFSEL |= (1<<3);   // Alternate function for pin (PE3)
	GPIOE -> DEN &=(~(1<<3));   // Configure the AINx signals to be analogue inputs
	GPIOE -> AMSEL |= (1<<3);   // Pin is capable of analog functions
	
	// Sample Sequencer Configuration

	ADC0 -> ACTSS &= (~(1<<3));         // Disable Sample Sequencer 3
	ADC0 -> EMUX &= (~(0xF000));		// Software trigger source for SS3
	ADC0 -> SSMUX3 = 0x0;				// SS3 reads AIN0 (PE3)
	ADC0 -> SSCTL3 |= (1<<1) | (1<<2);	// EN0 & IE0 set RIS flag when done
	ADC0 -> ACTSS |= (1<<3);			// Enable Sample Sequencer 3
	
	while (1){
		UART0_WriteString("hello\r\n");
		
		ADC0 -> PSSI = (1<<3);
		while ((ADC0 -> RIS & (1<<3)) == 0x0);
		adcValue = ADC0 -> SSFIFO3;
		ADC0 -> ISC = (1<<3);
		
		if (adcValue > 2048) {
			GPIOF -> DATA |= (1<<3);
		}
		else{
			GPIOF -> DATA &= (~(1<<3));
		}
	}
	
}
		
		