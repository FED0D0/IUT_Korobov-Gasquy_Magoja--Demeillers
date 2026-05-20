#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "ChipConfig.h"
#include "QEI.h"
#include "IO.h"
#include "ToolBox.h"
#include "Robot.h"
#include "math.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "Timer.h"

#define TEST_PID 0x0062

double QeiDroitPosition_T_1 = 0;
double QeiGauchePosition_T_1 = 0;
double QeiDroitPosition = 0;
double QeiGauchePosition = 0;

void InitQEI1() {
    QEI1IOCbits.SWPAB = 1; //QEAx and QEBx are swapped
    QEI1GECL = 0xFFFF;
    QEI1GECH = 0xFFFF;
    QEI1CONbits.QEIEN = 1; // Enable QEI Module
}

void InitQEI2() {
    QEI2IOCbits.SWPAB = 1; //QEAx and QEBx are not swapped
    QEI2GECL = 0xFFFF;
    QEI2GECH = 0xFFFF;
    QEI2CONbits.QEIEN = 1; // Enable QEI Module
}

void QEIUpdateData() {

    //On actualise les valeurs des positions
    long QEI1RawValue = POS1CNTL;
    QEI1RawValue += ((long) POS1HLD << 16);
    long QEI2RawValue = POS2CNTL;
    QEI2RawValue += ((long) POS2HLD << 16);
    //Conversion en mm (regle pour la taille des roues codeuses)
    double QeiDroitPosition = 0.00001620 * QEI1RawValue;
    double QeiGauchePosition = -0.00001620 * QEI2RawValue;
    //Calcul des deltas de position
    double delta_d = QeiDroitPosition - QeiDroitPosition_T_1;
    double delta_g = QeiGauchePosition - QeiGauchePosition_T_1;
        
    //On sauvegarde les anciennes valeurs
    QeiDroitPosition_T_1 = QeiDroitPosition;
    QeiGauchePosition_T_1 = QeiGauchePosition;
    
    //Calcul des vitesses
    //attention a remultiplier par la frequence d echantillonnage
    robotState.vitesseDroitFromOdometry = delta_d*FREQ_ECH_QEI;
    robotState.vitesseGaucheFromOdometry = delta_g*FREQ_ECH_QEI;
    robotState.vitesseLineaireFromOdometry = (robotState.vitesseDroitFromOdometry + robotState.vitesseGaucheFromOdometry) / 2;
    robotState.vitesseAngulaireFromOdometry = robotState.vitesseLineaireFromOdometry / DISTROUES;
    //Mise a jour du positionnement terrain a t-1
    robotState.xPosFromOdometry_1 = robotState.xPosFromOdometry;
    robotState.yPosFromOdometry_1 = robotState.yPosFromOdometry;
    robotState.angleRadianFromOdometry_1 = robotState.angleRadianFromOdometry;
    //Calcul des positions dans le referentiel du terrain
    robotState.xPosFromOdometry = robotState.xPosFromOdometry_1 + robotState.vitesseLineaireFromOdometry * cos(robotState.angleRadianFromOdometry_1) * FREQ_ECH_QEI;
    robotState.yPosFromOdometry = robotState.yPosFromOdometry_1 + robotState.vitesseLineaireFromOdometry * sin(robotState.angleRadianFromOdometry_1) * FREQ_ECH_QEI;
    robotState.angleRadianFromOdometry = robotState.vitesseAngulaireFromOdometry / FREQ_ECH_QEI;
    if (robotState.angleRadianFromOdometry > PI)
        robotState.angleRadianFromOdometry -= 2 * PI;
    if (robotState.angleRadianFromOdometry < -PI)
        robotState.angleRadianFromOdometry += 2 * PI;

}



#define QEI_FRAME_SIZE 12

//void QEI_SendPositionSpeed(uint32_t timestamp, float position, float speed) {
//    unsigned char txBuffer[QEI_FRAME_SIZE];
//
//    txBuffer[0] = (timestamp >> 0) & 0xFF;
//    txBuffer[1] = (timestamp >> 8) & 0xFF;
//    txBuffer[2] = (timestamp >> 16) & 0xFF;
//    txBuffer[3] = (timestamp >> 24) & 0xFF;
//
//    getBytesFromFloat(txBuffer, 4, position);
//
//    getBytesFromFloat(txBuffer, 8, speed);
//}
#define POSITION_DATA 0x0061

void SendPositionData() {
    unsigned char positionPayload[24];

    getBytesFromInt32(positionPayload, 0, timestamp);
    getBytesFromFloat(positionPayload, 4, (float) (robotState.xPosFromOdometry));
    getBytesFromFloat(positionPayload, 8, (float) (robotState.yPosFromOdometry));
    getBytesFromFloat(positionPayload, 12, (float) (robotState.angleRadianFromOdometry));
    getBytesFromFloat(positionPayload, 16, (float) (robotState.vitesseLineaireFromOdometry));
    getBytesFromFloat(positionPayload, 20, (float) (robotState.vitesseAngulaireFromOdometry));

    UartEncodeAndSendMessage(POSITION_DATA, 24, positionPayload);
}



