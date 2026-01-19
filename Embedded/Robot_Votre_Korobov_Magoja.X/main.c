#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <libpic30.h>
#include "ChipConfig.h"
#include "IO.h"
#include "Timer.h"
#include "PWM.h"
#include "ADC.h"
#include "Robot.h"
#include "main.h"
#include "UART.h"
#include "CB_TX1.h"
#include "CB_RX1.h"
#include "UART_Protocol.h"
#include "QEI.h"


int ADCValue0;
int ADCValue1;
int ADCValue2;
int ADCValue3;
int ADCValue4;
int counterDisplayAdc = 0;

int main(void) {
    InitOscillator();
    InitIO();
    InitUART();
    InitTimer23();
    InitTimer1();
    InitTimer4();

    InitADC1();
    InitPWM();

    // Initialisation simple de l'état du robot
    robotState.vitesseGaucheConsigne = 0;
    robotState.vitesseDroiteConsigne = 0;
    robotState.vitesseGaucheCommandeCourante = 0;
    robotState.vitesseDroiteCommandeCourante = 0;

    unsigned char octetRecu;
    //         PWMSetSpeedConsigne(0, 0);
    //         PWMSetSpeedConsigne(0, 1);

    //    LED_BLANCHE_1 = 1;
    //    LED_BLEUE_1 = 1;
    //    LED_ORANGE_1 = 1;
    //    LED_ROUGE_1 = 1;
    //    LED_VERTE_1 = 1;
    LED_BLANCHE_2 = 1;
    LED_BLEUE_2 = 1;
    LED_ORANGE_2 = 1;
    LED_ROUGE_2 = 1;
    LED_VERTE_2 = 1;
    //    unsigned char payload[] = {'B', 'o', 'n', 'j','o', 'u', 'r', '\n'};
//    InitQEI1();
//    InitQEI2();

    while (1) {
//        InitQEI1();
//        InitQEI2();
        // Vérifie si un octet a été reçu
        while (CB_RX1_IsDataAvailable()) {
    octetRecu = CB_RX1_Get();      
    UartDecodeMessage(octetRecu);  
        }
        PWMUpdateSpeed();   
        
        OperatingSystemLoop();
        //    SendMessage((unsigned char*) "impot", 5);
        ////SendMessageDirect((unsigned char*) "Bonjour", 7);


        //        UartEncodeAndSendMessage(0x0080, 8, payload);
        //
        //         __delay32(40000000);




////////        for (int i = 0; i < CB_RX1_GetDataSize(); i++) {
////////            unsigned char c = CB_RX1_Get();
////////            SendMessage(&c, 1);
////////        }
        //__delay32(1000);

        if (ADCIsConversionFinished() == 1) {
            ADCClearConversionFinishedFlag();
            unsigned int * result = ADCGetResult();
            ADCValue0 = result[0];
            ADCValue1 = result[1];
            ADCValue2 = result[2];
            ADCValue3 = result[3];
            ADCValue4 = result[4];
            float volts = ((float) result [1])* 3.3 / 4096;
            robotState.distanceTelemetreGauche = 34 / volts - 5;
            volts = ((float) result [2])* 3.3 / 4096;
            robotState.distanceTelemetreCentre = 34 / volts - 5;
            volts = ((float) result [3])* 3.3 / 4096;
            robotState.distanceTelemetreDroit = 34 / volts - 5;
            volts = ((float) result [0])* 3.3 / 4096;
            robotState.distanceTelemetreUltraGauche = 34 / volts - 5;
            volts = ((float) result [4])* 3.3 / 4096;
            robotState.distanceTelemetreUltraDroit = 34 / volts - 5;
            if (robotState.distanceTelemetreDroit <= 30) {
                LED_ROUGE_1 = 1;
            } else {
                LED_ROUGE_1 = 0;
            }

            if (robotState.distanceTelemetreCentre <= 30) {
                LED_ORANGE_1 = 1;
            } else {
                LED_ORANGE_1 = 0;

            }
            if (robotState.distanceTelemetreGauche <= 30) {
                LED_BLEUE_1 = 1;
            } else {
                LED_BLEUE_1 = 0;
            }
            if (robotState.distanceTelemetreUltraGauche <= 30) {
                LED_BLANCHE_1 = 1;
            } else {
                LED_BLANCHE_1 = 0;
            }
            if (robotState.distanceTelemetreUltraDroit <= 30) {
                LED_VERTE_1 = 1;
            } else {
                LED_VERTE_1 = 0;
            }

            if (counterDisplayAdc++ % 10 == 0) {
                unsigned char payload2[3];
                payload2[0] = robotState.distanceTelemetreGauche;
                payload2[1] = robotState.distanceTelemetreCentre;
                payload2[2] = robotState.distanceTelemetreDroit;
                UartEncodeAndSendMessage(0x0030, 3, payload2);
                unsigned char payload3[2];
                payload3[0] = (uint8_t)robotState.vitesseGaucheCommandeCourante;
                payload3[1] = (uint8_t)robotState.vitesseDroiteCommandeCourante;
                UartEncodeAndSendMessage(0x0040, 2, payload3);
            }
        }


    }
}

unsigned char stateRobot = 0;

