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

void PWM();
float duty_cycle = 0.9 ;
int frequency = 50 ;
int main(){
    // Set the clocking to run directly from the crystal at 50MHz
    SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_OSC | SYSCTL_SYSDIV_1);

    // disable the master interrupt globally
    IntMasterDisable();

    // Enable the peripherals used by this program.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);   // Enable Timer 0

    // Wait for the Timer0 module to be ready.
    while (! SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)){}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // Enable PORTF

    // Wait for the PORTF module to be ready.
    while (! SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)){}

    // Set pin 2 in PORTF as output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_2);

    // Timer Configurations

    // Set the clock source for Timer0 as the system clock
    TimerClockSourceSet(TIMER0_BASE,TIMER_CLOCK_SYSTEM);

    // Configure Timer0A as a 16-bit periodic timer. The timer will be in the half-width mode. The timer will be periodic. The timer will count down.
    TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC));

    // Interrupt Configurations

    // Enable the interrupt in the NVIC for Timer0A
    IntEnable(INT_TIMER0A);

    // Register the interrupt handler for Timer0A
    TimerIntRegister(TIMER0_BASE,TIMER_A,PWM);

    // Set the priority of the Timer0A interrupt to 0
    IntPrioritySet(INT_TIMER0A,0X00);
    
    // Enable the timeout and match interrupt for Timer0A (Overflow and match interrupt)
    TimerIntEnable(TIMER0_BASE,TIMER_TIMA_MATCH | TIMER_TIMA_TIMEOUT);

    // Clear the interrupt status of Timer0A for both the timeout and match interrupt
    TimerIntClear(TIMER0_BASE,TIMER_TIMA_MATCH | TIMER_TIMA_TIMEOUT);

    // Load the value of the timer to count down from for a 50Hz frequency
    // as the clock source is 16 mhz and by using prescaler of 255 we achieve 1 second by counting till 62754 (16Mhz/255)
    // so at any given frequency we can calculate the value to load by 62745/frequency

    TimerLoadSet(TIMER0_BASE,TIMER_A,62745/frequency);

    // Load the value of the match register to set the duty cycle
    // The match register is responsible for setting the duty cycle of the PWM signal
    // The match register is set to 90% of the period to achieve a duty cycle of 90%
    TimerMatchSet(TIMER0_BASE,TIMER_A,62745/frequency*duty_cycle);

    // Set the prescaler value to 255
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 255);

    // Enable Timer0A
    TimerEnable(TIMER0_BASE,TIMER_A);

    // Enable the master interrupt globally
    IntMasterEnable();

    // Set the pin 2 in PORTF to high
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);

    while(1){

    }

}
// ISR 
void PWM(){
    if (TimerIntStatus(TIMER0_BASE,1) == TIMER_TIMA_MATCH) {    // Check if the interrupt is due to the match interrupt
        TimerIntClear(TIMER0_BASE,TIMER_TIMA_MATCH);            // Clear the interrupt status
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2) ;   // Set the pin 2 in PORTF to high   
    }
    else if ( TimerIntStatus(TIMER0_BASE,1) == TIMER_TIMA_TIMEOUT ){ // Check if the interrupt is due to the timeout interrupt
        TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);              // Clear the interrupt status
        GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0) ;                // Set the pin 2 in PORTF to low
    }
}
