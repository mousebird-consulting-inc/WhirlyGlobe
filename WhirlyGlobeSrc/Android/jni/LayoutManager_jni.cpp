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

// Wrapper that tracks the generator as well
class LayoutManagerWrapper : public ClusterGenerator
{
public:
    
    // Used to keep track of cluster objects for callbacks
    class ClusterInfo
    {
    public:
        bool operator < (const ClusterInfo &that) const
        {
            return clusterID < that.clusterID;
        }
        
        void init(JNIEnv *env,int inClusterID,const Point2d &inLayoutSize,jobject inClusterObj)
        {
            clusterID = inClusterID;
            layoutSize = inLayoutSize;
            clusterObj = env->NewGlobalRef(inClusterObj);
            
            // Methods can be saved without consequence
            jclass theClass = env->GetObjectClass(clusterObj);
            startClusterGroupJava = env->GetMethodID(theClass, "startClusterGroup", "()V");
            makeClusterGroupJNIJava = env->GetMethodID(theClass, "makeClusterGroupJNI", "(I)J");
            endClusterGroupJava = env->GetMethodID(theClass, "endClusterGroup", "()V");
            env->DeleteLocalRef(theClass);
        }
        
        void clear(JNIEnv *env)
        {
            env->DeleteGlobalRef(clusterObj);
        }

        int clusterID;
        Point2d layoutSize;
        jobject clusterObj;
        bool selectable;

        // Methods we'll use to call into the Java cluster generator
        jmethodID startClusterGroupJava;
        jmethodID makeClusterGroupJNIJava;
        jmethodID endClusterGroupJava;
    };
    typedef std::set<ClusterInfo> ClusterInfoSet;

    LayoutManagerWrapper(Scene *scene,LayoutManager *layoutManager)
        : layoutManager(layoutManager), env(NULL)
    {
        OpenGLES2Program *program = scene->getProgramBySceneName(kToolkitDefaultScreenSpaceMotionProgram);
        if (program)
            motionShaderID = program->getId();

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
    void addClusterGenerator(jobject clusterObj, jint clusterID,bool selectable, double sizeX, double sizeY)
    {
        ClusterInfo clusterInfo;
        clusterInfo.init(env,clusterID,Point2d(sizeX,sizeY),clusterObj);
        clusterInfo.selectable = selectable;
        
        clusterGens.insert(clusterInfo);
    }
    
    void clearClusterGenerators()
    {
        for (auto &ci : clusterGens)
            ci.clear();
        clusterGens.clear();
    }

    /** ClusterGenerator virtual methods.
      */
    // Called right before we start generating layout objects
    void startLayoutObjects()
    {
        oldClusterTex = currentClusterTex;
        currentClusterTex.clear();

        // Notify all the cluster generators
        for (const auto &clusterGen : clusterGens)
            env->CallVoidMethod(clusterGen.clusterObj,clusterGen.startClusterGroupJava);
    }

    // Ask the appropriate cluster generator to make a cluster image
    void makeLayoutObject(int clusterID, const std::vector<LayoutObjectEntry *> &layoutObjects, LayoutObject &retObj)
    {
        ClusterInfo dummyInfo;
        dummyInfo.clusterID = clusterID;
        auto it = clusterGens.find(dummyInfo);
        if (it == clusterGens.end())
            return;
        
        const ClusterInfo &clusterGenerator = *it;
        
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

        // The texture gets created on the Java side, so we'll just use the ID
        long texID = env->CallLongMethod(clusterGenerator.clusterObj, clusterGenerator.makeClusterGroupJNIJava, layoutObjects.size());
        
        Point2d size = clusterGenerator.layoutSize;
        
        // Geometry for the new cluster object
        ScreenSpaceObject::ConvexGeometry smGeom;
        smGeom.progID = progID;
        smGeom.coords.push_back(Point2d(- size.x()/2.0,-size.y()/2.0));
        smGeom.texCoords.push_back(TexCoord(0,1));
        smGeom.coords.push_back(Point2d(size.x()/2.0,-size.y()/2.0));
        smGeom.texCoords.push_back(TexCoord(1,1));
        smGeom.coords.push_back(Point2d(size.x()/2.0,size.y()/2.0));
        smGeom.texCoords.push_back(TexCoord(1,0));
        smGeom.coords.push_back(Point2d(-size.x()/2.0,size.y()/2.0));
        smGeom.texCoords.push_back(TexCoord(0,0));
        smGeom.color = RGBAColor(255,255,255,255);
        smGeom.texIDs.push_back(texID);
        
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
        // Notify all the cluster generators
        for (const auto &clusterGen : clusterGens)
            env->CallVoidMethod(clusterGen.clusterObj,clusterGen.endClusterGroupJava);
    }
    
    virtual void paramsForClusterClass(int clusterID,ClusterClassParams &clusterParams)
    {
        ClusterInfo dummyInfo;
        dummyInfo.clusterID = clusterID;
        auto it = clusterGens.find(dummyInfo);
        if (it == clusterGens.end())
            return;
        
        const ClusterInfo &clusterGenerator = *it;
        
        clusterParams.motionShaderID = motionShaderID;
        clusterParams.selectable = it->selectable;
        // Note: Make this selectable
        clusterParams.markerAnimationTime = 0.2;
        clusterParams.clusterSize = it->layoutSize;
    }

public:
    LayoutManager *layoutManager;

    SimpleIDSet currentClusterTex,oldClusterTex;
    
    SimpleIdentity motionShaderID;
    ClusterInfoSet clusterGens;
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
        LayoutManagerWrapper *wrap = new LayoutManagerWrapper(scene,layoutManager);
        LayoutManagerWrapperClassInfo::getClassInfo()->setHandle(env, obj, wrap);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::initialise()");
    }
}

static std::mutex disposeMutex;

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_dispose
  (JNIEnv *env, jobject obj)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
            classInfo->clearHandle(env, obj);
            if (!wrap)
                return;
            delete wrap;
        }
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
(JNIEnv *env, jobject obj, jobject clusterObj, jint clusterID, jboolean selectable, jdouble sizeX, jdouble sizeY)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return;
        wrap->setEnv(env);

        wrap->addClusterGenerator(clusterObj,clusterID,selectable,sizeX,sizeY);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::addClusterGenerator()");
    }
}

JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_clearClusterGenerators
(JNIEnv *, jobject)
{
    try
    {
        LayoutManagerWrapperClassInfo *classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
        if (!wrap)
            return;
        wrap->setEnv(env);

        wrap->clearClusterGenerators();
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_VERBOSE, "Maply", "Crash in LayoutManager::addClusterGenerator()");
    }
}
