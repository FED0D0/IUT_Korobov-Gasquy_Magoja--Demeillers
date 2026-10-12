#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>

#include "main.h"
#include "ghost.h"
#include "QEI.h"
#include "ChipConfig.h"
#include "IO.h"
#include "ToolBox.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "Timer.h"
#include "Robot.h"


/* ============================================================
 * PARAMETRES POUR L'ASSERVISSEMENT
 * ============================================================ */

float KpLineaire = 0.8f;
float KpAngulaire = 1.5f;
float KpLateral = 1.0f;

float erreurLineaire = 0.0f;
float erreurAngulaire = 0.0f;

/* ============================================================
 * PARAMETRES DU GENERATEUR DE TRAJECTOIRE
 * ============================================================ */

float VitesseTheta = 0.5f;
float AccTheta = 1.0f;

float VitesseThetaMax = 0.6f;
float VitesseLineaire = 0.0f;

float AccLineaire = 0.5f;
float VitesseLineaireMax = 0.15f;

GhostState gState;

/* ============================================================
 * CALCUL DU GHOST
 * ============================================================ */


void ComputeGhost(void)
{
    const float dt = 1.0f / FREQ_ECH_QEI;

    float dx;
    float dy;
    float distance;
    float vitesseCible;
    float variationMax;
    float erreurLongitudinale;
    float erreurLaterale;
    float erreurOrientation;

    /*
     * ============================================
     * 1. GENERATION DE LA TRAJECTOIRE GHOST
     * ============================================
     */

    switch (gState.state)
    {
        case Idle:
        {
            VitesseTheta = 0.0f;
            VitesseLineaire = 0.0f;

            gState.incrementTheta = 0.0f;
            gState.incrementDistance = 0.0f;

            break;
        }

        case Rotation:
        {
            VitesseLineaire = 0.0f;
            gState.incrementDistance = 0.0f;

            // Direction du waypoint
            dx = gState.XWaypoint - gState.XGhost;
            dy = gState.YWaypoint - gState.YGhost;

            gState.ThetaWaypoint = atan2f(dy, dx);

            // Erreur angulaire normalisee
            gState.ThetaRestant = ModuloByAngle(gState.ThetaGhost, gState.ThetaWaypoint) - gState.ThetaGhost;

            // Distance de freinage angulaire
            gState.ThetaArret = (VitesseTheta * VitesseTheta) / (2.0f * AccTheta);

            // Vitesse compatible avec l'arret
            vitesseCible = sqrtf(2.0f * AccTheta * Abs(gState.ThetaRestant));
            vitesseCible = Min(vitesseCible, VitesseThetaMax);

            if (gState.ThetaRestant < 0.0f)
                vitesseCible = -vitesseCible;

            // Limitation de l'acceleration
            variationMax = AccTheta * dt;

            VitesseTheta += LimitToInterval(vitesseCible - VitesseTheta, -variationMax, variationMax);

            // Deplacement theorique
            gState.incrementTheta = VitesseTheta * dt;

            if (Abs(gState.incrementTheta) > Abs(gState.ThetaRestant))
            {
                gState.incrementTheta = gState.ThetaRestant;
            }

            gState.ThetaGhost += gState.incrementTheta;

            // Normalisation de l'orientation
            gState.ThetaGhost = ModuloByAngle(0.0f, gState.ThetaGhost);

            gState.ThetaRestant = ModuloByAngle(gState.ThetaGhost, gState.ThetaWaypoint) - gState.ThetaGhost;

            // Fin de la rotation
            if (Abs(gState.ThetaRestant) < 0.01f && Abs(VitesseTheta) < 0.03f)
            {
                gState.ThetaGhost = gState.ThetaWaypoint;

                VitesseTheta = 0.0f;
                gState.incrementTheta = 0.0f;

                gState.state = DeplacementLineaire;
            }

            break;
        }

        case DeplacementLineaire:
        {
            VitesseTheta = 0.0f;
            gState.incrementTheta = 0.0f;

            dx = gState.XWaypoint - gState.XGhost;
            dy = gState.YWaypoint - gState.YGhost;

            distance = sqrtf(dx * dx + dy * dy);

            // Projection sur l'axe du Ghost
            gState.DistanceRestante = dx * cosf(gState.ThetaGhost) + dy * sinf(gState.ThetaGhost);

            gState.AngleWaypoint = ModuloByAngle(gState.ThetaGhost, atan2f(dy, dx)) - gState.ThetaGhost;

            // Distance d'arret
            gState.DistanceArret = (VitesseLineaire * VitesseLineaire) / (2.0f * AccLineaire);

            // Vitesse cible progressive
            vitesseCible = sqrtf(2.0f * AccLineaire * Abs(gState.DistanceRestante));
            vitesseCible = Min(vitesseCible, VitesseLineaireMax);

            if (gState.DistanceRestante < 0.0f)
                vitesseCible = -vitesseCible;

            variationMax = AccLineaire * dt;

            VitesseLineaire += LimitToInterval(vitesseCible - VitesseLineaire, -variationMax, variationMax);

            // Increment de distance
            gState.incrementDistance = VitesseLineaire * dt;

            // Evite de depasser la cible
            if (Abs(gState.incrementDistance) > Abs(gState.DistanceRestante))
            {
                gState.incrementDistance = gState.DistanceRestante;
            }

            // Mise a jour du Ghost
            gState.XGhost += gState.incrementDistance * cosf(gState.ThetaGhost);
            gState.YGhost += gState.incrementDistance * sinf(gState.ThetaGhost);

            dx = gState.XWaypoint - gState.XGhost;
            dy = gState.YWaypoint - gState.YGhost;

            gState.DistanceRestante = dx * cosf(gState.ThetaGhost) + dy * sinf(gState.ThetaGhost);

            // Fin du mouvement theorique
            if (Abs(gState.DistanceRestante) < 0.01f && Abs(VitesseLineaire) < 0.02f)
            {
                VitesseLineaire = 0.0f;
                gState.incrementDistance = 0.0f;

                gState.state = Idle;
            }

            break;
        }

        default:
        {
            gState.state = Idle;
            VitesseTheta = 0.0f;
            VitesseLineaire = 0.0f;

            break;
        }
    }

    /*
     * ============================================
     * 2. ERREUR ENTRE ROBOT REEL ET GHOST
     * ============================================
     */

    dx = gState.XGhost - robotState.xPosFromOdometry;
    dy = gState.YGhost - robotState.yPosFromOdometry;

    // Erreur longitudinale signee
    erreurLongitudinale = dx * cosf(robotState.angleRadianFromOdometry) + dy * sinf(robotState.angleRadianFromOdometry);

    // Erreur laterale signee
    // Positive : Ghost a gauche du robot
    erreurLaterale = -dx * sinf(robotState.angleRadianFromOdometry) + dy * cosf(robotState.angleRadianFromOdometry);

    // Erreur angulaire normalisee
    erreurOrientation = ModuloByAngle(robotState.angleRadianFromOdometry, gState.ThetaGhost) - robotState.angleRadianFromOdometry;

    erreurLineaire = erreurLongitudinale;
    erreurAngulaire = erreurOrientation;

    /*
     * ============================================
     * 3. ASSERVISSEMENT EN POSITION
     * ============================================
     */

    // Vitesse lineaire : suivi du Ghost
    robotState.vitesseLineaireConsigne = VitesseLineaire + KpLineaire * erreurLongitudinale;

    // Vitesse angulaire : orientation + ecart lateral
    robotState.vitesseAngulaireConsigne = VitesseTheta + KpAngulaire * erreurOrientation;

    // Correction laterale pendant la translation
    if (gState.state == DeplacementLineaire || (gState.state == Idle && Abs(erreurLongitudinale) > 0.01f))
    {
        robotState.vitesseAngulaireConsigne += KpLateral * erreurLaterale;
    }

    // Limites provisoires pour les essais
    robotState.vitesseLineaireConsigne = LimitToInterval(robotState.vitesseLineaireConsigne, -0.15f, 0.15f);
    robotState.vitesseAngulaireConsigne = LimitToInterval(robotState.vitesseAngulaireConsigne, -0.6f, 0.6f);
}