void OperatingSystemLoop(void) {
        switch (stateRobot) {
            case STATE_ATTENTE:
                timestamp = 0;
                PWMSetSpeedConsigne(0, MOTEUR_DROIT);
                PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
                stateRobot = STATE_ATTENTE_EN_COURS;
                SendDeplacementStep(STATE_ATTENTE, timestamp);
            case STATE_ATTENTE_EN_COURS:
                if ( autoControlActivated == 1){
                if (timestamp > 1000)
                    stateRobot = STATE_AVANCE;
                }
                break;
            case STATE_AVANCE:
                PWMSetSpeedConsigne(-30, MOTEUR_DROIT);
                PWMSetSpeedConsigne(30, MOTEUR_GAUCHE);
                SendDeplacementStep(STATE_AVANCE, timestamp);
                stateRobot = STATE_AVANCE_EN_COURS;
                break;
            case STATE_AVANCE_EN_COURS:
                if ( autoControlActivated == 1)
                SetNextRobotStateInAutomaticMode();
                break;
            case STATE_TOURNE_GAUCHE:
                PWMSetSpeedConsigne(-13, MOTEUR_DROIT);
                PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
                SendDeplacementStep(STATE_TOURNE_GAUCHE, timestamp);
                stateRobot = STATE_TOURNE_GAUCHE_EN_COURS;
                break;
            case STATE_TOURNE_GAUCHE_EN_COURS:
                if ( autoControlActivated == 1)
                SetNextRobotStateInAutomaticMode();
                break;
            case STATE_TOURNE_DROITE:
                PWMSetSpeedConsigne(0, MOTEUR_DROIT);
                PWMSetSpeedConsigne(13, MOTEUR_GAUCHE);
                SendDeplacementStep(STATE_TOURNE_DROITE, timestamp);
                stateRobot = STATE_TOURNE_DROITE_EN_COURS;
                break;
            case STATE_TOURNE_DROITE_EN_COURS:
                if ( autoControlActivated == 1)
                SetNextRobotStateInAutomaticMode();
                break;
            case STATE_TOURNE_SUR_PLACE_GAUCHE:
                PWMSetSpeedConsigne(13, MOTEUR_DROIT);
                PWMSetSpeedConsigne(13, MOTEUR_GAUCHE);
                SendDeplacementStep(STATE_TOURNE_SUR_PLACE_GAUCHE, timestamp);
                stateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS;
                break;
            case STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS:
                if ( autoControlActivated == 1)
                SetNextRobotStateInAutomaticMode();
                break;
            case STATE_TOURNE_SUR_PLACE_DROITE:
                PWMSetSpeedConsigne(-13, MOTEUR_DROIT);
                PWMSetSpeedConsigne(-13, MOTEUR_GAUCHE);
                SendDeplacementStep(STATE_TOURNE_SUR_PLACE_DROITE, timestamp);
                stateRobot = STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS;
                break;
            case STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS:
                if ( autoControlActivated == 1)
                SetNextRobotStateInAutomaticMode();
                break;
            default:
                stateRobot = STATE_ATTENTE;
                break;
        }
}

unsigned char nextStateRobot = 0;

void SetNextRobotStateInAutomaticMode() {
    unsigned char positionObstacle = PAS_D_OBSTACLE;
    //?Dtermination de la position des obstacles en fonction des ???tlmtres
    if (robotState.distanceTelemetreDroit < 30 &&
            robotState.distanceTelemetreCentre > 25 &&
            robotState.distanceTelemetreGauche > 30 &&
            robotState.distanceTelemetreUltraGauche > 30) //Obstacle ?droite
        positionObstacle = OBSTACLE_A_DROITE;
    else if (robotState.distanceTelemetreUltraDroit < 35 &&
            robotState.distanceTelemetreCentre > 25 &&
            robotState.distanceTelemetreGauche > 30 &&
            robotState.distanceTelemetreUltraGauche > 30)
        positionObstacle = OBSTACLE_A_DROITE;
    else if (robotState.distanceTelemetreUltraDroit > 30 &&
            robotState.distanceTelemetreDroit > 30 &&
            robotState.distanceTelemetreCentre > 25 &&
            robotState.distanceTelemetreGauche < 30) //Obstacle ?gauche
        positionObstacle = OBSTACLE_A_GAUCHE;
    else if (robotState.distanceTelemetreUltraDroit > 30 &&
            robotState.distanceTelemetreDroit > 30 &&
            robotState.distanceTelemetreCentre > 25 &&
            robotState.distanceTelemetreUltraGauche < 35)
        positionObstacle = OBSTACLE_A_GAUCHE;
    else if (robotState.distanceTelemetreCentre < 40)
        positionObstacle = OBSTACLE_EN_FACE;
        //    else if (robotState.distanceTelemetreCentre < 35 &&
        //            robotState.distanceTelemetreGauche < 30)
        //        positionObstacle = OBSTACLE_EN_FACE;
        //    else if (robotState.distanceTelemetreCentre < 35 &&
        //            robotState.distanceTelemetreDroit < 30)
        //        positionObstacle = OBSTACLE_EN_FACE;
    else if (robotState.distanceTelemetreDroit < 30 &&
            robotState.distanceTelemetreGauche < 30)
        positionObstacle = OBSTACLE_EN_FACE;

    else if (robotState.distanceTelemetreUltraDroit > 35 &&
            robotState.distanceTelemetreDroit > 35 &&
            robotState.distanceTelemetreCentre > 30 &&
            robotState.distanceTelemetreGauche > 35 &&
            robotState.distanceTelemetreUltraGauche > 35) //pas d?obstacle
        positionObstacle = PAS_D_OBSTACLE;


    //?Dtermination de l??tat ?venir du robot
    if (positionObstacle == PAS_D_OBSTACLE)
        nextStateRobot = STATE_AVANCE;
    else if (positionObstacle == OBSTACLE_A_DROITE)
        nextStateRobot = STATE_TOURNE_GAUCHE;
    else if (positionObstacle == OBSTACLE_A_GAUCHE)
        nextStateRobot = STATE_TOURNE_DROITE;
    else if (positionObstacle == OBSTACLE_EN_FACE)
        nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE;

    //Si l?on n?est pas dans la transition de l??tape en cours
    if (nextStateRobot != stateRobot - 1)
        stateRobot = nextStateRobot;
}