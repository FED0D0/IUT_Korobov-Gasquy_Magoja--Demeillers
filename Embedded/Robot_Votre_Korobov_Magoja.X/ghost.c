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

float VitesseTheta = 0.5f;
float AccTheta = 1.0f;
float VitesseThetaMax = 1.0f;

float VitesseLineaire = 0.0f;
float AccLineaire = 0.5f;
float VitesseLineaireMax = 0.5f;

GhostState gState;

void ComputeGhost(void)
{
    float dx;
    float dy;
    float distance;
    float angle;
    float angleErreur;

    switch (gState.state)
    {
        case Idle:
        {
            // Le Ghost est à l'arrêt
            VitesseTheta = 0.0f;
            VitesseLineaire = 0.0f;

            gState.incrementTheta = 0.0f;
            gState.incrementDistance = 0.0f;

            break;
        }

        case Rotation:
        {
            // Calcul de l'angle vers le waypoint
            dx = gState.XWaypoint - gState.XGhost;
            dy = gState.YWaypoint - gState.YGhost;

            gState.ThetaWaypoint = atan2f(dy, dx);

            // Calcul de l'angle restant jusqu'au waypoint
            gState.ThetaRestant =
                    ModuloByAngle(gState.ThetaGhost,
                                  gState.ThetaWaypoint)
                    - gState.ThetaGhost;

            // Calcul de la distance angulaire nécessaire pour s'arrêter
            gState.ThetaArret =
                    (VitesseTheta * VitesseTheta)
                    / (2.0f * AccTheta);

            if (VitesseTheta < 0.0f)
            {
                gState.ThetaArret = -gState.ThetaArret;
            }

            // Calcul de l'incrément angulaire
            gState.incrementTheta =
                    VitesseTheta / FREQ_ECH_QEI;

            // Vérification de la possibilité d'accélérer
            // ou nécessité de freiner
            if (((gState.ThetaArret >= 0.0f &&
                  gState.ThetaRestant >= 0.0f) ||
                 (gState.ThetaArret <= 0.0f &&
                  gState.ThetaRestant <= 0.0f)) &&
                (Abs(gState.ThetaRestant) >=
                 Abs(gState.ThetaArret)))
            {
                // On accélère en rampe saturée
                if (gState.ThetaRestant > 0.0f)
                {
                    // Accélération positive
                    VitesseTheta =
                            Min(VitesseTheta +
                                AccTheta / FREQ_ECH_QEI,
                                VitesseThetaMax);
                }
                else if (gState.ThetaRestant < 0.0f)
                {
                    // Accélération négative
                    VitesseTheta =
                            Max(VitesseTheta -
                                AccTheta / FREQ_ECH_QEI,
                                -VitesseThetaMax);
                }
            }
            else
            {
                // On freine en rampe saturée
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

            // Recalcul de l'incrément après mise à jour
            // de la vitesse
            gState.incrementTheta =
                    VitesseTheta / FREQ_ECH_QEI;

            // Ne pas dépasser le waypoint angulaire
            if (Abs(gState.ThetaRestant) <
                Abs(gState.incrementTheta))
            {
                gState.incrementTheta =
                        gState.ThetaRestant;
            }

            // Intégration du déplacement angulaire
            gState.ThetaGhost +=
                    gState.incrementTheta;

            // Normalisation de l'angle entre -PI et PI
            if (gState.ThetaGhost > M_PI)
            {
                gState.ThetaGhost -= 2.0f * M_PI;
            }
            else if (gState.ThetaGhost < -M_PI)
            {
                gState.ThetaGhost += 2.0f * M_PI;
            }

            // Gestion des erreurs numériques
            if ((VitesseTheta == 0.0f) &&
                (Abs(gState.ThetaRestant) < 0.01f))
            {
                gState.ThetaGhost =
                        gState.ThetaWaypoint;

                VitesseTheta = 0.0f;

                // La rotation est terminée
                // On passe au déplacement linéaire
                gState.state = DeplacementLineaire;
            }

            break;
        }

        case DeplacementLineaire:
        {
            // Coordonnées du waypoint par rapport au Ghost
            dx = gState.XWaypoint - gState.XGhost;
            dy = gState.YWaypoint - gState.YGhost;

            // Distance entre le Ghost et le waypoint
            distance = sqrtf(dx * dx + dy * dy);

            // Angle entre l'axe du Ghost et le waypoint
            angle = atan2f(dy, dx);

            // Erreur angulaire entre la direction du Ghost
            // et la direction du waypoint
            angleErreur =
                    ModuloByAngle(gState.ThetaGhost, angle)
                    - gState.ThetaGhost;

            gState.AngleWaypoint = angleErreur;

            // Calcul de la distance projetée sur l'axe
            // longitudinal du robot
            gState.DistanceRestante =
                    distance * cosf(angleErreur);

            // Détermination du sens de déplacement
            // Le waypoint est devant si l'angle est compris
            // entre -90 et +90 degrés
            if (angleErreur >= -M_PI_2 &&
                angleErreur <= M_PI_2)
            {
                // Waypoint devant
                gState.DistanceRestante =
                        Abs(gState.DistanceRestante);
            }
            else
            {
                // Waypoint derrière
                gState.DistanceRestante =
                        -Abs(gState.DistanceRestante);
            }

            // Calcul de la distance nécessaire pour freiner
            gState.DistanceArret =
                    (VitesseLineaire * VitesseLineaire)
                    / (2.0f * AccLineaire);

            if (VitesseLineaire < 0.0f)
            {
                gState.DistanceArret =
                        -gState.DistanceArret;
            }

            // Calcul de l'incrément de distance
            gState.incrementDistance =
                    VitesseLineaire / FREQ_ECH_QEI;

            // Vérification de la possibilité d'accélérer
            // ou nécessité de freiner
            if (((gState.DistanceArret >= 0.0f &&
                  gState.DistanceRestante >= 0.0f) ||
                 (gState.DistanceArret <= 0.0f &&
                  gState.DistanceRestante <= 0.0f)) &&
                (Abs(gState.DistanceRestante) >=
                 Abs(gState.DistanceArret)))
            {
                // On accélère
                if (gState.DistanceRestante > 0.0f)
                {
                    // Accélération vers l'avant
                    VitesseLineaire =
                            Min(VitesseLineaire +
                                AccLineaire / FREQ_ECH_QEI,
                                VitesseLineaireMax);
                }
                else if (gState.DistanceRestante < 0.0f)
                {
                    // Accélération vers l'arrière
                    VitesseLineaire =
                            Max(VitesseLineaire -
                                AccLineaire / FREQ_ECH_QEI,
                                -VitesseLineaireMax);
                }
            }
            else
            {
                // On freine
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

            // Recalcul de l'incrément après mise à jour
            // de la vitesse
            gState.incrementDistance =
                    VitesseLineaire / FREQ_ECH_QEI;

            // Ne pas dépasser la destination
            if (Abs(gState.DistanceRestante) <
                Abs(gState.incrementDistance))
            {
                gState.incrementDistance =
                        gState.DistanceRestante;
            }

            // Intégration de la position du Ghost
            gState.XGhost +=
                    gState.incrementDistance *
                    cosf(gState.ThetaGhost);

            gState.YGhost +=
                    gState.incrementDistance *
                    sinf(gState.ThetaGhost);

            // Si la destination est atteinte
            if (Abs(gState.DistanceRestante) < 0.01f &&
                Abs(VitesseLineaire) < 0.01f)
            {
                // On place exactement le Ghost
                // sur le point cible
                gState.XGhost = gState.XWaypoint;
                gState.YGhost = gState.YWaypoint;

                VitesseLineaire = 0.0f;

                gState.incrementDistance = 0.0f;

                // Retour à l'état Idle
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
}

void SendGhostData(void)
{
    unsigned char payload[36];

    // XGhost
    payload[0] = ((unsigned char*)&gState.XGhost)[0];
    payload[1] = ((unsigned char*)&gState.XGhost)[1];
    payload[2] = ((unsigned char*)&gState.XGhost)[2];
    payload[3] = ((unsigned char*)&gState.XGhost)[3];

    // YGhost
    payload[4] = ((unsigned char*)&gState.YGhost)[0];
    payload[5] = ((unsigned char*)&gState.YGhost)[1];
    payload[6] = ((unsigned char*)&gState.YGhost)[2];
    payload[7] = ((unsigned char*)&gState.YGhost)[3];

    // ThetaGhost
    payload[8] = ((unsigned char*)&gState.ThetaGhost)[0];
    payload[9] = ((unsigned char*)&gState.ThetaGhost)[1];
    payload[10] = ((unsigned char*)&gState.ThetaGhost)[2];
    payload[11] = ((unsigned char*)&gState.ThetaGhost)[3];

    // XWaypoint
    payload[12] = ((unsigned char*)&gState.XWaypoint)[0];
    payload[13] = ((unsigned char*)&gState.XWaypoint)[1];
    payload[14] = ((unsigned char*)&gState.XWaypoint)[2];
    payload[15] = ((unsigned char*)&gState.XWaypoint)[3];

    // YWaypoint
    payload[16] = ((unsigned char*)&gState.YWaypoint)[0];
    payload[17] = ((unsigned char*)&gState.YWaypoint)[1];
    payload[18] = ((unsigned char*)&gState.YWaypoint)[2];
    payload[19] = ((unsigned char*)&gState.YWaypoint)[3];

    // ThetaWaypoint
    payload[20] = ((unsigned char*)&gState.ThetaWaypoint)[0];
    payload[21] = ((unsigned char*)&gState.ThetaWaypoint)[1];
    payload[22] = ((unsigned char*)&gState.ThetaWaypoint)[2];
    payload[23] = ((unsigned char*)&gState.ThetaWaypoint)[3];

    // VitesseTheta
    payload[24] = ((unsigned char*)&VitesseTheta)[0];
    payload[25] = ((unsigned char*)&VitesseTheta)[1];
    payload[26] = ((unsigned char*)&VitesseTheta)[2];
    payload[27] = ((unsigned char*)&VitesseTheta)[3];

    // VitesseLineaire
    payload[28] = ((unsigned char*)&VitesseLineaire)[0];
    payload[29] = ((unsigned char*)&VitesseLineaire)[1];
    payload[30] = ((unsigned char*)&VitesseLineaire)[2];
    payload[31] = ((unsigned char*)&VitesseLineaire)[3];

    // DistanceRestante
    payload[32] = ((unsigned char*)&gState.DistanceRestante)[0];
    payload[33] = ((unsigned char*)&gState.DistanceRestante)[1];
    payload[34] = ((unsigned char*)&gState.DistanceRestante)[2];
    payload[35] = ((unsigned char*)&gState.DistanceRestante)[3];

    UartEncodeAndSendMessage(0x0091, 36, payload);
}

void GhostStartPoint(void)
{
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