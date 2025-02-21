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
int main(){
    // Set the clocking to run directly from the crystal at 16MHz
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

    // Configure Wide Timer0A as a 16-bit periodic timer. The timer will be in the half-width mode. The timer will be periodic. The timer will count down.
    TimerConfigure(WTIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC));

    // Interrupt Configurations

    // Enable the interrupt in the NVIC for Wide Timer0A
    IntEnable(INT_WTIMER0A);

    // Register the interrupt handler for Wide Timer0A
    TimerIntRegister(WTIMER0_BASE,TIMER_A,handler);

    // Set the priority of the Wide Timer0A interrupt to 0
    IntPrioritySet(INT_WTIMER0A,0X00);

    // Enable the match interrupt for Wide Timer0A (match interrupt)
    TimerIntEnable(WTIMER0_BASE,TIMER_TIMA_MATCH);

    // Clear the interrupt status of Wide Timer0A for the match interrupt
    TimerIntClear(WTIMER0_BASE,TIMER_TIMA_MATCH);

    // as the clock source is 16 mhz and by using prescaler of 100 it counts 0.5 s in 80000 cycles
    TimerLoadSet(WTIMER0_BASE,TIMER_A,80000);

    // Set the match t0 40000
    TimerMatchSet(WTIMER0_BASE,TIMER_A,40000); // now the interrupt will be generated when the counter reaches 40000 which is after 0.25 seconds

    // Set the prescaler value to 1000
    TimerPrescaleSet(WTIMER0_BASE, TIMER_A, 100);

    // Enable Wide Timer0A
    TimerEnable(WTIMER0_BASE,TIMER_A);

    // Enable the master interrupt globally
    IntMasterEnable();

    // Set the pin 2 in PORTF to high
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);

    while(1){

    }

}
// ISR
void handler(){
    if ( TimerIntStatus(WTIMER0_BASE,1) == TIMER_TIMA_MATCH ){                              // Check if the interrupt is due to the match interrupt
        TimerIntClear(WTIMER0_BASE,TIMER_TIMA_MATCH);                                       // Clear the interrupt status
        TimerLoadSet(WTIMER0_BASE,TIMER_A,80000);                                           // Load the value of the timer to count down again from 80000
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,~GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_2)) ; // Toggle THE PIN
    }
}
