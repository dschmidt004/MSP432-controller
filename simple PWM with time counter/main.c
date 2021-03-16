#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
uint16_t x = 0;
int j = 0;
int i = 0;
#define NUM_SENSORS 2

#define COUNTS_PER_SECOND 2000 // ACLK set to 2KHz
/* Captured count values */
static volatile uint_fast16_t count[NUM_SENSORS];
/* delta counts - consecutive differences */

/* delta t - delta count divided by clock period */
static volatile float deltaT[NUM_SENSORS-1];
static volatile int done_flag = 0;

uint16_t y = 0;
float delta = 0;
float z = 2000;

const Timer_A_ContinuousModeConfig continuousModeConfig =
{
 TIMER_A_CLOCKSOURCE_ACLK, // ACLK Clock Source
 TIMER_A_CLOCKSOURCE_DIVIDER_1, // ACLK/1 = 2KHz
 TIMER_A_TAIE_INTERRUPT_DISABLE, // Disable Timer ISR
 TIMER_A_SKIP_CLEAR // Skip Clear Counter
};



/* UART Configuration Parameter 9600 baud rate 12MHz baud rate clock*/
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
/* Timer_A PWM Configuration Parameter
* SMCLK = 12MHz DOC
* SMCLK PWM Frequency = 24,000
* (12000000 ticks/s)/(500 ticks) = 24,000 counts/s */

Timer_A_PWMConfig pwmConfig =
{
 TIMER_A_CLOCKSOURCE_SMCLK,
 TIMER_A_CLOCKSOURCE_DIVIDER_8, // 24MHz/8 = 3MHz
 30000, // period 30,000 = 50Hz = 20ms
 TIMER_A_CAPTURECOMPARE_REGISTER_1, // compare OUT1 ***
 TIMER_A_OUTPUTMODE_RESET_SET,
 1500, // 30,000/1500 = 5% DC = 1ms
};
char new_D[] = "enter duty cycle\r\n";
// carriage return \r = 0x0D new line \n = 0x0A
char digits[3]; // 3-digit number
uint8_t char_count = 0; // number of char entered in digits
void transmit(char *str); //function for uart message
int convert(uint8_t char_count, char *data);//string to number
int main(void)
{
 /* Halting WDT */
 MAP_WDT_A_holdTimer();
 /* Setting DCO to 12MHz */
 CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
 /* Configure Timer_A 3MHz and
 * an initial duty cycle of 5% of that 1,500 ticks */



 MAP_CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_16);
 MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);
 MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN1|GPIO_PIN4);
 MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4);
 MAP_Interrupt_enableInterrupt(INT_PORT1);
 MAP_Timer_A_configureContinuousMode(TIMER_A0_BASE, &continuousModeConfig);

 MAP_Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig);



 /* Selecting P1.2
 * and P1.3 in UART mode */
 MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
 GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

 /* Selecting P5.6 PWM TA2.1 compare OUT1 output */
 MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P5,
 GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
 /* Configuring UART Module */
 MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
 /* Enable UART module */
 MAP_UART_enableModule(EUSCI_A0_BASE);
 /* Enabling interrupts */
 MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
 MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
 MAP_Interrupt_enableSleepOnIsrExit();
 MAP_Interrupt_enableMaster();
 /* uart print message */
 transmit(new_D); // "enter duty cycle\r\n"
 /* Low Power Mode */
 MAP_PCM_setPowerState(PCM_LPM0_LDO_VCORE0);
 MAP_Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_CONTINUOUS_MODE);
 while(1)
 {
 MAP_PCM_gotoLPM0();
 }
}
/* EUSCI A0 UART ISR - new duty cycle digit number */






void transmit(char *str){
 while(*str != 0) { // string not a null pointer
 MAP_UART_transmitData(EUSCI_A0_BASE, *str++);
 }
}
/* data array of ascii LSB last, ascii 0 is 0x30 */
int convert(uint8_t char_count, char *data) {
 char hundreds = '0', tens = '0', ones = '0';
 if (char_count == 1) ones = data[0];
 else if (char_count == 2) {
 ones = data[1];
 tens = data[0];
 }
 else {
 ones = data[2];
 tens = data[1];
 hundreds = data[0];
 }
 return (int) ((hundreds-0x30)*100 + (tens-0x30)*10 + (ones-0x30));
}
void PORT1_IRQHandler(void)
{
 uint32_t status;
 status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
 MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);
 if (status & GPIO_PIN1){
     x = Timer_A_getCounterValue ( TIMER_A0_BASE  );

     pwmConfig.dutyCycle = 300 * 13;
     MAP_Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig);

     while(j <500000)
         j++;


     j = 0;
}
 if (status & GPIO_PIN4){
      y = Timer_A_getCounterValue ( TIMER_A0_BASE  );
      y = 1;
      pwmConfig.dutyCycle = 300 * 3;
      MAP_Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfig);
      if(x != 0)
          {
              if (y != 0) delta = (y - x)/z;

          }
      while(j <500000)
          j++;


      j = 0;
 }


}


