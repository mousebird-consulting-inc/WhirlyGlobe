/*  ShapeManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2022 mousebird consulting
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
 */

#import "Shapes_jni.h"
#import "Scene_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_ShapeManager.h"

using namespace Eigen;
using namespace WhirlyKit;

template<> ShapeManagerClassInfo *ShapeManagerClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_nativeInit
  (JNIEnv *env, jclass cls)
{
    ShapeManagerClassInfo::getClassInfo(env, cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_initialise
  (JNIEnv *env, jobject obj, jobject sceneObj)
{
    try
    {
        if (Scene *scene = SceneClassInfo::getClassInfo()->getObject(env, sceneObj))
        {
            ShapeManagerRef shapeManager = scene->getManager<ShapeManager>(kWKShapeManager);
            ShapeManagerClassInfo::set(env, obj, new ShapeManagerRef(shapeManager));
        }
    }
    MAPLY_STD_JNI_CATCH()
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        delete classInfo->getObject(env, obj);
        classInfo->clearHandle(env, obj);
    }
    MAPLY_STD_JNI_CATCH()
}

// Convert points in-place from geo radians + height to display xyz.
template <typename TIter>
static void geoToDisplay(TIter pt, const TIter end, const CoordSystemDisplayAdapter *adapter)
{
    const auto cs = adapter->getCoordSystem();
    for (; pt != end; ++pt)
    {
        // We assume that the result of converting lat/long to display is normalized,
        // and that the height is in the same unit as display coords (radii).
        const auto r = 1 + pt->z();
        *pt = adapter->localToDisplay(cs->geographicToLocal(Slice(*pt))) * r;
    }
}

extern "C"
JNIEXPORT jlong JNICALL Java_com_mousebird_maply_ShapeManager_addShapes
  (JNIEnv *env, jobject obj, jobjectArray arrayObj, jobject shapeInfoObj, jobject changeObj)
{
    try
    {
        ShapeClassInfo *shapeClassInfo = ShapeClassInfo::getClassInfo();
        const ShapeManagerRef *inst = ShapeManagerClassInfo::get(env, obj);
        const ShapeInfoRef *shapeInfo = ShapeInfoClassInfo::get(env, shapeInfoObj);
        const ChangeSetRef *changeSet = ChangeSetClassInfo::get(env, changeObj);
        const auto scene = (inst && *inst) ? (*inst)->getScene() : nullptr;
        if (!scene || !shapeInfo || !changeSet)
        {
            return EmptyIdentity;
        }

        const auto adapter = scene->getCoordAdapter();

        // Work through the shapes
        std::vector<Shape *> shapes;
        JavaObjectArrayHelper arrayHelp(env,arrayObj);
        shapes.reserve(arrayHelp.numObjects());
        while (jobject shapeObj = arrayHelp.getNextObject())
        {
            Shape *shape = shapeClassInfo->getObject(env,shapeObj);
            Shape *res = shape;

            // Great circle is just a concept, not an actual object
            if (auto greatCircle = dynamic_cast<GreatCircle_Android *>(shape))
            {
                res = greatCircle->asLinear(adapter);
            }
            else if (auto lin = dynamic_cast<Linear *>(shape))
            {
                auto newLin = new Linear(*lin);
                geoToDisplay(newLin->pts.begin(), newLin->pts.end(), adapter);

                newLin->mbr.reset();
                for (const auto &p : newLin->pts)
                {
                    newLin->mbr.addPoint(Slice(p));
                }

                res = newLin;
            }

            if (res)
            {
                shapes.push_back(res);
            }
        }

        if ((*shapeInfo)->programID == EmptyIdentity)
        {
            if (ProgramGLES *prog = (ProgramGLES *)(*inst)->getScene()->findProgramByName(MaplyDefaultModelTriShader))
            {
                (*shapeInfo)->programID = prog->getId();
            }
        }

        SimpleIdentity shapeId = (*inst)->addShapes(shapes, *(*shapeInfo), *(changeSet->get()));
        return shapeId;
    }
    MAPLY_STD_JNI_CATCH()
    return EmptyIdentity;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_removeShapes
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jobject changeObj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        ShapeManagerRef *inst = classInfo->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;

        const SimpleIDSet idSet = ConvertLongArrayToSet(env, idArrayObj);
        (*inst)->removeShapes(idSet, **changeSet);
    }
    MAPLY_STD_JNI_CATCH()
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_ShapeManager_enableShapes
  (JNIEnv *env, jobject obj, jlongArray idArrayObj, jboolean enable, jobject changeObj)
{
    try
    {
        ShapeManagerClassInfo *classInfo = ShapeManagerClassInfo::getClassInfo();
        ShapeManagerRef *inst = classInfo->getObject(env, obj);
        ChangeSetRef *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env, changeObj);
        if (!inst || !changeSet)
            return;

        const SimpleIDSet idSet = ConvertLongArrayToSet(env, idArrayObj);

        (*inst)->enableShapes(idSet, enable, **changeSet);
    }
    MAPLY_STD_JNI_CATCH()
}
