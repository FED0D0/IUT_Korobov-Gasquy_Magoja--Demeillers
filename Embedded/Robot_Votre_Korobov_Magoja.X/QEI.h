/* 
 * File:   QEI.h
 * Author: E306-PC5
 *
 * Created on 16 janvier 2026, 08:12
 */

#ifndef QEI_H
#define	QEI_H
#include <stdint.h>
void InitQEI1();
void InitQEI2();
void SendPositionData(void);
void QEI_SendPositionSpeed(uint32_t timestamp, float position, float speed);

#endif	/* QEI_H */

