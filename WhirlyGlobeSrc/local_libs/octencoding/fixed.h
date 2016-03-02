#ifndef fixed_h
#define fixed_h

#include "common.h"
#include "snorm.h"

void fixedEncode(const float vec[3], Snorm<snormSize> projected[3]) {
    for (int i = 0; i < 3; ++i) {
        projected[i] = Snorm<snormSize>(vec[i]);
    }
};

void fixedDecode(const Snorm<snormSize> projected[3], float vec[3]) {
    for (int i = 0; i < 3; ++i) {
        vec[i] = (float)(projected[i]);
    }
}

#endif