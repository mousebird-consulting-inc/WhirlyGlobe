/*
 *  Geometry_jni.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/7/19.
 *  Copyright 2011-2019 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import "Maply_jni.h"
#import "com_mousebird_maply_Point2d.h"
#import "com_mousebird_maply_Point3d.h"
#import "com_mousebird_maply_Point4d.h"
#import "com_mousebird_maply_Matrix4d.h"
#import "com_mousebird_maply_Quaternion.h"
#import "WhirlyGlobe_Android.h"

namespace WhirlyKit
{
typedef Eigen::Vector4d Point4d;
typedef Eigen::Vector4f Point4f;
}

typedef JavaClassInfo<WhirlyKit::Point2d> Point2dClassInfo;
typedef JavaClassInfo<WhirlyKit::Point3d> Point3dClassInfo;
typedef JavaClassInfo<WhirlyKit::Point4d> Point4dClassInfo;
typedef JavaClassInfo<Eigen::Matrix3d> Matrix3dClassInfo;
typedef JavaClassInfo<Eigen::Matrix4d> Matrix4dClassInfo;
typedef JavaClassInfo<Eigen::Quaterniond> QuaternionClassInfo;
typedef JavaClassInfo<Eigen::AngleAxisd> AngleAxisClassInfo;

// Construct a Java-side Point2d
JNIEXPORT jobject JNICALL MakePoint2d(JNIEnv *env,const WhirlyKit::Point2d &pt);

// Construct a Java-side Point3d
JNIEXPORT jobject JNICALL MakePoint3d(JNIEnv *env,const WhirlyKit::Point3d &pt);

// Construct a Java-side Point4d
JNIEXPORT jobject JNICALL MakePoint4d(JNIEnv *env,const WhirlyKit::Point4d &pt);

// Construct a Java-side Matrix3d
JNIEXPORT jobject JNICALL MakeMatrix3d(JNIEnv *env,const Eigen::Matrix3d &mat);

// Construct a Java-side Matrix4d
JNIEXPORT jobject JNICALL MakeMatrix4d(JNIEnv *env,const Eigen::Matrix4d &mat);

// Construct a Java-side Quaternion
JNIEXPORT jobject JNICALL MakeQuaternion(JNIEnv *env,const Eigen::Quaterniond &quat);
