#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
typedef enum {reset, b1, b2} button_events;
static volatile button_events button_press;
static volatile button_events button_press1;
int x = 1;
int y = 1;
int period = 3000;
float D = 0.8f;
int state = 0;

void transmit(char *str);
void itoa(int value, char* result,int base);
void printFloat(float f);
#define TIMER_PERIOD 3000000
const eUSCI_UART_ConfigV1 uartConfig =
{
 EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
 78, // BRDIV = 78
 2, // UCxBRF = 2
 0, // UCxBRS = 0
 EUSCI_A_UART_NO_PARITY, // No Parity
 EUSCI_A_UART_LSB_FIRST, // LSB First
 EUSCI_A_UART_ONE_STOP_BIT, // One stop bit
 EUSCI_A_UART_MODE, // UART mode
 EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Over sampling
};
int main( void )
{
    MAP_WDT_A_holdTimer();
    MAP_PCM_setPowerState(PCM_AM_LDO_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);
    MAP_Interrupt_enableInterrupt(INT_PORT1);


    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
             /* Enable UART module */
        MAP_UART_enableModule(EUSCI_A0_BASE);

        MAP_UART_enableInterrupt(EUSCI_A0_BASE,EUSCI_A_UART_RECEIVE_INTERRUPT);
        MAP_Interrupt_enableInterrupt(INT_EUSCIA0);


    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,
            GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_CS_setExternalClockSourceFrequency(32000, CS_48MHZ);
    MAP_CS_startHFXT( false );
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_16, TIMER32_32BIT,
            TIMER32_PERIODIC_MODE);
    MAP_Timer32_setCount(TIMER32_0_BASE, TIMER_PERIOD);

    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableInterrupt(TIMER32_0_INTERRUPT);
    MAP_Timer32_enableInterrupt(TIMER32_0_BASE);
    MAP_Interrupt_enableMaster();

    MAP_Timer32_startTimer(TIMER32_0_BASE, false );

    PCM_setPowerState (PCM_LPM0_LDO_VCORE1);
}

void T32_INT1_IRQHandler( void )
{
    if(button_press == b2 & x == 0 ){
        MAP_Timer32_clearInterruptFlag(TIMER32_0_BASE);
        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);

    }
    if(button_press == b2 & x ==1){
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }
    if(button_press1 == b1 & y == 1){
        MAP_Timer32_setCount(TIMER32_0_BASE, (TIMER_PERIOD / 10));
        }
    if(button_press1 == b1 & y == 0){
        MAP_Timer32_setCount(TIMER32_0_BASE, (TIMER_PERIOD)) ;
            }

}
void PORT1_IRQHandler(void)
{
 uint32_t status;
 status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
 MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);
 if (status & GPIO_PIN4){
     button_press = b2;
     transmit("TempF = ");
     printFloat(1);
     if(x==1) x = 0;
     else x=1;
 }

 if (status & GPIO_PIN1) {
     button_press1 = b1;
     transmit("TempF = ");
     printFloat(1);
     if(y==1) y=0;
     else y=1;
 }

}
void transmit(char *str){
 while(*str != 0) { // string not a null pointer
 MAP_UART_transmitData(EUSCI_A0_BASE, *str++);
 }
}

void printFloat(float f)
{
 char buffer[10]; // placeholder
 int temp = (int) f; // integral part
 itoa(temp, buffer, 10);
 transmit(buffer); // print


}
/* integer to acsii */

void itoa(int value, char* result, int base)
 {
 // check that the base if valid
 if (base < 2 || base > 16) { *result = '\0';}
 char *ptr = result, *ptr1 = result, tmp_char;
 int tmp_value;
 int neg_flag = 0;
 // check negative number and convert magnitude
 if (value < 0) neg_flag = 1;
 value = abs(value);
 do {
 tmp_value = value ;
 value /= base;
 // remainder equals number - quotient*divider
 // indexes of the array of characters
 *ptr++ = "0123456789abcdef"[(tmp_value - (value * base))];
 } while ( value );
 // ptr holds string Least to most significant
 // Apply negative sign
 if (neg_flag) *ptr++ = '-';
 // end of string
 *ptr-- = '\0';
 // ptr points rightmost char, ptr1 points at leftmost char
 // reverse order string by swapping elements until halfway
 while(ptr1 < ptr) {
 tmp_char = *ptr;
 *ptr--= *ptr1;
 *ptr1++ = tmp_char;
 }
 }
