#ifndef GHOST_H
#define	GHOST_H

//#include "asservissement.h"

typedef struct gState {
    union {
        struct {
            float ThetaRestant;
            float ThetaGhost;
            float VitesseTheta;
            float AccTheta;
            float VitesseThetaMax;
            float incrementTheta;
            float ThetaWaypoint;
            float ThetaArret;
        };
    };
};

#endif 