/* 
 * File:   QEI.h
 * Author: E306-PC5
 *
 * Created on 16 janvier 2026, 08:12
 */

#ifndef QEI_H
#define	QEI_H

#define DISTROUES 0.2812
#define FREQ_ECH_QEI  250

//#include <stdint.h>
void InitQEI1();
void InitQEI2();
void SendPositionData();
void QEIUpdateData();
//void QEI_SendPositionSpeed(uint32_t timestamp, float position, float speed);
void PIDTest();



#endif	/* QEI_H */

