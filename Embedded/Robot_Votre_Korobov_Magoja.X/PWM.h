/* 
 * File:   PWM.h
 * Author: E306-PC5
 *
 * Created on 10 septembre 2025, 09:46
 */
#define MOTEUR_DROIT 1
#define MOTEUR_GAUCHE 0
#ifndef PWM_H


//void PWMSetSpeed(float vitesseEnPourcents, float Moteur);
void PWMSetSpeedConsigne(float vitesseEnPourcents, float Moteur);
void PWMUpdateSpeed();
void InitPWM(void );

#endif 