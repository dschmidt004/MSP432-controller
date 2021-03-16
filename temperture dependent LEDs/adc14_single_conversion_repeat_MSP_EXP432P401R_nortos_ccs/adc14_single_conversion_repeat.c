#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Statics */


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
Timer_A_PWMConfig pwmConfig =
{
 TIMER_A_CLOCKSOURCE_SMCLK,
 TIMER_A_CLOCKSOURCE_DIVIDER_1,
 3000,
 TIMER_A_CAPTURECOMPARE_REGISTER_1,
 TIMER_A_OUTPUTMODE_RESET_SET,
 2400
};
int period = 3000;
float D = 0.8f;
int state = 0;
float x;
void transmit(char *str);
void itoa(int value, char* result,int base);
void printFloat(float f);

static volatile uint16_t curADCResult;
static volatile float normalizedADCRes;
int main(void)
{
    /* Halting the Watchdog  */
    MAP_WDT_A_holdTimer();

    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4,
                                                    GPIO_PRIMARY_MODULE_FUNCTION);
        // **generate the mah'fahqn PWM
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);






    /* Initializing Variables */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
         GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    

    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
         /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);

    MAP_UART_enableInterrupt(EUSCI_A0_BASE,EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);


    curADCResult = 0;
    MAP_FlashCtl_setWaitState(FLASH_BANK0,1);
    MAP_FlashCtl_setWaitState(FLASH_BANK1,1);


    MAP_PCM_setPowerState(PCM_AM_LDO_VCORE1);
    MAP_CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);

    MAP_CS_initClockSignal(CS_SMCLK,CS_DCOCLK_SELECT,CS_CLOCK_DIVIDER_4);
    uint32_t snclk = CS_getSMCLK();




    MAP_FPU_enableModule();
    MAP_FPU_enableLazyStacking();

    //![Single Sample Mode Configure]
    /* Initializing ADC (MCLK/1/4) */
    MAP_ADC14_enableModule();
    MAP_ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_4,
            0);
            
    /* Configuring GPIOs (5.5 A0) */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5,
    GPIO_TERTIARY_MODULE_FUNCTION);

    /* Configuring ADC Memory */
    MAP_ADC14_configureSingleSampleMode(ADC_MEM0, true);
    MAP_ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS,
    ADC_INPUT_A0, false);

    /* Configuring Sample Timer */
    MAP_ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);

    /* Enabling/Toggling Conversion */
    MAP_ADC14_enableConversion();
    MAP_ADC14_toggleConversionTrigger();
    //![Single Sample Mode Configure]

    /* Enabling interrupts */
    MAP_ADC14_enableInterrupt(ADC_INT0);
    MAP_Interrupt_enableInterrupt(INT_ADC14);
    MAP_Interrupt_enableMaster();

    while (1)
    {
        MAP_PCM_gotoLPM0();
    }
    
}

 //![Single Sample Result]
/* ADC Interrupt Handler. This handler is called whenever there is a conversion
 * that is finished for ADC_MEM0.
 */
void ADC14_IRQHandler(void)
{
 uint64_t status = MAP_ADC14_getEnabledInterruptStatus();
 int i;
 MAP_ADC14_clearInterruptFlag(status);
 if (ADC_INT0 & status)
  {
     curADCResult = MAP_ADC14_getResult(ADC_MEM0);
     normalizedADCRes = (curADCResult *3.3) / 16383;
     float temperatureC = (normalizedADCRes - 0.5)*100;

     transmit("Vin = ");
     printFloat(normalizedADCRes);


     MAP_ADC14_toggleConversionTrigger();

             if(normalizedADCRes < .9){
                 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
                 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
             }
             else if(normalizedADCRes < 2){
                 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
                 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
             }
             else if(normalizedADCRes < 3){
                 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
                 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
             }
             else {
                 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
                );

             }
             pwmConfig.dutyCycle = period*x;
             Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);



     for (i=0; i<1000000; i++); //slower down printing
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
 transmit("."); //print decimal point
 // subtract out integral part shift left 2 positions
 if (f < 0) // e.g. f = -123.456, temp = -123,
 temp = (int)( ( (float)temp - f) *100.0f );
 else
 temp = (int)((f - (float) temp)*100.0f);

 itoa(temp, buffer, 10);
 transmit(buffer); // print fraction
 transmit("\n\r");
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
