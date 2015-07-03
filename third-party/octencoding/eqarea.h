#ifndef eqarea_h
#define eqarea_h

#include "common.h"
#include "snorm.h"

void eqareaDecode(Snorm<snormSize> projected[2], float vec[3]) {
    const float zSign = signNotZero((float)(projected[0]));

    const float u = (fabs(float(projected[0])) - .5f) * 2.0f;
    const float v = float(projected[1]);
    const float rSquared = u * u + v * v;
    const float temp = sqrt( (1.0f - square(1.0f - rSquared)) / rSquared);
    vec[0] = u * temp;
    vec[1] = v * temp;
    vec[2] = zSign * (1.0f - rSquared);
}

void eqareaEncode(const float vec[3], Snorm<snormSize> projected[2]) {

  const float verticalScale = 1.0f / (sqrt(fabs(vec[2]) + 1.0f));
    projected[0] = Snorm<snormSize>(((vec[0] * verticalScale * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(vec[1] * verticalScale);
}

void eqareaEncodeP(const float vec[3], Snorm<snormSize> projected[2]) {

    const float verticalScale = 1.0f / (sqrt(fabs(vec[2]) + 1.0f));
    projected[0] = Snorm<snormSize>(((vec[0] * verticalScale * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(vec[1] * verticalScale);

    Snorm<snormSize> bestProjected[2];
    unsigned int uBits = projected[0].bits();
    unsigned int vBits = projected[1].bits();

    float error = 0;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j< 2; ++j) {
            projected[0] = Snorm<snormSize>::fromBits(uBits + i);
            projected[1] = Snorm<snormSize>::fromBits(vBits + j);
            float decoded[3] = {0.0f, 0.0f, 0.0f};
            eqareaDecode(projected, decoded); 
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

void eqareaConcentricDecode(Snorm<snormSize> projected[2], float vec[3]) {
    
    const float zSign = signNotZero(projected[0]);

    float disk[2] = {static_cast<float>((fabs(float(projected[0])) - .5f) * 2.0f), float(projected[1])};
    concentricToDisk(disk);

    const float rSquared = disk[0] * disk[0] + disk[1] * disk[1];
    const float temp = sqrt( (1.0f - square(1.0f - rSquared)) / rSquared);
    vec[0] = disk[0] * temp;
    vec[1] = disk[1] * temp;
    vec[2] = zSign * (1.0f - rSquared);

}

void eqareaConcentricEncodeP(const float vec[3], Snorm<snormSize> projected[2]) {

    const float verticalScale = 1.0f / (sqrt(fabs(vec[2]) + 1.0f));
    float square[2] = {vec[0] * verticalScale, vec[1] * verticalScale};
    concentricFromDisk(square);

    projected[0] = Snorm<snormSize>(((square[0] * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(square[1]);

    Snorm<snormSize> bestProjected[2];
    unsigned int uBits = projected[0].bits();
    unsigned int vBits = projected[1].bits();

    float error = 0;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j< 2; ++j) {
            projected[0] = Snorm<snormSize>::fromBits(uBits + i);
            projected[1] = Snorm<snormSize>::fromBits(vBits + j);
            float decoded[3] = {0.0f, 0.0f, 0.0f};
            eqareaConcentricDecode(projected, decoded); 
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

void eqareaConcentricEncode(const float vec[3], Snorm<snormSize> projected[2]) {
    const float verticalScale = 1.0f / (sqrt(fabs(vec[2]) + 1.0f));
    float square[2] = {vec[0] * verticalScale, vec[1] * verticalScale};
    concentricFromDisk(square);

    projected[0] = Snorm<snormSize>(((square[0] * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(square[1]);
}


void eqareaEllipseDecode(Snorm<snormSize> projected[2], float vec[3]) {

    const float zSign = signNotZero((float)(projected[0]));

    float disk[2] = {static_cast<float>((fabs((float)(projected[0])) - .5f) * 2.0f), (float)(projected[1])};
    ellipseToDisk(disk);

    const float rSquared = disk[0] * disk[0] + disk[1] * disk[1];
    const float temp = sqrt( (1.0f - (1.0f - rSquared) * (1.0f - rSquared)) / rSquared);
    vec[0] = disk[0] * temp;
    vec[1] = disk[1] * temp;
    vec[2] = zSign * (1.0f - rSquared);
}

void eqareaEllipseEncodeP(const float vec[3], Snorm<snormSize> projected[2]) {

    const float verticalScale = 1.0f / (sqrt(fabs(vec[2]) + 1.0f));
    float square[2] = {vec[0] * verticalScale, vec[1] * verticalScale};
    ellipseFromDisk(square);

    projected[0] = Snorm<snormSize>(((square[0]* .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(square[1]);

    Snorm<snormSize> bestProjected[2];
    unsigned int uBits = projected[0].bits();
    unsigned int vBits = projected[1].bits();

    float error = 0;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j< 2; ++j) {
            projected[0] = Snorm<snormSize>::fromBits(uBits + i);
            projected[1] = Snorm<snormSize>::fromBits(vBits + j);
            float decoded[3] = {0.0f, 0.0f, 0.0f};
            eqareaEllipseDecode(projected, decoded); 
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

void eqareaEllipseEncode(const float vec[3], Snorm<snormSize> projected[2]) {

    const float verticalScale = 1.0f / (sqrt(fabs(vec[2]) + 1.0f));
    float square[2] = {vec[0] * verticalScale, vec[1] * verticalScale};
    ellipseFromDisk(square);

    projected[0] = Snorm<snormSize>(((square[0] * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(square[1]);
}

#endif
