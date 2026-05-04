#include <xc.h>
#include "IO.h"
#include "PWM.h"
#include "Robot.h"
#include "ToolBox.h"
#include "main.h"
#include "QEI.h"
#define PWMPER 24.0


void InitPWM(void) {
    PTCON2bits.PCLKDIV = 0b000; //Divide by 1
    PTPER = 100 * PWMPER; //éPriode en pourcentage
    //éRglage PWM moteur 1 sur hacheur 1
    IOCON1bits.PMOD = 0b11; //PWM I/O pin pair is in the True Independent Output mode
    IOCON1bits.PENL = 1;
    IOCON1bits.PENH = 1;
    FCLCON1 = 0x0003; //éDsactive la gestion des faults
    IOCON2bits.PMOD = 0b11; //PWM I/O pin pair is in the True Independent Output mode
    IOCON2bits.PENL = 1;
    IOCON2bits.PENH = 1;
    FCLCON2 = 0x0003; //éDsactive la gestion des faults
    /* Enable PWM Module */
    PTCONbits.PTEN = 1;
}
double talon = 50;

void PWMSetSpeed(float vitesseEnPourcents, float Moteur) {
    if (Moteur == MOTEUR_DROIT) {
        if (vitesseEnPourcents >= 0) {

            PDC1 = vitesseEnPourcents * PWMPER + talon;
            SDC1 = talon;

        } else {

            PDC1 = talon;
            SDC1 = -vitesseEnPourcents * PWMPER + talon;

        }
    }


    if (Moteur == MOTEUR_GAUCHE) {
        if (vitesseEnPourcents >= 0) {

            PDC2 = vitesseEnPourcents * PWMPER + talon;
            SDC2 = talon;

        } else {

            PDC2 = talon;
            SDC2 = -vitesseEnPourcents * PWMPER + talon;
        }
    }
}

//void PWMSetSpeedConsigne(float vitesseEnPourcents, float Moteur) {
//    if (Moteur == MOTEUR_GAUCHE)
//        robotState.vitesseGaucheConsigne = vitesseEnPourcents;
//    if (Moteur == MOTEUR_DROIT)
//        robotState.vitesseDroiteConsigne = vitesseEnPourcents;
//}

float acceleration = 99999;

void PWMUpdateSpeed() {
    // Cette fonction est appelee sur timer et permet de suivre des rampes d acceleration
    if (robotState.vitesseGaucheCommandeCourantePercent < robotState.vitesseGaucheConsignePercent)
        robotState.vitesseGaucheCommandeCourantePercent = Min(
            robotState.vitesseGaucheCommandeCourantePercent + acceleration,
            robotState.vitesseGaucheConsignePercent);
    if (robotState.vitesseGaucheCommandeCourantePercent > robotState.vitesseGaucheConsignePercent)
        robotState.vitesseGaucheCommandeCourantePercent = Max(
            robotState.vitesseGaucheCommandeCourantePercent - acceleration,
            robotState.vitesseGaucheConsignePercent);
    if (robotState.vitesseGaucheCommandeCourantePercent > 0) {
        PDC1 = robotState.vitesseGaucheCommandeCourantePercent * PWMPER + talon;
        SDC1 = talon;
    } else {
        PDC1 = talon;
        SDC1 = -robotState.vitesseGaucheCommandeCourantePercent * PWMPER + talon;
    }
    if (robotState.vitesseDroiteCommandeCourantePercent < robotState.vitesseDroiteConsignePercent)
        robotState.vitesseDroiteCommandeCourantePercent = Min(
            robotState.vitesseDroiteCommandeCourantePercent + acceleration,
            robotState.vitesseDroiteConsignePercent);
    if (robotState.vitesseDroiteCommandeCourantePercent > robotState.vitesseDroiteConsignePercent)
        robotState.vitesseDroiteCommandeCourantePercent = Max(
            robotState.vitesseDroiteCommandeCourantePercent - acceleration,
            robotState.vitesseDroiteConsignePercent);
    if (robotState.vitesseDroiteCommandeCourantePercent >= 0) {
        PDC2 = robotState.vitesseDroiteCommandeCourantePercent * PWMPER + talon;
        SDC2 = talon;
    } else {
        PDC2 = talon;
        SDC2 = -robotState.vitesseDroiteCommandeCourantePercent * PWMPER + talon;
    }
}

#define M_TO_PERCENT 35
void PWMSetSpeedConsignePolaire(float vitesseLineaire, float vitesseAngulaire) {
    robotState.vitesseDroiteConsigne = vitesseLineaire + (DISTROUES/2) * vitesseAngulaire;
    robotState.vitesseGaucheConsigne = vitesseLineaire - (DISTROUES/2) * vitesseAngulaire;
    
    robotState.vitesseDroiteConsignePercent = -M_TO_PERCENT * robotState.vitesseDroiteConsigne;
    robotState.vitesseGaucheConsignePercent = M_TO_PERCENT * robotState.vitesseGaucheConsigne;
    
    LimitToInterval(robotState.vitesseDroiteConsignePercent , -100, 100);
    LimitToInterval(robotState.vitesseGaucheConsignePercent , -100, 100);
}
