//***************************************************************************************
// MSP430g2231 4-bit Binary to Hexadecimal converter 
//
// Description; PushButton in P1.3 through interrupt turns on and off the LED in P1.0
// By changing the P1.3 interrupt edge, the interrupt is called every time the button
// is pushed and pulled; toggling the LED everytime. 
// ACLK = n/a, MCLK = SMCLK = default DCO
//
//***************************************************************************************
//#include <msp430x20x2.h> 
//#include <msp430g2231.h> 
#include <msp430g2211.h> 

// Shift register
#define DATA BIT0
#define CLK  BIT1

// Input toggle switches
#define SW0  BIT6	// LSB
#define SW1  BIT5
#define SW2  BIT4
#define SW3  BIT3	// MSB
//#define MASK (SW0+SW1+SW2+SW3)

// Piezo speaker
#define BEEP BIT2

// Declare functions
void delay(unsigned int);
void pulseclock(void);
void shiftout(unsigned char);
void pinwrite(unsigned int, unsigned char);
void display(unsigned char);
void cleardisplay(void);
void mybeep(unsigned int);
void cycle(unsigned int, unsigned int);
void check_state_and_update_edge(void);	// used as part of the switch interrupt handler
					// but could also be called from timer interrupt to ensure consistent state
					// in case of failed debounce, etc.


// Next thing to implement is timer interrupts
// Configure one for playing tones - can we actually play music?? No, requires data in memory...
// Configure one for checking button state consistencyy.
// Configure one as timer for the game - count down.

/*
       0x80
      ------
 0x04 |0x02| 0x40
      ------
 0x08 |    | 0x20
      ------ o 0x01
       0x10
*/


//unsigned char digits[16] = {B11111100, B00001010, B10110110, B10011110, B11001010, B11011100, B11111100, B00001110, B11111110, B11011110};
//                           0     1     2     3     4     5     6     7     8     9     A     b     C     d     E     F
unsigned char digits[16] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xE6, 0xEF, 0x3F, 0x9D, 0x7B, 0x9F, 0x8F};
//                           0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
//unsigned char digits[16] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xE6, 0xFB, 0x3F, 0x1B, 0x7B, 0xDF, 0x8F};

unsigned char cycle6[6] = {0x10, 0x08, 0x04, 0x80, 0x40, 0x20};
const int MASK = SW0+SW1+SW2+SW3;
unsigned char GMODE;
unsigned char BINVAL;

int main(void)
{
//	unsigned char mode;

	WDTCTL = WDTPW + WDTHOLD; 	// Stop watchdog timer

	P1DIR |= (CLK + DATA + BEEP); 		// Set clock, data and beep pins to output direction 

	P1DIR &= ~MASK;  // Set switches as inputs
	P1REN |= MASK;	 // Enable resistor pull
	P1OUT &= ~MASK;  // Select Pulldown resistors 
	P1IE |= MASK;  	 // interrupt enable

	// Set which edge for the interrupt to trigger on, depending on current state of the switches
        P1IES = (P1IES & ~MASK) | (P1IN & MASK);

	// Light-show during startup
	cycle(3, 50);

	// Read switches at startup to determine working mode
	BINVAL=((P1IN & SW0)?1:0)*1 + ((P1IN & SW1)?1:0)*2 + ((P1IN & SW2)?1:0)*4 + ((P1IN & SW3)?1:0)*8;
	display(BINVAL);
	GMODE=BINVAL;

	if(GMODE==1) {
		// set some flags to control behaviour of switch interrupt and timer interrupt routines
	}
	else if(GMODE==2){
		mybeep(200);
	}

//	__disable_interrupt();  

	P1IFG &= ~MASK; 	// clear IFG for our switches
	__enable_interrupt(); 	// enable all interrupts

	// Select mode of operation
	// 1 -  Game
	// Show a random hex number (blink?) - wait for player to set the correct bits
	// Count down time, adjust audio tick period
	// In this mode, do not update display when bits are toggled, but do check state agains correct answer
	for(;;)
	{}
}


// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	// Debounce
	delay(5);

	check_state_and_update_edge();
	P1IFG &= ~MASK; 			 // clear IFG 
}


void check_state_and_update_edge(void)
{
	// Read state of switches and convert bits to number
	BINVAL=((P1IN & SW0)?1:0)*1 + ((P1IN & SW1)?1:0)*2 + ((P1IN & SW2)?1:0)*4 + ((P1IN & SW3)?1:0)*8;

	// Here we check the game mode and take different actions
	switch(GMODE) {
		case 1:
			display(BINVAL);
			mybeep(2); // should instead signal that we want to beep
				// and let a timer interrupt handle it.
				// i.e. beep (toggle the ouput pin at desired interval)
				// until enough time has elapsed.
			break;
	   	case 2:
			break;
		default:
			display(BINVAL);
	}

	P1IES = (P1IES & ~MASK) | (P1IN & MASK); // update interrupt edge
}


void mybeep(unsigned int duration)
{
	unsigned int i;

	for(i=0; i<duration; i++){
		P1OUT |= BEEP;
		delay(2);
		P1OUT ^= BEEP;
		delay(2);
	}
}

void cycle(unsigned int ncycles, unsigned int cdelay)
{
	int j;

	shiftout((unsigned char)0x00);
	//delay(50);
	// Cycle display leds
	for(j=0; j<6*ncycles; j++) {
		shiftout(cycle6[j%6]);
		delay(cdelay);
	}
	shiftout((unsigned char)0x00);
	//delay(50);
}


void display(unsigned char num)
{
	shiftout(digits[num]);
}

void cleardisplay(void)
{
	shiftout(0x00);
}


// Pulse the clock pin
void pulseclock(void)
{
  P1OUT |= CLK;
//delay(1);
  P1OUT ^= CLK;
}


// Take the given 8-bit value and shift it out, LSB to MSB
void shiftout(unsigned char val)
{
  char i;

  // Iterate over each bit, set data pin, and pulse the clock to send it to the shift register
  for (i = 0; i<8; i++) {
      pinwrite(DATA, (val & (1 << i)));
//delay(1);
      pulseclock();
  }
}

void pinwrite(unsigned int bit, unsigned char val)
{
  if(val){
    P1OUT |= bit;
  } else {
    P1OUT &= ~bit;
  }
}

void delay(unsigned int ms)
{
 while(ms--)
    {
        __delay_cycles(1000); // set for 16Mhz change it to 1000 for 1 Mhz
    }
}
