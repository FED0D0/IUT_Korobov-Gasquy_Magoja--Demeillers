
#include "asservissement.h"
#include "Robot.h"
#include "QEI.h"
#include "PWM.h"
#include "ToolBox.h"

/*
 * Initialisation des correcteurs PID
 */
void SetupPidAsservissement(
        volatile PidCorrector* PidCorr,
        float Kp,
        float Ki,
        float Kd,
        float proportionelleMax,
        float integralMax,
        float deriveeMax) {
    PidCorr->Kp = Kp;
    PidCorr->Ki = Ki;
    PidCorr->Kd = Kd;

    PidCorr->erreurProportionelleMax = proportionelleMax;
    PidCorr->erreurIntegraleMax = integralMax;
    PidCorr->erreurDeriveeMax = deriveeMax;

    // Initialisation des variables internes
    PidCorr->erreur = 0.0f;
    PidCorr->erreurIntegrale = 0.0f;
    PidCorr->epsilon_1 = 0.0f;

    PidCorr->corrP = 0.0f;
    PidCorr->corrI = 0.0f;
    PidCorr->corrD = 0.0f;
}

/*
 * Calcul du correcteur PID
 *
 * P : correction proportionnelle
 * I : correction intégrale
 * D : correction dérivée
 *
 * Chaque correction est limitée indépendamment.
 */
double Correcteur(volatile PidCorrector* PidCorr, float erreur) {
    PidCorr->erreur = erreur;

    // CORRECTION PROPORTIONNELLE

    PidCorr->corrP = PidCorr->Kp * erreur;

    PidCorr->corrP = LimitToInterval(
            PidCorr->corrP,
            -PidCorr->erreurProportionelleMax,
            PidCorr->erreurProportionelleMax);

    // CORRECTION INTEGRALE

    if (PidCorr->Ki != 0.0f) {
        PidCorr->erreurIntegrale +=
                erreur / FREQ_ECH_QEI;

        float limiteIntegrale =
                PidCorr->erreurIntegraleMax /
                ((PidCorr->Ki > 0.0f) ?
                PidCorr->Ki : -PidCorr->Ki);

        PidCorr->erreurIntegrale = LimitToInterval(
                PidCorr->erreurIntegrale,
                -limiteIntegrale,
                limiteIntegrale);

        PidCorr->corrI =
                PidCorr->Ki * PidCorr->erreurIntegrale;
    } else {
        PidCorr->erreurIntegrale = 0.0f;
        PidCorr->corrI = 0.0f;
    }

    // CORRECTION DERIVEE

    if (PidCorr->Kd != 0.0f) {
        float erreurDerivee =
                (erreur - PidCorr->epsilon_1)
                * FREQ_ECH_QEI;

        PidCorr->corrD =
                PidCorr->Kd * erreurDerivee;

        PidCorr->corrD = LimitToInterval(
                PidCorr->corrD,
                -PidCorr->erreurDeriveeMax,
                PidCorr->erreurDeriveeMax);
    } else {
        PidCorr->corrD = 0.0f;
    }

    // Sauvegarde de l'erreur précédente
    PidCorr->epsilon_1 = erreur;

    // COMMANDE PID TOTALE

    return PidCorr->corrP
            + PidCorr->corrI
            + PidCorr->corrD;
}

/*
 * Mise à jour de l'asservissement polaire
 *
 * Cette fonction est appelée à 250 Hz
 * depuis l'interruption Timer1.
 */
void UpdateAsservissement(void) {
    // ERREURS DE VITESSE

    robotState.PidX.erreur =
            robotState.vitesseLineaireConsigne
            - robotState.vitesseLineaireFromOdometry;

    robotState.PidTheta.erreur =
            robotState.vitesseAngulaireConsigne
            - robotState.vitesseAngulaireFromOdometry;

    // CORRECTIONS PID

    robotState.CorrectionVitesseLineaire =
            Correcteur(
            &robotState.PidX,
            robotState.PidX.erreur);

    robotState.CorrectionVitesseAngulaire =
            Correcteur(
            &robotState.PidTheta,
            robotState.PidTheta.erreur);

    // APPLICATION DES COMMANDES AUX MOTEURS

    PWMSetSpeedCommandePolaire(
            robotState.CorrectionVitesseLineaire,
            robotState.CorrectionVitesseAngulaire);
}
