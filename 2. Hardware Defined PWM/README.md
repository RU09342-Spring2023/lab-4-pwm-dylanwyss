# Hardware PWM
The purpose of this part of the lab is to create a hardware PWM implementation using a RGB LED. The LED is intended to cycle through colors as follows: Red -> Orange -> Green -> Cyan -> Blue -> Purple -> Red and so on. The LED is connected to the MSP430 board with the four pins of the RGB LED being connected to pins P6.0 (red), P6.1 (green), P6.2 (blue) and GND (ground). Additionally, each LED was connected to a resistor to limit the current flow.

The first step of this program was the initialization, which is as follows. Period was defined to be 1000. Three states were defined for each of the 3 stages of the LED fading. The MSP430 library was included. The varaible LEDstate was defined to keep track of which of the three states the fading is in. Lastly, two functions were created for initialization purposes.
```c
#define PERIOD 1000
#define RED_TO_GREEN 0
#define GREEN_TO_BLUE 1
#define BLUE_TO_RED 2

#include <msp430.h>

// Red LED initialized on
char LEDstate = RED_TO_GREEN;                // LED goes RED -> orange -> GREEN -> cyan -> BLUE -> purple -> RED ...

void LEDSetup();
void TimerSetup();
```

Next, in the main function, the two initializtion functions are called, along with the standard protocols taken here such as the watchdog timer being initalized. The main function here is very similar to that of Part 1.
```c
void main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    LEDSetup();     // Initialize our LEDs
    TimerSetup();  // Initialize Timer

    PM5CTL0 &= ~LOCKLPM5;

    __bis_SR_register(LPM0_bits | GIE);       // Enter LPM0 w/ interrupt
    __no_operation();                         // For debugger
}
```

Each of the three pins used for the exterior LED are set up in the function LEDSetup. Each LED is set to an output and initialized to be powered off. Next, the select pins are configured. Lastly, interrupts are enabled for each LED as this is how the LED fading will be handled.
```c
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
```

Next up is the initializations for each timer in the function TimerSetup. There were two timers used in this program, Timer 1 which controls how often the LED changes colors and Timer 3 which changes the duty cycle of each of the three LEDs. For Timer 3, the timer is configured using SMCLK in up mode. CCR0 is set to the value of period - 1, and CCR1 through CCR3 control the duty cycle of each LED. CCR1 is for red, CCR2 is for green, and CCR3 is for blue. Since the LED begins as the color red, CCR1 is initialized to a period of 999, which is the max duty cycle while the two other LEDs are initialized to a duty cycle of 0. For Timer 1, the timer is onfigured using ACLK in continuous mode. The period is set to 1 as the LEDs need to continously be fading.
```c
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
```

The final part of this program is the interrupt for Timer 1. I will break interrupt down into smaller parts and explain each.
```c
// Timer B1 interrupt service routine
#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
{
    switch(LEDstate){
```
While the LED state is in between red and green, the red LED fades until it reaches a duty cycle of 0% and the green LED increases to a duty cycle of 100%. Somewhere in the middle, the combination of LEDs will create the color orange. When CCR1 which holds the duty cycle for red reaches 0, the LED then switches into the state between green and blue.
```c
    case RED_TO_GREEN:
        TB3CCR1--;                          // decrease RED to 0%
        TB3CCR2++;                          // increase GREEN to 100%
        if (TB3CCR1 == 0)
            LEDstate = GREEN_TO_BLUE;
        break;
```
While the LED state is in between green and blue, the green LED fades until it reaches a duty cycle of 0% and the blue LED increases to a duty cycle of 100%. Somewhere in the middle, the combination of LEDs will create the color cyan. When CCR2 which holds the duty cycle for green reaches 0, the LED then switches into the state between blue and red.
```c
      case GREEN_TO_BLUE:
        TB3CCR2--;                          // decrease GREEN to 0%
        TB3CCR3++;                          // increase BLUE to 100%
        if (TB3CCR2 == 0)
            LEDstate = BLUE_TO_RED;
        break;
```
While the LED state is in between blue and red, the blue LED fades until it reaches a duty cycle of 0% and the red LED increases to a duty cycle of 100%. Somewhere in the middle, the combination of LEDs will create the color purple. When CCR3 which holds the duty cycle for blue reaches 0, the LED then switches into the state between red and green.
```c
    case BLUE_TO_RED:
        TB3CCR3--;                          // decrease BLUE to 0%
        TB3CCR1++;                          // increase RED to 100%
        if (TB3CCR3 == 0)
            LEDstate = RED_TO_GREEN;
        break;
    }
```
If the timer register value reaches 60,000 it will be reset back to 1. This is so the capture control register does not hold a value that is greater than 65,535 which is the maximum value a 16-bit register can hold. Likewise, here TB1CCR0 increases by 20, which sets the speed of the LED. I attempted the functions 'TB1CCR0++' which faded the LED too quickly and 'TB1CCR0' which faded the LED too slowly. I untimately decided on 20 for an increment value that faded the LED at a reasonable speed.
```c
    if(TB3R >= 60000)
        TB3R = 1;                           // Reset timer register value to not go over 65535

    TB3CCR1 = TB3CCR1;
    TB3CCR2 = TB3CCR2;
    TB3CCR3 = TB3CCR3;
    TB1CCR0 += 20;                          // sets speed of fade
}
```
