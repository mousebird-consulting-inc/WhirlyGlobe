/*
 *  Sun_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/28/16.
 *  Copyright 2011-2016 mousebird consulting
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

#import <jni.h>
#import <AA+.h>
#import "Maply_jni.h"
#import "Maply_utils_jni.h"
#import "com_mousebird_maply_Sun.h"
#import "WhirlyGlobe.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{
class Sun
{
public:
    Sun() : time(0.0) { }
    
    double time;
    double sunLon,sunLat;
};
    
typedef JavaClassInfo<WhirlyKit::Sun> SunClassInfo;
template<> SunClassInfo *SunClassInfo::classInfoObj = NULL;

}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_nativeInit
(JNIEnv *env, jclass cls)
{
    SunClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_initialise
(JNIEnv *env, jobject obj)
{
    try {
        Sun *sun = new Sun();
        SunClassInfo::getClassInfo()->setHandle(env,obj,sun);
    } catch (...) {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Sun::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        SunClassInfo *classInfo = SunClassInfo::getClassInfo();
        Sun *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        delete inst;
        
        classInfo->clearHandle(env,obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Sun::dispose()");
    }
}

JNIEXPORT jobject JNICALL Java_com_mousebird_maply_Sun_getDirection
(JNIEnv *env, jobject obj)
{
    try
    {
        SunClassInfo *classInfo = SunClassInfo::getClassInfo();
        Sun *inst = classInfo->getObject(env,obj);
        if (!inst)
            return NULL;
        
        double z = sin(inst->sunLat);
        double rad = sqrt(1.0-z*z);
        Point3d pt(rad*cos(inst->sunLon),rad*sin(inst->sunLon),z);
        
        return MakePoint3d(env, pt);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Sun::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Sun_setTime
(JNIEnv *env, jobject obj, jdouble theTime, jdouble year, jdouble month, jdouble day, jdouble hour, jdouble minute, jdouble second)
{
    try
    {
        SunClassInfo *classInfo = SunClassInfo::getClassInfo();
        Sun *inst = classInfo->getObject(env,obj);
        if (!inst)
            return;
        inst->time = theTime;
        
        CAADate aaDate(year,month,day,hour,minute,second,true);
        double jdSun = CAADynamicalTime::UTC2TT(aaDate.Julian());
        
        // Position of the sun in equatorial
        double sunEclipticLong = CAASun::ApparentEclipticLongitude(jdSun);
        double sunEclipticLat = CAASun::ApparentEclipticLatitude(jdSun);
        double obliquity = CAANutation::TrueObliquityOfEcliptic(jdSun);
        CAA2DCoordinate sunEquatorial = CAACoordinateTransformation::Ecliptic2Equatorial(sunEclipticLong,sunEclipticLat,obliquity);
        
        double siderealTime = CAASidereal::MeanGreenwichSiderealTime(jdSun);
        
        inst->sunLon = CAACoordinateTransformation::DegreesToRadians(15*(sunEquatorial.X-siderealTime));
        inst->sunLat = CAACoordinateTransformation::DegreesToRadians(sunEquatorial.Y);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in Sun::dispose()");
    }
}
