#include "Utilities.h"
#include "math.h"
#include <xc.h>

double ModuloByAngle(double angleToCenterAround, double angle)
{
    double interAngle =
        Modulo2PIAngleRadian(angle - angleToCenterAround);

    if (interAngle > M_PI)
        interAngle -= 2.0 * M_PI;

    return interAngle + angleToCenterAround;
}

//double Modulo2PIAngleRadian(double angleRadian) {
//    double angleTemp = fmod(angleRadian - PI, 2 * PI) + PI;
//    return fmod(angleTemp + PI, 2 * PI) - PI;
//}

double Modulo2PIAngleRadian(double angle)
{
    while (angle < 0.0)
        angle += 2.0 * M_PI;

    while (angle >= 2.0 * M_PI)
        angle -= 2.0 * M_PI;

    return angle;
}

float getFloat(unsigned char *p, int index)
{
    float *result_ptr = (float*)(p + index);
    float result = *result_ptr;
    return result;
}

double getDouble(unsigned char *p, int index)
{
    double *result_ptr = (double*)(p + index);
    return *result_ptr;
}

void getBytesFromFloat(unsigned char *p, int index, float f)
{
    int i;
    unsigned char *f_ptr = (unsigned char*)&f;
    for (i = 0; i < 4; i++)
        p[index + i] = f_ptr[i];
}

float getFloatFromBytes(const unsigned char *p, int index)
{
    float f;
    unsigned char *f_ptr = (unsigned char*)&f;
    
    for(int i = 0; i < 4; i++)
        f_ptr[i] = p[index + i];
    return f;
}

void getBytesFromInt32(unsigned char *p, int index, long in)
{
    int i;
    unsigned char *f_ptr = (unsigned char*)&in;
    for (i = 0; i < 4; i++)
        p[index + i] = f_ptr[3-i];
}

void getBytesFromDouble(unsigned char *p, int index, double d)
{
    int i;
    unsigned char *f_ptr = (unsigned char*)&d;
    for (i = 0; i < 8; i++)
        p[index + i] = f_ptr[i];
}

