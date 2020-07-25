/*
 *  ShapeManager_jni.cpp
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

#import "Shapes_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_ShapeManager.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> ShapeManagerClassInfo *ShapeManagerClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_nativeInit
(JNIEnv *env, jclass cls)
{
    ShapeManagerClassInfo::getClassInfo(env, cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_initialise
(JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj);
        if (!scene)
            return;
        ShapeManager *shapeManager = dynamic_cast<ShapeManager *>(scene->getManager(kWKShapeManager));
        ShapeManagerClassInfo::getClassInfo()->setHandle(env,obj,shapeManager);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_dispose
(JNIEnv *env, jobject obj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        classInfo->clearHandle(env, obj);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::dispose()");
    }
}

JNIEXPORT jlong JNICALL Java_com_mousebird_maply_ShapeManager_addShapes
(JNIEnv *env, jobject obj, jobjectArray arrayObj, jobject shapeInfoObj, jobject changeObj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        ShapeManager *inst = classInfo->getObject(env, obj);
        ShapeInfoRef *shapeInfo = ShapeInfoClassInfo::getClassInfo()->getObject(env, shapeInfoObj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);

        if (!inst || !shapeInfo || !changeSet)
            return EmptyIdentity;

        ShapeClassInfo *shapeClassInfo = ShapeClassInfo::getClassInfo();

        // Work through the shapes
        std::vector<Shape *> shapes;
        JavaObjectArrayHelper arrayHelp(env,arrayObj);
        while (jobject shapeObj = arrayHelp.getNextObject()) {
            Shape *shape = shapeClassInfo->getObject(env,shapeObj);

            // Great circle is just a concept, not an actual object
            GreatCircle_Android *greatCircle = dynamic_cast<GreatCircle_Android *>(shape);
            if (greatCircle)
            {
                Linear *lin = greatCircle->asLinear(inst->getScene()->getCoordAdapter());
                if (lin)
                    shapes.push_back(lin);
            } else {
                shapes.push_back(shape);
            }
        }

        if ((*shapeInfo)->programID == EmptyIdentity) {
            ProgramGLES *prog = (ProgramGLES *)inst->getScene()->findProgramByName(MaplyDefaultModelTriShader);
            if (prog)
                (*shapeInfo)->programID = prog->getId();
        }

        SimpleIdentity shapeId = inst->addShapes(shapes, *(*shapeInfo), *(changeSet->get()));
        return shapeId;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::addShapes()");
    }
    
    return EmptyIdentity;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_removeShapes
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeObj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        ShapeManager *inst = classInfo->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;

        JavaLongArray ids(env,idArrayObj);
        SimpleIDSet idSet;
        for (unsigned int ii=0;ii<ids.len;ii++)
        {
            idSet.insert(ids.rawLong[ii]);
        }
        
        inst->removeShapes(idSet, *(changeSet->get()));
    }
    
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::removeShapes()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_enableShapes
(JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeObj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        ShapeManager *inst = classInfo->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;

        JavaLongArray ids(env,idArrayObj);
        SimpleIDSet idSet;
        for (unsigned int ii=0;ii<ids.len;ii++)
        {
            idSet.insert(ids.rawLong[ii]);
        }
        
        inst->enableShapes(idSet, enable, *(changeSet->get()));
    }
    
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in ShapeManager::enableShapes()");
    }
}
