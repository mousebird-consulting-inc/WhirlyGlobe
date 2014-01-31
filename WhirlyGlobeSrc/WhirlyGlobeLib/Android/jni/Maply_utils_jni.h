#import "com_mousebirdconsulting_maply_Point2d.h"
#import "com_mousebirdconsulting_maply_Point3d.h"
#import "com_mousebirdconsulting_maply_Matrix4d.h"
#import <WhirlyGlobe.h>

// Construct a Java-side Point3d
JNIEXPORT jobject JNICALL MakePoint3d(JNIEnv *env,const WhirlyKit::Point3d &pt);

// Construct a Java-side Matrix4d
JNIEXPORT jobject JNICALL MakeMatrix4d(JNIEnv *env,const Eigen::Matrix4d &mat);

// Construct a Java-side AttrDictionary wrapper
JNIEXPORT jobject JNICALL MakeAttrDictionary(JNIEnv *env,WhirlyKit::Dictionary *dict);

// Construct a Java-side Vector Object
JNIEXPORT jobject JNICALL MakeVectorObject(JNIEnv *env,WhirlyKit::VectorObject *vec);
