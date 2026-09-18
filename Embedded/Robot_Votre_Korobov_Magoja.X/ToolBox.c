#include "Toolbox.h"
#include "main.h"
#include <math.h>

float Abs(float value) {
    if (value >= 0)
        return value;
    else return -value;
}

float Max(float value, float value2) {
    if (value > value2)
        return value;
    else
        return value2;
}

float Min(float value, float value2) {
    if (value < value2)
        return value;
    else
        return value2;
}

float LimitToInterval(float value, float lowLimit, float highLimit) {
    if (value > highLimit)
        value = highLimit;
    else if (value < lowLimit)
        value = lowLimit;
    return value;
}

float RadianToDegree(float value) {
    return value / PI * 180.0;
}

float DegreeToRadian(float value) {
    return value * PI / 180.0;
}

float DistancePointToSegment(float px, float py,
                             float ax, float ay,
                             float bx, float by)
{
    float abx = bx - ax;
    float aby = by - ay;

    float apx = px - ax;
    float apy = py - ay;

    float ab2 = (abx * abx) + (aby * aby);

    /* Cas particulier : A et B sont confondus */
    if (ab2 == 0.0f)
    {
        float dx = px - ax;
        float dy = py - ay;

        return sqrtf((dx * dx) + (dy * dy));
    }

    /*
     * Projection de P sur la droite AB.
     * t = 0 -> A
     * t = 1 -> B
     */
    float t = ((apx * abx) + (apy * aby)) / ab2;

    /* On limite la projection au segment [A,B] */
    if (t < 0.0f)
    {
        t = 0.0f;
    }
    else if (t > 1.0f)
    {
        t = 1.0f;
    }

    /* Coordonnées du point projeté sur le segment */
    float closestX = ax + (t * abx);
    float closestY = ay + (t * aby);

    /* Distance entre P et le point projeté */
    float dx = px - closestX;
    float dy = py - closestY;

    return sqrtf((dx * dx) + (dy * dy));
}
