#ifndef oct_h
#define oct_h

#include "common.h"
#include "snorm.h"

void octDecode(const Snorm<snormSize> projected[2], float vec[3]) {

    vec[0] = float(projected[0]);
    vec[1] = float(projected[1]);
    vec[2] = 1.0f - (fabs(vec[0]) + fabs(vec[1]));

    if (vec[2] < 0.0f) {
        float oldX = vec[0];
        vec[0] = ((1.0f) - fabs(vec[1])) * signNotZero(oldX);
	    vec[1] = ((1.0f) - fabs(oldX))   * signNotZero(vec[1]);
    }
}

void octEncode(const float vec[3], Snorm<snormSize> projected[2]) {

    const float invL1Norm = (1.0f) / (fabs(vec[0]) + fabs(vec[1]) + fabs(vec[2]));

    if (vec[2] < 0.0f) {
        projected[0] = Snorm<snormSize>((1.0f - float(fabs(vec[1] * invL1Norm))) * signNotZero(vec[0]));
        projected[1] = Snorm<snormSize>((1.0f - float(fabs(vec[0] * invL1Norm))) * signNotZero(vec[1]));
    } else {
        projected[0] = Snorm<snormSize>(vec[0] * invL1Norm);
        projected[1] = Snorm<snormSize>(vec[1] * invL1Norm);
    }
}

// More computationally expensive, but higher quality results
void octPEncode(const float vec[3], Snorm<snormSize> projected[2]) {
    
    const float invL1Norm = (1.0f) / (fabs(vec[0]) + fabs(vec[1]) + fabs(vec[2]));

    if (vec[2] <= 0.0f) {
        projected[0] = Snorm<snormSize>::flooredSnorms( (1.0f - fabs(vec[1] * invL1Norm)) * signNotZero(vec[0]));
        projected[1] = Snorm<snormSize>::flooredSnorms( (1.0f - fabs(vec[0] * invL1Norm)) * signNotZero(vec[1]));
    } else {
        projected[0] = Snorm<snormSize>::flooredSnorms(vec[0] * invL1Norm);
        projected[1] = Snorm<snormSize>::flooredSnorms(vec[1] * invL1Norm);
    }
    
    Snorm<snormSize> bestProjected[2];
    unsigned int uBits = projected[0].bits();
    unsigned int vBits = projected[1].bits();

    float error = 0;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j< 2; ++j) {
            projected[0] = Snorm<snormSize>::fromBits(uBits + i);
            projected[1] = Snorm<snormSize>::fromBits(vBits + j);
            float decoded[3] = {0, 0, 0};
            octDecode(projected, decoded); 
            const float altError = fabs(vec[0]*decoded[0] + vec[1]*decoded[1] + vec[2]*decoded[2]);
            if (altError > error) {
                error = altError;
                bestProjected[0] = projected[0];
                bestProjected[1] = projected[1];
            }
        }
    }

    projected = bestProjected;
}

#endif
