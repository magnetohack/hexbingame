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

#define MASK SW0+SW1+SW2+SW3

// Declare functions
void delay(unsigned int);
void pulseclock(void);
void shiftout(unsigned char);
void pinwrite(unsigned int, unsigned char);
void display(unsigned char);
void cleardisplay(void);

/*
       0x80
      ------
 0x04 |0x02| 0x40
      ------
 0x08 |    | 0x20
      ------ o 0x01
       0x10
*/


//unsigned char digits[16] = {B01111110, B00001010, B10110110, B10011110, B11001010, B11011100, B11111100, B00001110, B11111110, B11011110};
//unsigned char digits[16] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0xFB, 0x60, 0xEB, 0x00, 0x00, 0x00, 0x00, 0x00};
//                           0     1     2     3     4     5     6     7     8     9     A     b     C     d     E     F
unsigned char digits[16] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xE6, 0xEF, 0x3F, 0x9D, 0x7B, 0x9F, 0x8F};
//                           0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f
//unsigned char digits[16] = {0xFC, 0x60, 0xDA, 0xF2, 0x66, 0xB6, 0xBE, 0xE0, 0xFE, 0xE6, 0xFB, 0x3F, 0x1B, 0x7B, 0xDF, 0x8F};

unsigned char cycle6[6] = {0x10, 0x08, 0x04, 0x80, 0x40, 0x20};

int main(void)
{
	unsigned char j;
	unsigned char mode;

	WDTCTL = WDTPW + WDTHOLD; 	// Stop watchdog timer

	P1DIR |= (CLK + DATA); 		// Set clock and data pins to output direction 

	//P1DIR &= ~(SW0 + SW1 + SW2 + SW3);  // Set as inputs (anyway done by default)
	//P1DIR &= ~MASK;  // Set as inputs (anyway done by default)
	P1DIR &= ~SW0;  // Set as inputs (anyway done by default)
	P1DIR &= ~SW1;  // Set as inputs (anyway done by default)
	P1DIR &= ~SW2;  // Set as inputs (anyway done by default)
	P1DIR &= ~SW3;  // Set as inputs (anyway done by default)

//	P1REN |= MASK;	//Enable resistor pull
	P1REN |= SW0;	//Enable resistor pull
	P1REN |= SW1;	//Enable resistor pull
	P1REN |= SW2;	//Enable resistor pull
	P1REN |= SW3;	//Enable resistor pull
	//P1OUT |= SW0; //Select Pullup resistor for P1.3
//	P1OUT &= ~MASK; //Select Pulldown resistor for P1.3
	P1OUT &= ~SW0;
	P1OUT &= ~SW1;
	P1OUT &= ~SW2;
	P1OUT &= ~SW3;

	//P1IE |= MASK; 	// interrupt enable
	P1IE |= SW0; 	// interrupt enable
	P1IE |= SW1; 	// interrupt enable
	P1IE |= SW2; 	// interrupt enable
	P1IE |= SW3; 	// interrupt enable
	// Set which edge for the interrupt to trigger on, depending on current state of the switches
//	P1IES |= (P1IN & MASK);  // 1:High -> Low edge, 0:Low->High edge
	if(P1IN & SW0) P1IES |= SW0; else P1IES &= ~SW0; 
	if(P1IN & SW1) P1IES |= SW1; else P1IES &= ~SW1; 
	if(P1IN & SW2) P1IES |= SW2; else P1IES &= ~SW2; 
	if(P1IN & SW3) P1IES |= SW3; else P1IES &= ~SW3; 


	//P1IFG &= ~MASK; 	// clear IFG for our switches
	P1IFG &= ~SW0; 	// clear IFG for our switches
	P1IFG &= ~SW1; 	// clear IFG for our switches
	P1IFG &= ~SW2; 	// clear IFG for our switches
	P1IFG &= ~SW3; 	// clear IFG for our switches
	__enable_interrupt(); 	// enable all interrupts

	shiftout((unsigned char)0x00);
	delay(50);
	// Cycle display leds
	for(j=0; j<12; j++) {
		shiftout(cycle6[j%6]);
		delay(50);
	}
	shiftout((unsigned char)0x00);
	delay(50);
	// Read switches at startup to determine working mode
	mode=((P1IN & SW0)?1:0)*1 + ((P1IN & SW1)?1:0)*2 + ((P1IN & SW2)?1:0)*4 + ((P1IN & SW3)?1:0)*8;
	display(mode);

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
//P1OUT ^= (CLK + DATA); // P1.0 = toggle
// Debounce?
// keep reading the input for while?
delay(5);

display( ((P1IN & SW0)?1:0)*1 + ((P1IN & SW1)?1:0)*2 + ((P1IN & SW2)?1:0)*4 + ((P1IN & SW3)?1:0)*8 );

//P1IFG &= ~MASK; 	// clear IFG 
P1IFG &= ~SW0; 	// clear IFG 
P1IFG &= ~SW1; 	// clear IFG 
P1IFG &= ~SW2; 	// clear IFG 
P1IFG &= ~SW3; 	// clear IFG 

// update interrupt edge
if(P1IN & SW0) P1IES |= SW0; else P1IES &= ~SW0; 
if(P1IN & SW1) P1IES |= SW1; else P1IES &= ~SW1; 
if(P1IN & SW2) P1IES |= SW2; else P1IES &= ~SW2; 
if(P1IN & SW3) P1IES |= SW3; else P1IES &= ~SW3; 

//P1IES ^= (P1IN & MASK); // toggle the interrupt edge,
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