void SendGhostData(void) {
    unsigned char payload[36];

    // XGhost
    payload[0] = ((unsigned char*) &gState.XGhost)[0];
    payload[1] = ((unsigned char*) &gState.XGhost)[1];
    payload[2] = ((unsigned char*) &gState.XGhost)[2];
    payload[3] = ((unsigned char*) &gState.XGhost)[3];

    // YGhost
    payload[4] = ((unsigned char*) &gState.YGhost)[0];
    payload[5] = ((unsigned char*) &gState.YGhost)[1];
    payload[6] = ((unsigned char*) &gState.YGhost)[2];
    payload[7] = ((unsigned char*) &gState.YGhost)[3];

    // ThetaGhost
    payload[8] = ((unsigned char*) &gState.ThetaGhost)[0];
    payload[9] = ((unsigned char*) &gState.ThetaGhost)[1];
    payload[10] = ((unsigned char*) &gState.ThetaGhost)[2];
    payload[11] = ((unsigned char*) &gState.ThetaGhost)[3];

    // XWaypoint
    payload[12] = ((unsigned char*) &gState.XWaypoint)[0];
    payload[13] = ((unsigned char*) &gState.XWaypoint)[1];
    payload[14] = ((unsigned char*) &gState.XWaypoint)[2];
    payload[15] = ((unsigned char*) &gState.XWaypoint)[3];

    // YWaypoint
    payload[16] = ((unsigned char*) &gState.YWaypoint)[0];
    payload[17] = ((unsigned char*) &gState.YWaypoint)[1];
    payload[18] = ((unsigned char*) &gState.YWaypoint)[2];
    payload[19] = ((unsigned char*) &gState.YWaypoint)[3];

    // ThetaWaypoint
    payload[20] = ((unsigned char*) &gState.ThetaWaypoint)[0];
    payload[21] = ((unsigned char*) &gState.ThetaWaypoint)[1];
    payload[22] = ((unsigned char*) &gState.ThetaWaypoint)[2];
    payload[23] = ((unsigned char*) &gState.ThetaWaypoint)[3];

    // VitesseTheta
    payload[24] = ((unsigned char*) &VitesseTheta)[0];
    payload[25] = ((unsigned char*) &VitesseTheta)[1];
    payload[26] = ((unsigned char*) &VitesseTheta)[2];
    payload[27] = ((unsigned char*) &VitesseTheta)[3];

    // VitesseLineaire
    payload[28] = ((unsigned char*) &VitesseLineaire)[0];
    payload[29] = ((unsigned char*) &VitesseLineaire)[1];
    payload[30] = ((unsigned char*) &VitesseLineaire)[2];
    payload[31] = ((unsigned char*) &VitesseLineaire)[3];

    // DistanceRestante
    payload[32] = ((unsigned char*) &gState.DistanceRestante)[0];
    payload[33] = ((unsigned char*) &gState.DistanceRestante)[1];
    payload[34] = ((unsigned char*) &gState.DistanceRestante)[2];
    payload[35] = ((unsigned char*) &gState.DistanceRestante)[3];

    UartEncodeAndSendMessage(0x0091, 36, payload);
}

void GhostStartPoint(void) {
    // Initialisation de la position du Ghost
    // avec la position réelle du robot

    gState.XGhost = 0.0f;
    gState.YGhost = 0.0f;
    gState.ThetaGhost = 0.0f;

    gState.XWaypoint = 0.0f;
    gState.YWaypoint = 0.0f;
    gState.ThetaWaypoint = 0.0f;

    gState.ThetaRestant = 0.0f;
    gState.incrementTheta = 0.0f;
    gState.ThetaArret = 0.0f;

    gState.DistanceRestante = 0.0f;
    gState.incrementDistance = 0.0f;
    gState.DistanceArret = 0.0f;

    gState.AngleWaypoint = 0.0f;

    VitesseTheta = 0.0f;
    VitesseLineaire = 0.0f;

    gState.state = Idle;
}