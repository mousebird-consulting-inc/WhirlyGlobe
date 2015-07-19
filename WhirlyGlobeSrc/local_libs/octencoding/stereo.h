#ifndef stereo_h
#define stereo_h

#include "common.h"
#include "snorm.h"

void stereoDecode(const Snorm<snormSize> projected[2], float vec[3]) {

    const float zSign = signNotZero((float)(projected[0]));

    const float u = (fabs((float)(projected[0])) - .5f) * 2.0f;
    const float v = (float)(projected[1]);
    const float rSquared = u*u + v*v;
    const float step1 = (1 - rSquared)/(1 + rSquared);
    const float intermediary = sqrt( (1.0f - step1 * step1) / rSquared);
    vec[0] = u * intermediary;
    vec[1] = v * intermediary;
    vec[2] = zSign * step1;

}

void stereoEncodeP(const float vec[3], Snorm<snormSize> projected[2]) {
    

    float verticalScale = 1.0f / (1.0f + fabs(vec[2]));
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
            stereoDecode(projected, decoded); 
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

void stereoEncode(const float vec[3], Snorm<snormSize> projected[2]) {
    
    float verticalScale = 1.0f / (1.0f + fabs(vec[2]));
    projected[0] = Snorm<snormSize>(((vec[0] * verticalScale * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(vec[1] * verticalScale);

}

void stereoConcentricDecode(Snorm<snormSize> projected[2], float vec[3]) {

    const float zSign = signNotZero((float)(projected[0]));

    float disk[2] = {static_cast<float>((fabs(float(projected[0])) - .5f) * 2.0f), float(projected[1])};
    concentricToDisk(disk);

    const float rSquared = disk[0]*disk[0] + disk[1]*disk[1];
    const float step1 = (1 - rSquared)/(1 + rSquared);
    const float intermediary = sqrt( (1.0f - step1 * step1) / rSquared);
    vec[0] = disk[0] * intermediary;
    vec[1] = disk[1] * intermediary;
    vec[2] = zSign * step1;

}

void stereoConcentricEncodeP(const float vec[3], Snorm<snormSize> projected[2]) { 

    float verticalScale = 1.0f / (1.0f + fabs(vec[2]));

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
            stereoConcentricDecode(projected, decoded); 
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

void stereoConcentricEncode(const float vec[3], Snorm<snormSize> projected[2]) { 

    float verticalScale = 1.0f / (1.0f + fabs(vec[2]));

    float square[2] = {vec[0] * verticalScale, vec[1] * verticalScale};
    concentricFromDisk(square);

    projected[0] = Snorm<snormSize>(((square[0]* .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(square[1]);

}

void stereoEllipseDecode(Snorm<snormSize> projected[2], float vec[3]) {
    const float zSign = signNotZero((float)(projected[0]));

    float disk[2] = {static_cast<float>((fabs((float)(projected[0])) - .5f) * 2.0f), (float)(projected[1])};
    ellipseToDisk(disk);

    const float rSquared = disk[0]*disk[0] + disk[1]*disk[1];
    const float step1 = (1 - rSquared)/(1 + rSquared);
    const float intermediary = sqrt( (1.0f - step1 * step1) / rSquared);
    vec[0] = disk[0] * intermediary;
    vec[1] = disk[1] * intermediary;
    vec[2] = zSign * step1;
}

void stereoEllipseEncodeP(const float vec[3], Snorm<snormSize> projected[2]) { 

    float verticalScale = 1.0f / (1.0f + fabs(vec[2]));

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
            stereoEllipseDecode(projected, decoded); 
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

void stereoEllipseEncode(const float vec[3], Snorm<snormSize> projected[2]) { 

    float verticalScale = 1.0f / (1.0f + fabs(vec[2]));

    float square[2] = {vec[0] * verticalScale, vec[1] * verticalScale};
    ellipseFromDisk(square);

    projected[0] = Snorm<snormSize>(((square[0] * .5f) + .5f) * signNotZero(vec[2]));
    projected[1] = Snorm<snormSize>(square[1]);
}


#endif
