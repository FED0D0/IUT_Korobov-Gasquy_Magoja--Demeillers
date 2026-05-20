#ifndef ASSERVISSEMENT_H
#define	ASSERVISSEMENT_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct _PidCorrector {
        double Kp;
        double Ki;
        double Kd;
        double erreurProportionelleMax;
        double erreurIntegraleMax;
        double erreurDeriveeMax;
        double erreurIntegrale;
        double epsilon_1;
        double erreur;
        //For Debug only
        double corrP;
        double corrI;
        double corrD;
    } PidCorrector;

void SetupPidAsservissement();
void UpdateAsservissement();

#ifdef	__cplusplus
}
#endif

#endif	/* ASSERVISSEMENT_H */

