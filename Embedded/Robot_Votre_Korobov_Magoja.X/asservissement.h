#ifndef ASSERVISSEMENT_H
#define	ASSERVISSEMENT_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct _PidCorrector {
        float Kp;
        float Ki;
        float Kd;
        float erreurProportionelleMax;
        float erreurIntegraleMax;
        float erreurDeriveeMax;
        float erreurIntegrale;
        float epsilon_1;
        float erreur;
        //For Debug only
        float corrP;
        float corrI;
        float corrD;
    } PidCorrector;

void SetupPidAsservissement(volatile PidCorrector* PidCorr, float Kp, float Ki, float Kd, float proportionelleMax, float integralMax, float deriveeMax);
void UpdateAsservissement();

#ifdef	__cplusplus
}
#endif

#endif	/* ASSERVISSEMENT_H */

