/*
 * part3.c
 *
 *  Created on: Mar 23, 2023
 *      Author: Dylan Wyss
 *
 *  Code taken primarily from part 1
 */

#include <msp430.h>

void ServoSetup();
void ButtonSetup();
void TimerA0Setup();

void main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    ServoSetup();     // Initialize our LEDs
    ButtonSetup();  // Initialize our button
    TimerA0Setup(); // Initialize Timer0

    PM5CTL0 &= ~LOCKLPM5;

    __bis_SR_register(LPM0_bits | GIE);       // Enter LPM0 w/ interrupt
    __no_operation();                         // For debugger
}

void ServoSetup(){
    // Configure P6.0 as Output
    P6OUT &= ~BIT0;                         // Clear P6.0 output latch for a defined power-on state
    P6DIR |= BIT0;                          // Set P6.0 to output direction
}

void ButtonSetup(){
    // Configure Button on P2.3 as input with pullup resistor
    P2DIR &= ~BIT3;                         // Set P2.3 to input direction
    P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
    P2REN |= BIT3;                          // P2.3 pull-up register enable
    P2IES &= ~BIT3;                         // P2.3 Low --> High edge
    P2IE |= BIT3;                           // P2.3 interrupt enabled

    // Configure Button on P4.1 as input with pullup resistor
    P4DIR &= ~BIT1;                         // Set P4.1 to input direction
    P4OUT |= BIT1;                          // Configure P4.1 as pulled-up
    P4REN |= BIT1;                          // P4.1 pull-up register enable
    P4IES &= ~BIT1;                         // P4.1 Low --> High edge
    P4IE |= BIT1;                           // P4.1 interrupt enabled
}

void TimerA0Setup(){
    // Setup Timer 0
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBIE;       // SMCLK, UP mode
    TB0CCTL1 |= CCIE;                             // Enable TB0 CCR1 Interrupt
    TB0CCTL2 |= CCIE;                             // Enable TB0 CCR2 Interrupt
    TB0CCR0 = 20000;                              // Set CCR0 to the value to set the period
    TB0CCR1 = 10000;                              // Set CCR1 to the duty cycle
}

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;
    if (TB0CCR1 < 19000)
        TB0CCR1 += 100;                            // Rotate servo clockwise
}

// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;
    if (TB0CCR1 > 1000)
        TB0CCR1 -= 100;                            // Rotate servo counter-clockwise
}

// Timer0_B1 Interrupt Vector (TBIV) handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B1_VECTOR))) TIMER0_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(TB0IV,TB0IV_TBIFG))
    {
        case TB0IV_NONE:
            break;                       // No interrupt
        case TB0IV_TBCCR1:
            if (TB0CCR1)
                P6OUT |= BIT0;           // overflow Set the pin to a 1
            break;
        case TB0IV_TBCCR2:
            break;
        case TB0IV_TBIFG:
            P6OUT &= ~BIT0;              // CCR1 Set the pin to a 0
            break;
        default:
            break;
    }
}



