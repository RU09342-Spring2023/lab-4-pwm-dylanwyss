/*
 * part1.c
 *
 *  Created on: Feb 21, 2023
 *      Author: Dylan Wyss
 *
 */

#define DUTY_CYCLE_50 0
#define DUTY_CYCLE_60 1
#define DUTY_CYCLE_70 2
#define DUTY_CYCLE_80 3
#define DUTY_CYCLE_90 4
#define DUTY_CYCLE_100 5
#define DUTY_CYCLE_0 6
#define DUTY_CYCLE_10 7
#define DUTY_CYCLE_20 8
#define DUTY_CYCLE_30 9
#define DUTY_CYCLE_40 10

#include <msp430.h>

unsigned int red_state = DUTY_CYCLE_50;
//unsigned int green_state = DUTY_CYCLE_50;

unsigned short initial_red_cycle = 500;           // 100 * (50/100)
//unsigned short initial_green_cycle = 500;

void main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    LEDSetup();     // Initialize our LEDS
    ButtonSetup();  // Initialize our button
    TimerA0Setup(); // Initialize Timer0
    TimerA1Setup(); // Initialize Timer1

    PM5CTL0 &= ~LOCKLPM5;

    __bis_SR_register(LPM0_bits | GIE);       // Enter LPM0 w/ interrupt
    __no_operation();                         // For debugger
}

void LEDSetup(){
    // Configure red LED on P1.0 as Output
    P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                          // Set P1.0 to output direction

    // Configure green LED on P6.6 as Output
    P6OUT &= ~BIT6;                         // Clear P6.6 output latch for a defined power-on state
    P6DIR |= BIT6;                          // Set P6.6 to output direction
}

void ButtonSetup(){
    // Configure Button on P2.3 as input with pullup resistor
    P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
    P2REN |= BIT3;                          // P2.3 pull-up register enable
    P2IES &= ~BIT3;                         // P2.3 Low --> High edge
    P2IE |= BIT3;                           // P2.3 interrupt enabled

    // Configure Button on P4.1 as input with pullup resistor
    P4OUT |= BIT1;                          // Configure P4.1 as pulled-up
    P4REN |= BIT1;                          // P4.1 pull-up register enable
    P4IES &= ~BIT1;                         // P4.1 Low --> High edge
    P4IE |= BIT1;                           // P4.1 interrupt enabled
}

void TimerA0Setup(){
    // Setup Timer 0
    TB0CTL = TBSSEL_2 | MC_UP | TBCLR | TBIE;      // SMCLK, up mode, clear
    TB0CCTL1 |= CCIE;                       // Enable TB0 CCR0 Overflow IRQ
    TB0CCR1 = initial_red_cycle;                   // CCR0 initialized to duty cycle value
}
/*
void TimerA1Setup(){
    // Setup Timer 1
    TB1CTL = TBSSEL_2 | MC_UP | TBCLR | TBIE;      // SMCLK, up mode, clear
    TB1CCTL1 |= CCIE;                       // Enable TB0 CCR0 Overflow IRQ
    TB1CCR1 = initial_green_cycle;                   // CCR0 initialized to duty cycle value
}
*/
// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    if (red_state < 10)
        red_state++;
    else
        red_state = 0;
}
/*
// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{

}
*/
// Timer0_B1 interrupt service routine
#pragma vector = TIMER0_B1_VECTOR
__interrupt void Timer0_B1_ISR(void)
{
    switch(red_state)
    case DUTY_CYCLE_50:
        red_duty_add = 0;
    case DUTY_CYCLE_60:
        red_duty_add = 100;
    case DUTY_CYCLE_70:
        red_duty_add = 200;
    case DUTY_CYCLE_80:
        red_duty_add = 300;
    case DUTY_CYCLE_90:
        red_duty_add = 400;
    case DUTY_CYCLE_100:
        red_duty_add = 500;
    case DUTY_CYCLE_0:
        red_duty_add = -500;
    case DUTY_CYCLE_10:
        red_duty_add = -400;
    case DUTY_CYCLE_20:
        red_duty_add = -300;
    case DUTY_CYCLE_30:
        red_duty_add = -200;
    case DUTY_CYCLE_40:
        red_duty_add = -100;
}

// Timer1_B1 interrupt service routine
#pragma vector = TIMER1_B1_VECTOR
__interrupt void Timer1_B1_ISR(void)
{
    P1OUT ^= BIT0;                          // Toggle Red LED
    TB1CCR1 += red_duty_add;     // Increment time between interrupts
}

/*
// Timer2_B1 interrupt service routine
#pragma vector = TIMER0_B1_VECTOR
__interrupt void Timer2_B1_ISR(void)
{

}

// Timer3_B1 interrupt service routine
#pragma vector = TIMER1_B1_VECTOR
__interrupt void Timer3_B1_ISR(void)
{
    P6OUT ^= BIT6;                          // Toggle Green LED
    TB1CCR1 += green_duty_cycle;   // Increment time between interrupts}
*/
