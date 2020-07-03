/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_mousebird_maply_VectorObject */

#ifndef _Included_com_mousebird_maply_VectorObject
#define _Included_com_mousebird_maply_VectorObject
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    getVectorTypeNative
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorObject_getVectorTypeNative
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    setSelectable
 * Signature: (Z)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_setSelectable
  (JNIEnv *, jobject, jboolean);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    getSelectable
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_getSelectable
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    getAttributes
 * Signature: ()Lcom/mousebird/maply/AttrDictionary;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_getAttributes
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    setAttributes
 * Signature: (Lcom/mousebird/maply/AttrDictionary;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_setAttributes
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    addPoint
 * Signature: (Lcom/mousebird/maply/Point2d;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addPoint
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    addLinear
 * Signature: ([Lcom/mousebird/maply/Point2d;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addLinear
  (JNIEnv *, jobject, jobjectArray);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    addAreal
 * Signature: ([Lcom/mousebird/maply/Point2d;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addAreal___3Lcom_mousebird_maply_Point2d_2
  (JNIEnv *, jobject, jobjectArray);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    addAreal
 * Signature: ([Lcom/mousebird/maply/Point2d;[[Lcom/mousebird/maply/Point2d;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_addAreal___3Lcom_mousebird_maply_Point2d_2_3_3Lcom_mousebird_maply_Point2d_2
  (JNIEnv *, jobject, jobjectArray, jobjectArray);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    mergeVectorsFrom
 * Signature: (Lcom/mousebird/maply/VectorObject;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_mergeVectorsFrom
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    center
 * Signature: ()Lcom/mousebird/maply/Point2d;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_center
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    centroid
 * Signature: ()Lcom/mousebird/maply/Point2d;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_centroid
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    largestLoopCenter
 * Signature: (Lcom/mousebird/maply/Point2d;Lcom/mousebird/maply/Point2d;)Lcom/mousebird/maply/Point2d;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_largestLoopCenter
  (JNIEnv *, jobject, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    linearMiddle
 * Signature: (Lcom/mousebird/maply/Point2d;)D
 */
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_linearMiddle__Lcom_mousebird_maply_Point2d_2
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    linearMiddle
 * Signature: (Lcom/mousebird/maply/Point2d;Lcom/mousebird/maply/CoordSystem;)D
 */
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_linearMiddle__Lcom_mousebird_maply_Point2d_2Lcom_mousebird_maply_CoordSystem_2
  (JNIEnv *, jobject, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    middleCoordinate
 * Signature: (Lcom/mousebird/maply/Point2d;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_middleCoordinate
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    pointInside
 * Signature: (Lcom/mousebird/maply/Point2d;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_pointInside
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    areaOfOuterLoops
 * Signature: ()D
 */
JNIEXPORT jdouble JNICALL Java_com_mousebird_maply_VectorObject_areaOfOuterLoops
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    countPoints
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_mousebird_maply_VectorObject_countPoints
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    boundingBox
 * Signature: (Lcom/mousebird/maply/Point2d;Lcom/mousebird/maply/Point2d;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_boundingBox
  (JNIEnv *, jobject, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    subdivideToGlobeNative
 * Signature: (Lcom/mousebird/maply/VectorObject;D)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToGlobeNative
  (JNIEnv *, jobject, jobject, jdouble);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    subdivideToGlobeGreatCircleNative
 * Signature: (Lcom/mousebird/maply/VectorObject;D)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToGlobeGreatCircleNative
  (JNIEnv *, jobject, jobject, jdouble);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    subdivideToFlatGreatCircleNative
 * Signature: (Lcom/mousebird/maply/VectorObject;D)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_subdivideToFlatGreatCircleNative
  (JNIEnv *, jobject, jobject, jdouble);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    tesselateNative
 * Signature: (Lcom/mousebird/maply/VectorObject;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_tesselateNative
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    clipToGridNative
 * Signature: (Lcom/mousebird/maply/VectorObject;DD)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_clipToGridNative
  (JNIEnv *, jobject, jobject, jdouble, jdouble);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    clipToMbrNative
 * Signature: (Lcom/mousebird/maply/VectorObject;DDDD)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_clipToMbrNative
  (JNIEnv *, jobject, jobject, jdouble, jdouble, jdouble, jdouble);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    reprojectNative
 * Signature: (Lcom/mousebird/maply/VectorObject;Lcom/mousebird/maply/CoordSystem;DLcom/mousebird/maply/CoordSystem;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_reprojectNative
  (JNIEnv *, jobject, jobject, jobject, jdouble, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    filterClippedEdgesNative
 * Signature: (Lcom/mousebird/maply/VectorObject;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_filterClippedEdgesNative
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    linearsToArealsNative
 * Signature: (Lcom/mousebird/maply/VectorObject;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_linearsToArealsNative
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    arealsToLinearsNative
 * Signature: (Lcom/mousebird/maply/VectorObject;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_arealsToLinearsNative
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    fromGeoJSON
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_fromGeoJSON
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    fromShapeFile
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_VectorObject_fromShapeFile
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    deepCopyNative
 * Signature: (Lcom/mousebird/maply/VectorObject;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_deepCopyNative
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    FromGeoJSONAssembly
 * Signature: (Ljava/lang/String;)Ljava/util/Map;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_VectorObject_FromGeoJSONAssembly
  (JNIEnv *, jclass, jstring);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    nativeInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_nativeInit
  (JNIEnv *, jclass);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    initialise
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_initialise
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_VectorObject
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_VectorObject_dispose
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif