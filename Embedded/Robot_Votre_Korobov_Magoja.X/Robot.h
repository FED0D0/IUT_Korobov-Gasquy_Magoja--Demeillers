#ifndef ROBOT_H
#define ROBOT_H
#include "asservissement.h"

typedef struct robotStateBITS {

    union {

        struct {
            unsigned char taskEnCours;
            float vitesseGaucheConsigne;
            float vitesseGaucheCommandeCourantePercent;
            float vitesseGaucheConsignePercent;
            float vitesseDroiteConsigne;
            float vitesseDroiteCommandeCourantePercent;
            float vitesseDroiteConsignePercent;
            float distanceTelemetreDroit;
            float distanceTelemetreCentre;
            float distanceTelemetreGauche;
            float distanceTelemetreUltraGauche;
            float distanceTelemetreUltraDroit;
            double vitesseDroitFromOdometry;
            double vitesseGaucheFromOdometry;
            double vitesseLineaireFromOdometry;
            double vitesseAngulaireFromOdometry;
            double xPosFromOdometry_1;
            double yPosFromOdometry_1;
            double angleRadianFromOdometry_1;
            double xPosFromOdometry;
            double yPosFromOdometry;
            double angleRadianFromOdometry;
            double CorrectionVitesseLineaire;
            double CorrectionVitesseAngulaire;
            double vitesseLineaireConsigne;
            double vitesseAngulaireConsigne;
            PidCorrector PidTheta;
            PidCorrector PidX;
            
        };
    };
}

ROBOT_STATE_BITS;
extern volatile ROBOT_STATE_BITS robotState;
#endif /* ROBOT_H */
