
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

// t_period ~> PWM period
int period = 3000;
float D = 0.8f;
int state = 0;
float x;

Timer_A_PWMConfig pwmConfig =
{
 TIMER_A_CLOCKSOURCE_SMCLK,
 TIMER_A_CLOCKSOURCE_DIVIDER_1,
 3000,
 TIMER_A_CAPTURECOMPARE_REGISTER_1,
 TIMER_A_OUTPUTMODE_RESET_SET,
 2400
};

int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();

    // set PWM output pin
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4,
                                                GPIO_PRIMARY_MODULE_FUNCTION);
    // **generate the mah'fahqn PWM
    Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);

    // init onboard button for interrupt to trigger
    // PWM duty cycle change
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);

    Interrupt_enableInterrupt(INT_PORT1);
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableMaster();

    PCM_setPowerState(PCM_LPM0_LDO_VCORE1);
}

void PORT1_IRQHandler(void) {

    uint32_t status = GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    if (status & GPIO_PIN1) {
        if(state == 0){
            x = 0;

        }
        else if(state == 1){
            x = .2;

        }
        else if(state == 2){
            x = .4;
        }
        else if (state == 3){
            x = .8;
            state = -1;
        }
        pwmConfig.dutyCycle = period*x;
        Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfig);
        state++;



    }
}







