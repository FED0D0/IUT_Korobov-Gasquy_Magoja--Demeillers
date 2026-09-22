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


/* ============================================================
 * PARAMETRES DU GENERATEUR DE TRAJECTOIRE
 * ============================================================ */

float VitesseTheta = 0.5f;
float AccTheta = 1.0f;
float VitesseThetaMax = 1.0f;

float VitesseLineaire = 0.0f;
float AccLineaire = 0.5f;
float VitesseLineaireMax = 0.5f;

GhostState gState;

/* ============================================================
 * CALCUL DU GHOST
 * ============================================================ */

void ComputeGhost(void)
{
    float dx;
    float dy;
    float distance;
    float angle;
    float angleErreur;

    switch (gState.state)
    {
        /* ============================================================
         * ETAT IDLE
         * ============================================================ */
        case Idle:
        {
            VitesseTheta = 0.0f;
            VitesseLineaire = 0.0f;

            gState.incrementTheta = 0.0f;
            gState.incrementDistance = 0.0f;

            break;
        }


        /* ============================================================
         * ETAT ROTATION
         * ============================================================ */
        case Rotation:
        {
            /* --------------------------------------------------------
             * Calcul de l'angle vers le waypoint
             * -------------------------------------------------------- */
            dx = gState.XWaypoint - gState.XGhost;
            dy = gState.YWaypoint - gState.YGhost;

            gState.ThetaWaypoint = atan2f(dy, dx);


            /* --------------------------------------------------------
             * Calcul de l'angle restant
             *
             * ModuloByAngle permet de choisir le chemin angulaire
             * le plus court.
             * -------------------------------------------------------- */
            gState.ThetaRestant =
                    ModuloByAngle(gState.ThetaGhost,
                                  gState.ThetaWaypoint)
                    - gState.ThetaGhost;


            /* --------------------------------------------------------
             * Distance nécessaire pour arrêter la rotation
             *
             * d = V² / (2 * Acc)
             * -------------------------------------------------------- */
            gState.ThetaArret =
                    (VitesseTheta * VitesseTheta)
                    / (2.0f * AccTheta);

            if (VitesseTheta < 0.0f)
            {
                gState.ThetaArret =
                        -gState.ThetaArret;
            }


            /* --------------------------------------------------------
             * Accélération ou freinage
             * -------------------------------------------------------- */
            if (((gState.ThetaArret >= 0.0f &&
                  gState.ThetaRestant >= 0.0f) ||
                 (gState.ThetaArret <= 0.0f &&
                  gState.ThetaRestant <= 0.0f)) &&
                (Abs(gState.ThetaRestant) >=
                 Abs(gState.ThetaArret)))
            {
                /* -------------------------
                 * Accélération
                 * ------------------------- */
                if (gState.ThetaRestant > 0.0f)
                {
                    VitesseTheta =
                            Min(VitesseTheta +
                                AccTheta / FREQ_ECH_QEI,
                                VitesseThetaMax);
                }
                else if (gState.ThetaRestant < 0.0f)
                {
                    VitesseTheta =
                            Max(VitesseTheta -
                                AccTheta / FREQ_ECH_QEI,
                                -VitesseThetaMax);
                }
            }
            else
            {
                /* -------------------------
                 * Freinage
                 * ------------------------- */
                if (VitesseTheta > 0.0f)
                {
                    VitesseTheta =
                            Max(VitesseTheta -
                                AccTheta / FREQ_ECH_QEI,
                                0.0f);
                }
                else if (VitesseTheta < 0.0f)
                {
                    VitesseTheta =
                            Min(VitesseTheta +
                                AccTheta / FREQ_ECH_QEI,
                                0.0f);
                }
            }


            /* --------------------------------------------------------
             * Incrément angulaire
             * -------------------------------------------------------- */
            gState.incrementTheta =
                    VitesseTheta / FREQ_ECH_QEI;


            /* --------------------------------------------------------
             * Ne pas dépasser la cible
             * -------------------------------------------------------- */
            if (Abs(gState.ThetaRestant) <
                Abs(gState.incrementTheta))
            {
                gState.incrementTheta =
                        gState.ThetaRestant;
            }


            /* --------------------------------------------------------
             * Intégration
             * -------------------------------------------------------- */
            gState.ThetaGhost +=
                    gState.incrementTheta;


            /* --------------------------------------------------------
             * Normalisation entre -PI et PI
             * -------------------------------------------------------- */
            if (gState.ThetaGhost > M_PI)
            {
                gState.ThetaGhost -= 2.0f * M_PI;
            }
            else if (gState.ThetaGhost < -M_PI)
            {
                gState.ThetaGhost += 2.0f * M_PI;
            }


            /* --------------------------------------------------------
             * Recalcul de l'erreur après déplacement
             *
             * Cela évite de tester une ancienne valeur de
             * ThetaRestant.
             * -------------------------------------------------------- */
            gState.ThetaRestant =
                    ModuloByAngle(gState.ThetaGhost,
                                  gState.ThetaWaypoint)
                    - gState.ThetaGhost;


            /* --------------------------------------------------------
             * Fin de rotation
             * -------------------------------------------------------- */
            if ((VitesseTheta == 0.0f) &&
                (Abs(gState.ThetaRestant) < 0.01f))
            {
                gState.ThetaGhost =
                        gState.ThetaWaypoint;

                VitesseTheta = 0.0f;

                gState.incrementTheta = 0.0f;

                /* On commence le déplacement */
                gState.state = DeplacementLineaire;
            }

            break;
        }


        /* ============================================================
         * ETAT DEPLACEMENT LINEAIRE
         * ============================================================ */
        case DeplacementLineaire:
        {
            /* --------------------------------------------------------
             * Vecteur Ghost -> Waypoint
             * -------------------------------------------------------- */
            dx = gState.XWaypoint - gState.XGhost;
            dy = gState.YWaypoint - gState.YGhost;


            /* --------------------------------------------------------
             * Distance géométrique
             * -------------------------------------------------------- */
            distance = sqrtf(dx * dx + dy * dy);


            /* --------------------------------------------------------
             * Angle entre l'axe du robot et le waypoint
             * -------------------------------------------------------- */
            angle = atan2f(dy, dx);

            angleErreur =
                    ModuloByAngle(gState.ThetaGhost, angle)
                    - gState.ThetaGhost;

            gState.AngleWaypoint = angleErreur;


            /* --------------------------------------------------------
             * Distance projetée sur l'axe du robot
             *
             * u = (cos(theta), sin(theta))
             *
             * d = (dx, dy)
             *
             * projection = d.u
             * -------------------------------------------------------- */
            gState.DistanceRestante =
                    dx * cosf(gState.ThetaGhost)
                    + dy * sinf(gState.ThetaGhost);


            /* --------------------------------------------------------
             * Distance d'arrêt
             * -------------------------------------------------------- */
            gState.DistanceArret =
                    (VitesseLineaire * VitesseLineaire)
                    / (2.0f * AccLineaire);

            if (VitesseLineaire < 0.0f)
            {
                gState.DistanceArret =
                        -gState.DistanceArret;
            }


            /* --------------------------------------------------------
             * Détermination accélération / freinage
             *
             * Même principe que pour la rotation.
             * -------------------------------------------------------- */
            if (((gState.DistanceArret >= 0.0f &&
                  gState.DistanceRestante >= 0.0f) ||
                 (gState.DistanceArret <= 0.0f &&
                  gState.DistanceRestante <= 0.0f)) &&
                (Abs(gState.DistanceRestante) >=
                 Abs(gState.DistanceArret)))
            {
                /* -------------------------
                 * Accélération
                 * ------------------------- */
                if (gState.DistanceRestante > 0.0f)
                {
                    VitesseLineaire =
                            Min(VitesseLineaire +
                                AccLineaire / FREQ_ECH_QEI,
                                VitesseLineaireMax);
                }
                else if (gState.DistanceRestante < 0.0f)
                {
                    VitesseLineaire =
                            Max(VitesseLineaire -
                                AccLineaire / FREQ_ECH_QEI,
                                -VitesseLineaireMax);
                }
            }
            else
            {
                /* -------------------------
                 * Freinage
                 * ------------------------- */
                if (VitesseLineaire > 0.0f)
                {
                    VitesseLineaire =
                            Max(VitesseLineaire -
                                AccLineaire / FREQ_ECH_QEI,
                                0.0f);
                }
                else if (VitesseLineaire < 0.0f)
                {
                    VitesseLineaire =
                            Min(VitesseLineaire +
                                AccLineaire / FREQ_ECH_QEI,
                                0.0f);
                }
            }


            /* --------------------------------------------------------
             * Incrément de distance
             * -------------------------------------------------------- */
            gState.incrementDistance =
                    VitesseLineaire / FREQ_ECH_QEI;


            /* --------------------------------------------------------
             * Ne pas dépasser la projection du waypoint
             * -------------------------------------------------------- */
            if (Abs(gState.DistanceRestante) <
                Abs(gState.incrementDistance))
            {
                gState.incrementDistance =
                        gState.DistanceRestante;
            }


            /* --------------------------------------------------------
             * Intégration longitudinale
             *
             * Le Ghost avance uniquement suivant son orientation.
             * -------------------------------------------------------- */
            gState.XGhost +=
                    gState.incrementDistance *
                    cosf(gState.ThetaGhost);

            gState.YGhost +=
                    gState.incrementDistance *
                    sinf(gState.ThetaGhost);


            /* --------------------------------------------------------
             * Recalcul de la distance après déplacement
             * -------------------------------------------------------- */
            dx = gState.XWaypoint - gState.XGhost;
            dy = gState.YWaypoint - gState.YGhost;

            gState.DistanceRestante =
                    dx * cosf(gState.ThetaGhost)
                    + dy * sinf(gState.ThetaGhost);


            /* --------------------------------------------------------
             * Fin du déplacement
             *
             * On considère le mouvement terminé lorsque :
             *
             * - la vitesse est nulle
             * - la distance projetée est suffisamment faible
             * -------------------------------------------------------- */
            if ((VitesseLineaire == 0.0f) &&
                (Abs(gState.DistanceRestante) < 0.01f))
            {
                VitesseLineaire = 0.0f;

                gState.incrementDistance = 0.0f;

                /*
                 * IMPORTANT :
                 *
                 * On ne met pas systématiquement XGhost/YGhost
                 * exactement sur XWaypoint/YWaypoint.
                 *
                 * Le sujet demande d'atteindre la projection
                 * du waypoint sur l'axe du robot.
                 *
                 * Donc on conserve la position obtenue.
                 */

                gState.state = Idle;
            }

            break;
        }


        /* ============================================================
         * ETAT INCONNU
         * ============================================================ */
        default:
        {
            gState.state = Idle;

            VitesseTheta = 0.0f;
            VitesseLineaire = 0.0f;

            gState.incrementTheta = 0.0f;
            gState.incrementDistance = 0.0f;

            break;
        }
    }
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