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
int LEDstate = RED_TO_GREEN;                // LED goes RED -> orange -> GREEN -> cyan -> BLUE -> purple -> RED ...

void LEDSetup();
void TimerSetup();
void ChangeBrightness();

void main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    LEDSetup();     // Initialize our LEDs
    TimerSetup();  // Initialize Timer

    PM5CTL0 &= ~LOCKLPM5;

    __bis_SR_register(LPM0_bits | GIE);       // Enter LPM0 w/ interrupt
    __no_operation();                         // For debugger

    while(1){
        ChangeBrightness();
    }
}

void LEDSetup(){
    // Configure P1.5-1.7 as Outputs
    P6DIR |= BIT0;                          // Set P6.0 to RED output
    P6DIR |= BIT1;                          // Set P6.1 to GREEN output
    P6DIR |= BIT2;                          // Set P6.2 to BLUE output

    // Initialize Outputs to off state
    P6OUT &= ~BIT0;                         // P6.0 to power-off state
    P6OUT &= ~BIT1;                         // P6.1 to power-off state
    P6OUT &= ~BIT2;                         // P6.2 to power-off state

    // Enable interrupts
    P6IE |= BIT0;                           // P4.1 interrupt enabled
    P6IE |= BIT1;                           // P4.1 interrupt enabled
    P6IE |= BIT2;                           // P4.1 interrupt enabled

    // Initialize select pins
    P6SEL0 |= BIT0;
    P6SEL0 |= BIT1;
    P6SEL0 |= BIT2;
    P6SEL1 &= ~BIT0;
    P6SEL1 &= ~BIT1;
    P6SEL1 &= ~BIT2;
}

void TimerSetup(){
    TB3CCR0 = PERIOD - 1;                     // PWM Period
    TB3CTL = TBSSEL__SMCLK | MC__UP | TBCLR;  // SMCLK, up mode, clear TBR
    TB3CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TB3CCTL2 = OUTMOD_7;                      // CCR2 reset/set
    TB3CCTL3 = OUTMOD_7;                      // CCR3 reset/set

    // initializing the CC registers for initial state red LED on
    TB3CCR1 = PERIOD;                           // red LED on
    TB3CCR2 = 0;                                // green LED off
    TB3CCR3 = 0;                                // blue LED off
}

void ChangeBrightness(){
    switch(LEDstate){
    case RED_TO_GREEN:
        while (TB3CCR1 > 0)
        {
            TB3CCR1--;                          // decrease RED to 0
            TB3CCR2++;                          // increase GREEN to 1000
        }
        LEDstate = GREEN_TO_BLUE;
        break;
    case GREEN_TO_BLUE:
        while (TB3CCR1 > 0)
        {
            TB3CCR2--;                          // decrease GREEN to 0
            TB3CCR3++;                          // increase BLUE to 1000
        }
        LEDstate = BLUE_TO_RED;
        break;
    case BLUE_TO_RED:
        while (TB3CCR1 > 0)
        {
            TB3CCR3--;                          // decrease BLUE to 0
            TB3CCR1++;                          // increase RED to 1000
        }
        LEDstate = BLUE_TO_RED;
        break;
    }
}

// Timer3_B1 Interrupt Vector (TBIV) handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER3_B1_VECTOR
__interrupt void TIMER3_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER3_B1_VECTOR))) TIMER3_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(TB3IV,TB3IV_TBIFG))
    {
        case TB3IV_NONE:
            break;                               // No interrupt
        case TB3IV_TBCCR1:
            P6OUT &= ~BIT0;
            break;                               // CCR1 Set the pin to a 0
        case TB3IV_TBCCR2:
            P6OUT &= ~BIT1;
            break;                               // CCR2 Set the pin to a 0
        case TB3IV_TBCCR3:
            P6OUT &= ~BIT2;
            break;                               // CCR3 Set the pin to a 0
        case TB3IV_TBIFG:
            P6OUT |= BIT0;
            P6OUT |= BIT1;
            P6OUT |= BIT2;
            break;                               // overflow Set the pin to a 1
        default:
            break;
    }
}
