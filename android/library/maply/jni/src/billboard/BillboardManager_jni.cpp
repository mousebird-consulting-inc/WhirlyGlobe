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

#import "Maply_jni.h"
#import "Scene_jni.h"
#import "Billboard_jni.h"
#import "com_mousebird_maply_BillboardManager.h"
#import "WhirlyGlobe_Android.h"

using namespace WhirlyKit;

typedef JavaClassInfo<WhirlyKit::BillboardManager> BillboardManagerClassInfo;
template<> BillboardManagerClassInfo *BillboardManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardManager_nativeInit
(JNIEnv *env, jclass cls)
{
    BillboardManagerClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardManager_initialise
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        BillboardManagerClassInfo *classInfo = BillboardManagerClassInfo::getClassInfo();
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        BillboardManager *billManager = dynamic_cast<BillboardManager *>(scene->getManager(kWKBillboardManager));
        classInfo->setHandle(env, obj, billManager);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardManager::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_BillboardManager_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        BillboardManagerClassInfo *classInfo = BillboardManagerClassInfo::getClassInfo();
		classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardManager::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_BillboardManager_addBillboards
(JNIEnv *env, jobject obj, jobjectArray objArray, jobject infoObj, jobject changeObj)
{
    try
    {
        BillboardManagerClassInfo *classInfo = BillboardManagerClassInfo::getClassInfo();
        BillboardManager *billManager = classInfo->getObject(env, obj);
        BillboardInfoRef *billInfo = BillboardInfoClassInfo::getClassInfo()->getObject(env, infoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!billManager || !billInfo || !changeSet)
            return EmptyIdentity;
        
        // Collect all the billboards
        std::vector<Billboard*> bills;
        BillboardClassInfo *billClassInfo = BillboardClassInfo::getClassInfo();
        JavaObjectArrayHelper billArrayHelp(env,objArray);
        while (jobject billObj = billArrayHelp.getNextObject()) {
            Billboard *bill = billClassInfo->getObject(env,billObj);
            bills.push_back(bill);
        }

        // Resolve a missing program
        if ((*billInfo)->programID == EmptyIdentity)
        {
            Program *prog = NULL;
            if ((*billInfo)->orient == BillboardInfo::Orient::Eye)
                prog = billManager->getScene()->findProgramByName(MaplyBillboardEyeShader);
            else
                prog = billManager->getScene()->findProgramByName(MaplyBillboardGroundShader);
            if (prog)
                (*billInfo)->programID = prog->getId();
        }


        SimpleIdentity billId = billManager->addBillboards(bills, *(*billInfo), *(changeSet->get()));

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
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;
        
        JavaLongArray ids(env, idArrayObj);
        SimpleIDSet idSet;
        for (unsigned int ii=0; ii<ids.len; ii++)
        {
            idSet.insert(ids.rawLong[ii]);
        }
        inst->enableBillboards(idSet, enable, *(changeSet->get()));
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
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;
        
        JavaLongArray ids(env, idArrayObj);
        SimpleIDSet idSet;
        for (unsigned int ii=0; ii<ids.len; ii++)
        {
            idSet.insert(ids.rawLong[ii]);
        }
        inst->removeBillboards(idSet, *(changeSet->get()));
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in BillboardManager::removeBillboards()");
    }
}
