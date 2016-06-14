/*
 *  LayoutManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
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
#import "com_mousebird_maply_LayoutManager.h"
#import "WhirlyGlobe.h"

using namespace WhirlyKit;
using namespace Maply;

typedef JavaClassInfo<LayoutManager> LayoutManagerClassInfo;
template<> LayoutManagerClassInfo *LayoutManagerClassInfo::classInfoObj = NULL;

typedef std::map<int, jobject> ClusterGenMap;

// Wrapper that tracks the generator as well
class LayoutManagerWrapper : public ClusterGenerator
{
public:
    LayoutManagerWrapper(LayoutManager *layoutManager)
        : layoutManager(layoutManager), env(NULL)
    {
        pthread_mutex_init(&changeLock,NULL);
        layoutManager->addClusterGenerator(this);
    }

    ~LayoutManagerWrapper() {
        // Note: Should clean up Java refs
    }
    
    void setEnv(JNIEnv *inEnv)
    {
        env = inEnv;
    }
    
    // Add a cluster generator for callback
    void addClusterGenerator(jobject clusterObj, jint clusterID)
    {
        // Note: Need to get a globe ref
        clusterGens[clusterID] = clusterObj;
    }

    /** ClusterGenerator virtual methods.
      */
    // Called right before we start generating layout objects
    void startLayoutObjects()
    {
        oldClusterTex = currentClusterTex;
        currentClusterTex.clear();
        
        pthread_mutex_lock(&changeLock);
        
        for (auto it = clusterGens.begin(); it != clusterGens.end(); ++it) {
            // Look for the wrapper object's methods
            jobject obj = it->second;
            jclass theClass = env->GetObjectClass(obj);
            jmethodID func = env->GetMethodID(theClass, "startClusterGroup", "()V");
            env->CallVoidMethod(obj, func);
            env->DeleteLocalRef(theClass);
        }
        
        pthread_mutex_unlock(&changeLock);
    }
    
    // Figure out
    void makeLayoutObject(int clusterID, const std::vector<LayoutObjectEntry *> &layoutObjects, LayoutObject &retObj)
    {
        // Find the right cluster generator
        jobject clusterGenerator;
        
        pthread_mutex_lock(&changeLock);
        clusterGenerator = clusterGens[clusterID];
        pthread_mutex_unlock(&changeLock);
        
        if (!clusterGenerator)
            return;
        
        jclass theClass = env->GetObjectClass(clusterGenerator);
        
        // Pick a representive screen object
        int drawPriority = -1;
        LayoutObject *sampleObj = NULL;
        for (auto obj : layoutObjects) {
            if (obj->obj.getDrawPriority() > drawPriority) {
                drawPriority = obj->obj.getDrawPriority();
                sampleObj = &obj->obj;
            }
        }
        
        SimpleIdentity progID = sampleObj->getTypicalProgramID();
        
        // Ask for a cluster image
        jclass clusterInfoClass = env->FindClass("com/mousebird/maply/MaplyClusterInfo");
        jmethodID initMethod = env->GetMethodID(clusterInfoClass, "<init>", "(I)V");
        
        jobject clusterInfoObj = env->NewObject(clusterInfoClass, initMethod, layoutObjects.size());
        
        env->DeleteLocalRef(clusterInfoClass);
        
        jmethodID func = env->GetMethodID(theClass,"makeClusterGroup","(Lcom/mousebird/maply/MaplyClusterInfo;)V");
        jobject group = env->CallObjectMethod(clusterGenerator, func, clusterInfoObj);
        
        jclass groupClass = env->GetObjectClass(group);
        jmethodID getSize = env->GetMethodID(groupClass, "getSize", "()Lcom/mousebird/maply/Point2d");
        
        jobject point2dObj = env->CallObjectMethod(group, getSize);
        
        env->DeleteLocalRef(groupClass);
        
        Point2d *size = Point2dClassInfo::getClassInfo(env, env->GetObjectClass(point2dObj))->getObject(env, point2dObj);
        
        if (!size)
            return;
        
        env->DeleteLocalRef(theClass);
        
        // Geometry for the new cluster object
        ScreenSpaceObject::ConvexGeometry smGeom;
        smGeom.progID = progID;
        smGeom.coords.push_back(Point2d(- size->x()/2.0,-size->y()/2.0));
        smGeom.texCoords.push_back(TexCoord(0,1));
        smGeom.coords.push_back(Point2d(size->x()/2.0,-size->y()/2.0));
        smGeom.texCoords.push_back(TexCoord(1,1));
        smGeom.coords.push_back(Point2d(size->x()/2.0,size->y()/2.0));
        smGeom.texCoords.push_back(TexCoord(1,0));
        smGeom.coords.push_back(Point2d(-size->x()/2.0,size->y()/2.0));
        smGeom.texCoords.push_back(TexCoord(0,0));
        smGeom.color = RGBAColor(255,255,255,255);
        
        retObj.layoutPts = smGeom.coords;
        retObj.selectPts = smGeom.coords;
        
        /** NOTE: PORTING
         // Create the texture
         // Note: Keep this around
         MaplyTexture *maplyTex = [self addTexture:group.image desc:@{kMaplyTexFormat: @(MaplyImageIntRGBA),
         kMaplyTexAtlas: @(true),
         kMaplyTexMagFilter: kMaplyMinFilterNearest}
         mode:MaplyThreadCurrent];
         currentClusterTex.push_back(maplyTex);
         if (maplyTex.isSubTex)
         {
         SubTexture subTex = scene->getSubTexture(maplyTex.texID);
         subTex.processTexCoords(smGeom.texCoords);
         smGeom.texIDs.push_back(subTex.texId);
         } else
         smGeom.texIDs.push_back(maplyTex.texID);
         */
        
        retObj.setDrawPriority(drawPriority);
        retObj.addGeometry(smGeom);
    }
    
    // Called right after all the layout objects are generated
    virtual void endLayoutObjects()
    {
        // Layout of new objects is over, so schedule the old textures for removal
        if (!oldClusterTex.empty())
        {
            // Note: Remove old textures
            
            oldClusterTex = currentClusterTex;
        }
        
        pthread_mutex_lock(&changeLock);
        for (auto it = clusterGens.begin(); it != clusterGens.end(); ++it) {
            jobject obj = it->second;
            jclass theClass = env->GetObjectClass(obj);
            jmethodID func = env->GetMethodID(theClass, "endClusterGroup", "()V");
            env->CallVoidMethod(obj, func);
            env->DeleteLocalRef(theClass);
        }
        
        pthread_mutex_unlock(&changeLock);
    }
    
    void paramsForClusterClass(int clusterID,ClusterClassParams &clusterParams)
    {
        jobject clusterGenerator;
        
        pthread_mutex_lock(&changeLock);
        clusterGenerator = clusterGens[clusterID];
        pthread_mutex_unlock(&changeLock);
        
        jclass theClass = env->GetObjectClass(clusterGenerator);
        
        // Ask for the shader for moving objects
        clusterParams.motionShaderID = EmptyIdentity;
        
        jmethodID motionShader = env->GetMethodID(theClass, "motionShader", "()Lcom/mousebird/maply/Shader");
        
        jobject programObj = env->CallObjectMethod(clusterGenerator, motionShader);
        
        OpenGLES2Program *program =  OpenGLES2ProgramClassInfo::getClassInfo(env,env->GetObjectClass(programObj))->getObject(env, programObj);
        
        if (program) {
            clusterParams.motionShaderID = program->getId();
        }
        else {
            //NOTE: Porting
            //program = scene->getProgramBySceneName(kToolkitDefaultScreenSpaceMotionProgram);
            //if (program)
            //  clusterParams.motionShaderID = program->getId();
        }
        
        jmethodID clusterLayoutSize = env->GetMethodID(theClass, "clusterLayoutSize", "()Lcom/mousebird/maply/Point2d");
        
        jobject sizeObj = env->CallObjectMethod(clusterGenerator, clusterLayoutSize);
        
        Point2d *size = Point2dClassInfo::getClassInfo(env, env->GetObjectClass(sizeObj))->getObject(env, sizeObj);
        
        if (!size)
            return;
        
        clusterParams.clusterSize = *size;
        
        jmethodID selectable = env->GetMethodID(theClass, "selectable", "()Z");
        
        clusterParams.selectable = env->CallBooleanMethod(clusterGenerator, selectable);
        
        jmethodID markerAnimationTime = env->GetMethodID(theClass, "markerAnimationTime", "()D");
        
        clusterParams.markerAnimationTime = env->CallDoubleMethod(clusterGenerator, markerAnimationTime);
    }
    
