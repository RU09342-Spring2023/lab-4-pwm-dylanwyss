# Servo Control
The objective of this part of the lab is to control a servo via PWM. This was done by taking the program written for Part 1 (software PWM) and making several changes. These changes are detailed as follows: 

Configure pin 6.0 as the output pin for the servo:
```c
    P6OUT &= ~BIT0;                         // Clear P6.0 output latch for a defined power-on state
    P6DIR |= BIT0;                          // Set P6.0 to output direction
```

Timer configured as follows:
```c
    TB0CTL = TBSSEL__SMCLK | MC__UP | TBIE;       // SMCLK, UP mode
    TB0CCTL1 |= CCIE;                             // Enable TB0 CCR1 Interrupt
    TB0CCTL2 |= CCIE;                             // Enable TB0 CCR2 Interrupt
    TB0CCR0 = 20000;                              // Set CCR0 to the value to set the period
    TB0CCR1 = 10000;                              // Set CCR1 to the duty cycle
```

Interrupt for button 2.3:
If the duty cycle is less than 19000 (95%) rotate servo clockwise.
```c
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;
    if (TB0CCR1 < 19000)
        TB0CCR1 += 100;                            // Rotate servo clockwise
}
```

Interrupt for button 4.1:
If the duty cycle is greater than 1000 (5%) rotate servo counter-clockwise.
```c
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;
    if (TB0CCR1 > 1000)
        TB0CCR1 -= 100;                            // Rotate servo counter-clockwise
}
```

Other than the button interrupts, this program is identical to that of Part 1 aside from changing the pins used.
