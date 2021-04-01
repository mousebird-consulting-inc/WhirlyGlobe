/*  LayoutManager_jni.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2021 mousebird consulting
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

#import <jni.h>
#import "Maply_jni.h"
#import "Scene_jni.h"
#import "View_jni.h"
#import "Renderer_jni.h"
#import "com_mousebird_maply_LayoutManager.h"
#import "WhirlyGlobe.h"
#import <Exceptions_jni.h>

using namespace WhirlyKit;
using namespace Maply;

// Wrapper that tracks the generator as well
class LayoutManagerWrapper : public ClusterGenerator
{
public:
    
    // Used to keep track of cluster objects for callbacks
    struct ClusterInfo
    {
        ClusterInfo() : clusterObj(nullptr)
        {
        }

        // Move only
        ClusterInfo(const ClusterInfo &) = delete;
        ClusterInfo(ClusterInfo &&other) noexcept
        {
            copy(other);
            other.clusterObj = nullptr;
        }

        ~ClusterInfo()
        {
            if (clusterObj) {
                wkLogLevel(Warn, "ClusterInfo not cleaned up");
            }
        }

        bool operator < (const ClusterInfo &that) const { return clusterID < that.clusterID; }

        // Move only
        ClusterInfo& operator =(const ClusterInfo &) = delete;
        ClusterInfo& operator =(ClusterInfo &&other) noexcept {
            if (this != &other) {
                if (clusterObj) {
                    // Replacing this one leaks it
                    wkLogLevel(Warn, "ClusterInfo not cleaned up");
                }
                copy(other);
                other.clusterObj = nullptr;
            }
            return *this;
        }

        void init(JNIEnv *env,int inClusterID,const Point2d &inLayoutSize,jobject inClusterObj)
        {
            clusterID = inClusterID;
            layoutSize = inLayoutSize;
            clusterObj = env->NewGlobalRef(inClusterObj);
            
            // Methods can be saved without consequence
            // (as long as the class isn't unloaded and reloaded)
            jclass theClass = env->GetObjectClass(clusterObj);
            startClusterGroupJava = env->GetMethodID(theClass, "startClusterGroup", "()V");
            makeClusterGroupJNIJava = env->GetMethodID(theClass, "makeClusterGroupJNI", "(I)J");
            endClusterGroupJava = env->GetMethodID(theClass, "endClusterGroup", "()V");
            env->DeleteLocalRef(theClass);
        }
        
        void clear(JNIEnv *env)
        {
            env->DeleteGlobalRef(clusterObj);
            clusterObj = nullptr;
        }

        int clusterID;
        Point2d layoutSize;
        jobject clusterObj;
        bool selectable;

        // Methods we'll use to call into the Java cluster generator
        jmethodID startClusterGroupJava;
        jmethodID makeClusterGroupJNIJava;
        jmethodID endClusterGroupJava;

    private:
        void copy(const ClusterInfo &other) {
            clusterID = other.clusterID;
            layoutSize = other.layoutSize;
            clusterObj = other.clusterObj;
            selectable = other.selectable;
            startClusterGroupJava = other.startClusterGroupJava;
            makeClusterGroupJNIJava = other.makeClusterGroupJNIJava;
            endClusterGroupJava = other.endClusterGroupJava;
        }
        friend class LayoutManagerWrapper;
    };
    typedef std::set<ClusterInfo> ClusterInfoSet;

    LayoutManagerWrapper(PlatformThreadInfo *threadInfo, LayoutManagerRef layoutManager)
        : layoutManager(layoutManager), motionShaderID(EmptyIdentity)
    {
        layoutManager->addClusterGenerator(threadInfo,this);
    }

    virtual ~LayoutManagerWrapper()
    {
    }
    
    void updateShader()
    {
        if (motionShaderID == EmptyIdentity)
        {
            ProgramGLES *program = (ProgramGLES *)layoutManager->getScene()->findProgramByName(MaplyScreenSpaceDefaultMotionShader);
            if (program)
                motionShaderID = program->getId();
        }
    }

    // Add a cluster generator for callback
    void addClusterGenerator(JNIEnv* env, jobject clusterObj, jint clusterID,bool selectable, double sizeX, double sizeY)
    {
        ClusterInfo clusterInfo;
        clusterInfo.init(env,clusterID,Point2d(sizeX,sizeY),clusterObj);
        clusterInfo.selectable = selectable;
        
        clusterGens.insert(std::move(clusterInfo));
    }
    
    void clearClusterGenerators(JNIEnv* env)
    {
        for (auto &ci : clusterGens)
        {
            // why TF does this produce a const ref?
            // (answer: std::set has `typedef typename __base::const_iterator iterator`, bug?)
            const_cast<ClusterInfo&>(ci).clear(env);
        }
        clusterGens.clear();
    }

    /** ClusterGenerator virtual methods.
      */
    // Called right before we start generating layout objects
    virtual void startLayoutObjects(PlatformThreadInfo *threadInfo) override
    {
        const auto env = ((PlatformInfo_Android*)threadInfo)->env;

        oldClusterTex = currentClusterTex;
        currentClusterTex.clear();

        // Notify all the cluster generators
        for (const auto &clusterGen : clusterGens)
        {
            env->CallVoidMethod(clusterGen.clusterObj,clusterGen.startClusterGroupJava);
        }
    }

    // Ask the appropriate cluster generator to make a cluster image
    virtual void makeLayoutObject(PlatformThreadInfo *threadInfo,int clusterID,
                                  const std::vector<LayoutObjectEntry *> &layoutObjects,
                                  LayoutObject &retObj) override
    {
        const auto env = ((PlatformInfo_Android*)threadInfo)->env;

        ClusterInfo dummyInfo;
        dummyInfo.clusterID = clusterID;
        const auto it = clusterGens.find(dummyInfo);
        if (it == clusterGens.end())
            return;
        
        const ClusterInfo &clusterGenerator = *it;
        
        // Pick a representative screen object
        int drawPriority = -1;
        LayoutObject *sampleObj = nullptr;
        for (auto obj : layoutObjects) {
            if (obj->obj.getDrawPriority() > drawPriority) {
                drawPriority = obj->obj.getDrawPriority();
                sampleObj = &obj->obj;
            }
        }
        const SimpleIdentity progID = sampleObj->getTypicalProgramID();

        // The texture gets created on the Java side, so we'll just use the ID
        const long texID = env->CallLongMethod(clusterGenerator.clusterObj, clusterGenerator.makeClusterGroupJNIJava, layoutObjects.size());
        if (logAndClearJVMException(env,"makeClusterGroup"))
        {
            return;
        }
        if (texID == EmptyIdentity)
        {
            __android_log_print(ANDROID_LOG_WARN, "Maply", "No tex from makeClusterGroup");
        }
        
        const Point2d size = clusterGenerator.layoutSize;
        
        // Geometry for the new cluster object
        ScreenSpaceConvexGeometry smGeom;
        smGeom.progID = progID;
        smGeom.coords.push_back(Point2d(- size.x()/2.0,-size.y()/2.0));
        smGeom.texCoords.emplace_back(0,1);
        smGeom.coords.push_back(Point2d(size.x()/2.0,-size.y()/2.0));
        smGeom.texCoords.emplace_back(1,1);
        smGeom.coords.push_back(Point2d(size.x()/2.0,size.y()/2.0));
        smGeom.texCoords.emplace_back(1,0);
        smGeom.coords.push_back(Point2d(-size.x()/2.0,size.y()/2.0));
        smGeom.texCoords.emplace_back(0,0);
        smGeom.color = RGBAColor(255,255,255,255);
        smGeom.texIDs.push_back(texID);
        
        retObj.layoutPts = smGeom.coords;
        retObj.selectPts = smGeom.coords;

        retObj.setDrawPriority(drawPriority);
        retObj.addGeometry(smGeom);
    }
    
    // Called right after all the layout objects are generated
    virtual void endLayoutObjects(PlatformThreadInfo *threadInfo) override
    {
        const auto env = ((PlatformInfo_Android*)threadInfo)->env;

        // Notify all the cluster generators
        for (const auto &clusterGen : clusterGens)
        {
            env->CallVoidMethod(clusterGen.clusterObj,clusterGen.endClusterGroupJava);
        }
    }
    
    virtual void paramsForClusterClass(PlatformThreadInfo *,int clusterID,ClusterClassParams &clusterParams) override
    {
        ClusterInfo dummyInfo;
        dummyInfo.clusterID = clusterID;
        auto it = clusterGens.find(dummyInfo);
        if (it == clusterGens.end())
            return;
        
        const ClusterInfo &clusterGenerator = *it;
        
        clusterParams.motionShaderID = motionShaderID;
        clusterParams.selectable = clusterGenerator.selectable;
        // Note: Make this selectable
        clusterParams.markerAnimationTime = 0.2;
        clusterParams.clusterSize = clusterGenerator.layoutSize;
    }

