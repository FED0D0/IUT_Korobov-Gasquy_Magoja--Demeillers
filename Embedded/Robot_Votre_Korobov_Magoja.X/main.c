#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "ChipConfig.h"
#include "IO.h"
#include "Timer.h"
#include "PWM.h"
#include "ADC.h"
#include "Robot.h"

int ADCValue0;
int ADCValue1;
int ADCValue2;


int main(void) {
    InitOscillator();
    InitIO();

    InitTimer23();
    InitTimer1();

    InitADC1();
    //  InitPWM();
    // PWMSetSpeedConsigne(0, 0);
    // PWMSetSpeedConsigne(0, 1);

//    LED_BLANCHE_1 = 1;
//    LED_BLEUE_1 = 1;
//    LED_ORANGE_1 = 1;
    LED_ROUGE_1 = 1;
    LED_VERTE_1 = 1;
    LED_BLANCHE_2 = 1;
    LED_BLEUE_2 = 1;
    LED_ORANGE_2 = 1;
    LED_ROUGE_2 = 1;
    LED_VERTE_2 = 1;


    while (1) {

        if (ADCIsConversionFinished() == 1) {
            ADCClearConversionFinishedFlag();
            unsigned int * result = ADCGetResult();
            ADCValue0 = result[0];
            ADCValue1 = result[1];
            ADCValue2 = result[2];

            float volts = ((float) result [0])* 3.3 / 4096;
            robotState.distanceTelemetreGauche = 34 / volts - 5;
            volts = ((float) result [1])* 3.3 / 4096;
            robotState.distanceTelemetreCentre = 34 / volts - 5;
            volts = ((float) result [2])* 3.3 / 4096;
            robotState.distanceTelemetreDroit = 34 / volts - 5;
        }
//                        if (robotState.distanceTelemetreDroit <= 30) {
//                            LED_ORANGE_1 = 1;
//                        } else {
//                            LED_ORANGE_1 = 0;
//                        }
//                
//                        if (robotState.distanceTelemetreCentre <= 30) {
//                            LED_BLEUE_1 = 1;
//                        } else {
//                            LED_BLEUE_1 = 0;
//                
//                        }
//                        if (robotState.distanceTelemetreGauche <= 30) {
//                            LED_BLANCHE_1 = 1;
//                        } else {
//                            LED_BLANCHE_1 = 0;
//                        }

    }
}
