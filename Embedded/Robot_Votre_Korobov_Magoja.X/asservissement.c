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
    float erreurProportionnelle = LimitToInterval(erreur, -PidCorr->erreurProportionelleMax / PidCorr->Kp, PidCorr->erreurProportionelleMax / PidCorr->Kp);
    PidCorr->corrP = erreurProportionnelle * PidCorr->Kp;
    PidCorr->erreurIntegrale += erreur / FREQ_ECH_QEI;
    PidCorr->erreurIntegrale = LimitToInterval(PidCorr->erreurIntegrale,
            -PidCorr->erreurIntegraleMax / PidCorr->Ki,
            PidCorr->erreurIntegraleMax / PidCorr->Ki);
    PidCorr->corrI = PidCorr->erreurIntegrale * PidCorr->Ki;
    float erreurDerivee = (erreur - PidCorr->epsilon_1) * FREQ_ECH_QEI;
    float deriveeBornee = LimitToInterval(erreurDerivee, -PidCorr->erreurDeriveeMax / PidCorr->Kd,
            PidCorr->erreurDeriveeMax / PidCorr->Kd);
    PidCorr->epsilon_1 = erreur;
    PidCorr->corrD = deriveeBornee * PidCorr->Kd;
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