public:
    LayoutManagerRef layoutManager;

    SimpleIDSet currentClusterTex,oldClusterTex;
    
    SimpleIdentity motionShaderID;
    ClusterInfoSet clusterGens;
};

typedef JavaClassInfo<LayoutManagerWrapper> LayoutManagerWrapperClassInfo;
template<> LayoutManagerWrapperClassInfo *LayoutManagerWrapperClassInfo::classInfoObj = nullptr;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_nativeInit(JNIEnv *env, jclass cls)
{
    LayoutManagerWrapperClassInfo::getClassInfo(env,cls);
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_initialise(JNIEnv *env, jobject obj,jobject sceneObj)
{
    try
    {
        const auto scene = SceneClassInfo::get(env,sceneObj);
        const auto layoutManager = scene->getManager<LayoutManager>(kWKLayoutManager);
        PlatformInfo_Android threadInfo(env);
        auto wrap = new LayoutManagerWrapper(&threadInfo, layoutManager);
        LayoutManagerWrapperClassInfo::getClassInfo()->setHandle(env, obj, wrap);
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LayoutManager::initialise()");
    }
}

static std::mutex disposeMutex;

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_dispose(JNIEnv *env, jobject obj)
{
    try
    {
        auto classInfo = LayoutManagerWrapperClassInfo::getClassInfo();
        {
            std::lock_guard<std::mutex> lock(disposeMutex);
            LayoutManagerWrapper *wrap = classInfo->getObject(env, obj);
            classInfo->clearHandle(env, obj);
            delete wrap;
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LayoutManager::dispose()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_setMaxDisplayObjects(JNIEnv *env, jobject obj, jint maxObjs)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            wrap->layoutManager->setMaxDisplayObjects(maxObjs);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LayoutManager::setMaxDisplayObjects()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_updateLayout
  (JNIEnv *env, jobject obj, jobject viewStateObj, jobject changeSetObj)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            if (auto viewState = ViewStateRefClassInfo::get(env, viewStateObj))
            {
                if (auto changeSet = ChangeSetClassInfo::get(env, changeSetObj))
                {
                    wrap->updateShader();

                    PlatformInfo_Android threadInfo(env);
                    wrap->layoutManager->updateLayout(&threadInfo,*viewState,**changeSet);
                }
            }
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LayoutManager::updateLayout()");
    }
}

extern "C"
JNIEXPORT jboolean JNICALL Java_com_mousebird_maply_LayoutManager_hasChanges(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            return wrap->layoutManager->hasChanges();
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LayoutManager::hasChanges()");
    }
    return false;
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_addClusterGenerator
    (JNIEnv *env, jobject obj, jobject clusterObj, jint clusterID, jboolean selectable, jdouble sizeX, jdouble sizeY)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            wrap->addClusterGenerator(env, clusterObj, clusterID, selectable, sizeX, sizeY);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LayoutManager::addClusterGenerator()");
    }
}

extern "C"
JNIEXPORT void JNICALL Java_com_mousebird_maply_LayoutManager_clearClusterGenerators(JNIEnv *env, jobject obj)
{
    try
    {
        if (auto wrap = LayoutManagerWrapperClassInfo::get(env, obj))
        {
            wrap->clearClusterGenerators(env);
        }
    }
    catch (...)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Maply", "Crash in LayoutManager::addClusterGenerator()");
    }
}
