#ifndef common_h
#define common_h
/** Common constants and functions and headers */
#include <cmath>
#include <algorithm>

// Change this to modify the algorithms to use different snorm sizes
#define snormSize 16 

#define PI 3.1415926535898f

inline float signNotZero(float v) {
    return (v < 0.0f) ? -1.0f : 1.0f;
}

inline float square(float x) {
  return x*x;
}

// disk <-> square mappings
void concentricFromDisk(float vec[2]) {
    float r = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
    const float pi4 = PI / 4.0f;

    float phi;
    bool switched;
    if(::fabs(vec[0]) >= ::fabs(vec[1])) {
        phi = atan2(vec[0], vec[1]);
	switched = true;
    } else { 
        phi = atan2(vec[1], vec[0]);
	switched = false;
    }

    if (phi < -pi4) {
        phi += 2.0f * PI;
    }
    float a, b = 0.0f;
    if (phi < pi4) {
        a = r;
        b = phi * a / pi4;
    } else if (phi < 3.0f * pi4) {
        b = r;
        a = -(phi - 2.0f*pi4) * b / pi4;
    } else if (phi < 5.0f*pi4) {
        a = -r;
        b = (phi - PI) * a / pi4;
    } else {
        b = -r;
        a = -(phi - 6.0f*pi4) * b / pi4;
    }
    if (switched) {
      vec[0] = b;
      vec[1] = a;
    } else {
      vec[0] = a;
      vec[1] = b;
    }
}

void concentricToDisk(float vec[2]) {
    bool switched = false;
    float a,b;
    if (fabs(vec[0]) >= fabs(vec[1])) {
        switched = true;
	a = vec[1];
	b = vec[0];
    } else {
	a = vec[0];
	b = vec[1];
    }
    float phi, r = 0;
        
    const float pi4 = PI / 4.0f;
    if (a > -b) {
        if (a > b) {
            r = a;
            phi = pi4 * b / a;
        } else {
            r = b;
            phi = pi4 * (2 - (a / b));
        }
    } else {
        if (a < b) {
            r = -a;
            phi = pi4 * (4 + (b / a));
        } else {
            r = -b;
            if (b != 0) {
                phi = pi4 * (6 - (a / b));
            } else {
                phi = 0;
            }
        }
    }

    if (switched) {
        vec[0] = r * sin(phi);
        vec[1] = r * cos(phi);
    } else {
        vec[0] = r * cos(phi);
        vec[1] = r * sin(phi);
    }
}


void ellipseFromDisk (float vec[2]) {
    float step1 = (2.0f) + vec[0]*vec[0] - vec[1]*vec[1];
    float intermediary = sqrt((-8.0f)*vec[0]*vec[0] + step1*step1);
    float x = sqrt(std::max((2.0f + vec[0]*vec[0] - vec[1]*vec[1] - intermediary)/2.0f, 0.0f)) * signNotZero(vec[0]);
    float y = 2.0f*vec[1] / sqrt(2.0f - vec[0]*vec[0] + vec[1]*vec[1] + intermediary);
    vec[0] = x;
    vec[1] = y;
}

void ellipseToDisk (float vec[2]) {
    float x = vec[0] * sqrt(1.0f - vec[1]*vec[1]/2.0f);
    float y = vec[1] * sqrt(1.0f - vec[0]*vec[0]/2.0f);
    vec[0] = x;
    vec[1] = y;
}


#endif
