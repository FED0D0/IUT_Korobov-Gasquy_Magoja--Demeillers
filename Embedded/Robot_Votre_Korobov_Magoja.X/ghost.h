#ifndef GHOST_H
#define	GHOST_H

//#include "asservissement.h"

typedef struct gState {
    float XGhost;
    float YGhost;
    float ThetaGhost;
    
    float XWaypoint;
    float YWaypoint;
    
    float ThetaRestant;
    float VitesseTheta;
    float AccTheta;
    float VitesseThetaMax;
    float incrementTheta;
    
    //float ThetaWaypoint;
    float ThetaArret;
    //float GhostEtape;
} GhostState;

extern GhostState gState;
void ComputeGhost();
void SendGhostData(void);
void GhostStartPoint(void);
#endif 