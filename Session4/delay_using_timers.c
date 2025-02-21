#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "inc/hw_types.h"
#include "driverlib/interrupt.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ints.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"

void handler();
void delay(int time);
int delay_finished = 0;
int main(){
    // Set the clocking to run directly from the crystal at 50MHz
    SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_OSC | SYSCTL_SYSDIV_1);

    // disable the master interrupt globally
    IntMasterDisable();

    // Enable the peripherals used by this program.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);   // Enable wide Timer 0

    // Wait for the wide Timer0 module to be ready.
    while (! SysCtlPeripheralReady(SYSCTL_PERIPH_WTIMER0)){}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // Enable PORTF

    // Wait for the PORTF module to be ready.
    while (! SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}

    // Set pin 2 in PORTF as output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_2);

    // Timer Configurations

    // Set the clock source for Wide Timer0 as the system clock
    TimerClockSourceSet(WTIMER0_BASE,TIMER_CLOCK_SYSTEM);

    // Configure Wide Timer0A as a 16-bit periodic timer. The timer will be in the half-width mode. The timer will be one shot to count only one time . The timer will count down.
    TimerConfigure(WTIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT));

    // Interrupt Configurations

    // Enable the interrupt in the NVIC for Wide Timer0A
    IntEnable(INT_WTIMER0A);

    // Register the interrupt handler for Wide Timer0A
    TimerIntRegister(WTIMER0_BASE,TIMER_A,handler);

    // Set the priority of the Wide Timer0A interrupt to 0
    IntPrioritySet(INT_WTIMER0A,0X00);

    // Enable the timeout interrupt for Wide Timer0A (Match interrupt)
    TimerIntEnable(WTIMER0_BASE,TIMER_TIMA_TIMEOUT);

    // Clear the interrupt status of Wide Timer0A for the timeout interrupt
    TimerIntClear(WTIMER0_BASE,TIMER_TIMA_TIMEOUT);

    // Set the prescaler value to 1000 in order to let every 16 clock cycles to be equal to 1 millisecond 
    // for wide timer the prescaler can be any value between 1 and 65536
    // but for normal timers the prescaler can be any value between 1 and 255
    TimerPrescaleSet(WTIMER0_BASE, TIMER_A, 1000);

    // Enable the master interrupt globally
    IntMasterEnable();

    // Set the pin 2 in PORTF to high
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);

    while(1){
        delay(1000);
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0);
        delay(1000);
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);

    }

}
// ISR
void handler(){
    if ( TimerIntStatus(WTIMER0_BASE,1) == TIMER_TIMA_TIMEOUT ){      // Check if the interrupt is due to the timeout interrupt
        TimerIntClear(WTIMER0_BASE,TIMER_TIMA_TIMEOUT);               // Clear the interrupt status
        delay_finished = 1;                                           // Set the flag to indicate that the delay has finished
    }
}

void delay(int time){
    delay_finished = 0;                                        // Reset the flag
    // the timer is loaded with value which corresponds to the required delay and it operates as one shot so after the timeout it does not restart counting again 
    TimerLoadSet(WTIMER0_BASE,TIMER_A,time*16);            // Load the value of the timer to count down from 16 * number of milliseconds in the delay functions 
    TimerEnable(WTIMER0_BASE,TIMER_A);                     // enable the counter to start counting              
    while(!delay_finished);                               // Wait for the flag to be set by the timeout interrupt from the timer 
}
