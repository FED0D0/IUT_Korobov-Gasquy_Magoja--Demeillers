#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "main.h"
#include "ghost.h"
#include "QEI.h"
#include "ChipConfig.h"
#include "QEI.h"
#include "IO.h"
#include "ToolBox.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "Timer.h"
#include <math.h>

float VitesseTheta = 0.5f;
float AccTheta = 1.0f;

GhostState gState;

void ComputeGhost() {
    //gState.ThetaWaypoint = atan2f(gState.YWaypoint, gState.XWaypoint);
    if (gState.XWaypoint==0)
        if(gState.YWaypoint>0)
            gState.ThetaWaypoint=M_2_PI;
        else if(gState.YWaypoint<0)
            gState.ThetaWaypoint=-M_2_PI;
    if (gState.XWaypoint>0)        
        gState.ThetaWaypoint = atan2f(gState.YWaypoint, gState.XWaypoint);

    //ThetaWaypoint = atan2f(gState.YWaypoint, gState.XWaypoint);

    // Calcul de l'angle restant jusqu'au waypoint
    gState.ThetaRestant = ModuloByAngle(gState.ThetaGhost, gState.ThetaWaypoint) - gState.ThetaGhost;

    // Calcul de la distance angulaire nécessaire pour s'arrêter
    gState.ThetaArret = (VitesseTheta * VitesseTheta) / (2.0f * AccTheta);

    if (VitesseTheta < 0) {
        gState.ThetaArret = -gState.ThetaArret;
    }

    // Calcul de l'incrément angulaire
    gState.incrementTheta = VitesseTheta / FREQ_ECH_QEI;

    // Vérification de la possibilité d'accélérer ou nécessité de freiner
    if (((gState.ThetaArret >= 0.0f && gState.ThetaRestant >= 0.0f) ||
            (gState.ThetaArret <= 0.0f && gState.ThetaRestant <= 0.0f)) &&
            (Abs(gState.ThetaRestant) >= Abs(gState.ThetaArret))) {
        // On accélère en rampe saturée
        if (gState.ThetaRestant > 0.0f) {
            // Si la destination est devant,
            // on accélère en positif en saturant la vitesse à VitesseThetaMax
            VitesseTheta = Min(VitesseTheta + AccTheta / FREQ_ECH_QEI, gState.VitesseThetaMax);
        } else if (gState.ThetaRestant < 0.0f) {
            // Si la destination est derrière,
            // on accélère en négatif en saturant la vitesse à -VitesseThetaMax
            VitesseTheta = Max(VitesseTheta - AccTheta / FREQ_ECH_QEI, -gState.VitesseThetaMax);
        }
    } else {
        // On freine en rampe saturée
        if (VitesseTheta > 0.0f) {
            // Si la vitesse est positive,
            // on freine en positif en saturant la vitesse à 0
            VitesseTheta = Max(VitesseTheta - AccTheta / FREQ_ECH_QEI, 0.0f);
        } else if (VitesseTheta < 0.0f) {
            // Si la vitesse est négative,
            // on freine en négatif en saturant la vitesse à 0
            VitesseTheta = Min(VitesseTheta + AccTheta / FREQ_ECH_QEI, 0.0f);
        }
    }

    // Calcul de l'incrément angulaire après mise à jour de la vitesse
    gState.incrementTheta = VitesseTheta / FREQ_ECH_QEI;

    // Si on dépasse la destination, on s'arrête exactement dessus
    if (Abs(gState.ThetaRestant) < Abs(gState.incrementTheta)) {
        gState.incrementTheta = gState.ThetaRestant;
    }

    // On intègre le déplacement
    gState.ThetaGhost = gState.ThetaGhost + gState.incrementTheta;

    // On gère les erreurs numériques d'arrondis
    if ((VitesseTheta == 0.0f) && (Abs(gState.ThetaRestant) < 0.01f)) {
        gState.ThetaGhost = gState.ThetaWaypoint;
    }


}

void SendGhostData(void) {
    unsigned char payload7[13];

    // ThetaGhost
    payload7[0] = ((unsigned char*) &gState.ThetaGhost)[0];
    payload7[1] = ((unsigned char*) &gState.ThetaGhost)[1];
    payload7[2] = ((unsigned char*) &gState.ThetaGhost)[2];
    payload7[3] = ((unsigned char*) &gState.ThetaGhost)[3];

    // ThetaWaypoint
    payload7[4] = ((unsigned char*) &gState.ThetaWaypoint)[0];
    payload7[5] = ((unsigned char*) &gState.ThetaWaypoint)[1];
    payload7[6] = ((unsigned char*) &gState.ThetaWaypoint)[2];
    payload7[7] = ((unsigned char*) &gState.ThetaWaypoint)[3];

    // VitesseTheta
    payload7[8] = ((unsigned char*) &VitesseTheta)[0];
    payload7[9] = ((unsigned char*) &VitesseTheta)[1];
    payload7[10] = ((unsigned char*) &VitesseTheta)[2];
    payload7[11] = ((unsigned char*) &VitesseTheta)[3];



    // Envoi vers C#
    UartEncodeAndSendMessage(0x0091, 12, payload7);
}

void GhostStartPoint(void) {
    gState.ThetaGhost = 0.0f;
    gState.ThetaRestant = 0.0f;
    gState.ThetaArret = 0.0f;
    gState.incrementTheta = 0.0f;
}
