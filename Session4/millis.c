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

unsigned long millis();
void Handler();
volatile unsigned long millisCount = 0;

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
    TimerIntRegister(TIMER0_BASE,TIMER_A,Handler);

    // Set the priority of the Timer0A interrupt to 0
    IntPrioritySet(INT_TIMER0A,0X00);
    
    // Enable the timeout for Timer0A (Overflow interrupt)
    TimerIntEnable(TIMER0_BASE,TIMER_TIMA_TIMEOUT);

    // Clear the interrupt status of Timer0A for the timeout interrupt
    TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);

    // as the clock source is 16 mhz and by using prescaler of 0 it counts 1 ms in 16000 cycles
    TimerLoadSet(TIMER0_BASE,TIMER_A,16000);

    // Set the prescaler value to 0
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0);

    // Enable Timer0A
    TimerEnable(TIMER0_BASE,TIMER_A);

    // Enable the master interrupt globally
    IntMasterEnable();

    // Set the pin 2 in PORTF to high
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,GPIO_PIN_2);

    unsigned long currentMillis = millis();         // Get the current time in milliseconds
    unsigned long previousMillis = currentMillis;   // Save the current time to compare the next time
    unsigned long interval = 1000;                  // Interval to toggle the pin

    while(1){
        if (currentMillis - previousMillis >= interval){                                        // Check if the interval has passed
            previousMillis = currentMillis;                                                     // Save the current time to compare the next time    
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,~GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_2));  // Toggle the pin
        }
        currentMillis = millis();                                                               // Update the current time
        
    }

}
// ISR 
void Handler(){
    if ( TimerIntStatus(TIMER0_BASE,1) == TIMER_TIMA_TIMEOUT ){ // Check if the interrupt is due to the timeout interrupt
        TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);              // Clear the interrupt status
        millisCount++;                                              // Increment the millisCount
    }
}

// Function to return the current time in milliseconds
unsigned long millis(){
    return millisCount;
}
