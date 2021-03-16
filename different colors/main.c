#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
typedef enum {S0, S1, S2, S3} states;
typedef enum {reset, b1, b2} button_events;
static volatile button_events button_press;
static volatile uint8_t event_flag = 0;
int main(void)
{
 /* Halting the Watchdog */
 MAP_WDT_A_holdTimer();
 /* Configuring P2.0 P2.1 P2.2 as output */
 MAP_GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2);
 /* Configuring P1.1, P1.4, P1.5 as inputs and enabling interrupts */

 MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4|GPIO_PIN5);
 MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4|GPIO_PIN5);
 MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN4|GPIO_PIN5);
 MAP_Interrupt_enableInterrupt(INT_PORT1);
 /* Enabling SRAM Bank Retention */
 MAP_SysCtl_enableSRAMBankRetention(SYSCTL_SRAM_BANK1);

 /* Enabling MASTER interrupts */
 MAP_Interrupt_enableMaster();
 /*************************************
 * sequence detector "b2 b2 b1"
 *
 * -----------------------
 * | |b1 |b1
 * reset v b2 | b2 b1 |
 * ----> S0 ---> S1 ---> S2 ---> S3
 * ^ | |
 * | |b2 |b2
 * ---------------
 *
 *
 * ***************************************/
 /* next state and present state */
 states n_s;
 states p_s = S0;
 while (1)
 {
 MAP_PCM_gotoLPM3(); // Low Power Mode 3 deep sleep
 while (!event_flag){};
 if (button_press == reset) {
 n_s = S0;
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
 }
 else {
 if (p_s == S0) {
 if (button_press == b2) {
 n_s = S1;
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
 }
 else if (button_press == b1) {

 n_s = S0;
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
 }
 }
 else if (p_s == S1) {
 if (button_press == b1) {
 n_s = S0;
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
 }
 else if (button_press == b2) {
 n_s = S2;
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2);
 }
 }
 else if (p_s == S2) {
 if (button_press == b1) {
 n_s = S3;
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0);
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1);
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2);
 }
 else if (button_press == b2) {
 n_s = S2;
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN2);
 }
 }
 else if (p_s == S3) {
 if (button_press == b1) {
 n_s = S0;
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN0);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
 }
 else if (button_press == b2) {
 n_s = S1;
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
 MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN1);
 MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
 }
 }

 p_s = n_s; // state transition
 event_flag = 0;
 }
 }
}
void PORT1_IRQHandler(void)
{
 uint32_t status;
 status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
 MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);
 if (status & GPIO_PIN5) button_press = reset;
 else if (status & GPIO_PIN4) button_press = b2;
 else if (status & GPIO_PIN1) button_press = b1;
 event_flag = 1;
}