//void PIDTest() {
//    unsigned char pidPayload[108];
//
//    float CON_X = robotState.vitesseDroiteConsigne;
//    float CON_T = robotState.vitesseGaucheConsigne;
//    float MES_X = robotState.vitesseLineaireFromOdometry;
//    float MES_T = robotState.vitesseAngulaireFromOdometry;
//    float ERR_X = robotState.PidX.erreur;
//    float ERR_T = robotState.PidTheta.erreur;
//    float COM_X = 7;
//    float COM_T = 8;
//    float KP_X = 9;
//    float KP_T = 10;
//    float COR_P_X = robotState.CorrectionVitesseLineaire;
//    float COR_P_T = robotState.CorrectionVitesseAngulaire;
//    float COR_P_MAX_X = 13;
//    float COR_P_MAX_T = 14;
//    float KI_X = 15;
//    float KI_T = 16;
//    float COR_I_X = 17;
//    float COR_I_T = 18;
//    float COR_I_MAX_X = 19;
//    float COR_I_MAX_T = 20;
//    float KD_X = 21;
//    float KD_T = 22;
//    float COR_D_X = 23;
//    float COR_D_T = 24;
//    float COR_D_MAX_X = 25;
//    float COR_D_MAX_T = 26;
//
//    getBytesFromInt32(pidPayload, 0, timestamp);
//    getBytesFromFloat(pidPayload, 4, CON_X);
//    getBytesFromFloat(pidPayload, 8, CON_T);
//    getBytesFromFloat(pidPayload, 12, MES_X);
//    getBytesFromFloat(pidPayload, 16, MES_T);
//    getBytesFromFloat(pidPayload, 20, ERR_X);
//    getBytesFromFloat(pidPayload, 24, ERR_T);
//    getBytesFromFloat(pidPayload, 28, COM_X);
//    getBytesFromFloat(pidPayload, 32, COM_T);
//    getBytesFromFloat(pidPayload, 36, KP_X);
//    getBytesFromFloat(pidPayload, 40, KP_T);
//    getBytesFromFloat(pidPayload, 44, COR_P_X);
//    getBytesFromFloat(pidPayload, 48, COR_P_T);
//    getBytesFromFloat(pidPayload, 52, COR_P_MAX_X);
//    getBytesFromFloat(pidPayload, 56, COR_P_MAX_T);
//    getBytesFromFloat(pidPayload, 60, KI_X);
//    getBytesFromFloat(pidPayload, 64, KI_T);
//    getBytesFromFloat(pidPayload, 68, COR_I_X);
//    getBytesFromFloat(pidPayload, 72, COR_I_T);
//    getBytesFromFloat(pidPayload, 76, COR_I_MAX_X);
//    getBytesFromFloat(pidPayload, 80, COR_I_MAX_T);
//    getBytesFromFloat(pidPayload, 84, KD_X);
//    getBytesFromFloat(pidPayload, 88, KD_T);
//    getBytesFromFloat(pidPayload, 92, COR_D_X);
//    getBytesFromFloat(pidPayload, 96, COR_D_T);
//    getBytesFromFloat(pidPayload, 100, COR_D_MAX_X);
//    getBytesFromFloat(pidPayload, 104, COR_D_MAX_T);
//
//    UartEncodeAndSendMessage(TEST_PID, 108, pidPayload);
//}

void SendPIDData() {

    unsigned char pidPayload[108];

    // Timestamp
    getBytesFromInt32(pidPayload, 0, timestamp);

    // CONSIGNES
    getBytesFromFloat(pidPayload, 4,
            robotState.vitesseLineaireConsigne);

    getBytesFromFloat(pidPayload, 8,
            robotState.vitesseAngulaireConsigne);

    // MESURES
    getBytesFromFloat(pidPayload, 12,
            robotState.vitesseLineaireFromOdometry);

    getBytesFromFloat(pidPayload, 16,
            robotState.vitesseAngulaireFromOdometry);

    // ERREURS
    getBytesFromFloat(pidPayload, 20,
            robotState.PidX.erreur);

    getBytesFromFloat(pidPayload, 24,
            robotState.PidTheta.erreur);

    // SORTIES PID
    getBytesFromFloat(pidPayload, 28,
            robotState.CorrectionVitesseLineaire);

    getBytesFromFloat(pidPayload, 32,
            robotState.CorrectionVitesseAngulaire);

    // KP
    getBytesFromFloat(pidPayload, 36,
            robotState.PidX.Kp);

    getBytesFromFloat(pidPayload, 40,
            robotState.PidTheta.Kp);

    // CORRECTION P
    getBytesFromFloat(pidPayload, 44,
            robotState.PidX.corrP);

    getBytesFromFloat(pidPayload, 48,
            robotState.PidTheta.corrP);

    // MAX ERREUR P
    getBytesFromFloat(pidPayload, 52,
            robotState.PidX.erreurProportionelleMax);

    getBytesFromFloat(pidPayload, 56,
            robotState.PidTheta.erreurProportionelleMax);

    // KI
    getBytesFromFloat(pidPayload, 60,
            robotState.PidX.Ki);

    getBytesFromFloat(pidPayload, 64,
            robotState.PidTheta.Ki);

    // CORRECTION I
    getBytesFromFloat(pidPayload, 68,
            robotState.PidX.corrI);

    getBytesFromFloat(pidPayload, 72,
            robotState.PidTheta.corrI);

    // MAX ERREUR I
    getBytesFromFloat(pidPayload, 76,
            robotState.PidX.erreurIntegraleMax);

    getBytesFromFloat(pidPayload, 80,
            robotState.PidTheta.erreurIntegraleMax);

    // KD
    getBytesFromFloat(pidPayload, 84,
            robotState.PidX.Kd);

    getBytesFromFloat(pidPayload, 88,
            robotState.PidTheta.Kd);

    // CORRECTION D
    getBytesFromFloat(pidPayload, 92,
            robotState.PidX.corrD);

    getBytesFromFloat(pidPayload, 96,
            robotState.PidTheta.corrD);

    // MAX ERREUR D
    getBytesFromFloat(pidPayload, 100,
            robotState.PidX.erreurDeriveeMax);

    getBytesFromFloat(pidPayload, 104,
            robotState.PidTheta.erreurDeriveeMax);

    UartEncodeAndSendMessage(TEST_PID, 108, pidPayload);
}