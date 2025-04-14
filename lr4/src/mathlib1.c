#include <math.h>

float SinIntegral(float A, float B, float e) {
    float integral = 0.0;
    for(float x = A; x < B; x += e) {
        integral += sin(x) * e;
    }
    return integral;
}

float Derivative(float A, float deltaX) {
    return (cos(A + deltaX) - cos(A)) / deltaX;
}