
#include <xc.h>
#include "IO.h"
#include "PWM.h"
#include "Robot.h"
#include "ToolBox.h"
#include "main.h"
#include "QEI.h"

#define PWMPER 24.0f
#define M_TO_PERCENT 35.0f

// Valeur minimale appliquee aux registres PWM
static const float talon = 50.0f;

// Variation maximale de commande par appel a 250 Hz
// 1% par 4 ms : environ 0.4 s de 0 a 100%
static const float acceleration = 1.0f;

/*
 * INITIALISATION PWM
 */
void InitPWM(void) {
    PTCON2bits.PCLKDIV = 0b000;
    PTPER = (unsigned int) (100.0f * PWMPER);

    // Moteur 1
    IOCON1bits.PMOD = 0b11;
    IOCON1bits.PENL = 1;
    IOCON1bits.PENH = 1;
    FCLCON1 = 0x0003;

    // Moteur 2
    IOCON2bits.PMOD = 0b11;
    IOCON2bits.PENL = 1;
    IOCON2bits.PENH = 1;
    FCLCON2 = 0x0003;

    // Commandes au repos
    PDC1 = (unsigned int) talon;
    SDC1 = (unsigned int) talon;
    PDC2 = (unsigned int) talon;
    SDC2 = (unsigned int) talon;

    PTCONbits.PTEN = 1;
}

/*
 * COMMANDE DIRECTE D'UN MOTEUR
 *
 * Convention du projet :
 * Moteur gauche : PWM1
 * Moteur droit  : PWM2
 *
 * Attention :
 * Le signe reel des moteurs sera valide lors des tests.
 */
void PWMSetSpeed(float vitesseEnPourcents, float Moteur) {
    vitesseEnPourcents = LimitToInterval(
            vitesseEnPourcents, -100.0f, 100.0f);

    float duty = Abs(vitesseEnPourcents) * PWMPER + talon;

    if (Moteur == MOTEUR_GAUCHE) {
        if (vitesseEnPourcents >= 0.0f) {
            PDC1 = (unsigned int) duty;
            SDC1 = (unsigned int) talon;
        } else {
            PDC1 = (unsigned int) talon;
            SDC1 = (unsigned int) duty;
        }
    } else if (Moteur == MOTEUR_DROIT) {
        if (vitesseEnPourcents >= 0.0f) {
            PDC2 = (unsigned int) duty;
            SDC2 = (unsigned int) talon;
        } else {
            PDC2 = (unsigned int) talon;
            SDC2 = (unsigned int) duty;
        }
    }
}

/*
 * MISE A JOUR DES COMMANDES MOTEURS
 *
 * Cette fonction doit etre appelee a 250 Hz.
 *
 * Les commandes courantes suivent progressivement
 * les consignes en pourcentage.
 */
void PWMUpdateSpeed(void) {
    // Saturation des consignes
    robotState.vitesseGaucheConsignePercent = LimitToInterval(robotState.vitesseGaucheConsignePercent, -100.0f, 100.0f);

    robotState.vitesseDroiteConsignePercent = LimitToInterval(robotState.vitesseDroiteConsignePercent, -100.0f, 100.0f);

    // Rampe moteur gauche
    float erreurGauche = robotState.vitesseGaucheConsignePercent - robotState.vitesseGaucheCommandeCourantePercent;

    erreurGauche = LimitToInterval(erreurGauche, -acceleration, acceleration);

    robotState.vitesseGaucheCommandeCourantePercent += erreurGauche;

    // Rampe moteur droit
    float erreurDroite =
            robotState.vitesseDroiteConsignePercent
            - robotState.vitesseDroiteCommandeCourantePercent;

    erreurDroite = LimitToInterval(erreurDroite, -acceleration, acceleration);

    robotState.vitesseDroiteCommandeCourantePercent += erreurDroite;

    // Application moteur gauche (PWM1)
    float commandeGauche = robotState.vitesseGaucheCommandeCourantePercent;

    if (commandeGauche >= 0.0f) {
        PDC1 = (unsigned int) (commandeGauche * PWMPER + talon);
        SDC1 = (unsigned int) talon;
    } else {
        PDC1 = (unsigned int) talon;
        SDC1 = (unsigned int) (-commandeGauche * PWMPER + talon);
    }

    // Application moteur droit (PWM2)
    float commandeDroite =
            robotState.vitesseDroiteCommandeCourantePercent;

    if (commandeDroite >= 0.0f) {
        PDC2 = (unsigned int) (commandeDroite * PWMPER + talon);
        SDC2 = (unsigned int) talon;
    } else {
        PDC2 = (unsigned int) talon;
        SDC2 = (unsigned int) (-commandeDroite * PWMPER + talon);
    }
}

/*
 * CONVERSION DES COMMANDES POLAIRES
 * Vitesse lineaire : m/s
 * Vitesse angulaire : rad/s
 *
 * DISTROUES = 0.23 m
 */
void PWMSetSpeedCommandePolaire(float vitesseLineaire, float vitesseAngulaire) {
    float vitesseDroite = vitesseLineaire + (DISTROUES / 2.0f) * vitesseAngulaire;

    float vitesseGauche = vitesseLineaire - (DISTROUES / 2.0f) * vitesseAngulaire;

    // Signe inverse du moteur droit :
    // convention du cablage actuel
    robotState.vitesseDroiteConsignePercent = -M_TO_PERCENT * vitesseDroite;

    robotState.vitesseGaucheConsignePercent = M_TO_PERCENT * vitesseGauche;

    // Saturation obligatoire
    robotState.vitesseDroiteConsignePercent = LimitToInterval(robotState.vitesseDroiteConsignePercent, -100.0f, 100.0f);

    robotState.vitesseGaucheConsignePercent = LimitToInterval(robotState.vitesseGaucheConsignePercent, -100.0f, 100.0f);
}

/*
 * CONSIGNES DE VITESSE POUR L'ASSERVISSEMENT
 *
 * Les PID compareront ces vitesses
 * aux vitesses mesurees par odometrie.
 */
void PWMSetSpeedConsignePolaire(float vitesseLineaire, float vitesseAngulaire) {
    robotState.vitesseLineaireConsigne = vitesseLineaire;

    robotState.vitesseAngulaireConsigne = vitesseAngulaire;
}
