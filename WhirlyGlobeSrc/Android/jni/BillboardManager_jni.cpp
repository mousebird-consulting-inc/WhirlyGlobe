/*
 *  BillboardManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
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
#import "Maply_jni.h"
#import "com_mousebird_maply_BillboardManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardManager_nativeInit
(JNIEnv *env, jclass cls)
{
    BillboardManagerClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardManager_initialise
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardManagerClassInfo *classInfo = BillboardManagerClassInfo::getClassInfo();
        BillboardManager *inst = new BillboardManager();
        classInfo->setHandle(env, obj, inst);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardManager::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardManager_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardManagerClassInfo *classInfo = BillboardManagerClassInfo::getClassInfo();
        BillboardManager *inst = classInfo->getObject(env, obj);
        if (!inst)
            return;
        
        delete inst;
		classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardManager::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_BillboardManager_addBillboards
(JNIEnv *env, jobject obj, jobject arrayObj, jobject infoObj, jlong billShader, jobject changeObj)
{
    try
    {
        BillboardManagerClassInfo *classInfo = BillboardManagerClassInfo::getClassInfo();
        BillboardManager *inst = classInfo->getObject(env, obj);
        BillboardInfo *info = BillboardInfoClassInfo::getClassInfo()->getObject(env, infoObj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !info || !changeSet)
            return EmptyIdentity;
        
        // Get the iterator
        // Note: Look these up once
        jclass listClass = env->GetObjectClass(arrayObj);
        jclass iterClass = env->FindClass("java/util/Iterator");
        jmethodID literMethod = env->GetMethodID(listClass,"iterator","()Ljava/util/Iterator;");
        jobject liter = env->CallObjectMethod(arrayObj,literMethod);
        jmethodID hasNext = env->GetMethodID(iterClass,"hasNext","()Z");
        jmethodID next = env->GetMethodID(iterClass,"next","()Ljava/lang/Object;");
        env->DeleteLocalRef(iterClass);
        env->DeleteLocalRef(listClass);
        
        std::vector<Billboard*> bills;
        BillboardClassInfo *billClassInfo = BillboardClassInfo::getClassInfo();
        while (env->CallBooleanMethod(liter, hasNext))
        {
            jobject javaVecObj = env->CallObjectMethod(liter, next);
            Billboard *bill = billClassInfo->getObject(env,javaVecObj);
            bills.push_back(bill);
            env->DeleteLocalRef(javaVecObj);
        }
        env->DeleteLocalRef(liter);
        
        SimpleIdentity billId = inst->addBillboards(bills, info, billShader, *changeSet);

        return billId;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardManager::addBillboards()");
    }
    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardManager_enableBillboards
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeObj)
{
    try
    {
        BillboardManagerClassInfo *classInfo = BillboardManagerClassInfo::getClassInfo();
        BillboardManager *inst = classInfo->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;
        
        JavaLongArray ids(env, idArrayObj);
        SimpleIDSet idSet;
        for (unsigned int ii=0; ii<ids.len; ii++)
        {
            idSet.insert(ids.rawLong[ii]);
        }
        inst->enableBillboards(idSet, enable, *changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardManager::enableBillboards()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardManager_removeBillboards
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeObj)
{
    try
    {
        BillboardManagerClassInfo *classInfo = BillboardManagerClassInfo::getClassInfo();
        BillboardManager *inst = classInfo->getObject(env, obj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;
        
        JavaLongArray ids(env, idArrayObj);
        SimpleIDSet idSet;
        for (unsigned int ii=0; ii<ids.len; ii++)
        {
            idSet.insert(ids.rawLong[ii]);
        }
        inst->removeBillboards(idSet, *changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardManager::removeBillboards()");
    }
}
