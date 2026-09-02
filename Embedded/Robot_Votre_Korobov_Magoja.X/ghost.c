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
#include "math.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "Timer.h"

// Calcul de l'angle restant jusqu'au waypoint
ghostState.ThetaRestant = ModuloByAngle(ghostState.ThetaGhost, ghostState.ThetaWaypoint) - ghostState.ThetaGhost;

// Calcul de la distance angulaire nécessaire pour s'arrêter
ghostState.ThetaArret = (ghostState.VitesseTheta * ghostState.VitesseTheta) / (2.0f * ghostState.AccTheta);

if (ghostState.VitesseTheta < 0) {
    ghostState.ThetaArret = -ghostState.ThetaArret;
}

// Calcul de l'incrément angulaire
ghostState.incrementTheta = ghostState.VitesseTheta * Tsampling;

// Vérification de la possibilité d'accélérer ou nécessité de freiner
if (((ghostState.ThetaArret >= 0.0f && ghostState.ThetaRestant >= 0.0f) ||
        (ghostState.ThetaArret <= 0.0f && ghostState.ThetaRestant <= 0.0f)) &&
        (fabsf(ghostState.ThetaRestant) >= fabsf(ghostState.ThetaArret))) {
    // On accélère en rampe saturée
    if (ghostState.ThetaRestant > 0.0f) {
        // Si la destination est devant,
        // on accélère en positif en saturant la vitesse à VitesseThetaMax
        ghostState.VitesseTheta = fminf(ghostState.VitesseTheta + ghostState.AccTheta / FREQ_ECH_QEI, ghostState.VitesseThetaMax);
    } else if (ghostState.ThetaRestant < 0.0f) {
        // Si la destination est derrière,
        // on accélère en négatif en saturant la vitesse à -VitesseThetaMax
        ghostState.VitesseTheta = fmaxf(ghostState.VitesseTheta - ghostState.AccTheta / FREQ_ECH_QEI, -ghostState.VitesseThetaMax);
    }
} else {
    // On freine en rampe saturée
    if (ghostState.VitesseTheta > 0.0f) {
        // Si la vitesse est positive,
        // on freine en positif en saturant la vitesse à 0
        ghostState.VitesseTheta = fmaxf(ghostState.VitesseTheta - ghostState.AccTheta / FREQ_ECH_QEI, 0.0f);
    } else if (ghostState.VitesseTheta < 0.0f) {
        // Si la vitesse est négative,
        // on freine en négatif en saturant la vitesse à 0
        ghostState.VitesseTheta = fminf(ghostState.VitesseTheta + ghostState.AccTheta / FREQ_ECH_QEI, 0.0f);
    }
}

// Calcul de l'incrément angulaire après mise à jour de la vitesse
ghostState.incrementTheta = ghostState.VitesseTheta * Tsampling;

// Si on dépasse la destination, on s'arrête exactement dessus
if (fabsf(ghostState.ThetaRestant) < fabsf(ghostState.incrementTheta)) {
    ghostState.incrementTheta = ghostState.ThetaRestant;
}

// On intègre le déplacement
ghostState.ThetaGhost = ghostState.ThetaGhost + ghostState.incrementTheta;

// On gère les erreurs numériques d'arrondis
if ((ghostState.VitesseTheta == 0.0f) && (fabsf(ghostState.ThetaRestant) < 0.01f)) {
    ghostState.ThetaGhost = ghostState.ThetaWaypoint;
}
