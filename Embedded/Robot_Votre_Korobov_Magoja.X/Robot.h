#ifndef ROBOT_H
#define ROBOT_H
#include "asservissement.h"

typedef struct robotStateBITS {

    union {

        struct {
            unsigned char taskEnCours;
            float vitesseGaucheConsigne;
            float vitesseGaucheCommandeCourantePercent;
            float vitesseDroiteCommandeCourantePercent;
            float vitesseGaucheConsignePercent;
            float vitesseDroiteConsigne;
            float vitesseDroiteConsignePercent;
            float distanceTelemetreDroit;
            float distanceTelemetreCentre;
            float distanceTelemetreGauche;
            float distanceTelemetreUltraGauche;
            float distanceTelemetreUltraDroit;
            float vitesseDroitFromOdometry;
            float vitesseGaucheFromOdometry;
            float vitesseLineaireFromOdometry;
            float vitesseAngulaireFromOdometry;
            float xPosFromOdometry_1;
            float yPosFromOdometry_1;
            float angleRadianFromOdometry_1;
            float xPosFromOdometry;
            float yPosFromOdometry;
            float angleRadianFromOdometry;
            float CorrectionVitesseLineaire;
            float CorrectionVitesseAngulaire;
            float vitesseLineaireConsigne;
            float vitesseAngulaireConsigne;
            PidCorrector PidTheta;
            PidCorrector PidX;
            
        };
    };
}

ROBOT_STATE_BITS;
extern volatile ROBOT_STATE_BITS robotState;
#endif /* ROBOT_H */
