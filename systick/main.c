#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
void SysTick_delay_ms(uint32_t delay);
static volatile uint32_t count = 0;

int main(void)
{
 /* Halting the Watchdog */
 MAP_WDT_A_holdTimer();
 /* Configuring GPIO as an output */
 MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

 /* Enabling MASTER interrupts */
 MAP_Interrupt_enableMaster();
 while (1) {
 SysTick_delay_ms(100);
 MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);







 }
}
 void SysTick_delay_ms(uint32_t delay) {
 /* Configuring SysTick to trigger at 1500
 * MCLK is 1.5MHz
 * interrupts every 1ms) */
 MAP_SysTick_enableModule();
 MAP_SysTick_setPeriod(1500);
 MAP_SysTick_enableInterrupt();
 count = 0;
 while (count != delay);
 MAP_SysTick_disableModule();
 MAP_SysTick_disableInterrupt();
 return;
 }
void SysTick_Handler(void)
{
 count++;
}
