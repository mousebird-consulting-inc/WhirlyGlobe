/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_mousebird_maply_AttrDictionary */

#ifndef _Included_com_mousebird_maply_AttrDictionary
#define _Included_com_mousebird_maply_AttrDictionary
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    parseFromJSON
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_AttrDictionary_parseFromJSON
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    hasField
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_AttrDictionary_hasField
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    getString
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_AttrDictionary_getString
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    getInt
 * Signature: (Ljava/lang/String;)Ljava/lang/Integer;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getInt
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    getDouble
 * Signature: (Ljava/lang/String;)Ljava/lang/Double;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getDouble
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    getIdentity
 * Signature: (Ljava/lang/String;)Ljava/lang/Long;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getIdentity
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    get
 * Signature: (Ljava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_get
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    getDict
 * Signature: (Ljava/lang/String;)Lcom/mousebird/maply/AttrDictionary;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getDict
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    getEntry
 * Signature: (Ljava/lang/String;)Lcom/mousebird/maply/AttrDictionaryEntry;
 */
JNIEXPORT jobject JNICALL Java_com_mousebird_maply_AttrDictionary_getEntry
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    getArray
 * Signature: (Ljava/lang/String;)[Lcom/mousebird/maply/AttrDictionaryEntry;
 */
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_AttrDictionary_getArray
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    getKeys
 * Signature: ()[Ljava/lang/String;
 */
JNIEXPORT jobjectArray JNICALL Java_com_mousebird_maply_AttrDictionary_getKeys
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    setString
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setString
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    setInt
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setInt
  (JNIEnv *, jobject, jstring, jint);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    setDouble
 * Signature: (Ljava/lang/String;D)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setDouble
  (JNIEnv *, jobject, jstring, jdouble);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    setDict
 * Signature: (Ljava/lang/String;Lcom/mousebird/maply/AttrDictionary;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setDict
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    setArray
 * Signature: (Ljava/lang/String;[Lcom/mousebird/maply/AttrDictionaryEntry;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_setArray
  (JNIEnv *, jobject, jstring, jobjectArray);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    toString
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_mousebird_maply_AttrDictionary_toString
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    addEntries
 * Signature: (Lcom/mousebird/maply/AttrDictionary;)V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_addEntries
  (JNIEnv *, jobject, jobject);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    nativeInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_nativeInit
  (JNIEnv *, jclass);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    initialise
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_initialise
  (JNIEnv *, jobject);

/*
 * Class:     com_mousebird_maply_AttrDictionary
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_mousebird_maply_AttrDictionary_dispose
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
