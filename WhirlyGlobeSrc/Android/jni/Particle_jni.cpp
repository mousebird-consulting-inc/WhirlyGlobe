/*
 *  Particle_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 20/1/16.
 *  Copyright 2011-2015 mousebird consulting
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
#import "Maply_jni.h"
#import "com_mousebird_maply_Particle.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_Particle_nativeInit
(JNIEnv *env, jclass cls)
{
    ParticleClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Particle_initialise
(JNIEnv *, jobject obj)
{
    try {
        ParticleClassInfo *classInfo = ParticleClassInfo::getClassInfo();
        Particle *inst = new Particle();
        classInfo->setHandle(env, obj, inst);
    }
    catch(...) {
        _android_log_print(ANDROID_LOG_VERBOSE), "Maply", "Crash in Particle::initialise()"
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Particle_dispose
(JNIEnv *, jobject obj)
{
    try {
        ParticleClassInfo *classInfo = ParticleClassInfo::getClassInfo();
        Particle *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

		delete inst;
        
        classInfo->clearHandle(env, obj);
    }
    catch(...) {
        _android_log_print(ANDROID_LOG_VERBOSE), "Maply", "Crash in Particle::dispose()"
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Particle_setLoc
(JNIEnv *, jobject obj, jobject loc)
{
    try {
        ParticleClassInfo *classInfo = ParticleClassInfo::getClassInfo();
        Particle *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

		Point3d *pointLoc = Point3dClassInfo::getClassInfo()->getObject(env, loc);
        inst->loc = pointLoc;
    }
    catch(...) {
        _android_log_print(ANDROID_LOG_VERBOSE), "Maply", "Crash in Particle::setLoc()"
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Particle_setColor
(JNIEnv *, jobject, jfloat r, jfloat g, jfloat b, jfloat a)
{
    try {
        ParticleClassInfo *classInfo = ParticleClassInfo::getClassInfo();
        Particle *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

		inst->color = RGBAColor(r*255f, g*255f, b*255f, a*255f);
    }
    catch(...) {
        _android_log_print(ANDROID_LOG_VERBOSE), "Maply", "Crash in Particle::setColor()"
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Particle_setVelocity
(JNIEnv *, jobject, jfloat vel)
{
    try {
        ParticleClassInfo *classInfo = ParticleClassInfo::getClassInfo();
        Particle *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

		inst->velocity = vel;
    }
    catch(...) {
        _android_log_print(ANDROID_LOG_VERBOSE), "Maply", "Crash in Particle::setVelocity()"
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_Particle_setExpiration
(JNIEnv *, jobject, jdouble exp)
{
    try {
        ParticleClassInfo *classInfo = ParticleClassInfo::getClassInfo();
        Particle *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;

		inst->expiration = exp;
    }
    catch(...) {
        _android_log_print(ANDROID_LOG_VERBOSE), "Maply", "Crash in Particle::setExpiration()"
    }
}