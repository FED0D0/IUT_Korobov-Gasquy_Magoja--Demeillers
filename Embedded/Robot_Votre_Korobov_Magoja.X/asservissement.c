#include "asservissement.h"
#include "Robot.h"
#include "QEI.h"
#include "PWM.h"
#include "ToolBox.h"

void SetupPidAsservissement(volatile PidCorrector* PidCorr, float Kp, float Ki, float Kd, float proportionelleMax, float integralMax, float deriveeMax) {
    PidCorr->Kp = Kp;
    PidCorr->erreurProportionelleMax = proportionelleMax; //On limite la correction due au Kp
    PidCorr->Ki = Ki;
    PidCorr->erreurIntegraleMax = integralMax; //On limite la correction due au Ki
    PidCorr->Kd = Kd;
    PidCorr->erreurDeriveeMax = deriveeMax;

//    // DEBUG 
//    PidCorr->erreur = 123;
//
//    PidCorr->corrP = 10;
//    PidCorr->corrI = 20;
//    PidCorr->corrD = 30;
//
//    PidCorr->erreurIntegrale = 40;
//
//    PidCorr->epsilon_1 = 50;
}

double Correcteur(volatile PidCorrector* PidCorr, float erreur) {
    PidCorr->erreur = erreur;

    // Correction proportionnelle
    PidCorr->corrP = LimitToInterval(
            PidCorr->Kp * erreur,
            -PidCorr->erreurProportionelleMax,
            PidCorr->erreurProportionelleMax);

    // Correction intégrale
    if (PidCorr->Ki != 0.0f) {
        PidCorr->erreurIntegrale += erreur / FREQ_ECH_QEI;

        PidCorr->corrI = LimitToInterval(
                PidCorr->Ki * PidCorr->erreurIntegrale,
                -PidCorr->erreurIntegraleMax,
                PidCorr->erreurIntegraleMax);

        PidCorr->erreurIntegrale = PidCorr->corrI / PidCorr->Ki;
    }
    else {
        PidCorr->erreurIntegrale = 0.0f;
        PidCorr->corrI = 0.0f;
    }

    // Correction dérivée
    PidCorr->corrD = LimitToInterval(
            PidCorr->Kd * (erreur - PidCorr->epsilon_1) * FREQ_ECH_QEI,
            -PidCorr->erreurDeriveeMax,
            PidCorr->erreurDeriveeMax);

    PidCorr->epsilon_1 = erreur;

    return PidCorr->corrP + PidCorr->corrI + PidCorr->corrD;
}

void UpdateAsservissement() {
    robotState.PidX.erreur = 
            robotState.vitesseLineaireConsigne -
            robotState.vitesseLineaireFromOdometry;

    robotState.PidTheta.erreur = 
            robotState.vitesseAngulaireConsigne -
            robotState.vitesseAngulaireFromOdometry;

    robotState.CorrectionVitesseLineaire =
            Correcteur(&robotState.PidX, robotState.PidX.erreur);

    robotState.CorrectionVitesseAngulaire =
            Correcteur(&robotState.PidTheta, robotState.PidTheta.erreur);

    PWMSetSpeedCommandePolaire(
            robotState.CorrectionVitesseLineaire,
            robotState.CorrectionVitesseAngulaire
            );
}