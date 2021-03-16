#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
/* Standard Includes */
#include <stdint.h>
#include <math.h>
/* Timer_A Continuous Mode Configuration Parameter */
const Timer_A_ContinuousModeConfig continuousModeConfig =
{
 TIMER_A_CLOCKSOURCE_ACLK, // ACLK Clock Source
 TIMER_A_CLOCKSOURCE_DIVIDER_1, // ACLK/1 = 2KHz
 TIMER_A_TAIE_INTERRUPT_DISABLE, // Disable Timer ISR
 TIMER_A_SKIP_CLEAR // Skip Clear Counter
};

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
 EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Oversampling
};

Timer_A_PWMConfig pwmConfig =
{
 TIMER_A_CLOCKSOURCE_SMCLK,
 TIMER_A_CLOCKSOURCE_DIVIDER_8, // 24MHz/8 = 3MHz
 30000, // period 30,000 = 50Hz = 20ms
 TIMER_A_CAPTURECOMPARE_REGISTER_1, // compare OUT1 ***
 TIMER_A_OUTPUTMODE_RESET_SET,
 1500, // 30,000/1500 = 5% DC = 1ms
};
#define NUM_SENSORS 2

#define COUNTS_PER_SECOND 2000 // ACLK set to 2KHz
/* Captured count values */
static volatile uint_fast16_t count[NUM_SENSORS];
/* delta counts - consecutive differences */

/* delta t - delta count divided by clock period */
static volatile float deltaT[NUM_SENSORS-1];
static volatile int done_flag = 0;
uint16_t x = 0;
uint16_t y = 0;


float delta = 0;
float z = 2000;
int main(void)
{
 CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
 MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_16);


 /* Stop watchdog timer */
 MAP_WDT_A_holdTimer();


 /* Initializing the clock source as
 * ACLK = REFO/16 = 2kHz */
 MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_16);
 MAP_GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5);
 MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);
 MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);
 MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);



 MAP_Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

 MAP_Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig);


 MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5,
  GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);

 MAP_Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

 MAP_Interrupt_enableInterrupt(INT_PORT1);

 MAP_Interrupt_enableMaster();
 /* Starting the Timer_A0 in continuous mode */
 MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN3);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN1|GPIO_PIN2|GPIO_PIN4|GPIO_PIN5);


 while(1)
 {

 /* Goto LPM3 - Low Power Mode 3 deep sleep */
 MAP_PCM_gotoLPM3();
 }
}
/* GPIO ISR */
void PORT1_IRQHandler(void)
{

    uint32_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);

    GPIO_clearInterruptFlag(GPIO_PORT_P1,status);
    if(status & GPIO_PIN1){
        x = Timer_A_getCounterValue ( TIMER_A0_BASE  );
                y= Timer_A_getCounterValue ( TIMER_A0_BASE);
                MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN2);

                while(((y-x)/z)<1)y= Timer_A_getCounterValue ( TIMER_A0_BASE);
                pwmConfig.dutyCycle = 300 * 13;
                MAP_Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig);
                MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN1);

                while(((y-x)/z)<2)y= Timer_A_getCounterValue ( TIMER_A0_BASE);
                pwmConfig.dutyCycle = 300 * 3;
                MAP_Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig);
                MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN1);

                while(((y-x)/z)<3) y= Timer_A_getCounterValue ( TIMER_A0_BASE);
                MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN1);
                pwmConfig.dutyCycle = 300 * 13;
                MAP_Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig);

                while(((y-x)/z)<4) y= Timer_A_getCounterValue ( TIMER_A0_BASE);
                MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN1|GPIO_PIN5);
                MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN3);
    }
    if(status & GPIO_PIN4){
        x = Timer_A_getCounterValue ( TIMER_A0_BASE  );
        y= Timer_A_getCounterValue ( TIMER_A0_BASE);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN3);

        while(((y-x)/z)<1)y= Timer_A_getCounterValue ( TIMER_A0_BASE);

        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4);
        while(((y-x)/z)<2)y= Timer_A_getCounterValue ( TIMER_A0_BASE);

        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4);
        while(((y-x)/z)<3) y= Timer_A_getCounterValue ( TIMER_A0_BASE);
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN4);

        while(((y-x)/z)<4) y= Timer_A_getCounterValue ( TIMER_A0_BASE);
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN4);
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN2|GPIO_PIN5);

    }


}
