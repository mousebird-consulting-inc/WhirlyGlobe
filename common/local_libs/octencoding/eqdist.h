#ifndef eqdist_h
#define eqdist_h

#include "common.h"
#include "snorm.h"

void eqdistDecode(const Snorm<snormSize> projected[2], float vec[3]) {

    const float zSign = signNotZero((float)(projected[0]));

    const float u = (fabs((float)(projected[0])) - .5f) * 2.0f;
    const float v = (float)(projected[1]);
    const float r = sqrt(u*u + v*v);
    const float angle = PI * r * .5f;
    const float sinAngle = sin(angle);
    vec[0] = u/r * sinAngle;
    vec[1] = v/r * sinAngle;
    vec[2] = zSign * cos(angle);
}

void eqdistEncodeP(float vec[3], Snorm<snormSize> projected[2]) {

    float verticalScale = 0;

    if (fabs(vec[2]) >= 1.0f) {
        verticalScale = 1.0f;
    } else {
        verticalScale = (2.0f * acos(fabs(vec[2]))) / (PI * sqrt( 1.0f - vec[2] * vec[2]));
    }

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
            eqdistDecode(projected, decoded); 
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

void eqdistEncode(float vec[3], Snorm<snormSize> projected[2]) {

    float verticalScale = 0;

    if (fabs(vec[2]) >= 1.0f) {
        verticalScale = 1.0f;
    } else {
        verticalScale = (2.0f * acos(fabs(vec[2]))) / (PI * sqrt( 1.0f - vec[2] * vec[2]));
    }

    projected[0] = Snorm<snormSize>(((vec[0] * verticalScale * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(vec[1] * verticalScale);
}

void eqdistConcentricDecode(const Snorm<snormSize> projected[2], float vec[3]) {

    const float zSign = signNotZero((float)(projected[0]));

    float disk[2] = {static_cast<float>((fabs((float)(projected[0])) - .5f) * 2.0f), (float)(projected[1])};
    concentricToDisk(disk);

    const float r = sqrt(disk[0]*disk[0] + disk[1]*disk[1]);
    const float angle = PI * r * .5f;
    const float signAngle = sin(angle);
    vec[0] = disk[0]/r * signAngle;
    vec[1] = disk[1]/r * signAngle;
    vec[2] = zSign * cos(angle);
}

void eqdistConcentricEncodeP(const float vec[3], Snorm<snormSize> projected[2]) {

    float verticalScale = 0;

    if (fabs(vec[2]) >= 1.0f) {
        verticalScale = 1.0f;
    } else {
        verticalScale = (2.0f * acos(fabs(vec[2]))) / (PI * sqrt( 1.0f - vec[2] * vec[2]));
    }

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
            eqdistConcentricDecode(projected, decoded); 
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

void eqdistConcentricEncode(const float vec[3], Snorm<snormSize> projected[2]) {

    float verticalScale = 0;

    if (fabs(vec[2]) >= 1.0f) {
        verticalScale = 1.0f;
    } else {
        verticalScale = (2.0f * acos(fabs(vec[2]))) / (PI * sqrt( 1.0f - vec[2] * vec[2]));
    }

    float square[2] = {vec[0] * verticalScale, vec[1] * verticalScale};
    concentricFromDisk(square);

    projected[0] = Snorm<snormSize>(((square[0] * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(square[1]);
}

void eqdistEllipseDecode(const Snorm<snormSize> projected[2], float vec[3]) {

    const float zSign = signNotZero((float)(projected[0]));

    float disk[2] = {static_cast<float>((fabs((float)(projected[0])) - .5f) * 2.0f), (float)(projected[1])};
    ellipseToDisk(disk);

    const float r = sqrt(disk[0]*disk[0] + disk[1]*disk[1]);
    const float angle = PI * r * .5f;
    const float signAngle = sin(angle);
    vec[0] = disk[0]/r * signAngle;
    vec[1] = disk[1]/r * signAngle;
    vec[2] = zSign * cos(angle);
}

void eqdistEllipseEncodeP(const float vec[3], Snorm<snormSize> projected[2]) {
    float verticalScale = 0;

    if (fabs(vec[2]) >= 1.0f) {
        verticalScale = 1.0f;
    } else {
        verticalScale = (2.0f * acos(fabs(vec[2]))) / (PI * sqrt( 1.0f - vec[2] * vec[2]));
    }

    float square[2] = {vec[0] * verticalScale, vec[1] * verticalScale};
    ellipseFromDisk(square);

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
            eqdistEllipseDecode(projected, decoded); 
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

void eqdistEllipseEncode(const float vec[3], Snorm<snormSize> projected[2]) {
    float verticalScale = 0;

    if (fabs(vec[2]) >= 1.0f) {
        verticalScale = 1.0f;
    } else {
        verticalScale = (2.0f * acos(fabs(vec[2]))) / (PI * sqrt( 1.0f - vec[2] * vec[2]));
    }

    float square[2] = {vec[0] * verticalScale, vec[1] * verticalScale};
    ellipseFromDisk(square);

    projected[0] = Snorm<snormSize>(((square[0] * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(square[1]);
}

#endif
