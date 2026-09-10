#ifndef GHOST_H
#define	GHOST_H

//#include "asservissement.h"

typedef struct gState {
    float XGhost;
    float YGhost;
    float ThetaGhost;
    float ThetaWaypoint;
    
    float XWaypoint;
    float YWaypoint;
    
    float ThetaRestant;
    float VitesseThetaMax;
    float incrementTheta;
    
    float ThetaArret;
    //float GhostEtape;
} GhostState;

extern GhostState gState;
void ComputeGhost();
void SendGhostData(void);
void GhostStartPoint(void);
#endif 