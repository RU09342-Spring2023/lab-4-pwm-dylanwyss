/*
 * part1.c
 *
 *  Created on: Feb 21, 2023
 *      Author: Dylan Wyss
 *
 *  Code from in-class hardware PWM example used for assistance
 */

#define PERIOD 1000
#define RED_TO_GREEN 0
#define GREEN_TO_BLUE 1
#define BLUE_TO_RED 2

#include <msp430.h>

// Red LED initialized on
char LEDstate = RED_TO_GREEN;                // LED goes RED -> orange -> GREEN -> cyan -> BLUE -> purple -> RED ...

void LEDSetup();
void TimerSetup();

void main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    LEDSetup();     // Initialize our LEDs
    TimerSetup();  // Initialize Timer

    PM5CTL0 &= ~LOCKLPM5;

    __bis_SR_register(LPM0_bits | GIE);       // Enter LPM0 w/ interrupt
    __no_operation();                         // For debugger
}

void LEDSetup(){
    // Configure P6.0-6.2 as Outputs
    P6DIR |= BIT0;                          // Set P6.0 to RED output
    P6DIR |= BIT1;                          // Set P6.1 to GREEN output
    P6DIR |= BIT2;                          // Set P6.2 to BLUE output

    // Initialize pins to power-power off state
    P6OUT &= ~BIT0;                         // P6.0 to power-off state
    P6OUT &= ~BIT1;                         // P6.1 to power-off state
    P6OUT &= ~BIT2;                         // P6.2 to power-off state

    // Initialize select pins
    P6SEL0 |= BIT0;
    P6SEL0 |= BIT1;
    P6SEL0 |= BIT2;
    P6SEL1 &= ~BIT0;
    P6SEL1 &= ~BIT1;
    P6SEL1 &= ~BIT2;

    // Enable interrupts
    P6IE |= BIT0;                           // P6.0 interrupt enabled
    P6IE |= BIT1;                           // P6.1 interrupt enabled
    P6IE |= BIT2;                           // P6.2 interrupt enabled
}

void TimerSetup(){
    TB3CCR0 = PERIOD - 1;                     // PWM Period
    TB3CTL = TBSSEL__SMCLK | MC__UP | TBCLR;  // SMCLK, up mode, clear TBR
    TB3CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TB3CCTL2 = OUTMOD_7;                      // CCR2 reset/set
    TB3CCTL3 = OUTMOD_7;                      // CCR3 reset/set

    // Initial duty cycle for each LED
    TB3CCR1 = PERIOD - 1;                       // red LED on
    TB3CCR2 = 0;                                // green LED off
    TB3CCR3 = 0;                                // blue LED off

    // Setup Timer Compare IRQ
    TB1CCTL0 |= CCIE;                           // Enable TB1 CCR0 Overflow IRQ
    TB1CCR0 = 1;                                // Initial Period
    TB1CTL = TBSSEL_1 | MC_2 | ID_3 | TBCLR | TBIE;
                                                // ACLK, continuous mode, /8, clear TBR, enable interrupt
}

// Timer B1 interrupt service routine
#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
{
    switch(LEDstate){
    case RED_TO_GREEN:
        TB3CCR1--;                          // decrease RED to 0%
        TB3CCR2++;                          // increase GREEN to 100%
        if (TB3CCR1 == 0)
            LEDstate = GREEN_TO_BLUE;
        break;
    case GREEN_TO_BLUE:
        TB3CCR2--;                          // decrease GREEN to 0%
        TB3CCR3++;                          // increase BLUE to 100%
        if (TB3CCR2 == 0)
            LEDstate = BLUE_TO_RED;
        break;
    case BLUE_TO_RED:
        TB3CCR3--;                          // decrease BLUE to 0%
        TB3CCR1++;                          // increase RED to 100%
        if (TB3CCR3 == 0)
            LEDstate = RED_TO_GREEN;
        break;
    }

    if(TB3R >= 60000)
        TB3R = 1;                           // Reset timer register value to not go over 65535

    TB3CCR1 = TB3CCR1;
    TB3CCR2 = TB3CCR2;
    TB3CCR3 = TB3CCR3;
    TB1CCR0 += 20;                          // sets speed of fade
}

