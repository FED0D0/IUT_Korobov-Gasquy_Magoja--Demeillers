#ifndef GHOST_H
#define GHOST_H

typedef enum
{
    Idle,
    Rotation,
    DeplacementLineaire
} GhostMovementState;

typedef struct gState
{
    // Position du Ghost
    float XGhost;
    float YGhost;
    float ThetaGhost;

    // Position du waypoint
    float XWaypoint;
    float YWaypoint;
    float ThetaWaypoint;

    // Rotation
    float ThetaRestant;
    float incrementTheta;
    float ThetaArret;

    // Déplacement linéaire
    float DistanceRestante;
    float incrementDistance;
    float DistanceArret;

    // Angle entre le Ghost et le waypoint
    float AngleWaypoint;

    // Etat du générateur de trajectoire
    GhostMovementState state;

} GhostState;

extern GhostState gState;

// Génération de la trajectoire du Ghost
void ComputeGhost(void);

// Envoi des données du Ghost vers le PC
void SendGhostData(void);

// Initialisation du Ghost
void GhostStartPoint(void);

#endif