public:
    LayoutManager *layoutManager;

    SimpleIDSet currentClusterTex,oldClusterTex;
    pthread_mutex_t changeLock;
    
    ClusterGenMap clusterGens;
    JNIEnv *env;
};

typedef JavaClassInfo<LayoutManagerWrapper> LayoutManagerWrapperClassInfo;
template<> LayoutManagerWrapperClassInfo *LayoutManagerWrapperClassInfo::classInfoObj = NULL;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_nativeInit
  (JNIEnv *env, jclass cls)
{
    LayoutManagerWrapperClassInfo::getClassInfo(env,cls);
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_initialise
  (JNIEnv *env, jobject obj,jobject sceneObj)
{
    try
    {
        Scene *scene = SceneClassInfo::getClassInfo()->getObject(env,sceneObj);
        LayoutManager *layoutManager = dynamic_cast<LayoutManager *>(scene->getManager(kWKLayoutManager));
        LayoutManagerWrapper *wrap = new LayoutManagerWrapper(layoutManager);
        LayoutManagerWrapperClassInfo::getClassInfo()->setHandle(env, obj, wrap);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::initialise()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return
        classInfo->clearHandle(env, obj);
        delete wrap;
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::dispose()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_setMaxDisplayObjects
  (JNIEnv *env, jobject obj, jint maxObjs)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return
            
        wrap->layoutManager->setMaxDisplayObjects(maxObjs);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::setMaxDisplayObjects()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_updateLayout
  (JNIEnv *env, jobject obj, jobject viewStateObj, jobject changeSetObj)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        ViewState *viewState = ViewStateClassInfo::getClassInfo()->getObject(env,viewStateObj);
        ChangeSet *changeSet = ChangeSetClassInfo::getClassInfo()->getObject(env,changeSetObj);
        if (!wrap || !viewState || !changeSet)
            return;
        wrap->setEnv(env);

        wrap->layoutManager->updateLayout(viewState,*changeSet);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::updateLayout()");
    }
}

JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_LayoutManager_hasChanges
  (JNIEnv *env, jobject obj)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return false;
        wrap->setEnv(env);
        
        return wrap->layoutManager->hasChanges();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::hasChanges()");
    }
    
    return false;
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_addClusterGenerator
(JNIEnv *env, jobject obj, jobject clusterObj, jint clusterID)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return;
        wrap->setEnv(env);

        wrap->addClusterGenerator(clusterObj,clusterID);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::addClusterGenerator()");
    }

}
