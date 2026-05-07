#include "asservissement.h"
#include "Robot.h"
#include "QEI.h"
#include "PWM.h"

void SetupPidAsservissement(volatile PidCorrector* PidCorr, double Kp, double Ki, double Kd, double proportionelleMax, double integralMax, double deriveeMax) {
    PidCorr->Kp = Kp;
    PidCorr->erreurProportionelleMax = proportionelleMax; //On limite la correction due au Kp
    PidCorr->Ki = Ki;
    PidCorr->erreurIntegraleMax = integralMax; //On limite la correction due au Ki
    PidCorr->Kd = Kd;
    PidCorr->erreurDeriveeMax = deriveeMax;
}

double Correcteur(volatile PidCorrector* PidCorr, double erreur) {
    PidCorr->erreur = erreur;
    double erreurProportionnelle = LimitToInterval(erreur, -PidCorr->erreurProportionelleMax / PidCorr->Kp, PidCorr->erreurProportionelleMax / PidCorr->Kp);
    PidCorr->corrP = erreurProportionnelle * PidCorr->Kp;
    PidCorr->erreurIntegrale += erreur / FREQ_ECH_QEI;
    PidCorr->erreurIntegrale = LimitToInterval(PidCorr->erreurIntegrale,
            -PidCorr->erreurIntegraleMax / PidCorr->Ki,
            PidCorr->erreurIntegraleMax / PidCorr->Ki);
    PidCorr->corrI = PidCorr->erreurIntegrale * PidCorr->Ki;
    double erreurDerivee = (erreur - PidCorr->epsilon_1) * FREQ_ECH_QEI;
    double deriveeBornee = LimitToInterval(erreurDerivee, -PidCorr->erreurDeriveeMax / PidCorr->Kd,
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

    PWMSetSpeedConsignePolaire(
            robotState.CorrectionVitesseLineaire,
            robotState.CorrectionVitesseAngulaire
            );
}