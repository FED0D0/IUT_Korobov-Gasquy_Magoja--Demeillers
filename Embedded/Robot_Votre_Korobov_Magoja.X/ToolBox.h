#ifndef TOOLBOX_H
#define PI 3.141592653589793

float Abs(float value);
float Max(float value, float value2);
float Min(float value, float value2);
float LimitToInterval(float value, float lowLimit, float highLimit);
float RadianToDegree(float value);
float DegreeToRadian(float value);
float DistancePointToSegment(float px, float py,
                             float ax, float ay,
                             float bx, float by);
#endif