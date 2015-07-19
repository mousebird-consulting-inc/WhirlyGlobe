#include "snorm.h"

#include "eqarea.h"
#include "eqdist.h"
#include "fixed.h"
#include "LatLongD.h"
#include "oct.h"
#include "spherical.h"
#include "stereo.h"

#include <stdlib.h>

void normalize(float vec[3]) {
    float len = sqrt(square(vec[0]) + square(vec[1]) + square(vec[2]));
    vec[0] /= len;
    vec[1] /= len;
    vec[2] /= len;
}

/* Simple test code... just used to check that there were no glaring errors translating to portable, library-free C++ */
void printRoundTripVectorEncodeDecode(float vec[3]) {
    printf("%f %f %f\n", vec[0], vec[1], vec[2]);
  Snorm<snormSize> projected[2];
  sphericalEncode(vec, projected);
  sphericalDecode(projected, vec);
  normalize(vec);
  printf("SC: %f %f %f\n", vec[0], vec[1], vec[2]);
  octEncode(vec, projected);
  octDecode(projected, vec);
  normalize(vec);
  printf("Oct: %f %f %f\n", vec[0], vec[1], vec[2]);
  octPEncode(vec, projected);
  octDecode(projected, vec);
  normalize(vec);
  printf("OctP: %f %f %f\n", vec[0], vec[1], vec[2]);

  eqareaEncode(vec, projected);
  eqareaDecode(projected, vec);
  normalize(vec);
  printf("EqArea: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqareaEncodeP(vec, projected);
  eqareaDecode(projected, vec);
  normalize(vec);
  printf("EqAreaP: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqdistEncode(vec, projected);
  eqdistDecode(projected, vec);
  normalize(vec);
  printf("EqDist: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqdistEncodeP(vec, projected);
  eqdistDecode(projected, vec);
  normalize(vec);
  printf("EqDistP: %f %f %f\n", vec[0], vec[1], vec[2]);
  stereoEncode(vec, projected);
  stereoDecode(projected, vec);
  normalize(vec);
  printf("Stereo: %f %f %f\n", vec[0], vec[1], vec[2]);
  stereoEncodeP(vec, projected);
  stereoDecode(projected, vec);
  normalize(vec);
  printf("StereoP: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqareaConcentricEncode(vec, projected);
  eqareaConcentricDecode(projected, vec);
  normalize(vec);
  printf("EqAreaConcentric: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqareaConcentricEncodeP(vec, projected);
  eqareaConcentricDecode(projected, vec);
  normalize(vec);
  printf("EqAreaConcentricP: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqdistConcentricEncode(vec, projected);
  eqdistConcentricDecode(projected, vec);
  normalize(vec);
  printf("EqDistConcentric: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqdistConcentricEncodeP(vec, projected);
  eqdistConcentricDecode(projected, vec);
  normalize(vec);
  printf("EqDistConcentricP: %f %f %f\n", vec[0], vec[1], vec[2]);
  stereoConcentricEncode(vec, projected);
  stereoConcentricDecode(projected, vec);
  normalize(vec);
  printf("StereoConcentric: %f %f %f\n", vec[0], vec[1], vec[2]);
  stereoConcentricEncodeP(vec, projected);
  stereoConcentricDecode(projected, vec);
  normalize(vec);
  printf("StereoConcentricP: %f %f %f\n", vec[0], vec[1], vec[2]);


  eqareaEllipseEncode(vec, projected);
  eqareaEllipseDecode(projected, vec);
  normalize(vec);
  printf("EqAreaEllipse: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqareaEllipseEncodeP(vec, projected);
  eqareaEllipseDecode(projected, vec);
  normalize(vec);
  printf("EqAreaEllipseP: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqdistEllipseEncode(vec, projected);
  eqdistEllipseDecode(projected, vec);
  normalize(vec);
  printf("EqDistEllipse: %f %f %f\n", vec[0], vec[1], vec[2]);
  eqdistEllipseEncodeP(vec, projected);
  eqdistEllipseDecode(projected, vec);
  normalize(vec);
  printf("EqDistEllipseP: %f %f %f\n", vec[0], vec[1], vec[2]);
  stereoEllipseEncode(vec, projected);
  stereoEllipseDecode(projected, vec);
  normalize(vec);
  printf("StereoEllipse: %f %f %f\n", vec[0], vec[1], vec[2]);
  stereoEllipseEncodeP(vec, projected);
  stereoEllipseDecode(projected, vec);
  normalize(vec);
  printf("StereoEllipseP: %f %f %f\n", vec[0], vec[1], vec[2]);
  LatLongD encoder(32);
  unsigned int index;
  encoder.encode(vec, &index);
  encoder.decode(index, vec);
  normalize(vec);
  printf("LatLongD: %f %f %f\n", vec[0], vec[1], vec[2]);
}

/*Simple test code */
int main() {
  float vec[3] = {1.0f, -5.0f, 9.0f};
  normalize(vec);
  printRoundTripVectorEncodeDecode(vec);
}

