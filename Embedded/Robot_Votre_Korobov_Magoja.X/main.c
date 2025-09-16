#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "ChipConfig.h"
#include "IO.h"
#include "Timer.h"
#include "PWM.h"
#include "ADC.h"
int main(void) {
    InitOscillator();
    InitIO();

    InitTimer23();
    InitTimer1();

    InitADC1();
    
    InitPWM();
    PWMSetSpeedConsigne(0,0);
    PWMSetSpeedConsigne(0,1);
    LED_BLANCHE_1 = 1;
    LED_BLEUE_1 = 1;
    LED_ORANGE_1 = 1;
    LED_ROUGE_1 = 1;
    LED_VERTE_1 = 1;
    LED_BLANCHE_2 = 1;
    LED_BLEUE_2 = 1;
    LED_ORANGE_2 = 1;
    LED_ROUGE_2 = 1;
    LED_VERTE_2 = 1;

    while (1) {

    }
}
