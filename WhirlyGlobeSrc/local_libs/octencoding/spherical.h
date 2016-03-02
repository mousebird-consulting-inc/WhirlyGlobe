#ifndef spherical_h
#define spherical_h

#include "common.h"
#include "snorm.h"

void sphericalEncode(const float vec[3], Snorm<snormSize> projected[2]) {
    projected[0] = Snorm<snormSize>(float((acos(vec[1])/PI - .5f)) * 2.0f);
    projected[1] = Snorm<snormSize>(float(atan2(vec[0], vec[2]))/PI);

}

void sphericalDecode(const Snorm<snormSize> projected[2], float vec[3]) {
    const float theta = ((float(projected[0]) * .5f) + .5f) * PI;
    const float phi   = (float(projected[1])) * PI;
    vec[0] = sin(theta) * sin(phi);
    vec[1] = cos(theta);
    vec[2] = sin(theta) * cos(phi);
}
#endif